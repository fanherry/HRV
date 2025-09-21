import serial, re, time, threading
from collections import deque
import numpy as np
import pyqtgraph as pg

# ---- Qt 安全导入 ----
from pyqtgraph.Qt import QtCore
try:
    from pyqtgraph.Qt import QtWidgets
except Exception:
    print("当前 Qt 绑定缺少 QtWidgets（无法创建 QApplication）。")
    print("请在你的虚拟环境中安装完整的 Qt 绑定，例如：")
    print(r'"C:\PyCharm\NEZA\GITHUB\ENVs\env11\Scripts\pip.exe" install PyQt5')
    raise SystemExit(1)

# 现在安全创建 QApplication
app = QtWidgets.QApplication([])

# === 参数 ===
PORT = 'COM3'   # 修改为你的端口
BAUD = 115200
MAXLEN = 600

class Reader:
    def __init__(self, port, baud, maxlen=1000):
        self.ser = serial.Serial(port, baud, timeout=1)
        self.ir = deque(maxlen=maxlen)
        self.lock = threading.Lock()
        self.running = True

    def parse(self, line):
        if line.startswith('IR:'):
            m = re.search(r'IR:\s*(\d+)', line)
            if m: return int(m.group(1))
        return None

    def run(self):
        while self.running:
            try:
                if self.ser.in_waiting:
                    raw = self.ser.readline().decode('utf-8', errors='ignore').strip()
                    v = self.parse(raw)
                    if v is not None:
                        with self.lock:
                            self.ir.append(v)
                else:
                    time.sleep(0.002)
            except Exception as e:
                print("读取线程异常:", e)
                break

    def close(self):
        self.running = False
        try:
            self.ser.close()
        except:
            pass

def main():
    reader = Reader(PORT, BAUD, MAXLEN)
    t = threading.Thread(target=reader.run, daemon=True)
    t.start()

    win = pg.GraphicsLayoutWidget(show=True, title="Real-time IR")
    p = win.addPlot(title="IR")
    curve = p.plot()
    p.setLabel('left', 'IR 值')
    p.setLabel('bottom', '样本（最近）')

    def update():
        with reader.lock:
            y = np.array(reader.ir, dtype=float)
        if y.size:
            x = np.arange(-y.size + 1, 1)
            curve.setData(x, y)
            p.setXRange(x[0], x[-1])
            ymin, ymax = y.min(), y.max()
            if ymin == ymax:
                ymin -= 1; ymax += 1
            pad = (ymax - ymin) * 0.12 + 1
            p.setYRange(ymin - pad, ymax + pad)

    timer = QtCore.QTimer()
    timer.timeout.connect(update)
    timer.start(30)  # 每30ms更新一次

    try:
        app.exec_()
    finally:
        print("退出，正在关闭串口线程...")
        reader.close()
        t.join(timeout=1)

if __name__ == "__main__":
    main()
