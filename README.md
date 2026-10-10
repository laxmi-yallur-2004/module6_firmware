# Module 6 – ADC DMA and UART DMA

## 1. Objective

**Task 1:** ADC sampling using a circular DMA buffer with a stable processing pipeline.

**Task 2:** UART reception using DMA buffering, safe ring buffering, error handling, and stress testing.

## 2. Hardware Connections

| STM32 Black Pill | Connection |
|---|---|
| PA0 | Analog input |
| PA9 (USART1 TX) | USB-to-TTL RX |
| PA10 (USART1 RX) | USB-to-TTL TX |
| GND | Common GND |

## 3. Configuration

- ADC resolution: 12-bit
- ADC samples: 8 (two halves of 4 samples)
- ADC mode: Circular DMA
- UART: USART1, 115200 baud, 8-N-1
- UART DMA buffer: 256 bytes
- UART ring buffer: 1024 bytes
- ADC output interval: 500 ms

## 4. Working

**Task 1 – ADC Sampling**

ADC continuously samples the analog input on PA0 using circular DMA. Half-transfer and transfer-complete callbacks copy samples into separate blocks. The firmware calculates average ADC values and converts them into millivolts.

**Task 2 – UART Reception**

UART receives data through circular DMA and transfers it into a software ring buffer. Received data is transmitted back for stress testing. The firmware monitors DMA data loss, ring-buffer overflow, UART errors, ADC errors, and dropped ADC blocks.

## 5. Test Results

### Test 1 – ADC Sampling

**First Half**

| Parameter | Result |
|---|---:|
| Block number | 247917 |
| Sample 1 | 2607 counts / 2100 mV |
| Sample 2 | 2608 counts / 2101 mV |
| Sample 3 | 2606 counts / 2100 mV |
| Sample 4 | 2606 counts / 2100 mV |
| Average | 2606 counts / 2100 mV |
| ADC blocks dropped | 495740 |

**Second Half**

| Parameter | Result |
|---|---:|
| Block number | 250584 |
| Sample 5 | 2612 counts / 2104 mV |
| Sample 6 | 2610 counts / 2103 mV |
| Sample 7 | 2612 counts / 2104 mV |
| Sample 8 | 2612 counts / 2104 mV |
| Average | 2611 counts / 2104 mV |
| ADC blocks dropped | 501074 |

### Test 2 – UART Stress Testing

Repeated messages containing alphabet sequences were transmitted, including `A01`–`A20`, `B01`–`B20`, `C01`–`C20`, `D01`–`D20`, and `E01`–`E20`.

| Parameter | Result |
|---|---|
| UART baud rate | 115200 |
| DMA buffer size | 256 bytes |
| Ring buffer size | 1024 bytes |
| DMA high-water mark | 128 / 256 bytes |
| Message reception | Data visible in output |
| Zero data loss | Not yet verified |

## 6. Observations

- ADC readings remained around 2100–2104 mV.
- ADC dropped-block counts were very high, indicating that the application was not processing ADC blocks fast enough.
- UART stress-test messages were visible in the output.
- The DMA high-water mark reached 128 bytes.
- Complete UART data integrity has not been established from the provided output.

## 7. Conclusion

ADC circular DMA sampling and UART DMA reception with ring buffering were implemented on the STM32 Black Pill. The test confirmed ADC readings and UART message reception. Further optimization is required to reduce ADC dropped blocks and verify zero UART data loss under sustained stress testing.
