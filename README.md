# BarnaNet - LedController

## TODO
- [ ] Test sunrise
- [x] Test TCP task
- [ ] Test LED renderers
- [ ] Update Readme
- [ ] Format everything

## ESP
- [Pinout refference](https://randomnerdtutorials.com/esp32-pinout-reference-gpios/)
- Configure config by: ```idf.py menuconfig```

## Circuit
- [ESP32 Wroom CH340](https://www.emag.hu/esp32-wroom-ch340-dual-core-nodemcu-1-2-18/pd/D0Q9R6MBM/)
- 3 [10K resistor](https://www.conrad.hu/p/femreteg-ellenallas-06-w-01-10k-423483)
- Used to have 3 [BS170 mosfet](https://www.conrad.hu/hu/p/on-semiconductor-bs170-mosfet-1-n-csatornas-350-mw-to-92-158950) - [datasheet](https://asset.conrad.com/media10/add/160267/c1/-/en/000158950DS01/adatlap-158950-on-semiconductor-bs170-mosfet-1-n-csatornas-350-mw-to-92.pdf) (Max drain current is 0.5 Amps, thus unable to handle a whole led strip)
- 6 [IRFZ44N mosfet](https://www.microcontroller.hu/termek/irfz44n-n-csatornas-mosfet-tranzisztor/) - [datashet](https://www.infineon.com/dgdl/irfz44npbf.pdf?fileId=5546d462533600a40153563b3a9f220d)
- Alternatively 3 [IRL3705ZPBF mosfet](https://www.conrad.hu/hu/p/tranzisztor-unipolaris-mosfet-international-rectifier-irl3705zpbf-n-csatornas-to-220-ab-i-d-a-86-a-u-ds-55-v-161143) - [datasheet](https://asset.conrad.com/media10/add/160267/c1/-/en/000161143DS01/adatlap-161143-tranzisztor-unipolaris-mosfet-international-rectifier-irl3705zpbf-n-csatornas-to-220-ab-i-d-a-86-a-uds-55-v.pdf)

## LED Controller
- Using the LEDC library to set up PWM chanels for each pin
- The pins controlling the LEDs are defined in the config
- The LED update frequecy in defined in the [B_ledController.h](/main/B_ledController.h) file
- The LED controller receives the commands from the TCP server using a queue, defined in the main function
