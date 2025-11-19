# ESP32 AES-256 Hardware vs Software Encryption Benchmark
# Authors: Troy Bates, Nilay Jadav

This project compares **AES-256 encryption** on the **ESP32** using both hardware-accelerated and software-only AES.  
It includes execution time, throughput, and power consumption measurements, along with tools for real‑time power analysis.

---

##  Project Structure

```
esp32_HW_AES.ino      # Hardware AES-256 using ESP32 crypto accelerator
esp32_SW_AES.ino      # Software AES-256 using mbedTLS AES CPU implementation
plotter.ino           # ADC logger for power measurement
plot_serial.py        # Python plotting tool
README.md
```

---

## Libraries Used

### **Arduino / ESP32**
| Library | Purpose |
|--------|---------|
| **mbedtls/aes.h** | AES hardware library |
| **AESLib.h**      | AES Software library |


### **Python**
| Library | Purpose |
|--------|---------|
| **pyserial** | Reading ESP32's serial data stream |
| **matplotlib** | Live plotting of voltage/current/power curves |
| **numpy** | Smoothing, averaging, numerical analysis |

Install Python dependencies:

```bash
pip install pyserial matplotlib numpy
```

---

##  Benchmark Overview

This project measures:

- AES‑256 ECB encryption time  
- 1 KB and 1 MB throughput  
- Power consumption during operation  
- Energy per byte/block  
- Performance per watt  

Hardware and software AES both use the **same 32‑byte key** and identical plaintext for fair comparison.

---

##  Features

###  Hardware AES (AES‑ECB‑256)
- Uses ESP32’s built‑in AES accelerator  
- Very fast  
- Lower CPU load  
- Better energy efficiency  

###  Software AES
- Pure CPU-based AES‑256  
- Slower but portable  
- Useful for side‑channel studies  

###  Power Measurement
- `plotter.ino` streams ADC readings  
- `plot_serial.py` plots real‑time curves  

---

##  Benchmark Results

### **Single 32‑Byte Block**

| Implementation | Time (µs) |
|----------------|-----------|
| Hardware AES   | **~99 µs** |
| Software AES   | **~324 µs** |

---

### **1 KB Block**

| Implementation | Time (ms) | Throughput |
|----------------|-----------|------------|
| Hardware AES   | 0.527 ms  | **1897.53 KB/s** |
| Software AES   | 8.133 ms  | **122.96 KB/s** |

---

### **1 MB Block**

| Implementation | Time (ms) | Throughput |
|----------------|-----------|------------|
| Hardware AES   | 534.867 ms | **1.87 MB/s** |
| Software AES   | 8316.111 ms | **0.12 MB/s** |

---

### **Power Consumption**

| Mode | Voltage | Current | Power |
|------|---------|---------|--------|
| Software AES | 4.42–4.85 V | 56–62 mA | **275–300 mW** |
| Hardware AES | 4.86 V | 60 mA | **291.6 mW** |

---

##  Usage

### **1. Upload AES Test**
Choose one:

```
esp32_HW_AES.ino
esp32_SW_AES.ino
```

### **2. Open Serial Monitor**
Set **115200 baud**.

### **3. Power Measurement (Optional)**

Upload:

```
plotter.ino
```

Run:

```bash
python plot_serial.py
```

---

##  Requirements

- ESP32 Development Board  
- Arduino IDE with ESP32 package  
- Python 3.x  

---

##  License
Academic and research use only.

