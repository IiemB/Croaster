#pragma once
#include <Arduino.h>

/**
 * @struct CroasterPinConfig
 * @brief Pin assignments for the thermocouple (MAX6675) sensors.
 *
 * The firmware core is decoupled from any specific wiring: every implementation
 * provides its own CroasterPinConfig (in its config.h) and passes it to the
 * CroasterCore constructor. There is no library default — pins are always
 * explicit.
 */
struct CroasterPinConfig
{
    int8_t sckPin;  ///< MAX6675 SPI clock (SCK)
    int8_t soPin;   ///< MAX6675 SPI MISO (SO)
    int8_t csPinBt; ///< MAX6675 chip-select for the bean (BT) thermocouple
    int8_t csPinEt; ///< MAX6675 chip-select for the environment (ET) thermocouple
};
