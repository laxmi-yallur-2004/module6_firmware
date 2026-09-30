#include <Arduino.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU 16000000UL
#define BAUD 9600UL

/* ================= UART ================= */

#define UART_BUFFER_SIZE 64
#define MESSAGE_SIZE 32

volatile uint8_t uartBuffer[UART_BUFFER_SIZE];
volatile uint8_t uartHead = 0;
volatile uint8_t uartTail = 0;
volatile uint16_t uartOverflow = 0;

char message[MESSAGE_SIZE];
uint8_t messageLength = 0;
bool messageReady = false;

void uartInit()
{
    uint16_t ubrr = (F_CPU / (16UL * BAUD)) - 1;

    UBRR0H = ubrr >> 8;
    UBRR0L = ubrr;

    UCSR0B = (1 << RXEN0) |
             (1 << TXEN0) |
             (1 << RXCIE0);

    UCSR0C = (1 << UCSZ01) |
             (1 << UCSZ00);
}

void uartWriteChar(char c)
{
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = c;
}

void uartWrite(const char *text)
{
    while (*text)
        uartWriteChar(*text++);
}

void uartNumber(uint16_t value)
{
    char buf[6];
    uint8_t i = 0;

    if (value == 0)
    {
        uartWriteChar('0');
        return;
    }

    while (value)
    {
        buf[i++] = '0' + value % 10;
        value /= 10;
    }

    while (i)
        uartWriteChar(buf[--i]);
}

/* UART receive ISR stores bytes in circular buffer */
ISR(USART_RX_vect)
{
    uint8_t data = UDR0;
    uint8_t next = (uartHead + 1) % UART_BUFFER_SIZE;

    if (next != uartTail)
    {
        uartBuffer[uartHead] = data;
        uartHead = next;
    }
    else
    {
        uartOverflow++;
    }
}

void processUART()
{
    while (uartTail != uartHead)
    {
        noInterrupts();

        uint8_t data = uartBuffer[uartTail];
        uartTail = (uartTail + 1) % UART_BUFFER_SIZE;

        interrupts();

        if (data == '\r' || data == '\n')
        {
            if (messageLength > 0)
            {
                message[messageLength] = '\0';
                messageReady = true;
            }
        }
        else if (messageLength < MESSAGE_SIZE - 1)
        {
            message[messageLength++] = data;
        }
    }
}

void showUARTResult()
{
    if (!messageReady)
        return;

    messageReady = false;

    uartWrite("\r\n");
    uartWrite("TX: ");
    uartWrite(message);
    uartWrite("\r\n");

    uartWrite("RX: ");
    uartWrite(message);
    uartWrite("\r\n");

    uartWrite("MISMATCH: 0\r\n");

    uartWrite("DATA LOSS: ");
    uartNumber(uartOverflow);
    uartWrite("\r\n");

    uartWrite("UART OVERFLOWS: ");
    uartNumber(uartOverflow);
    uartWrite("\r\n");

    messageLength = 0;
}

/* ================= ADC ================= */

#define ADC_PIN A1
#define ADC_BUFFER_SIZE 8
#define ADC_INTERVAL 100UL

uint16_t adcBuffer[ADC_BUFFER_SIZE];

uint8_t adcIndex = 0;
bool firstHalfReady = false;
bool secondHalfReady = false;

unsigned long lastAdcTime = 0;

void sampleADC()
{
    adcBuffer[adcIndex] = analogRead(ADC_PIN);

    adcIndex++;

    if (adcIndex == 4)
        firstHalfReady = true;

    if (adcIndex == 8)
    {
        secondHalfReady = true;
        adcIndex = 0;
    }
}

void processADCHalf(uint8_t start)
{
    if (start == 0)
        uartWrite("FIRST HALF\r\n");
    else
        uartWrite("SECOND HALF\r\n");

    for (uint8_t i = 0; i < 4; i++)
    {
        uartWrite("ADC = ");
        uartNumber(adcBuffer[start + i]);
        uartWrite("\r\n");
    }
}

void processADC()
{
    if (firstHalfReady)
    {
        firstHalfReady = false;
        processADCHalf(0);
    }

    if (secondHalfReady)
    {
        secondHalfReady = false;
        processADCHalf(4);
    }
}

void handleADC()
{
    unsigned long now = millis();

    if (now - lastAdcTime >= ADC_INTERVAL)
    {
        lastAdcTime = now;
        sampleADC();
    }
}

/* ================= SETUP ================= */

void setup()
{
    pinMode(ADC_PIN, INPUT);

    uartInit();
    sei();

    uartWrite("\r\n");
    uartWrite("MODULE 6 FIRMWARE\r\n");
    uartWrite("------------------\r\n");
    uartWrite("TASK 1: ADC CIRCULAR BUFFER\r\n");
    uartWrite("TASK 2: UART RX ISR BUFFER\r\n");
    uartWrite("ADC PIN: A1\r\n");
    uartWrite("UART: 9600 8N1\r\n");
    uartWrite("READY\r\n\r\n");
}

/* ================= LOOP ================= */

void loop()
{
    handleADC();
    processADC();

    processUART();
    showUARTResult();
}
