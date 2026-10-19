# Smart Greenhouse & Water Tank Manager

CSE331L final lab project (Group 3, Summer 2025). An STM32F103C8T6 system that monitors a small greenhouse and its water tank, and waters the plants automatically.

- **Senses** air temperature and humidity (DHT11), soil moisture (analog sensor → ADC) and the tank's water level (HC-SR04 ultrasonic).
- **Shows** the readings on a 16×2 I²C LCD, swapping pages every 8 s, with an RGB status LED.
- **Acts** by driving an SG90 servo as the irrigation valve when the soil is critically dry or the tank level is out of range.

![Wiring / pin diagram](images/pin-diagram.png)

## Pin map (final firmware, `Greenhouse_MS`)

| Device | Signal | STM32 pin |
|--------|--------|-----------|
| DHT11 | DATA | PB9 (bit-banged, TIM1 1 µs time base) |
| HC-SR04 | TRIG / ECHO | PA9 / PA8 |
| Soil-moisture sensor | AO / VCC control | PB1 (ADC1_IN9) / PA7 |
| RGB status LED | Red / Green / Blue | PA5 / PA3 / PA0 |
| SG90 servo | PWM, 50 Hz | PA1 (TIM2_CH2) |
| 16×2 LCD (PCF8574 @ 0x27) | SCL / SDA | PB6 / PB7 (I2C1, 100 kHz) |

Clock: 8 MHz HSE × 9 PLL = 72 MHz.

## How it works

- **Soil moisture** (raw 12-bit ADC, averaged): > 3100 is dry, > 2600 is medium, < 2100 is wet. The LED shows red, green or blue respectively.
- **Tank level**: a distance of 5–20 cm from the sensor counts as OK (median-filtered ultrasonic readings).
- **Alarm**: soil critically dry (> 3200, with hysteresis) *or* tank out of range. The alarm has to be detected 3 times in a row before it triggers; then the servo valve opens and the red LED blinks.
- **Safety**: a start-up health check of every device, per-sensor error counters, and **maintenance mode** if an alarm lasts more than 5 minutes.
- **LCD pages**: temperature and humidity, soil and tank status, alarm statistics, min/max data log, error counts.

## Firmware versions

All are STM32CubeMX + Keil µVision projects (see [stm32-labs](../stm32-labs#building-a-project) for how to build one).

| Version | Date | Display | Notes |
|---------|------|---------|-------|
| [`Smart_Greenhouse_Manager`](firmware/Smart_Greenhouse_Manager) | 2025-08-26 | LCD | First integrated build: intro screens, temp/RH + tank % pages |
| [`Smart_Greenhouse_and_Water_Tank_Manager`](firmware/Smart_Greenhouse_and_Water_Tank_Manager) | 2025-08-27 | LCD | Draft of the enhanced firmware, kept as `main.c` + `.ioc` only. Run *Generate Code* in CubeMX to create the remaining project files |
| [`sensor-integration-test`](firmware/sensor-integration-test) | 2025-08-27 | LCD | Single `main.c` used to bring up all the sensors and actuators together |
| [`Greenhouse_project`](firmware/Greenhouse_project) | 2025-09-01 | LCD | Enhanced firmware: filtering, alarms, maintenance mode |
| [`Auto Water Plant`](firmware/Auto%20Water%20Plant) | 2025-09-01 | **OLED** | The enhanced firmware ported to the SSD1306 OLED |
| [`Greenhouse_MS`](firmware/Greenhouse_MS) | 2025-09-02 | LCD | ✅ **Final submitted version** |

## Documents

| File | Contents |
|------|----------|
| [Proposal-Smart-Parking-System.pdf](docs/Proposal-Smart-Parking-System.pdf) ([.docx](docs/Proposal-Smart-Parking-System.docx)) | The group's original proposal (Smart Parking System), later replaced by the greenhouse idea |
| [Project-Overview-Smart-Greenhouse.pdf](docs/Project-Overview-Smart-Greenhouse.pdf) | Revised project overview and required sensors |
| [Core-Sensors-and-Software.docx](docs/Core-Sensors-and-Software.docx) | Component list and LCD screen plan |
| [Greenhouse-System-Report.pdf](docs/Greenhouse-System-Report.pdf) | Final project report |
| [Greenhouse-System-Report-Enhanced.docx](docs/Greenhouse-System-Report-Enhanced.docx) | Extended report draft |

## Team: Group 3

- MD. Rokib Hasan Oli
- Muhaiminul Islam
- Md Mubtasim Fuad
- Mohammad Muhibul Islam
