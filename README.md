# Module 6 Firmware

## Platform

* Arduino Uno
* ATmega328P
* CPU: 16 MHz
* ADC: 10-bit
* UART: 9600 8N1

## Tasks

### Task 1: ADC Interrupt Circular Buffer

* ADC input: A1
* ADC conversion is handled using `ADC_vect` interrupt.
* 8 ADC samples are stored in a circular/reused buffer.
* Samples are processed as two halves:

  * First half: 4 samples
  * Second half: 4 samples
* Average ADC value is calculated for each half.
* No ADC polling is used.

### Task 2: UART RX ISR Buffer

* UART reception uses `USART_RX_vect` interrupt.
* 64-byte circular buffer is used for received data.
* Received messages are processed in the main loop.
* UART buffer overflow is counted.
* Test verifies received data and overflow status.

## Connections

### Potentiometer

| Potentiometer   | Arduino Uno |
| --------------- | ----------- |
| Outer pin       | 5V          |
| Other outer pin | GND         |
| Middle/wiper    | A1          |

### UART

Connect Arduino Uno to the PC using the USB cable.

Serial Monitor:

```text
Baud Rate: 9600
Data: 8 bits
Parity: None
Stop bits: 1
```

## Expected Output

```text
MODULE 6 FIRMWARE
------------------
TASK 1: ADC INTERRUPT CIRCULAR BUFFER
TASK 2: UART RX ISR BUFFER
ADC PIN: A1
UART: 9600 8N1
READY

FIRST HALF
ADC = 500
ADC = 503
ADC = 506
ADC = 508
AVERAGE ADC = 504

SECOND HALF
ADC = 514
ADC = 515
ADC = 516
ADC = 516
AVERAGE ADC = 515


RX: laxmi
UART OVERFLOWS: 0
```

## DMA Limitation

The ATmega328P does not have a hardware DMA controller.

Therefore, Task 1 uses **ADC interrupt-driven circular buffering** instead of true DMA.

## Result

* ADC interrupt sampling: PASS
* ADC circular buffer: PASS
* UART RX ISR: PASS
* UART circular buffer: PASS
* Overflow detection: PASS

