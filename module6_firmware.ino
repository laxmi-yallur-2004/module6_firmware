#include <Arduino.h>
#include "stm32f4xx_hal.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#define ADC_SAMPLES     8
#define ADC_HALF        4
#define UART_DMA_SIZE   256
#define UART_RING_SIZE  256

ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc;
UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_uart;

volatile uint16_t adcBuffer[ADC_SAMPLES];

struct AdcBlock
{
  volatile uint16_t samples[ADC_HALF];
  volatile uint32_t seq;
  volatile bool ready;
};

AdcBlock blocks[2];                 
volatile uint32_t adcDropped = 0;
volatile uint32_t adcErrors = 0;


volatile uint16_t dmaPos = 0;
volatile uint32_t dmaCycles = 0;    /* completed DMA cycles counted by the IRQ */
uint32_t cyclesUsed = 0;
volatile uint16_t ringHead = 0;
volatile uint16_t ringTail = 0;
volatile uint16_t dmaHighWater = 0;
volatile uint32_t dmaLost = 0;      /* bytes overwritten by DMA before being read */
volatile uint32_t ringOverflow = 0;
volatile uint32_t uartErrors = 0;
volatile bool uartRestart = false;
uint32_t txErrors = 0;


static void fatalError()
{
  while (true) {}
}

bool uartPrintf(const char *fmt, ...)
{
  char buf[160];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);

  if (HAL_UART_Transmit(&huart1, (uint8_t *)buf, strlen(buf), 100) != HAL_OK)
  {
    txErrors++;
    return false;
  }
  return true;
}

static void drainUart()
{
  uint16_t cur = (UART_DMA_SIZE - __HAL_DMA_GET_COUNTER(&hdma_uart)) % UART_DMA_SIZE;
  uint16_t last = dmaPos;
  uint16_t waiting = (cur + UART_DMA_SIZE - last) % UART_DMA_SIZE;

  int32_t wrapped = (cur < last) ? 1 : 0;
  int32_t extra = (int32_t)(dmaCycles - cyclesUsed) - wrapped;
  if (extra < 0) extra = 0;
  cyclesUsed += (uint32_t)(wrapped + extra);

  uint16_t start = last;
  uint32_t count = waiting;

  if (extra > 0)
  {
    dmaLost += waiting + (uint32_t)(extra - 1) * UART_DMA_SIZE;
    start = cur;
    count = UART_DMA_SIZE;
    waiting = UART_DMA_SIZE;
  }

  if (waiting > dmaHighWater) dmaHighWater = waiting;

  for (uint32_t i = 0; i < count; i++)
  {
    uint8_t data = uartDmaBuffer[(start + i) % UART_DMA_SIZE];

    uint16_t next = (ringHead + 1) % UART_RING_SIZE;
    if (next != ringTail)
    {
      uartRing[ringHead] = data;
      ringHead = next;
    }
    else
    {
      ringOverflow++;
    }
  }

  dmaPos = cur;
}

static void uartDmaCb(DMA_HandleTypeDef *hdma)
{
  (void)hdma;
  drainUart();
}

static void uartDmaErrCb(DMA_HandleTypeDef *hdma)
{
  (void)hdma;
  uartErrors++;
  uartRestart = true;
}

static bool startUartRx()
{
  HAL_NVIC_DisableIRQ(DMA2_Stream2_IRQn);
  dmaPos = 0;
  dmaCycles = 0;
  cyclesUsed = 0;

  if (HAL_UART_Receive_DMA(&huart1, uartDmaBuffer, UART_DMA_SIZE) != HAL_OK)
    return false;

  hdma_uart.XferHalfCpltCallback = uartDmaCb;
  hdma_uart.XferCpltCallback = uartDmaCb;
  hdma_uart.XferErrorCallback = uartDmaErrCb;

  __HAL_UART_DISABLE_IT(&huart1, UART_IT_PE);
  __HAL_UART_DISABLE_IT(&huart1, UART_IT_ERR);

  HAL_NVIC_EnableIRQ(DMA2_Stream2_IRQn);
  return true;
}

static void captureBlock(int slot, int offset)
{
  if (blocks[slot].ready) adcDropped++;

  for (int i = 0; i < ADC_HALF; i++)
    blocks[slot].samples[i] = adcBuffer[offset + i];

  blocks[slot].seq++;
  blocks[slot].ready = true;
}

extern "C" void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc)
{
  if (hadc->Instance == ADC1) captureBlock(0, 0);
}

extern "C" void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
  if (hadc->Instance == ADC1) captureBlock(1, ADC_HALF);
}

extern "C" void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc)
{
  if (hadc->Instance == ADC1) adcErrors++;
}

extern "C" void DMA2_Stream0_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_adc);
}

extern "C" void DMA2_Stream2_IRQHandler(void)
{
  if (__HAL_DMA_GET_FLAG(&hdma_uart, __HAL_DMA_GET_TC_FLAG_INDEX(&hdma_uart)))
    dmaCycles++;

  HAL_DMA_IRQHandler(&hdma_uart);
}

extern "C" void ADC_IRQHandler(void)
{
  HAL_ADC_IRQHandler(&hadc1);
}

