#include <Arduino.h>

/*
============================================================
                    MODULE 6 FIRMWARE
                    Arduino UNO / ATmega328P
============================================================

TASK 1:
DMA CIRCULAR BUFFER CONCEPT
----------------------------
Simulated DMA producer
        ↓
Circular buffer
        ↓
Half buffer ready
        ↓
Process first half
        ↓
Full buffer ready
        ↓
Process second half

TASK 2:
UART RX CIRCULAR BUFFER
-----------------------
Serial RX
    ↓
Circular buffer
    ↓
Main-loop processing

IMPORTANT:
ATmega328P does NOT have a hardware DMA controller.
Therefore Task 1 demonstrates the DMA circular-buffer
concept using software.

FEATURES:
- Single .ino file
- One setup()
- One loop()
- No delay()
- No dynamic memory
- Circular buffers
- Half/full buffer processing
- UART overflow detection
- Non-blocking timing
============================================================
*/


/* =========================================================
   TASK 1
   DMA CIRCULAR BUFFER
   ========================================================= */

#define DMA_SIZE 8

uint16_t dmaBuffer[DMA_SIZE];

uint8_t dmaIndex = 0;

bool halfFlag = false;
bool fullFlag = false;


/*
   Simulated DMA sampling interval.

   One sample is generated every 100 ms.
*/

#define DMA_SAMPLE_INTERVAL 100UL

unsigned long lastDmaSampleTime = 0;


/*
   Simulated DMA producer
*/

void dmaFill()
{
    /*
       Put sample into buffer.
    */

    dmaBuffer[dmaIndex] =
        dmaIndex * 10;

    dmaIndex++;


    /*
       Half of buffer filled.
    */

    if (dmaIndex == DMA_SIZE / 2)
    {
        halfFlag = true;
    }


    /*
       Entire buffer filled.
    */

    if (dmaIndex == DMA_SIZE)
    {
        fullFlag = true;

        /*
           Circular buffer starts again.
        */

        dmaIndex = 0;
    }
}


/*
   Process DMA data
*/

void processDMA(uint16_t *data, uint8_t length)
{
    Serial.print("Processing: ");

    for (uint8_t i = 0; i < length; i++)
    {
        Serial.print(data[i]);

        if (i < length - 1)
        {
            Serial.print(" ");
        }
    }

    Serial.println();
}


/*
   Process half/full DMA events
*/

void processDMAEvents()
{
    /*
       First half ready
    */

    if (halfFlag)
    {
        halfFlag = false;

        processDMA(
            &dmaBuffer[0],
            DMA_SIZE / 2
        );
    }


    /*
       Second half ready
    */

    if (fullFlag)
    {
        fullFlag = false;

        processDMA(
            &dmaBuffer[DMA_SIZE / 2],
            DMA_SIZE / 2
        );
    }
}


/*
   Non-blocking DMA scheduler
*/

void handleDmaSampling()
{
    unsigned long currentTime = millis();

    if ((unsigned long)(currentTime - lastDmaSampleTime)
        >= DMA_SAMPLE_INTERVAL)
    {
        lastDmaSampleTime = currentTime;

        dmaFill();
    }
}


/* =========================================================
   TASK 2
   UART RX CIRCULAR BUFFER
   ========================================================= */

#define UART_BUFFER_SIZE 16

uint8_t uartBuffer[UART_BUFFER_SIZE];

uint8_t uartHead = 0;
uint8_t uartTail = 0;

uint16_t uartOverflowCount = 0;


/*
   Receive UART data and place it
   into circular buffer.
*/

void uartReceive()
{
    while (Serial.available())
    {
        uint8_t data =
            (uint8_t)Serial.read();


        /*
           Calculate next head position.
        */

        uint8_t next =
            (uint8_t)(
                (uartHead + 1) %
                UART_BUFFER_SIZE
            );


        /*
           Check whether buffer is full.
        */

        if (next != uartTail)
        {
            uartBuffer[uartHead] = data;

            uartHead = next;
        }
        else
        {
            /*
               Buffer overflow.
            */

            uartOverflowCount++;
        }
    }
}


/*
   Process UART data from circular buffer.
*/

void uartProcess()
{
    while (uartTail != uartHead)
    {
        uint8_t data =
            uartBuffer[uartTail];


        /*
           Move tail forward.
        */

        uartTail =
            (uint8_t)(
                (uartTail + 1) %
                UART_BUFFER_SIZE
            );


        /*
           Ignore ENTER/newline characters.
        */

        if (data != '\n' &&
            data != '\r')
        {
            Serial.print("RX: ");

            Serial.println(
                (char)data
            );
        }
    }
}


/* =========================================================
   STATUS
   ========================================================= */

unsigned long lastStatusTime = 0;

void printStatus()
{
    unsigned long currentTime =
        millis();


    /*
       Print status once every second.
    */

    if ((unsigned long)(
            currentTime - lastStatusTime
        ) < 1000UL)
    {
        return;
    }


    lastStatusTime =
        currentTime;


    Serial.print("UART Overflows: ");

    Serial.println(
        uartOverflowCount
    );
}


/* =========================================================
   SETUP
   ========================================================= */

void setup()
{
    Serial.begin(9600);


    Serial.println();

    Serial.println("============================");

    Serial.println("MODULE 6 FIRMWARE");

    Serial.println("============================");

    Serial.println();

    Serial.println("TASK 1: ADC/DMA CIRCULAR BUFFER");

    Serial.println("TASK 2: UART RX CIRCULAR BUFFER");

    Serial.println();

    Serial.println("Sampling started...");

    Serial.println("Type characters to test UART RX");

    Serial.println();
}


/* =========================================================
   LOOP
   ========================================================= */

void loop()
{
    /*
       ------------------------------
       TASK 1
       ------------------------------
    */

    handleDmaSampling();

    processDMAEvents();


    /*
       ------------------------------
       TASK 2
       ------------------------------
    */

    uartReceive();

    uartProcess();


    /*
       ------------------------------
       STATUS
       ------------------------------
    */

    printStatus();
}
