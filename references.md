## Add esp32c3 super mini to platformio

https://github.com/sigmdel/supermini_esp32c3_sketches.git

---

## Setup custom partitions on Arduino IDE 2.X

* **Copy** `custom32c3sm.csv` to `/Users/USER/Library/Arduino15/packages/esp32/hardware/esp32/3.x.x/tools/partitions/`

* Add to `/Users/USER/Library/Arduino15/packages/esp32/hardware/esp32/3.x.x/board.txt` in `MakerGO ESP32 C3 SuperMini` section

	- ```makergo_c3_supermini.menu.PartitionScheme.custom32c3sm=Custom SuperMini```
	- ```makergo_c3_supermini.menu.PartitionScheme.custom32c3sm.build.partitions=custom32c3sm```
	- ```makergo_c3_supermini.menu.PartitionScheme.custom32c3sm.upload.maximum_size=1900544```

* Delete the "**User data**" folder:
	**Windows:**
    -  ```C:\Users\<user name>\AppData\Roaming\arduino-ide\```

	**Linux:**
    - ```~/.config/arduino-ide/```
    
	**macOS:**
    - ```~/Library/Application Support/arduino-ide/```
            
* Restart the Arduino IDE.
