import sys
import re
import argparse
from collections import deque

import serial
import pyqtgraph as pg
from pyqtgraph.Qt import QtWidgets, QtCore

# --- CLI options ---
parser = argparse.ArgumentParser(description="Real-time serial plotter")
parser.add_argument("--port", default="COM3", help="Serial port (e.g. COM3)")
parser.add_argument("--baud", type=int, default=115200, help="Baud rate")
parser.add_argument("--interval", type=int, default=20, help="Timer interval in ms")
parser.add_argument("--buf", type=int, default=1000, help="Max samples to keep in memory")
parser.add_argument("--verbose", action="store_true", help="Print raw serial lines")
args = parser.parse_args()

PORT = args.port
BAUD = args.baud
TIMER_MS = max(1, args.interval)
MAX_SAMPLES = max(10, args.buf)
VERBOSE = args.verbose

# --- Serial setup with error handling ---
try:
    ser = serial.Serial(PORT, BAUD, timeout=1)
except Exception as e:
    print(f"Failed to open serial port {PORT} @ {BAUD}: {e}")
    sys.exit(1)

# --- PyQtGraph setup ---
app = QtWidgets.QApplication([])
pg.setConfigOption("background", "w")
pg.setConfigOption("foreground", "k")

win = pg.GraphicsLayoutWidget(show=True, title="Real-Time Sensor Plot")
win.resize(900, 700)

p1 = win.addPlot(title="Voltage (V)")
p1.addLegend(); p1.showGrid(x=True, y=True)
win.nextRow()

p2 = win.addPlot(title="Current (mA)")
p2.addLegend(); p2.showGrid(x=True, y=True)
win.nextRow()

p3 = win.addPlot(title="Power (mW)")
p3.addLegend(); p3.showGrid(x=True, y=True)

pen_v = pg.mkPen(color='r', width=2)
pen_c = pg.mkPen(color='g', width=2)
pen_p = pg.mkPen(color='b', width=2)

curve_v = p1.plot(pen=pen_v)
curve_c = p2.plot(pen=pen_c)
curve_p = p3.plot(pen=pen_p)

# --- Data buffers (bounded) ---
voltage = deque(maxlen=MAX_SAMPLES)
current = deque(maxlen=MAX_SAMPLES)
power = deque(maxlen=MAX_SAMPLES)
x_time = deque(maxlen=MAX_SAMPLES)

encrypt_time_ms = None  # last parsed "Time to encrypt"

# regex to match lines like: "Time to encrypt : 534.894 ms"
encrypt_re = re.compile(r"Time\s+to\s+encrypt\s*[:\-]?\s*([\d.]+)\s*ms", re.IGNORECASE)

def update():
    global encrypt_time_ms

    try:
        raw = ser.readline()
    except Exception:
        return

    if not raw:
        return

    try:
        line = raw.decode(errors="ignore").strip()
    except Exception:
        return

    if not line:
        return

    if VERBOSE:
        print(line)

    # Check encryption time
    m = encrypt_re.search(line)
    if m:
        try:
            encrypt_time_ms = float(m.group(1))
            # reset x_time baseline when new encrypt time found
            x_time.clear()
            if VERBOSE:
                print(f"[Parsed encryption time] {encrypt_time_ms} ms")

            # If we already have samples, rebuild x_time in ms and update plots + axis labels
            n = len(voltage)
            if n > 0:
                dt = encrypt_time_ms / max(n - 1, 1) if n > 1 else 0.0
                x_time.clear()
                for i in range(n):
                    x_time.append(i * dt)

                # update curves with ms x axis
                curve_v.setData(list(x_time), list(voltage))
                curve_c.setData(list(x_time), list(current))
                curve_p.setData(list(x_time), list(power))

                # label axes in ms and set range
                for p in (p1, p2, p3):
                    p.setLabel('bottom', 'Time', units='ms')
                    p.setXRange(0, encrypt_time_ms, padding=0.02)
        except ValueError:
            pass
        return

    # Skip obvious header lines
    if any(tok in line for tok in ("Voltage", "Current", "Power")):
        return

    # Parse CSV line (v, c, p)
    if "," in line:
        parts = [s.strip() for s in line.split(",")]
        if len(parts) < 3:
            return
        try:
            v = float(parts[0])
            c = float(parts[1])
            p = float(parts[2])
        except ValueError:
            return

        voltage.append(v)
        current.append(c)
        power.append(p)

        n = len(voltage)
        # Build x_time: either normalized to encrypt_time_ms or sample index
        if encrypt_time_ms is not None and n > 1:
            dt = encrypt_time_ms / max(n - 1, 1)
            x_time.clear()
            for i in range(n):
                x_time.append(i * dt)
        else:
            x_time.clear()
            for i in range(n):
                x_time.append(i)

        curve_v.setData(list(x_time), list(voltage))
        curve_c.setData(list(x_time), list(current))
        curve_p.setData(list(x_time), list(power))

# Timer
timer = QtCore.QTimer()
timer.timeout.connect(update)
timer.start(TIMER_MS)

def on_exit():
    try:
        # If we have a final encrypt_time_ms, ensure x-axis shows ms on exit
        if encrypt_time_ms is not None and len(voltage) > 0:
            n = len(voltage)
            if n > 1:
                dt = encrypt_time_ms / max(n - 1, 1)
            else:
                dt = 0.0
            x_time.clear()
            for i in range(n):
                x_time.append(i * dt)

            curve_v.setData(list(x_time), list(voltage))
            curve_c.setData(list(x_time), list(current))
            curve_p.setData(list(x_time), list(power))

            for p in (p1, p2, p3):
                p.setLabel('bottom', 'Time', units='ms')
                p.setXRange(0, encrypt_time_ms, padding=0.02)

        if ser and ser.is_open:
            ser.close()
    except Exception:
        pass

app.aboutToQuit.connect(on_exit)

if __name__ == "__main__":
    QtWidgets.QApplication.instance().exec_()
