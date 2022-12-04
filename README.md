# Croaster

#### Coraster is device for monitoring coffee roaster machine with [Croaster App](https://play.google.com/store/apps/details?id=com.iiemb.croaster) and can be connect with [Artisan Reoaster Scope](https://artisan-scope.org/)

[![Croaster](https://i.ibb.co/BtSCgP0/icon.png)](https://play.google.com/store/apps/details?id=com.iiemb.croaster)
[![Artisan](https://i.ibb.co/JxS5ZsD/artisan.png)](https://artisan-scope.org/) 

#### Build Your own Croaster now.

---

## Component

* [NodeMCU ESP8266](https://www.google.com/search?q=nodemcu+esp8266)
* [MAX6675](https://www.google.com/search?q=max6675) x2
* [Thermocoule](https://www.google.com/search?q=thermocouple+tipe+k) 
* [2x16 LCD Display with I2C Interface](https://www.google.com/search?q=2x16+LCD+Display+with+I2C+Interface)
* [DHT11](https://www.google.com/search?q=dht11) 
* [Jumper Cable](https://www.google.com/search?q=jumper+cable) - F to F

## App

* [Arduino IDE](https://www.arduino.cc/en/software/) or
* [Platform IO](https://platformio.org/)
* [NodeMCU PyFlasher](https://github.com/marcelstoer/nodemcu-pyflasher) if you want to flash .bin file (for windows or macOS), or if you are using using linux you can use [esptool](https://github.com/espressif/esptool) 
#

### [. BIN Files](https://github.com/IiemB/Croaster/releases/tag/Release)

#

## Libraries Dependencies

* [ESP 8266](https://github.com/esp8266/Arduino)
* [LiquidCrystal_I2C](https://github.com/marcoschwartz/LiquidCrystal_I2C.git)
* [MAX6675_Thermocouple](https://github.com/YuriiSalimov/MAX6675_Thermocouple)
* [WiFiManager](https://github.com/tzapu/WiFiManager.git)
* [Adafruit_BusIO](https://github.com/adafruit/Adafruit_BusIO)
* [Adafruit-GFX-Library](https://github.com/adafruit/Adafruit-GFX-Library)
* [Adafruit Unified Sensor](https://github.com/adafruit/Adafruit_Sensor)
* [WebSockets](https://github.com/Links2004/arduinoWebSockets)
* [DHT sensor library](https://github.com/adafruit/DHT-sensor-library)
#

## Wiring

####  2x16 LCD Display with I2C

| LCD Dispaly || NodeMCU |
| ------ |------| ------ |
| GND |<=>| GND |
| VCC |<=>| 3V |
| SCL |<=>| D1 |
| SDA |<=>| D2 |
#

####  DHT 11

| DHT 11 || NodeMCU |
| ------ |------| ------ |
| GND |<=>| GND |
| VCC |<=>| 3V |
| DATA |<=>| D4 |
#

####  MAX6675 #1 (for ET)

| MAX6675 || NodeMCU |
| ------ | ------ | ------ |
| GND |<=>| GND |
| VCC |<=>| 3V |
| SCK |<=>| D8 |
| SO |<=>| D7 |
| CS |<=>| D6 |
#

####  MAX6675 #2 (for BT)

| MAX6675 || NodeMCU |
| ------ | ------ | ------ |
| GND |<=>| GND |
| VCC |<=>| 3V |
| SCK |<=>| D8 |
| SO |<=>| D7 |
| CS |<=>| D5 |
#
