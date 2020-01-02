## ArduinoSwitchTAS

This project should emulate a controller with the Arduino and play TAS on the Nintendo Switch.

#### How to use

Besides the Arduino you also need an UART to USB-Bridge. [This one](https://www.amazon.de/USB-TTL-Konverter-Modul-mit-eingebautem-CP2102/dp/B00AFRXKFU/) from Amazon works for me.

You only need to connect TX to TX and RX to RX. Normally, you connect TX to RX, but we are using the ATmega16u2 in the Arduino, which inverses TX and RX.

After connecting the bridge to your PC, you need to check, which COM-Port it uses. Go into your "Device Manager" and check below "COM & LPT" for "Silicon Labs CP210x USB to UART Bridge (COM?)" (for my device, linked above). That ? after COM is your needed COM port. Insert it into the "portName" in the Main.java of "ArduinoTAS".

In case you see issues with controller conflicts while in docked mode, try using a USB-C to USB-A adapter in handheld mode. In dock mode, changes in the HDMI connection will briefly make the Switch not respond to incoming USB commands, skipping parts of the sequence. These changes may include turning off the TV, or switching the HDMI input. (Switching to the internal tuner will be OK, if this doesn't trigger a change in the HDMI input.)

This repository has been tested using an Arduino Uno.

#### Compiling this Project

First of all, you need a Linux VM (for example in [VirtualBox](https://www.virtualbox.org/), tested with Ubuntu). Here you need to install the [ArduinoIDE](https://www.arduino.cc/download_handler.php?f=/arduino-1.8.10-linux64.tar.xz) and make. Next, you edit the makefile and insert your installation dir at ARDUINO_PATH (keep the additions at the end to let it point to the correct dir).

After every restart of the Linux VM you need to extend the $PATH-Variable by running the following command: `sudo export $PATH=(your ArduinoIDE-Installation-dir)/hardware/tools/avr/bin/:$PATH` (also look if it points to an existing directory).

Now you should be ready to rock. Open a terminal window in the `Arduino`-subdirectory, type `make`, and hit enter to compile. If all goes well, the printout in the terminal will let you know it finished the build! Follow the directions on flashing `Joystick.hex` onto your Arduino, which can be found below.

#### Flashing it onto the Arduino Uno

You need the program called `Flip` on your Windows PC to flash the compiled `Joystick.hex` file onto your Arduino. You can download it (here)[https://www.microchip.com/developmenttools/ProductDetails/flip].

Before flashing, you always need to disconnect your TX and RX-Pins.

First you need to put your Arduino in DFU-Mode while it's connected to your PC. To do that, follow the image below:

[![Short the pins in the red circle](http://1.bp.blogspot.com/-jMM85F4r6ww/T8u-74eiNkI/AAAAAAAAGZ0/EWa0TdA26A4/s1600/DFU_mode_pins.png)](https://forum.arduino.cc/index.php?topic=380103.0)

If it's the first time for your PC to be connected to an Arduino in DFU-Mode, you need to follow (these)[https://youtu.be/fSXZMVdO5Sg?t=322] instructions to connect the Arduino to FLIP.

In Flip, first select `ATmega16u2` in Device -> Select and press OK. After that, go into File -> Load HEX File and search for `Joystick.hex` you compiled earlier. Then press on the USB-cable in the top menu and select USB, then press Open. Now you are ready to flash!

Select Run in the bottom left corner to flash the .hex file onto the Arduino. When done, disconnect the Arduino from your computer and reconnect the TX & RX-Pins to the bridge.

#### Run

To start a TAS file, first start the ArduinoTAS-Project from Eclipse and select the TAS file you want to run. Make sure your `portName` in `Main.java` is correct and the pins of TX & RX on your Arduino are connected correctly. Then connect your Arduino via USB to the Switch and see the magic happen!

#### Thanks

Thanks to Shiny Quagsire for his [Splatoon post printer](https://github.com/shinyquagsire23/Switch-Fightstick) and progmem for his [original discovery](https://github.com/progmem/Switch-Fightstick).

Thanks to [exsilium](https://github.com/bertrandom/snowball-thrower/pull/1) for improving the command structure, optimizing the waiting times, and handling the failure scenarios. It can now run indefinitely!