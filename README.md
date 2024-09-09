#### Note: The project is in active development

# DCC (Digital Command Control) / WCC (Wireless Command Control) decoders based on Arduino and ESP32S3 SOCs

License: Attribution-NonCommercial 4.0 International

For commercial use, please contact us at hey@loco.engineering with a description of your project and how you intend to use it.

**If you like this project and its concept, please spread the word and don't forget to give us a star on GitHub. Your support helps us grow!**

## Why We Started Working on SimpleDCC

DCC (Digital Command Control) for model railways operates on a basic principle—a command station sends messages to DCC decoders via two wires or rails. Unlike most wireless systems, it doesn't involve encryption, authorization, or acknowledgment packets. However, updating the logic on DCC decoders remains a challenge, as it can't be done directly from a laptop or mobile phone without a command station. DCC decoder logic is typically predefined by the manufacturer, only allowing users to toggle certain features or outputs.

For example, if you want to test a new level crossing signal before installing it on your layout, you need to program decoders and sometimes use additional proprietary modules just to get the signals working (as seen in the Viessmann 5057 manual). The core concept behind SimpleDCC is to streamline DCC by enabling direct programming and control of DCC decoders from a browser on a computer, mobile phone, or tablet. A SimpleDCC decoder connects to a laptop or mobile device via Wi-Fi, USB, or even Bluetooth, eliminating the need for a command station. You can add custom logic to any decoder or use our DCC templates. The project is open-source (though a special commercial license is required for commercial use—contact us for more information).

## What is WCC (Wireless Command Control)?

WCC (Wireless Command Control) is an open wireless system for operating model railways, LEGO® trains, RC cars, and other models or toys. WCC can run on DC, AC (including tracks with DCC), or battery power and requires only one essential hardware component:

- A decoder (for trains, cars, or accessories) to control trains, cars, and layout elements (such as signals and switches)

Additionally, unpowered NFC stickers can be used optionally to provide more interaction possibilities.

WCC consists of:
- An open protocol (defining how trains, cars, and layout elements communicate)
- An open API for updating logic via BLE, Wi-Fi, and USB
- Code examples demonstrating how to use various WCC features in your own projects

The main difference between DCC and WCC lies in how messages or packets are transmitted. WCC messages are sent wirelessly, eliminating the need for wires, while DCC packets/messages are delivered through tracks or wires. Furthermore, WCC enables two-way communication between decoders, allowing trains, cars, and accessories to send messages directly to each other. Unlike DCC, WCC doesn't require a command station at all.

For more detailed information about WCC, please visit [https://loco.engineering/docs](https://loco.engineering/docs/)


## How SimpleDCC/WCC Decoders Differs from Other DCC Projects/Decoders

The main differences are:

- You don't need to understand DCC's inner workings because the decoder's logic can be set in the web app without additional programming tools.

- There's no need to study lengthy manuals. SimpleDCC performs actions (e.g., changing semaphore aspects, turning lights on/off) without strictly adhering to often ambiguous DCC protocols. You can view all messages received by a SimpleDCC decoder in your browser (with options to group and filter them), and define the decoder's response to received packets. In our experience, this greatly simplifies working with DCC.

- SimpleDCC decoders are compatible with virtually all command stations and DCC protocols (with mfx support on our roadmap). Even if you don't understand a DCC packet's format, you can still assign actions to it. For instance, Märklin Mobile Station sends accessory DCC packets in a non-standard format that most accessory decoders don't understand, but with SimpleDCC, you can assign actions to these packets as-is.

- SimpleDCC/WCC decoders can control trains, vehicles, and layout accessories like signals and turnouts without needing to know the specific types connected. With a traditional decoder, changing a signal aspect requires finding the correct command to send from the command station. With SimpleDCC/WCC, the user defines what action should occur when a specific DCC packet is received. You're not limited to predefined signal aspects or features set by manufacturers like Pico or Marklin. Instead, you can create virtually any state and action you need. No programming is required, but for advanced users, the source code is available for full control over the decoder's behavior.

- You can control almost everything with SimpleDCC/WCC from a command station or web browser, including RGB addressable LEDs, NFC readers, Hall sensors, servo motors, DC motors/single solenoids (an external DC motor driver is required) and more.


## Getting Started

- Purchase a [Loco.Engineering](https://loco.engineering) decoder or build your own (any ESP32S3 dev board can serve as a WCC decoder when powered by USB, but for handling DCC signals, you'll need a DCC converter with a DC drop-down circuit).
- (Optional for Loco.Engineering decoders, required for custom decoders) Open simpledcc-arduino.ino in Arduino IDE (we test with Arduino IDE 2.3.2) and upload the firmware to the decoder (follow the steps in "How to build and upload").
- (Optional for WCC but required for DCC) Connect the decoder to DCC-enabled wires or track.
- Power the decoder
- On your phone, tablet, or laptop, connect to a Wi-Fi network named loco-xxxxx, where xxxxx is the serial number on your Loco.Engineering decoder or the wifi_network_name string in config.h if you're using custom hardware.
- Open http://loco.local or http://192.168.4.1 in your browser.
- If your DCC command station is sending messages to the connected track/wires, you should see them in your browser. Use filters to hide unwanted messages.
- To add actions for the decoder to perform when receiving a specific DCC packet, click "Add state" next to that packet. Loco.Engineering decoders have marked outputs for connecting wires from accessories/trains/vehicles, so you'll know that a red LED on a semaphore is connected to output number 5, etc.


## How to build and upload

Steps below are tested with ESP32-S3-WROOM-1-N16R8 modules. All Loco.Engineering boards are shipped with the default firmware that's why you shouldn't upload the firmware manually.

- Prepare a board with ESP32S3 SoC (2 cores are required to run the firmware that's why other 1-core ESP32xx are not supported in this version)
- Install Arduino IDE (we test with Arduino IDE 2.3.2)
- Install ESP32 for Arduino IDE (we test with version 2.0.17, later versions could not work and you will have build errors)
- Install libraries: AsyncTCP (tested with version 1.1.4), ESPAsyncTCP (1.2.4), ESPAsyncWebServer (tested with 3.1.0), LEDDriver_NXP_Arduino (tested with 1.0.2), I2C_device_Arduino (remove 2 tests file after you install it - Arduino/libraries/I2C_device_Arduino/src/test_LM75B.h and .cpp with the same name otherwise you'll get errors during building), MFRC522 (tested with 1.4.11), LiteLED (version 1.2.0). Don't forget to select "Install all dependencies" if Arduino IDE asks about that.
- Change configuration in config.h if required
- Set Flash Size to what your board has (Tools -> Flash Size) and rename partitions-4MB-flash.csv or partitions-16MB-flash.csv depending on the flash size to partitions.csv.
- Set PSRAM type to OPI PSRAM (Tools -> PSRAM -> OPI PSRAM) if you use a module like ESP32-S3-WROOM-1-N...R... or QSPI PSRAM if you use ESP32-S3FH4R2 SoC.
- Create and upload LittleFS image with files in the simpledcc-arduino/data folder - https://github.com/earlephilhower/arduino-littlefs-upload (for Arduino 2.2.1 and higner). You should update partions.csv file with your flash amount: open particions.csv and set spiffs size according your board flash size
- (Optional) To show all logs from your decoder, enable Verbose Debug Mode. Select Tools-> Core Debug Level -> Verbose. If you don't see any logs check "I don't see any logs" below
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
