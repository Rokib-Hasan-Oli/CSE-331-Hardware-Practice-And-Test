#ifndef DHT11_H
#define DHT11_H
#include "stm32f1xx_hal.h"
typedef struct {
  uint8_t humidity_int, humidity_dec;
  uint8_t temp_int, temp_dec;
} DHT11_Data;

void DHT11_Init(GPIO_TypeDef *port, uint16_t pin);
HAL_StatusTypeDef DHT11_Read(DHT11_Data *out);

#endif
