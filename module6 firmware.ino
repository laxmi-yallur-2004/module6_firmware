#include <Arduino.h>
#include "stm32f4xx_hal.h"
#include <string.h>
#include <stdio.h>

#define ADC_SAMPLES       8
#define UART_DMA_SIZE     128
#define UART_RING_SIZE    256

ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc;

UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_uart;

volatile uint16_t adcBuffer[ADC_SAMPLES];

volatile uint16_t firstHalfBuffer[4];
volatile uint16_t secondHalfBuffer[4];

volatile bool firstHalfReady = false;
volatile bool secondHalfReady = false;

uint8_t uartDmaBuffer[UART_DMA_SIZE];
uint8_t uartRingBuffer[UART_RING_SIZE];

uint16_t uartDmaPosition = 0;
uint16_t uartHead = 0;
uint16_t uartTail = 0;


/* ---------- ADC DMA callbacks ---------- */

void adcHalfCallback(DMA_HandleTypeDef *hdma)
{
  for (int i = 0; i < 4; i++)
  {
    firstHalfBuffer[i] = adcBuffer[i];
  }

  firstHalfReady = true;
}

void adcFullCallback(DMA_HandleTypeDef *hdma)
{
  for (int i = 0; i < 4; i++)
  {
    secondHalfBuffer[i] = adcBuffer[i + 4];
  }

  secondHalfReady = true;
}


/* ---------- DMA interrupts ---------- */

extern "C" void DMA2_Stream0_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_adc);
}

extern "C" void DMA2_Stream2_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_uart);
}


/* ---------- UART initialization ---------- */

void UART_Init()
{
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_DMA2_CLK_ENABLE();
  __HAL_RCC_USART1_CLK_ENABLE();

  GPIO_InitTypeDef gpio = {};

  gpio.Pin = GPIO_PIN_9 | GPIO_PIN_10;
  gpio.Mode = GPIO_MODE_AF_PP;
  gpio.Pull = GPIO_PULLUP;
  gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  gpio.Alternate = GPIO_AF7_USART1;

  HAL_GPIO_Init(GPIOA, &gpio);


  hdma_uart.Instance = DMA2_Stream2;
  hdma_uart.Init.Channel = DMA_CHANNEL_4;
  hdma_uart.Init.Direction = DMA_PERIPH_TO_MEMORY;
  hdma_uart.Init.PeriphInc = DMA_PINC_DISABLE;
  hdma_uart.Init.MemInc = DMA_MINC_ENABLE;
  hdma_uart.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  hdma_uart.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
  hdma_uart.Init.Mode = DMA_CIRCULAR;
  hdma_uart.Init.Priority = DMA_PRIORITY_HIGH;
  hdma_uart.Init.FIFOMode = DMA_FIFOMODE_DISABLE;

  HAL_DMA_Init(&hdma_uart);

  __HAL_LINKDMA(&huart1, hdmarx, hdma_uart);


  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;

  HAL_UART_Init(&huart1);


  HAL_NVIC_SetPriority(DMA2_Stream2_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream2_IRQn);


  HAL_UART_Receive_DMA(
    &huart1,
    uartDmaBuffer,
    UART_DMA_SIZE
  );
}


/* ---------- ADC initialization ---------- */

void ADC_Init()
{
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_DMA2_CLK_ENABLE();
  __HAL_RCC_ADC1_CLK_ENABLE();

  GPIO_InitTypeDef gpio = {};

  gpio.Pin = GPIO_PIN_0;
  gpio.Mode = GPIO_MODE_ANALOG;
  gpio.Pull = GPIO_NOPULL;

  HAL_GPIO_Init(GPIOA, &gpio);


  hdma_adc.Instance = DMA2_Stream0;
  hdma_adc.Init.Channel = DMA_CHANNEL_0;
  hdma_adc.Init.Direction = DMA_PERIPH_TO_MEMORY;
  hdma_adc.Init.PeriphInc = DMA_PINC_DISABLE;
  hdma_adc.Init.MemInc = DMA_MINC_ENABLE;
  hdma_adc.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
  hdma_adc.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
  hdma_adc.Init.Mode = DMA_CIRCULAR;
  hdma_adc.Init.Priority = DMA_PRIORITY_HIGH;
  hdma_adc.Init.FIFOMode = DMA_FIFOMODE_DISABLE;

  HAL_DMA_Init(&hdma_adc);

  __HAL_LINKDMA(&hadc1, DMA_Handle, hdma_adc);


  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = ENABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;

  HAL_ADC_Init(&hadc1);


  ADC_ChannelConfTypeDef channel = {};

  channel.Channel = ADC_CHANNEL_0;
  channel.Rank = 1;
  channel.SamplingTime = ADC_SAMPLETIME_480CYCLES;

  HAL_ADC_ConfigChannel(&hadc1, &channel);


  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);


  HAL_ADC_Start_DMA(
    &hadc1,
    (uint32_t *)adcBuffer,
    ADC_SAMPLES
  );


  /*
     HAL_ADC_Start_DMA() sets the HAL DMA callbacks.
     Replace them after DMA has started.
  */
  hdma_adc.XferHalfCpltCallback = adcHalfCallback;
  hdma_adc.XferCpltCallback = adcFullCallback;
}


