#### Note: This project is under active development

# DCC (Digital Command Control) / WCC (Wireless Command Control) system for model railways, LEGO, and toys based on ESP32-XX SoCs

**License:** You can use SimpleDCC in any project except for selling hardware with preinstalled SimpleDCC firmware. Contact us at [hey@loco.engineering](mailto:hey@loco.engineering) if you would like to include SimpleDCC firmware with your decoders, command stations, or other products.

**If you like this project and its concept, please spread the word and don't forget to give us a star on GitHub. Your support helps us grow!**

## Why We Started Working on SimpleDCC

DCC (Digital Command Control) for model railways operates on a basic principle: a command station sends messages to DCC decoders via two wires or the rails. Unlike most wireless systems, it doesn't involve encryption, authorization, or acknowledgment packets.

However, updating the logic on DCC decoders remains a challenge, as it can't be done directly from a laptop or mobile device without a command station. DCC decoder logic is typically predefined by the manufacturer, allowing users to configure only certain features, parameters, or outputs.

SimpleDCC removes this complexity by providing simple-to-use, flexible command stations and introducing two-way wireless communication between command stations and decoders.

## How the SimpleDCC/WCC System Differs from Other DCC Systems

The main differences are:

* SimpleDCC is designed to be as simple as possible. A mobile device or computer can connect to SimpleDCC over Wi-Fi without any additional tools.
* SimpleDCC is flexible and extensible. You can control not only signals, trains, and other accessories available from model railway brands, but also displays, NFC readers, sensors, and other devices. These devices can also send information and notifications back to the command station.
* SimpleDCC comes with a user-friendly web app designed specifically for the system.
* A SimpleDCC command station requires just two boards: any supported ESP32-XX-based board, such as an ESP32-S3 board, and a DC motor driver. You can assemble the command station yourself or purchase a ready-to-use version from our online shop.
* SimpleDCC also includes a wireless protocol called **Wireless Command Control (WCC)**, which enables two-way communication between a command station and decoders.
* SimpleDCC provides firmware and hardware designs for DCC/WCC command stations and DCC/WCC decoders.
* You don't need to understand the inner workings of DCC. Decoder logic can be configured through the web app without additional programming tools.
* You can control almost anything with SimpleDCC/WCC from a command station or web browser, including RGB addressable LEDs, NFC readers, Hall sensors, servo motors, DC motors, single solenoids (an external DC motor driver is required), and more.

## Getting Started

Documentation is in progress.

## Contact Us

Feel free to contact us at [hey@loco.engineering](mailto:hey@loco.engineering) if you have any questions or feedback.
