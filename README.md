# Croaster

## Component
- [NodeMCU](https://www.google.com/search?hl=en&sxsrf=ALeKk034slgQQ-qStLjnv_7chJWDa-S5Gw%3A1612689502190&source=hp&ei=XrAfYNbhCNrbz7sPpMib4Aw&q=nodemcu&oq=&gs_lcp=CgZwc3ktYWIQAxgAMgcIIxDqAhAnMgcIIxDqAhAnMgcIIxDqAhAnMgcIIxDqAhAnMgcIIxDqAhAnMgcIIxDqAhAnMgcIIxDqAhAnMgcIIxDqAhAnMgcIIxDqAhAnMgcIIxDqAhAnUABYAGChHWgBcAB4AIABAIgBAJIBAJgBAKoBB2d3cy13aXqwAQo&sclient=psy-ab)
- [MAX6675](https://www.google.com/search?hl=en&sxsrf=ALeKk01VCkowocv1aZqhuJ0eXzh2awhIeQ%3A1612691825963&source=hp&ei=cbkfYNnCOMjSz7sP5rO0mAU&q=max6675&oq=max6675&gs_lcp=CgZwc3ktYWIQAzIECCMQJzIECCMQJzIECCMQJzIECAAQQzICCAAyAggAMgIIADICCAAyBQgAEMsBMgIIADoHCCMQ6gIQJzoHCAAQsQMQQzoICAAQsQMQgwE6CAguEMcBEKMCOggILhCxAxCDAToHCAAQFBCHAjoFCAAQkQI6BQgAELEDUN8OWOsgYIYjaAFwAHgAgAHBAogB5QWSAQU1LjMtMZgBAKABAaoBB2d3cy13aXqwAQo&sclient=psy-ab&ved=0ahUKEwiZv6qbwdfuAhVI6XMBHeYZDVMQ4dUDCAY&uact=5) x2
- [Thermocoule](https://www.google.com/search?hl=en&sxsrf=ALeKk00S7-Ax-oK2YpkQu3bOj4OHgYm28A%3A1612691851875&ei=i7kfYMHhNOrFz7sPztmxiAE&q=thermocouple&oq=thermocouple&gs_lcp=CgZwc3ktYWIQAzIECCMQJzIECCMQJzIECCMQJzIECAAQQzIECAAQQzIECAAQQzIECAAQQzIECAAQQzIECAAQQzIECAAQQzoHCCMQsAMQJzoHCAAQRxCwAzoQCC4QxwEQowIQsAMQyAMQQ0oFCDgSATFQvBVYvBVglRxoAXACeACAAYwDiAH7A5IBBzAuMS4wLjGYAQCgAQGqAQdnd3Mtd2l6yAELwAEB&sclient=psy-ab&ved=0ahUKEwiB09mnwdfuAhXq4nMBHc5sDBEQ4dUDCAw&uact=5) (I use type K thermocouple) 
- [Oled LCD SS1306](https://www.google.com/search?hl=en&sxsrf=ALeKk03RkFFDBFM4KPNI0_dUTtMN7Ot6qw%3A1612692050424&ei=UrofYMitGYG_8QO9t524Ag&q=ssd1306&oq=ssd1306&gs_lcp=CgZwc3ktYWIQAzIECCMQJzIECCMQJzIECCMQJzIFCAAQkQIyBQgAEJECMgIIADICCAAyAggAMgIIADIFCAAQywE6BwgjELADECc6BwgAEEcQsAM6BwgAELADEENQ4TxY2D5gz0VoAXACeACAAWmIAf0BkgEDMi4xmAEAoAEBqgEHZ3dzLXdpesgBCsABAQ&sclient=psy-ab&ved=0ahUKEwjInrCGwtfuAhWBX3wKHb1bBycQ4dUDCAw&uact=5) I2C
- Some wire
## App
- [Arduino IDE](https://www.arduino.cc/en/software/)
#
## Library Dependencies
- [Adafruit_BusIO](https://github.com/adafruit/Adafruit_BusIO)
- [Adafruit-GFX-Library](https://github.com/adafruit/Adafruit-GFX-Library)
- [Adafruit_SSD1306](https://github.com/adafruit/Adafruit_SSD1306)
- [WebSockets](https://github.com/Links2004/arduinoWebSockets)
- [MAX6675_Thermocouple](https://github.com/YuriiSalimov/MAX6675_Thermocouple)
- [WiFiManager](https://github.com/tzapu/WiFiManager.git)
#
## Wiring
####  Oled LCD SSD1306
| SSD1306 || NodeMCU |
| ------ |------| ------ |
| GND |<->| GND |
| VCC |<->| 3V |
| SCL |<->| D1 |
| SDA |<->| D2 |
#
####  MAX6675 #1 (for BT)
| MAX6675 || NodeMCU |
| ------ | ------ | ------ |
| GND |<->| GND |
| VCC |<->| 3V |
| SCK |<->| D8 |
| CS |<->| D6 |
| SO |<->| D7 |
#
####  MAX6675 #2 (for ET)
| MAX6675 || NodeMCU |
| ------ | ------ | ------ |
| GND |<->| GND |
| VCC |<->| 3V |
| SCK |<->| D8 |
| CS |<->| D5 |
| SO |<->| D7 |
#
