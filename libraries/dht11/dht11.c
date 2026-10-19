#include "dht11.h"
#include "dwt_delay.h"

static GPIO_TypeDef *DHT_PORT;
static uint16_t DHT_PIN;

static void DHT_SetOutput(void) {
  GPIO_InitTypeDef g = {0};
  g.Pin = DHT_PIN;
  g.Mode = GPIO_MODE_OUTPUT_PP;
  g.Pull = GPIO_NOPULL;
  g.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(DHT_PORT, &g);
}

static void DHT_SetInput(void) {
  GPIO_InitTypeDef g = {0};
  g.Pin = DHT_PIN;
  g.Mode = GPIO_MODE_INPUT;
  g.Pull = GPIO_NOPULL; // external pull-up on module
  HAL_GPIO_Init(DHT_PORT, &g);
}

void DHT11_Init(GPIO_TypeDef *port, uint16_t pin) {
  DHT_PORT = port; DHT_PIN = pin;
  DWT_Delay_Init();
}

static uint8_t DHT_ReadBit(void) {
  // Each bit: ~50us LOW, then HIGH: ~26-28us=0, ~70us=1
  // Wait for 50us low to end
  uint32_t timeout = 100;
  while (!HAL_GPIO_ReadPin(DHT_PORT, DHT_PIN)) { if(--timeout==0) return 0; }
  // Start of HIGH, measure duration
  DWT_Delay_us(40); // sample at ~40us
  return HAL_GPIO_ReadPin(DHT_PORT, DHT_PIN) ? 1 : 0;
}

static uint8_t DHT_ReadByte(void) {
  uint8_t v=0;
  for (int i=0;i<8;i++){
    // Wait for low start
    uint32_t t=200;
    while (HAL_GPIO_ReadPin(DHT_PORT, DHT_PIN)) { if(--t==0) break; }
    // Now wait for rising edge
    t=200;
    while (!HAL_GPIO_ReadPin(DHT_PORT, DHT_PIN)) { if(--t==0) break; }
    v <<= 1;
    v |= DHT_ReadBit();
    // Wait for line to go low again before next bit (guard)
    t=200;
    while (HAL_GPIO_ReadPin(DHT_PORT, DHT_PIN)) { if(--t==0) break; }
  }
  return v;
}

HAL_StatusTypeDef DHT11_Read(DHT11_Data *out) {
  uint8_t b0,b1,b2,b3,chk;

  // Start signal
  DHT_SetOutput();
  HAL_GPIO_WritePin(DHT_PORT, DHT_PIN, GPIO_PIN_RESET);
  HAL_Delay(18);                 // >18ms
  HAL_GPIO_WritePin(DHT_PORT, DHT_PIN, GPIO_PIN_SET);
  DWT_Delay_us(30);              // 20–40us
  DHT_SetInput();

  // Response: ~80us low + ~80us high
  uint32_t t=200;
  while (HAL_GPIO_ReadPin(DHT_PORT, DHT_PIN)) { if(--t==0) return HAL_TIMEOUT; }
  t=200;
  while (!HAL_GPIO_ReadPin(DHT_PORT, DHT_PIN)) { if(--t==0) return HAL_TIMEOUT; }
  t=200;
  while (HAL_GPIO_ReadPin(DHT_PORT, DHT_PIN)) { if(--t==0) return HAL_TIMEOUT; }

  // Read 5 bytes
  b0 = DHT_ReadByte();
  b1 = DHT_ReadByte();
  b2 = DHT_ReadByte();
  b3 = DHT_ReadByte();
  chk = DHT_ReadByte();

  if (((uint8_t)(b0 + b1 + b2 + b3)) != chk) return HAL_ERROR;

  out->humidity_int = b0;  out->humidity_dec = b1;
  out->temp_int     = b2;  out->temp_dec     = b3;
  return HAL_OK;
}
