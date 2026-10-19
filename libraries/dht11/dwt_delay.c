#include "dwt_delay.h"

void DWT_Delay_Init(void) {
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
  DWT->CYCCNT = 0;
  __ASM volatile ("NOP"); __ASM volatile ("NOP"); __ASM volatile ("NOP");
}

void DWT_Delay_us(uint32_t us) {
  uint32_t cycles = (SystemCoreClock/1000000U) * us;
  uint32_t start = DWT->CYCCNT;
  while ((DWT->CYCCNT - start) < cycles) { /* spin */ }
}
