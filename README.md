# BarnaNet - LedController

## TODO
- [ ] Combine test scripts and make it interactive
- [ ] Cleanup

## Developement in docker
- [Documentation](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-guides/tools/idf-docker-image.html)
- [Required drivers](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/establish-serial-connection.html#connect-esp32-to-pc)
	- USB to UART bridge [CP210x Windows Drivers](https://www.silabs.com/developer-tools/usb-to-uart-bridge-vcp-drivers?tab=downloads)
- The version is controlled by the Dockerfile
	- There are [docs](https://docs.espressif.com/projects/esp-idf/en/v5.5-beta1/esp32/versions.html#updating-to-stable-release) on how to upgrade manualy
- Git needs to be configured to auto convert to CRLF (otherwise it would mess up the commits): `git config --global core.autocrlf true`

## Developement in GitHub Codespaces
- Does not handle submodules well
	- From a submodule, you can only push to the repo that instantiated it. This means you cannot push changes in a submodule
	- To combat this, the `devcontainer.json` file needs to define the permissions for the codespace. If you have already created the codespace, you need to recreate it after changing these permissions.
	- However, if you have made changes in the submodule, you cannot just recreate the codespace, as it would lose your changes. There is a way to combat this:
		1. Create a Personal Access Token (PAT) with permissions to read-write both repositories contents
		2. In the codespace, pipe the token into a file
		3. Authenticate to github with this token: `gh auth login --with-token < ~/path/to/tokenfile`
			- This may fail with: `The value of the GITHUB_TOKEN environment variable is being used for authentication.`. In this case unset the variable: `unset GITHUB_TOKEN`
		4. Check if the authentication was successful: `gh auth status`
		5. Tell git to use the gh cli for authentication: `gh auth setup-git`, if thats not enough overwrite the default credential helper: `git config --global credential.helper "!gh auth git-credential"`
		6. Delete the temp file containing the token
		7. Push the changes and recreate the codespace from scratch to apply the new permissions
- Issues I haven't solved yet:
	- Code highlighting and code completion doesn't work in codespaces for some reason
	- VS Code flags the repos as being unsafe directories on first boot

### Flasing and monitoring
Windows WSL does not support USB passthrough. To get around this, a [remote serial port](https://docs.espressif.com/projects/esptool/en/latest/esp32/remote-serial-ports.html#pyserial-example-servers) needs to be created on the host machine.
1. Download the [esptool](https://github.com/espressif/esptool) utility from either [github](https://github.com/espressif/esptool/releases) or [pip](https://pypi.org/project/esptool/)
2. Run the RFC2217 server on the host machine: `esp_rfc2217_server -v -p 4000 COM4`
3. Flash or monitor the device `idf.py --port rfc2217://host.docker.internal:4000?ign_set_control flash`

## ESP
- [Pinout refference](https://randomnerdtutorials.com/esp32-pinout-reference-gpios/)
- Configure config by: ```idf.py menuconfig```

## Circuit
- [ESP32 Wroom CH340](https://www.emag.hu/esp32-wroom-ch340-dual-core-nodemcu-1-2-18/pd/D0Q9R6MBM/)
- 3x [10K resistor](https://www.conrad.hu/p/femreteg-ellenallas-06-w-01-10k-423483)
- Used to have 3 [BS170 mosfet](https://www.conrad.hu/hu/p/on-semiconductor-bs170-mosfet-1-n-csatornas-350-mw-to-92-158950) - [datasheet](https://asset.conrad.com/media10/add/160267/c1/-/en/000158950DS01/adatlap-158950-on-semiconductor-bs170-mosfet-1-n-csatornas-350-mw-to-92.pdf) (Max drain current is 0.5 Amps, thus unable to handle a whole led strip)
- 6x [IRFZ44N mosfet](https://www.microcontroller.hu/termek/irfz44n-n-csatornas-mosfet-tranzisztor/) - [datashet](https://www.infineon.com/dgdl/irfz44npbf.pdf?fileId=5546d462533600a40153563b3a9f220d)
- Alternatively 3x [IRL3705ZPBF mosfet](https://www.conrad.hu/hu/p/tranzisztor-unipolaris-mosfet-international-rectifier-irl3705zpbf-n-csatornas-to-220-ab-i-d-a-86-a-u-ds-55-v-161143) - [datasheet](https://asset.conrad.com/media10/add/160267/c1/-/en/000161143DS01/adatlap-161143-tranzisztor-unipolaris-mosfet-international-rectifier-irl3705zpbf-n-csatornas-to-220-ab-i-d-a-86-a-uds-55-v.pdf)

## Project structure
- The project uses the [BarnaNetLibrary](https://github.com/Blasileosz/BarnaNetLibrary) as an ESP-IDF component
- This component is set up as a git submodule
- The task definitions and command handling abide by the library's rules

## BarnaNet Protocol
- For the definition, see the readme in the BarnaNetLibrary
- For the API endpoints, see [Protocol.md](/Protocol.md)

## LED Controller
For definition, see [B_ledController.h](/main/B_ledController.h)
- Task function: `B_LedControllerTask`
- For the task parameter, the given `B_LedControllerTaskParameter` struct should be filled
- Using the LEDC library to set up PWM channels for each pin
- The pins controlling the LEDs are defined in the menuconfig
- The LED update frequency in defined in the [B_ledController.h](/main/B_ledController.h) file
- The LED controller receives commands through queues, using the address map system
- The task saves its state to the NVS flash and loads it back when started
