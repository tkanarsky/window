# Window 

![](https://i.imgur.com/1xV0GTJ.jpg)

Window is a solar-powered smart dorm whiteboard that allows passerby to see
whether a dorm room's occupants are present, and send them voice messages over the
Internet. Alberto Mancia and I built this gadget during IDEAHacks 2020,
addressing the 'College Lifestyle' theme. 

Hardware: The firmware in this repository ran on an ESP32 devkit board. It had
a small Adafruit SSD1306 OLED display attached to the I2C interface, a
microphone (electret mic with builtin amplifier) attached to an ADC pin, and two capacitive buttons attached to the
capacitive touch register pins. 

Firmware: Use PlatformIO to compile: run `platformio update` to fetch the
Espressif32 framework and the Arduino runtime for ESP32. After that, run
`platformio run` to fetch dependencies from GitHub and finally `platformio
upload` to flash a connected ESP32 controller. 

This project depends on the Adafruit GFX library and the Adafruit SSD1306 OLED
driver library, as well as the built-in BLE and HTTP client stacks supplied with
the ESP framework. 

Audio recording was an interesting challenge. I was able to record a 8-bit audio
clip at an 8kHz sample rate by simply calling analogRead really fast. Since we
had no external SD card to record to, free RAM was the limiting factor in clip
length. Through trial and error, I settled on 100KB (slightly more than 10 seconds)
worth of audio. To send the raw audio data over the Internet, I gave the file
a valid .wav file header and POST'd it to a server running on my laptop. The
result was a phone-message-quality recording which was good enough for a
demonstration. 
