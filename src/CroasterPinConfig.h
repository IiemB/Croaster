#pragma once
#include <Arduino.h>

/**
 * @struct CroasterPinConfig
 * @brief Pin assignments for the thermocouple (MAX6675) sensors.
 *
 * The firmware core is decoupled from any specific wiring: a consuming project
 * targets a different board simply by passing its own CroasterPinConfig to the
 * CroasterCore constructor. `defaults()` returns sensible assignments for the
 * reference ESP8266 (NodeMCU) and ESP32-C3 Super Mini boards.
 */
struct CroasterPinConfig
{
    int8_t sckPin;  ///< MAX6675 SPI clock (SCK)
    int8_t soPin;   ///< MAX6675 SPI MISO (SO)
    int8_t csPinBt; ///< MAX6675 chip-select for the bean (BT) thermocouple
    int8_t csPinEt; ///< MAX6675 chip-select for the environment (ET) thermocouple

    /**
     * @brief Reference defaults for the ESP8266 / ESP32-C3 boards.
     * @return A CroasterPinConfig with the reference board pin layout.
     */
    static CroasterPinConfig defaults();
};
