# Module 6 Firmware

## Platform

* Arduino Uno
* ATmega328P
* CPU: 16 MHz
* ADC: 10-bit
* UART: 9600 8N1

## Tasks

### Task 1: ADC Interrupt Circular Buffer

ADC reads **A1 using an interrupt** and stores **8 samples in a circular buffer**, then calculates the average of two 4-sample halves.

### Task 2: UART RX ISR Buffer

UART receives data using an **RX interrupt and circular buffer**; extra data is detected and counted using the **overflow counter**.

## Connections

### Potentiometer

| Potentiometer   | Arduino Uno |
| --------------- | ----------- |
| Outer pin       | 5V          |
| Other outer pin | GND         |
| Middle/wiper    | A1          |

### UART

Connect Arduino Uno to the PC using the USB cable.

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

## Overflow Test

```text
RX: ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789A
UART OVERFLOWS: 2
```

A non-zero value confirms that extra data was detected.

## DMA Limitation

The ATmega328P has **no hardware DMA controller**, so ADC sampling uses an **ADC interrupt and circular buffer** instead.

## Result

* ADC interrupt sampling: PASS
* ADC circular buffer: PASS
* UART RX ISR: PASS
* UART circular buffer: PASS
* Overflow detection: PASS
