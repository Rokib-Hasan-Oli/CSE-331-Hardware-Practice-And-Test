# Reusable Peripheral Drivers

Standalone copies of the driver files used across the labs, for the STM32F1 HAL. To use one, copy the `.c` files into `Core/Src` and the `.h` files into `Core/Inc` of a CubeMX project.

| Folder | Device | Notes |
|--------|--------|-------|
| [`lcd-i2c-hd44780`](lcd-i2c-hd44780) | 16×2 HD44780 LCD with a PCF8574 I²C backpack | `HD44780_Init(rows)`, `HD44780_SetCursor(col,row)`, `HD44780_PrintStr()`. Uses `hi2c1`; the I²C address is set in `liquidcrystal_i2c.h` (usually `0x27`) |
| [`oled-ssd1306`](oled-ssd1306) | 0.96″ 128×64 SSD1306 OLED (I²C) | `SSD1306_Init()`, `SSD1306_GotoXY()`, `SSD1306_Puts(text, &Font_11x18, 1)`, `SSD1306_UpdateScreen()`; fonts in `fonts.c` |
| [`dht11`](dht11) | DHT11 temperature & humidity sensor | `DHT11_Init(port, pin)`, `DHT11_Read(&data)`; needs `dwt_delay.c` for microsecond timing (DWT cycle counter) |