/* ---------- UART transmit ---------- */

void uartPrint(const char *text)
{
  HAL_UART_Transmit(
    &huart1,
    (uint8_t *)text,
    strlen(text),
    100
  );
}


/* ---------- First half processing ---------- */

void processFirstHalf()
{
  uint32_t sum = 0;

  uartPrint("\r\nFIRST HALF PROCESSING\r\n");

  for (int i = 0; i < 4; i++)
  {
    uint16_t adc = firstHalfBuffer[i];

    uint32_t mv =
      ((uint32_t)adc * 3300UL) / 4095UL;

    sum += adc;

    char line[50];

    snprintf(
      line,
      sizeof(line),
      "Sample %d: %u | %lu mV\r\n",
      i + 1,
      adc,
      mv
    );

    uartPrint(line);
  }

  uint16_t average = sum / 4;

  uint32_t averageMv =
    ((uint32_t)average * 3300UL) / 4095UL;

  char line[60];

  snprintf(
    line,
    sizeof(line),
    "FIRST HALF AVERAGE: %u | %lu mV\r\n",
    average,
    averageMv
  );

  uartPrint(line);
}


/* ---------- Second half processing ---------- */

void processSecondHalf()
{
  uint32_t sum = 0;

  uartPrint("\r\nSECOND HALF PROCESSING\r\n");

  for (int i = 0; i < 4; i++)
  {
    uint16_t adc = secondHalfBuffer[i];

    uint32_t mv =
      ((uint32_t)adc * 3300UL) / 4095UL;

    sum += adc;

    char line[50];

    snprintf(
      line,
      sizeof(line),
      "Sample %d: %u | %lu mV\r\n",
      i + 5,
      adc,
      mv
    );

    uartPrint(line);
  }

  uint16_t average = sum / 4;

  uint32_t averageMv =
    ((uint32_t)average * 3300UL) / 4095UL;

  char line[60];

  snprintf(
    line,
    sizeof(line),
    "SECOND HALF AVERAGE: %u | %lu mV\r\n",
    average,
    averageMv
  );

  uartPrint(line);
}


/* ---------- UART DMA processing ---------- */

void processUART()
{
  uint16_t currentPosition =
    UART_DMA_SIZE - __HAL_DMA_GET_COUNTER(&hdma_uart);

  while (uartDmaPosition != currentPosition)
  {
    uint8_t data =
      uartDmaBuffer[uartDmaPosition];

    uartDmaPosition++;

    if (uartDmaPosition >= UART_DMA_SIZE)
      uartDmaPosition = 0;

    uint16_t nextHead = uartHead + 1;

    if (nextHead >= UART_RING_SIZE)
      nextHead = 0;

    if (nextHead != uartTail)
    {
      uartRingBuffer[uartHead] = data;
      uartHead = nextHead;
    }
  }

  while (uartTail != uartHead)
  {
    uint8_t data =
      uartRingBuffer[uartTail];

    uartTail++;

    if (uartTail >= UART_RING_SIZE)
      uartTail = 0;

    HAL_UART_Transmit(
      &huart1,
      &data,
      1,
      10
    );
  }
}


/* ---------- Setup ---------- */

void setup()
{
  Serial.begin(115200);

  /*
     Initialize UART first so we can immediately see
     the startup messages.
  */
  UART_Init();

  uartPrint("\r\nMODULE 6 FIRMWARE\r\n");
  uartPrint("------------------\r\n");
  uartPrint("TASK 1: ADC DMA CIRCULAR BUFFER\r\n");
  uartPrint("TASK 2: UART RX DMA BUFFER\r\n");
  uartPrint("ADC PIN: PA0\r\n");
  uartPrint("UART TX: PA9\r\n");
  uartPrint("UART RX: PA10\r\n");
  uartPrint("UART: 115200 8N1\r\n");
  uartPrint("ADC: 12-BIT, 4 + 4 SAMPLES\r\n");

  ADC_Init();

  uartPrint("READY\r\n");
}


/* ---------- Main loop ---------- */

void loop()
{
  processUART();

  static uint32_t lastPrintTime = 0;
  static bool printFirstHalf = true;

  if (millis() - lastPrintTime >= 500)
  {
    lastPrintTime = millis();

    if (firstHalfReady && secondHalfReady)
    {
      if (printFirstHalf)
      {
        processFirstHalf();
      }
      else
      {
        processSecondHalf();
      }

      printFirstHalf = !printFirstHalf;
    }
  }
}