bool UART_Init()
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
  if (HAL_DMA_Init(&hdma_uart) != HAL_OK) return false;

  __HAL_LINKDMA(&huart1, hdmarx, hdma_uart);

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK) return false;

  HAL_NVIC_SetPriority(DMA2_Stream2_IRQn, 1, 0);

  return startUartRx();
}

bool ADC_Init()
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
  if (HAL_DMA_Init(&hdma_adc) != HAL_OK) return false;

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
  if (HAL_ADC_Init(&hadc1) != HAL_OK) return false;

  ADC_ChannelConfTypeDef channel = {};
  channel.Channel = ADC_CHANNEL_0;
  channel.Rank = 1;
  channel.SamplingTime = ADC_SAMPLETIME_480CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &channel) != HAL_OK) return false;

  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
  HAL_NVIC_SetPriority(ADC_IRQn, 3, 0);
  HAL_NVIC_EnableIRQ(ADC_IRQn);

  return HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adcBuffer, ADC_SAMPLES) == HAL_OK;
}

bool processBlock(int slot)
{
  uint16_t s[ADC_HALF];
  uint32_t seq = 0, dropped = 0;

  __disable_irq();
  bool fresh = blocks[slot].ready;
  if (fresh)
  {
    for (int i = 0; i < ADC_HALF; i++) s[i] = blocks[slot].samples[i];
    seq = blocks[slot].seq;
    dropped = adcDropped;
    blocks[slot].ready = false;
  }
  __enable_irq();

  if (!fresh) return false;

  const char *name = (slot == 0) ? "FIRST" : "SECOND";
  uint32_t sum = 0;

  uartPrintf("\r\n%s HALF (block #%lu)\r\n", name, (unsigned long)seq);

  for (int i = 0; i < ADC_HALF; i++)
  {
    sum += s[i];
    uartPrintf("Sample %d: %u | %lu mV\r\n", slot * ADC_HALF + i + 1,
               (unsigned)s[i], (unsigned long)(s[i] * 3300UL / 4095UL));
  }

  uint16_t avg = sum / ADC_HALF;
  uartPrintf("%s HALF AVERAGE: %u | %lu mV\r\n", name,
             (unsigned)avg, (unsigned long)(avg * 3300UL / 4095UL));
  uartPrintf("ADC BLOCKS DROPPED: %lu\r\n", (unsigned long)dropped);
  uartPrintf("UART DMA HIGH-WATER: %u / %u\r\n",
             (unsigned)dmaHighWater, (unsigned)UART_DMA_SIZE);
  return true;
}

void reportFaults()
{
  static uint32_t last = 0;
  uint32_t total = ringOverflow + dmaLost + uartErrors + adcErrors + txErrors;

  if (total == last) return;
  last = total;

  uartPrintf("\r\n[FAULTS] ring overflow: %lu | dma lost: %lu | uart err: %lu | adc err: %lu | tx err: %lu\r\n",
             (unsigned long)ringOverflow, (unsigned long)dmaLost,
             (unsigned long)uartErrors, (unsigned long)adcErrors,
             (unsigned long)txErrors);
}

void processUART()
{
  static bool lastLineErr = false;
  bool lineErr = (huart1.Instance->SR &
                  (USART_SR_ORE | USART_SR_NE | USART_SR_FE)) != 0;
  if (lineErr && !lastLineErr) uartErrors++;
  lastLineErr = lineErr;

  __disable_irq();
  drainUart();
  __enable_irq();

  if (uartRestart)
  {
    uartRestart = false;
    HAL_UART_DMAStop(&huart1);
    if (!startUartRx())
    {
      uartPrintf("ERROR: UART RX RESTART FAILED\r\n");
      fatalError();
    }
  }

  uint8_t chunk[32];

  while (ringTail != ringHead)
  {
    uint16_t n = 0;

    while (n < sizeof(chunk) && ringTail != ringHead)
    {
      chunk[n++] = uartRing[ringTail];
      ringTail = (ringTail + 1) % UART_RING_SIZE;
    }

    if (HAL_UART_Transmit(&huart1, chunk, n, 100) != HAL_OK)
      txErrors++;
  }
}

void setup()
{
  if (!UART_Init()) fatalError();

  bool ok = true;
  ok &= uartPrintf("\r\nMODULE 6 FIRMWARE\r\n");
  ok &= uartPrintf("TASK 1: ADC DMA CIRCULAR BUFFER (PA0, 12-bit, 4+4 samples)\r\n");
  ok &= uartPrintf("TASK 2: UART RX DMA BUFFER (PA9 TX, PA10 RX, 115200 8N1)\r\n");
  if (!ok) fatalError();

  if (!ADC_Init())
  {
    uartPrintf("ERROR: ADC INIT FAILED\r\n");
    fatalError();
  }

  if (!uartPrintf("READY\r\n")) fatalError();
}

void loop()
{
  static uint32_t lastPrint = 0;
  static int nextSlot = 0;

  processUART();

  if (millis() - lastPrint >= 500)
  {
    lastPrint = millis();

    if (processBlock(nextSlot))
      nextSlot ^= 1;

    reportFaults();
  }
}
