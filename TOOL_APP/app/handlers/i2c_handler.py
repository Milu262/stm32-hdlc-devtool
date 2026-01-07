import app.CMD as CMD
import threading
import tkinter.messagebox as messagebox
# from app.main_window import FlashToolApp 

def i2c_read(app_instance: "FlashToolApp"):
    # 检查I2C地址是否为空
    if not app_instance.i2c_addr.get():
        messagebox.showerror("Error", "I2C address is empty")
        return
    try:
        dev_addr = int(app_instance.i2c_addr.get(), 16)
        reg_addr = int(app_instance.i2c_reg.get(), 16)
    except Exception as e:
        messagebox.showerror("Error", f"Invalid I2C address: {e}")
        return
    # 禁用按钮防止重复点击
    app_instance.i2c_read_btn.config(state='disabled')
    if app_instance.i2c_addr_size.get() == "8":
        app_instance._log_to_queue(f"I2C Read: Dev=0x{dev_addr:02X}, Reg=0x{reg_addr:02X}")
    else:
        app_instance._log_to_queue(f"I2C Read: Dev=0x{dev_addr:02X}, Reg=0x{reg_addr:04X}")

    # 启动后台线程
    threading.Thread(target=_i2c_read_worker, args=(app_instance, dev_addr, reg_addr), daemon=True).start()

def _i2c_read_worker(app_instance, dev_addr: int, reg_addr: int):
    if app_instance.i2c_addr_size.get() == "8":
        payload = bytes([dev_addr, reg_addr])
    else:
        payload = bytes([dev_addr, reg_addr >> 8, reg_addr & 0xFF])
    if app_instance.i2c_addr_size.get() == "8":
        resp = app_instance.send_with_retry(
            CMD.CMD_I2C_READ_REG,
            payload,
            expected_response_cmd=CMD.CMD_I2C_READ_RESULT
        )
    else:
        resp = app_instance.send_with_retry(
            CMD.CMD_I2C_16READ_REG,
            payload,
            expected_response_cmd=CMD.CMD_I2C_READ_RESULT
        )
    # 回到主线程更新 UI
    app_instance.root.after(0, _on_i2c_read_complete,app_instance,resp, dev_addr, reg_addr)

def _on_i2c_read_complete(app_instance: "FlashToolApp", resp, dev_addr, reg_addr):
    app_instance.i2c_read_btn.config(state='normal')  # 恢复按钮
    if resp is not None and len(resp) >= 1:
        value = resp[0]
        app_instance.i2c_write_data.set(f"0x{value:02X}")
        # app_instance.i2c_result.set(f"0x{value:02X} ({value})")
        app_instance._log_to_queue(f"I2C Read OK: 0x{value:02X}")
    else:
        #弹出窗口提示I2C读取超时
        messagebox.showerror("Error", f"I2C Read Timeout")

        # app_instance.i2c_result.set("Timeout")
        # 注意：device disconnect 已在 send_with_retry 中处理，这里无需重复弹窗

def i2c_write(app_instance: "FlashToolApp"):
    try:
        dev_addr = int(app_instance.i2c_addr.get(), 16)
        reg_addr = int(app_instance.i2c_reg.get(), 16)
        write_data = int(app_instance.i2c_write_data.get(), 16)
    except Exception as e:
        messagebox.showerror("Error", f"Invalid I2C address or write data: {e}")
        return
    app_instance.i2c_write_btn.config(state='disabled')
    if app_instance.i2c_addr_size.get() == "8":
        app_instance._log_to_queue(f"I2C Write: Dev=0x{dev_addr:02X}, Reg=0x{reg_addr:02X}, Data=0x{write_data:02X}")
    else:
        app_instance._log_to_queue(f"I2C Write: Dev=0x{dev_addr:02X}, Reg=0x{reg_addr:04X}, Data=0x{write_data:02X}")
     # 启动后台线程
    threading.Thread(target=_i2c_write_worker, args=(app_instance, dev_addr, reg_addr, write_data), daemon=True).start()
def _i2c_write_worker(app_instance: "FlashToolApp", dev_addr: int, reg_addr: int, write_data: int):
    if app_instance.i2c_addr_size.get() == "8":
        payload = bytes([dev_addr, reg_addr, write_data])
    else:
        payload = bytes([dev_addr, reg_addr >> 8, reg_addr & 0xFF, write_data])
    if app_instance.i2c_addr_size.get() == "8":
        resp = app_instance.send_with_retry(
            CMD.CMD_I2C_WRITE_REG,
            payload,
            expected_response_cmd=CMD.CMD_I2C_WRITE_ACK
        )
    else:
        resp = app_instance.send_with_retry(
            CMD.CMD_I2C_16WRITE_REG,
            payload,
            expected_response_cmd=CMD.CMD_I2C_WRITE_ACK
        )
    # 回到主线程更新 UI
    app_instance.root.after(0, _on_i2c_write_complete, app_instance, resp, dev_addr, reg_addr, write_data)
def _on_i2c_write_complete(app_instance: "FlashToolApp", resp, dev_addr, reg_addr, write_data):
    app_instance.i2c_write_btn.config(state='normal')
    if resp is not None:
        app_instance._log_to_queue(f"I2C Write OK")
    else:
        # 弹出窗口提示I2C写入超时
        messagebox.showerror("Error", f"I2C Write Timeout")

def i2c_find(app_instance: "FlashToolApp"):
    # 禁用按钮防止重复点击
    app_instance.i2c_read_btn.config(state='disabled')
    # 启动后台线程
    threading.Thread(target=_i2c_find_worker, args=(app_instance,), daemon=True).start()

def _i2c_find_worker(app_instance:"FlashToolApp"):
    payload  = b''
    resp = app_instance.send_with_retry(
        CMD.CMD_I2C_ADDRESS_FIND,
        payload,
        expected_response_cmd=CMD.CMD_I2C_ADDRESS_FIND_ACK
    )
    # 回到主线程更新 UI
    app_instance.root.after(0, _on_i2c_find_complete,app_instance,resp)

def _on_i2c_find_complete(app_instance: "FlashToolApp", resp):
    app_instance.i2c_read_btn.config(state='normal')  # 恢复按钮
    if resp is not None and len(resp) >= 1:
        # 解析响应数据以获取实际的I2C地址
        # 假设响应数据格式为 [地址1, 地址2, ...] 或类似的格式
        found_addresses = []
        for byte_val in resp:
            # 将字节值转换为十六进制格式的I2C地址
            i2c_addr = f"0x{byte_val:02X}"
            found_addresses.append(i2c_addr)
        
        if found_addresses:
            addresses_str = ", ".join(found_addresses)
            # 弹出窗口提示I2C读取成功,并且输出找到的I2C地址
            messagebox.showinfo("I2C Address Find", f"I2C Addresses Found: {addresses_str}")
        else:
            messagebox.showinfo("I2C Address Find", "No I2C addresses found")
    else:
        # 弹出窗口提示I2C读取超时
        messagebox.showerror("Error", f"I2C Address Find Timeout")
