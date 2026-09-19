# Module 6 Firmware

## Overview

Module 6 contains two tasks based on safe continuous data handling using circular buffers.

* **Task 1:** ADC/DMA Circular Buffer Concept
* **Task 2:** UART RX Circular Buffer

The implementation is combined into a **single `module6_firmware.ino` file**.

The code uses:

* Circular buffers
* Half-buffer and full-buffer processing
* UART receive buffering
* UART overflow detection
* Non-blocking `millis()` timing
* No `delay()`
* No dynamic memory

> **Hardware note:** The Arduino Uno ATmega328P does not contain a hardware DMA controller. Therefore, Task 1 demonstrates the DMA circular-buffer concept using a software DMA producer.

---

# Task 1 — ADC/DMA Circular Buffer

## What We Did

A circular buffer of **8 samples** was created.

The buffer is filled continuously with sample values:

```text
0 10 20 30 40 50 60 70
```

The buffer is divided into two halves:

```text
First Half              Second Half
0  10  20  30          40  50  60  70
```

When the first half is filled, it is processed.

When the complete buffer is filled, the second half is processed.

After that, the buffer starts again from the beginning.

This demonstrates the basic **circular-buffer and half/full-buffer processing pipeline**.

## Output Obtained

```text
Processing: 0 10 20 30
Processing: 40 50 60 70

Processing: 0 10 20 30
Processing: 40 50 60 70

Processing: 0 10 20 30
Processing: 40 50 60 70

Processing: 0 10 20 30
Processing: 40 50 60 70
```

## Result

```text
Task 1: Circular buffer processing working successfully.
```

---

# Task 2 — UART RX Circular Buffer

## What We Did

A **16-byte UART RX circular buffer** was implemented.

Incoming UART data is first stored in the buffer instead of being processed immediately.

The flow is:

```text
UART RX
   ↓
RX Circular Buffer
   ↓
Main Loop
   ↓
Process Received Data
```

The buffer uses:

* `head` — position where new data is stored
* `tail` — position from where data is processed
* Overflow counter — detects when the buffer becomes full

Newline (`\n`) and carriage return (`\r`) characters are ignored while displaying received data.

## Output Obtained

During the test, the UART buffer reported:

```text
UART Overflows: 0
```

This means no UART circular-buffer overflow occurred during the observed test.

When characters are received, the output format is:

```text
RX: H
RX: E
RX: L
RX: L
RX: O
```

## Result

```text
Task 2: UART RX circular buffer working successfully.
UART Overflows: 0
```

---

# Final Module 6 Output

```text
============================
MODULE 6 FIRMWARE
============================

TASK 1: ADC/DMA CIRCULAR BUFFER
TASK 2: UART RX CIRCULAR BUFFER

Sampling started...
Type characters to test UART RX

Processing: 0 10 20 30
Processing: 40 50 60 70
UART Overflows: 0

Processing: 0 10 20 30
Processing: 40 50 60 70
UART Overflows: 0

Processing: 0 10 20 30
Processing: 40 50 60 70
UART Overflows: 0
```

## Final Result

```text
TASK 1: PASS
Circular buffer half/full processing demonstrated.

TASK 2: PASS
UART RX circular buffer active.
UART Overflows: 0
```
