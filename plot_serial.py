import sys
import serial
import pyqtgraph as pg
from pyqtgraph.Qt import QtWidgets, QtCore

# Serial Setup
PORT = "COM3"
BAUD = 115200
ser = serial.Serial(PORT, BAUD, timeout=1)

# PyQtGraph Setup
app = QtWidgets.QApplication([])
pg.setConfigOption('background', 'w')
pg.setConfigOption('foreground', 'k')

win = pg.GraphicsLayoutWidget(show=True, title="Real-Time Sensor Plot")
win.resize(900, 700)

# Stacked plots
p1 = win.addPlot(title="Voltage (V)")
p1.addLegend(); p1.showGrid(x=True, y=True)
win.nextRow()

p2 = win.addPlot(title="Current (mA)")
p2.addLegend(); p2.showGrid(x=True, y=True)
win.nextRow()

p3 = win.addPlot(title="Power (mW)")
p3.addLegend(); p3.showGrid(x=True, y=True)

# Pens and curve objects
pen_v = pg.mkPen(color='r', width=2)
pen_c = pg.mkPen(color='g', width=2)
pen_p = pg.mkPen(color='b', width=2)

curve_v = p1.plot(name="Voltage", pen=pen_v)
curve_c = p2.plot(name="Current", pen=pen_c)
curve_p = p3.plot(name="Power",   pen=pen_p)

# Data buffers
voltage = []
current = []
power = []
x_time = []

encrypt_time_ms = None     # The last "Time to encrypt" value
sample_count = 0           # Number of data samples received

def update():
    global encrypt_time_ms, sample_count, x_time

    line = ser.readline().decode(errors="ignore").strip()
    if not line:
        return

    print(line)  # print raw serial line

    # ----------------------------
    # Handle encryption time line
    # ----------------------------
    if "Time to encrypt" in line:
        try:
            # Format: "Time to encrypt : 534.894 ms"
            encrypt_time_ms = float(line.split(":")[1].replace("ms", "").strip())
            print(f"\n[Parsed encryption time] {encrypt_time_ms} ms\n")
        except:
            pass
        return

    # Skip headers
    if ("Voltage" in line or 
        "Current" in line or 
        "Power"   in line):
        return

    # ----------------------------
    # Parse numeric data
    # ----------------------------
    if "," in line:
        try:
            v, c, p = [float(x) for x in line.split(",")]

            voltage.append(v)
            current.append(c)
            power.append(p)
            sample_count += 1

            # ----------------------------
            # Build time axis (0 → encrypt_time)
            # ----------------------------
            if encrypt_time_ms is not None:
                # Evenly space samples from 0 → encrypt_time_ms
                dt = encrypt_time_ms / max(sample_count - 1, 1)
                x_time = [i * dt for i in range(sample_count)]
            else:
                # Fallback: simple sample number
                x_time = list(range(sample_count))

            # Update plots
            curve_v.setData(x_time, voltage)
            curve_c.setData(x_time, current)
            curve_p.setData(x_time, power)

        except:
            pass


timer = QtCore.QTimer()
timer.timeout.connect(update)
timer.start(5)

if __name__ == "__main__":
    QtWidgets.QApplication.instance().exec_()
