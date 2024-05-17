# SimpleDCC Arduino

License: Attribution-NonCommercial 4.0 International

For commercial use, please contact us at hey@loco.engineering with a description of your project and how you plan to use it.

## Why We Started Working on SimpleDCC

DCC (Digital Command Control) for model railways operates in a straightforward manner—a command station sends messages to DCC decoders over two wires/rails. Unlike most wireless systems, there are no encryption, authorization, or acknowledgment packets involved. However, updating the logic on DCC decoders remains challenging, as it cannot be done directly from a laptop or mobile phone without a command station. The entire logic of DCC decoders is predefined by the manufacturer, allowing you only to switch certain features or outputs on and off. 

For instance, if you want to evaluate a new level crossing signal before installing it on your layout, you need to program decoders and sometimes use additional proprietary modules just to get the signals working (e.g., the Viessmann 5057 manual). The main idea behind SimpleDCC is to simplify DCC by allowing you to program and control DCC decoders directly from a browser on a computer, mobile phone, or tablet. A SimpleDCC decoder connects to a laptop or mobile device using Wi-Fi, USB, or even Bluetooth, without a command station. You can add custom logic to any decoder or use our DCC templates. The project is open-source (a special commercial license is required for commercial usage, contact us for more information).

## How SimpleDCC Differs from Other DCC Projects/Decoders

The main differences are:
- You don't need to understand how DCC works because the decoder's logic can be set in the web app without using any additional tools for programming.
- You don't need to study lengthy manuals, as SimpleDCC performs actions (e.g., changing semaphore aspects, turning lights on/off) without strictly adhering to DCC protocols, which often have many unclear parts. You can see all messages received by a SimpleDCC decoder in your browser (with options to group and filter them by various values), and define what a decoder should do based on a received packet. From our experience, this greatly simplifies working with DCC.
- SimpleDCC decoders work with virtually all command stations and DCC protocols (mfx support on our roadmap), if you don't know the format of a DCC packet you still can assign actions to DCC packets. For example, Märklin Mobile Station sends accessory DCC packets in non-standard format and most accessory decoders don't understand them but with SimpleDCC you can assign actions to a DCC packet as is without understanding it format.

## Getting Started

1. Order a [Loco.Engineering](https://loco.engineering) decoder or build one yourself.
2. (Optional for Loco.Engineering decoders, but required for customized decoders) Open simpledcc-arduino.ino in Arduino IDE (we test with Arduino IDE 2.3.2) and upload firmware to the decoder (follow steps in How to build and upload)
3. Connect the decoder to wires or a track with DCC.
4. Open the Wi-Fi networks on your phone, tablet, or laptop and connect to a network named loco-xxxxx, where xxxxx is the serial number on your Loco.Engineering decoder or the wifi_network_name string in config.h if you are using customized hardware.
5. Open http://loco.local in your browser.
6. If your DCC command station is sending messages to the track/wires the decoder is connected to, you should see them in your browser. Use filters to hide messages you don't need.
7. Click "Add action" next to a DCC packet to add actions that the decoder should perform when that packet is received. For example, turn on red and green LEDs on a semaphore. The decoder has marked outputs where you connect wires from accessories/trains/vehicles, so you know that a red LED on a semaphore is connected to output number 5, etc.


## How to build and upload

- Install Arduino IDE (we test with Arduino IDE 2.3.2)
- Install libraries: AsyncTCP (tested with version 1.1.4), ESPAsyncTCP (1.2.4), ESPAsyncWebServer (tested with 3.1.0)
- Change configuration in config.h if required
- (Optional) If you change files in the simpledcc-arduino/data folder, you should recreate and upload SPIFFS to your board (for Arduino 2.2.1 - https://github.com/espx-cz/arduino-spiffs-upload?tab=readme-ov-file, Arduino 1.x.x - https://randomnerdtutorials.com/install-esp32-filesystem-uploader-arduino-ide/)
- Build and upload the firmware


## I don't see any logs

In case if you use Loco.Engineering decoder and don't see any logs in Arduino IDE, use the follow settings (Top Menu -> Tools -> ...)

- Select the ESP32-S3 DevKit board
- Select the USB port
- Enable CDC on Boot (IDE Board Menu)
- Select "Upload Mode" as "UART0 / Hardware CDC"
- Select "USB Mode" as "Hardware CDC and JTAG"
- Select "JTAG Adapter" as "Integrated USB JTAG"
- Select and load the Sketch that will be debugged
- Build and Upload it

## Contact Us

Feel free to contact us at hey@loco.engineering if you have any questions or feedback.
