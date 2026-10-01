# Module 6 Firmware

## Hardware

* Arduino Uno / ATmega328P
* Potentiometer
* USB cable

## Connections

| Connection          | Arduino |
| ------------------- | ------- |
| Potentiometer VCC   | 5V      |
| Potentiometer Wiper | A1      |
| Potentiometer GND   | GND     |

UART is through the Arduino USB connection.

## Task 1: ADC Circular Buffer

The Arduino reads the analog voltage from **A1** every 100 ms.

The ADC values are stored in an **8-sample circular buffer**.

```text
Potentiometer
      ↓
     A1
      ↓
   ADC Read
      ↓
8-sample buffer
      ↓
First 4 samples → Process
Last 4 samples  → Process
      ↓
Buffer starts again
```

The buffer is divided into two halves:

* Samples 0–3 → First half
* Samples 4–7 → Second half

The processed ADC values are printed through UART.

## Task 2: UART RX Using ISR

UART receive is handled using the **USART RX interrupt**.

Each received byte is stored in a **64-byte circular buffer**.

```text
UART RX
   ↓
RX Interrupt
   ↓
Circular Buffer
   ↓
Main Loop
   ↓
Process Received Data
```

The ISR only stores the received byte quickly. The main loop processes the buffered data.

If the circular buffer becomes full, the received byte cannot be stored and `uartOverflow` is increased.

This allows overflow/data loss to be detected during a stress test.

## Stress Test

Send continuous or repeated UART data from the PC.

Example:

```text
HELLO
HELLO
HELLO
HELLO
HELLO
```

The Serial Monitor displays the received message and the overflow count.

Expected result when the buffer does not overflow:

```text
RX: HELLO
UART OVERFLOWS: 0
```

`UART OVERFLOWS: 0` means no circular-buffer overflow occurred during that test.

## UART Settings

```text
Baud Rate: 9600
Data Bits: 8
Parity: None
Stop Bits: 1
```

## Example Output

```text
MODULE 6 FIRMWARE
------------------
TASK 1: ADC CIRCULAR BUFFER
TASK 2: UART RX ISR BUFFER
ADC PIN: A1
UART: 9600 8N1
READY

FIRST HALF
ADC = 450
ADC = 472
ADC = 510
ADC = 530

SECOND HALF
ADC = 550
ADC = 570
ADC = 600
ADC = 620

RX: HELLO
UART OVERFLOWS: 0
```

The ADC values will change when the potentiometer is rotated.

## Key Points

* No `delay()` is used.
* No dynamic memory is used.
* UART reception uses an interrupt.
* UART data is stored safely in a circular buffer.
* Buffer overflow is detected.
* ADC data is collected in an 8-sample circular buffer.
* First and second ADC halves are processed separately.
