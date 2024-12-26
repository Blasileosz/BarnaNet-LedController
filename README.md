# BarnaNet - LedController

## TODO
- [ ] Alarms: https://docs.espressif.com/projects/esp-idf/en/v4.3/esp32/api-reference/peripherals/timer.html#functional-overview
- [ ] NVS: https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/storage/nvs_flash.html#application-example
- [ ] Test sunrise
- [x] Test TCP task
- [ ] Test LED renderers
- [ ] Export the TCP, time, colorUtil, (alarm) and wifi files to a libs folder that is a github submodule https://github.blog/open-source/git/working-with-submodules/
- [ ] Azure IoT https://github.com/espressif/esp-azure/tree/master is deprecated, see my Github stars
- [ ] Update Readme
- [ ] Format everything
	- [ ] Change formating of structs to not use typedef and remove _t for clarity

## BarnaNet Protocol
Actual definition is in [B_lightCommandStruct.h](/include/B_lightCommandStruct.h)
- Header
	- First 6 bits of command ID
	- After that, 2 bits of strip ID (still unused)
- Color change data
	- First 3 bytes are RGB values
	- Bytes 3-4 is treated as a uint16_t for the transition time

## Circuit
- [ESP32 Wroom CH340](https://www.emag.hu/esp32-wroom-ch340-dual-core-nodemcu-1-2-18/pd/D0Q9R6MBM/)
- 3 [10K resistor](https://www.conrad.hu/p/femreteg-ellenallas-06-w-01-10k-423483)
- Used to have 3 [BS170 mosfet](https://www.conrad.hu/hu/p/on-semiconductor-bs170-mosfet-1-n-csatornas-350-mw-to-92-158950) - [datasheet](https://asset.conrad.com/media10/add/160267/c1/-/en/000158950DS01/adatlap-158950-on-semiconductor-bs170-mosfet-1-n-csatornas-350-mw-to-92.pdf) (Max drain current is 0.5 Amps, thus unable to handle a whole led strip)
- 6 [IRFZ44N mosfet](https://www.microcontroller.hu/termek/irfz44n-n-csatornas-mosfet-tranzisztor/) - [datashet](https://www.infineon.com/dgdl/irfz44npbf.pdf?fileId=5546d462533600a40153563b3a9f220d)
- Alternatively 3 [IRL3705ZPBF mosfet](https://www.conrad.hu/hu/p/tranzisztor-unipolaris-mosfet-international-rectifier-irl3705zpbf-n-csatornas-to-220-ab-i-d-a-86-a-u-ds-55-v-161143) - [datasheet](https://asset.conrad.com/media10/add/160267/c1/-/en/000161143DS01/adatlap-161143-tranzisztor-unipolaris-mosfet-international-rectifier-irl3705zpbf-n-csatornas-to-220-ab-i-d-a-86-a-uds-55-v.pdf)

## TCP Server
- TCP server does not currently bind to IPv6 (clients are still supported through IPv6)
- Only accepts 1 client at a time

## NTP client
- [Documentation for this section](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/system/system_time.html)
- The [esp_netif_sntp.h](https://github.com/espressif/esp-idf/blob/f404fe96b17692e3f1de536a3d73a180cdb53b42/components/esp_netif/include/esp_netif_sntp.h) header and [esp_netif_sntp.c](https://github.com/espressif/esp-idf/blob/f404fe96b17692e3f1de536a3d73a180cdb53b42/components/esp_netif/lwip/esp_netif_sntp.c) definition files had to be downloaded from the github page to (.platformio\packages\framework-espidf\components\esp_netif\include) and (.platformio\packages\framework-espidf\components\esp_netif\lwip) respectively

## LED Controller
- Using the LEDC library to set up PWM chanels for each pin
- The pins controlling the LEDs are defined in the [B_SECRET.h](/include/B_SECRET.h) file
- The LED update frequecy in defined in the [B_ledController.h](/include/B_ledController.h) file
- The LED controller receives the commands from the TCP server using a queue, defined in the main function
