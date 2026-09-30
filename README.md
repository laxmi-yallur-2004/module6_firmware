# Module 6 Firmware

## Hardware

* Arduino Uno / ATmega328P
* Potentiometer
* USB cable
* Potentiometer middle pin → A1
* Potentiometer outer pins → 5V and GND

## Task 1 — ADC Sampling Using Circular Buffer

### What we did

We read the analog voltage from **A1** and store the ADC values in an **8-sample circular buffer**.

The buffer is divided into two halves:

* Samples 1–4 → First Half
* Samples 5–8 → Second Half

After 8 samples, the buffer starts again from position 1.

### Working

```text
Potentiometer
     ↓
    A1
     ↓
analogRead()
     ↓
8-sample circular buffer
     ↓
First 4 samples → Process
Next 4 samples  → Process
     ↓
Repeat
```

### Why we used it

A circular buffer allows continuous sampling without creating a new buffer every time.

The same memory is reused again and again.

### Output

```text
FIRST HALF
ADC = 683
ADC = 682
ADC = 682
ADC = 683

SECOND HALF
ADC = 682
ADC = 683
ADC = 682
ADC = 683

FIRST HALF
ADC = 682
ADC = 683
...
```

The ADC values change when the potentiometer position changes.

**Note:** ATmega328P does not have hardware DMA. Therefore, this module demonstrates the **DMA circular-buffer concept using software**.

---

## Task 2 — UART RX Using ISR and Safe Circular Buffer

### What we did

We receive characters from the PC using the **UART receive interrupt (ISR)**.

Every received character is placed into a **64-byte circular buffer**.

The main loop removes the data from the buffer and forms the complete message.

### Working

```text
PC
 ↓
UART RX
 ↓
RX Interrupt
 ↓
64-byte circular buffer
 ↓
Main loop
 ↓
Complete message
 ↓
TX / RX / MISMATCH / DATA LOSS
```

### Why we used it

The ISR receives data quickly and stores it safely while the main program continues running.

The circular buffer prevents the main program from needing to process every character immediately.

Overflow is counted if the buffer becomes full.

### UART Configuration

```text
Baud Rate : 9600
Data      : 8 bits
Parity    : None
Stop      : 1
```

### Test

Send:

```text
HELLO
```

and press Enter.

### Output

```text
TX: HELLO
RX: HELLO
MISMATCH: 0
DATA LOSS: 0
UART OVERFLOWS: 0
```

This shows that the message was received correctly and no UART buffer overflow occurred during the test.

## Main Concepts Learned

* ADC sampling
* Circular buffers
* Half-buffer processing
* Continuous buffer reuse
* UART receive interrupt
* Safe ISR buffering
* Overflow detection
* Non-blocking timing using `millis()`
* No `delay()`
* No dynamic memory

## Limitations

The Arduino Uno ATmega328P has no hardware DMA controller, so Task 1 is a **software implementation of the DMA circular-buffer concept**.

Task 2 uses the UART RX interrupt directly instead of Arduino `Serial`, avoiding conflicts with the Arduino HardwareSerial RX interrupt.

```
```
