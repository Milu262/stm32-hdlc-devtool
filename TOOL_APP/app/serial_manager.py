# app/serial_manager.py

import serial
import threading
import time
from typing import Optional, Callable
from collections import deque
from protocol.hdlc import build_hdlc_frame, HDLC_FLAG, HDLC_ESCAPE, HDLC_XOR
from protocol.crc16 import crc16_ccitt

class SerialManager:
    def __init__(self, log_callback: Callable[[str], None]):
        self.serial_port = None
        self.log = log_callback
        self.last_response = None # 数据格式为（cmd_id: int, payload: bytes）
        self.is_running = True
        self._response_lock = threading.Lock()
        self._response_event = threading.Event()
        # 启动接收线程
        self._reader_thread = threading.Thread(target=self._read_loop, daemon=True)
        self._reader_thread.start()

    def open(self, port: str, baudrate=2000000) -> bool:
        try:
            self.serial_port = serial.Serial(port, baudrate, timeout=0.01)
            return True
        except Exception as e:
            self.log(f"Failed to open {port}: {e}")
            return False

    def close(self):
        if self.serial_port:
            self.serial_port.close()
            self.serial_port = None

    def is_open(self) -> bool:
        return self.serial_port and self.serial_port.is_open

    def send_frame(self, frame: bytes):
        if self.is_open():
            self.serial_port.write(frame)

    def wait_for_response(self, expected_cmd: int, timeout=1.0) -> Optional[bytes]:
        """
        等待具有指定 CMD ID 的 HDLC 响应帧。
    
        Args:
            expected_cmd: 期望的命令 ID。
            timeout: 等待超时时间（秒）。
    
        Returns:
            如果在超时时间内收到匹配的响应，则返回响应的 payload (bytes)；
            否则返回 None。
        """
        start_time = time.time()
        remaining_timeout = timeout
    
        while remaining_timeout > 0:
            # 在每次等待前，先清除 event 的内部标志。
            # 这很重要，确保我们等待的是 *下一次* set() 调用。
            self._response_event.clear()
    
            # 等待 event 被 set（由 _process_temp_buffer 触发）
            # 或者超时。wait() 返回 True 表示被 set 唤醒，False 表示超时。
            event_signaled = self._response_event.wait(timeout=remaining_timeout)
    
            # 即使 event 被 set，也需要再次检查 last_response，
            # 因为 set 可能是由一个不相关的帧触发的。
            with self._response_lock:
                if self.last_response and self.last_response[0] == expected_cmd:
                    # 找到匹配的响应
                    resp = self.last_response[1]
                    self.last_response = None # 清空，防止重复处理
                    return resp
    
            # 如果走到这里，说明要么超时了，要么收到的帧 CMD 不匹配。
            # 更新剩余的超时时间，准备下一次（如果有的话）等待。
            elapsed_time = time.time() - start_time
            remaining_timeout = timeout - elapsed_time
    
        # 超时退出
        return None

    def _find_hdlc_frames_in_buffer(self, buffer: bytearray) -> list:
    
    #在给定的字节缓冲区中查找所有潜在的 HDLC 帧。
    #返回一个列表，其中每个元素是一个元组 (start_index, end_index_exclusive, frame_data)。
    
        frames = []
        i = 0
        while i < len(buffer):
            # 查找起始标志
            start_idx = buffer.find(HDLC_FLAG, i)
            if start_idx == -1:
                break # 缓冲区剩余部分没有起始标志了
            # 从起始标志后开始查找结束标志
            # 注意：结束标志不能是紧接着起始标志的那个 FLAG
            end_idx = buffer.find(HDLC_FLAG, start_idx + 1)
            if end_idx == -1:
                # 没有找到结束标志，说明帧不完整，保留从 start_idx 到末尾的数据
                # 等待下次缓冲区更新再处理
                break
            # 提取潜在的帧数据（包括首尾的 FLAG）
            potential_frame_data = buffer[start_idx : end_idx + 1]

            # 验证帧长度（至少：FLAG + CMD(2) + CRC(2) + FLAG = 6 bytes）
            if len(potential_frame_data) < 6:
                # 太短，不是有效帧，跳过起始标志继续查找
                i = start_idx + 1
                continue
            # 尝试去转义
            unescaped_data = bytearray()
            j = 1 # 跳过第一个 FLAG
            escaped = False
            valid_frame = True
            try:
                while j < len(potential_frame_data) - 1: # -1 跳过最后一个 FLAG
                    byte_val = potential_frame_data[j]
                    if byte_val == HDLC_ESCAPE:
                        if escaped:
                            # 连续的 ESCAPE，无效
                            valid_frame = False
                            break
                        escaped = True
                    elif escaped:
                        unescaped_data.append(byte_val ^ HDLC_XOR)
                        escaped = False
                    else:
                        unescaped_data.append(byte_val)
                    j += 1

                if escaped: # 帧以 ESCAPE 结尾，无效
                    valid_frame = False
            except Exception as e:
                self.log(f"Error during HDLC unescaping: {e}")
                valid_frame = False
            if valid_frame and len(unescaped_data) >= 4: # 至少包含 CMD(2) + CRC(2)
                # 计算并验证 CRC
                try:
                    cmd_id_bytes = unescaped_data[:2]
                    payload = unescaped_data[2:-2] # Payload 可能为空
                    crc_received_bytes = unescaped_data[-2:]

                    # 注意：CRC 是对 CMD_ID + Payload 计算的
                    data_for_crc = cmd_id_bytes + payload
                    crc_received = int.from_bytes(crc_received_bytes, 'big')
                    crc_calculated = crc16_ccitt(data_for_crc)

                    if crc_received == crc_calculated:
                        cmd_id = int.from_bytes(cmd_id_bytes, 'big')
                        frames.append((start_idx, end_idx + 1, cmd_id, payload))
                        # 跳过已处理的整个帧
                        i = end_idx + 1
                    else:
                        self.log(f"[CRC Error] Frame at [{start_idx}:{end_idx+1}] Recv:0x{crc_received:04X} Calc:0x{crc_calculated:04X}")
                        # CRC 错误，跳过起始标志继续查找
                        i = start_idx + 1
                except Exception as e:
                    self.log(f"Error processing potential HDLC frame: {e}")
                    i = start_idx + 1
            else:
                # 不是有效帧或解码失败，跳过起始标志继续查找
                i = start_idx + 1

        return frames

    def _read_loop(self):
        temp_buffer = bytearray() # 用于累积所有原始串口数据
        last_activity_time = time.time() # 记录上次接收到数据的时间
        idle_check_interval = 0.01  # 每 10ms 检查一次是否空闲
        idle_threshold = 0.001       # 如果超过 1ms 没有新数据，则认为是空闲
        
        while self.is_running:
            if not self.is_open():
                time.sleep(idle_check_interval)
                continue

            try:
                # 非阻塞读取，尝试读取尽可能多的数据
                # 但为了及时响应 idle，我们限制一次读取的数量或使用超时
                # 这里采用一个小的超时和有限读取结合的方式
                bytes_to_read = max(1, min(1024, self.serial_port.in_waiting)) # 读取等待中的或至少1个
                data = self.serial_port.read(bytes_to_read) 
                
                current_time = time.time()
                
                if data:
                    temp_buffer.extend(data)
                    last_activity_time = current_time

                # 检查是否达到空闲时间阈值
                if current_time - last_activity_time > idle_threshold:
                    # 达到空闲，处理缓冲区
                    if temp_buffer:
                        self._process_temp_buffer(temp_buffer)
                        
            except Exception as e:
                self.log(f"Serial read error: {e}")
            
            # 等待下一个检查周期
            time.sleep(idle_check_interval)


    def _process_temp_buffer(self, buffer: bytearray):
        """
        处理累积的 temp_buffer，分离 HDLC 帧和 Debug 数据。
        """
        if not buffer:
            return

        # 1. 查找所有潜在的 HDLC 帧及其位置
        hdlc_frames_info = self._find_hdlc_frames_in_buffer(buffer)
        
        if not hdlc_frames_info:
            # 没有找到任何有效帧，全部当作 Debug 输出
            self._flush_raw_data(buffer)
            buffer.clear()
            return

        # 2. 分离 HDLC 帧和 Debug 数据
        debug_segments = [] # 存放 (start, end) 元组
        last_end = 0
        
        for start_idx, end_idx_excl, cmd_id, payload in hdlc_frames_info:
            # 添加帧前面的 Debug 数据（如果有的话）
            if start_idx > last_end:
                debug_segments.append((last_end, start_idx))
            
            # 触发 HDLC 帧响应处理
            with self._response_lock:
                self.last_response = (cmd_id, payload)
                self._response_event.set() # 如果你在 wait_for_response 中用了 Event
            
            # 更新最后处理的位置
            last_end = end_idx_excl

        # 添加最后一帧后面的 Debug 数据（如果有的话）
        if last_end < len(buffer):
            debug_segments.append((last_end, len(buffer)))

        # 3. 输出 Debug 数据
        for start, end in debug_segments:
            if end > start: # 确保片段不为空
                self._flush_raw_data(buffer[start:end])

        # 4. 从 temp_buffer 中移除已处理的数据
        # 为了效率，我们从后往前删除，避免索引变化
        # 或者，更简单地，反转列表处理（但这会改变顺序）
        # 最稳妥的方法是构建一个新的 buffer，只保留未处理的部分
        # 但我们已经处理了所有内容，所以可以直接清空
        # 或者，如果只想移除已识别的部分，逻辑会更复杂
        # --- 简化处理：既然我们分析了整个 buffer，就清空它 ---
        # 但如果 find_hdlc_frames_in_buffer 只处理了前面的部分，
        # 我们应该只移除那些部分。
        # 为了精确，我们应该只移除到已知处理完的最后位置
        # 但是 find_hdlc_frames_in_buffer 本身遍历了整个 buffer
        # 所以我们可以认为整个 buffer 都被分析过了。
        # 未被识别为帧的部分已经在 debug_segments 中输出了。
        # 因此，清空整个 buffer 是合理的。
        buffer.clear() # 清空已处理的缓冲区

        # 注意：如果 find_hdlc_frames_in_buffer 没有处理完 buffer 的末尾
        # (例如，末尾有一个孤立的 FLAG)，这部分数据会被当作 Debug 输出
        # 但如果 buffer 末尾是不完整帧的一部分，它应该被保留下来
        # 供下次 _process_temp_buffer 调用时继续处理。
        # 当前的 find_hdlc_frames_in_buffer 实现会在找不到结束 FLAG 时停止，
        # 这意味着 buffer 末尾的不完整数据会被保留。这符合预期。
    def _flush_raw_data(self, data: bytearray):
        """将原始字节尝试解码为可读字符串并输出到 log"""
        if not data:
            return
        try:
            # 尝试 UTF-8 解码，替换非法字符
            text = data.decode('utf-8', errors='replace')
            # 去掉末尾可能的乱码，保留可打印部分
            clean_lines = []
            for line in text.splitlines(keepends=True):
                clean_lines.append(line)
            output = ''.join(clean_lines).rstrip('\n\r')  # 避免多余空行
            if output:
                self.log(f"[UART] {output}")
        except Exception as e:
            # 如果无法解码，以 hex 形式显示
            hex_str = ' '.join(f"{x:02X}" for x in data)
            self.log(f"[UART RAW] {hex_str}")
    def shutdown(self):
        self.is_running = False
        self.close()