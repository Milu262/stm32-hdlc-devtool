# build.py
import os
import subprocess
import sys

def build_executable():
    """打包Python程序为Windows可执行文件"""
    
    # 定义打包命令
    cmd = [
        'pyinstaller',
        '--name=STM32_Tool',  # 可执行文件名
        '--windowed',  # GUI应用，不显示控制台窗口
        '--onefile',  # 打包成单个exe文件
        '--icon=icon.ico',  # 图标文件（如果有的话）
        '--add-data=app;app',  # 添加app目录下的文件
        '--hidden-import=tkinter',  # 隐式导入tkinter
        '--hidden-import=app.main_window',  # 隐式导入主窗口模块
        'main.py'  # 主入口文件
    ]
    
    # 执行打包命令
    try:
        subprocess.run(cmd, check=True)
        print("打包成功！")
        print("可执行文件位于 dist 文件夹中")
    except subprocess.CalledProcessError as e:
        print(f"打包失败: {e}")
        sys.exit(1)

if __name__ == "__main__":
    build_executable()