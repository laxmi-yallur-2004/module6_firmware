# Module 6 Firmware — STM32F401CCU6

## 1. Overview

Module 6 implements two firmware tasks using the STM32F401CCU6:

* **Task 1:** ADC sampling using DMA circular buffer
* **Task 2:** UART RX using DMA circular buffer and software ring buffer

The ADC uses **PA0** and the UART uses **PA9/PA10**.

---

## 2. Hardware

* STM32F401CCU6
* Potentiometer
* USB-TTL converter
* ST-LINK programmer

---

## 3. Connections

### 3.1 ST-LINK → STM32F401CCU6

| ST-LINK | STM32F401CCU6 |
| ------- | ------------- |
| SWDIO   | PA13 / SWDIO  |
| SWCLK   | PA14 / SWCLK  |
| GND     | GND           |
| 3.3V    | 3.3V          |

ST-LINK is used for programming/debugging the STM32.

---

### 3.2 Potentiometer → STM32F401CCU6

| Potentiometer      | STM32F401CCU6 |
| ------------------ | ------------- |
| 3.3V               | 3.3V          |
| GND                | GND           |
| Wiper / Middle pin | PA0           |

The potentiometer provides a variable analog voltage to **PA0**.

---

### 3.3 USB-TTL → STM32F401CCU6

| USB-TTL | STM32F401CCU6 |
| ------- | ------------- |
| RX      | PA9 (TX)      |
| TX      | PA10 (RX)     |
| GND     | GND           |

The UART uses:

**115200 baud, 8 data bits, no parity, 1 stop bit (115200 8N1).**

---

## 4. Common Ground

The GND connections are common.

```text
ST-LINK GND
     |
     +------ STM32 GND
     |
Potentiometer GND
     |
USB-TTL GND
```

All devices use the same STM32 ground reference.

---

# 5. Task 1 — ADC DMA Circular Buffer

## Purpose

The ADC reads the analog voltage on **PA0**.

The STM32 ADC is **12-bit**, so the ADC result is:

```text
0 to 4095
```

DMA automatically transfers ADC samples into an 8-sample circular buffer.

The 8 samples are divided into two halves:

```text
Sample 1  ┐
Sample 2  │
Sample 3  │ FIRST HALF
Sample 4  ┘

Sample 5  ┐
Sample 6  │
Sample 7  │ SECOND HALF
Sample 8  ┘
```

The DMA generates:

* Half-transfer event → first 4 samples ready
* Full-transfer event → second 4 samples ready

The firmware copies each completed half into a separate buffer before processing it.

---

## 6. ADC Voltage Calculation

The ADC reference used by the program is **3.3 V**.

The voltage is calculated using:

```text
Voltage = ADC × 3.3 / 4095
```

The program displays the result in millivolts:

```text
mV = ADC × 3300 / 4095
```

Example:

```text
ADC = 2255

Voltage ≈ 2255 × 3300 / 4095
        ≈ 1817 mV
```

---

# 7. Task 2 — UART RX DMA Buffer

UART communication uses:

```text
PA9  → TX
PA10 → RX
```

Configuration:

```text
115200 8N1
```

UART RX uses a **DMA circular buffer**.

Received data is moved into a software ring buffer and then transmitted back through UART.

Therefore, typing:

```text
laxmi
```

produces:

```text
laxmi
```

as the received echo.

---

# 8. DMA Configuration

### ADC DMA

```text
DMA Controller : DMA2
Stream         : Stream 0
Channel        : Channel 0
Mode           : Circular
Data           : 16-bit
Buffer         : 8 samples
```

### UART RX DMA

```text
DMA Controller : DMA2
Stream         : Stream 2
Channel        : Channel 4
Mode           : Circular
Data           : 8-bit
DMA Buffer     : 128 bytes
Ring Buffer    : 256 bytes
```

---

# 9. Startup Output

The firmware displays:

```text
MODULE 6 FIRMWARE
------------------
TASK 1: ADC DMA CIRCULAR BUFFER
TASK 2: UART RX DMA BUFFER
ADC PIN: PA0
UART TX: PA9
UART RX: PA10
UART: 115200 8N1
ADC: 12-BIT, 4 + 4 SAMPLES
READY
```

---

# 10. ADC Test Output

### First Half

```text
FIRST HALF PROCESSING

Sample 1: 2255 | 1817 mV
Sample 2: 2256 | 1818 mV
Sample 3: 2255 | 1817 mV
Sample 4: 2257 | 1818 mV

FIRST HALF AVERAGE: 2255 | 1817 mV
```

### Second Half

```text
SECOND HALF PROCESSING

Sample 5: 2256 | 1818 mV
Sample 6: 2252 | 1814 mV
Sample 7: 2257 | 1818 mV
Sample 8: 2257 | 1818 mV

SECOND HALF AVERAGE: 2255 | 1817 mV
```

This proves that the DMA buffer is being divided and processed as:

```text
First half  = 4 samples
Second half = 4 samples
Total       = 8 samples
```

---

# 11. UART Test

Typing characters through the USB-TTL serial terminal produces the same characters back.

Example:

```text
Input:
laxmi

Output:
laxmi
```

A continuous character test also produced echoed characters successfully.

---

# 12. Pin Summary

| Function      | STM32 Pin |
| ------------- | --------- |
| ADC Input     | PA0       |
| UART TX       | PA9       |
| UART RX       | PA10      |
| ST-LINK SWDIO | PA13      |
| ST-LINK SWCLK | PA14      |
| Common Ground | GND       |
| Supply        | 3.3V      |

---

# 13. Final Result

| Task                           | Result |
| ------------------------------ | ------ |
| ADC input on PA0               | PASS   |
| ADC 12-bit conversion          | PASS   |
| ADC DMA circular buffer        | PASS   |
| First 4 samples processing     | PASS   |
| Second 4 samples processing    | PASS   |
| ADC average calculation        | PASS   |
| UART TX on PA9                 | PASS   |
| UART RX on PA10                | PASS   |
| UART 115200 8N1                | PASS   |
| UART DMA circular buffer       | PASS   |
| UART echo test                 | PASS   |
| ST-LINK programming connection | PASS   |

## Module 6 Status: PASS

The STM32F401CCU6 successfully implements ADC DMA circular buffering with 4 + 4 sample processing and UART RX DMA buffering using the confirmed connections above.
