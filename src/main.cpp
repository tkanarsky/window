#include <Arduino.h>
#include "BLEDevice.h"
#include "Adafruit_SSD1306.h"
#include <Wire.h>
#include "WaveHandler.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <esp32-hal-dac.h>
#include <EEPROM.h>

// OLED Display Constants

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET 4

// OLED Display Variables

Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


// OLED Methods

void initOled();
void displayStatus();
void displayRecordConfirmMessage();
void displayRecordingMessage(bool animate, uint8_t progress);
void displayRecordingAskMessage();
void displayRecordingSendingMessage();
void displayRecordingSentMessage();


// BLE Constants

#define BLE_WAIT_MULTIPLIER 1000000
#define BLE_SCAN_TIME 18
#define BLE_SLEEP_TIME 60
#define BLE_SCAN_INTERVAL 128
#define BLE_SCAN_WINDOW 128

// BLE Variables

BLEScan* pBleScan;
bool initd = false;
bool scanStarted = false;
bool scanFinished = false;
char mac1[] = "MAC_ADDR_1";
char mac2[] = "MAC_ADDR_2";
RTC_DATA_ATTR bool mac1found;
RTC_DATA_ATTR bool mac2found;
bool mac1currfound;
bool mac2currfound;
// BLE Methods

void initBLE();
void runScan();
void shutdownBLE();

// BLE Helper Classes

class Callback: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice d) {
        if (strcmp(d.getAddress().toString().c_str(), mac1) == 0) {
            mac1currfound = true;
            Serial.println("FOUND!!!!!");
        }
        if (strcmp(d.getAddress().toString().c_str(), mac2) == 0) {
            mac2currfound = true;
        }
    }
};

// Wifi Constants

#define SSID "YOUR_SSID"
#define PASS "YOUR_PASSWD"
#define URL "192.168.1.50:8080"

// Wifi Variables

HTTPClient http;
void initHttp();
void shutdownHTTP();
// Audio Constants

#define MAX_SAMPLES 100000
#define SAMPLE_CHUNK_SIZE 10000
#define SAMPLE_RATE 16000
#define MIC_PIN 36


// Audio Variables

uint8_t* messageBuffer;

// Audio Methods

void sendWaveFile(uint8_t* buf, uint32_t len);
int captureMessage(uint8_t* &buf, bool(*f)(), void(*g)(bool, uint8_t));
void play(uint8_t* buf, uint32_t len);

// Capacitive Touch constants
#define TOP_BUTTON 32 // actually connect to 33
#define BOTTOM_BUTTON 27 // connect to 27
#define TOUCH_THRESHOLD 35

// Capacitive Touch methods

boolean buttonHeld() {
    return ((touchRead(TOP_BUTTON) < TOUCH_THRESHOLD) || (touchRead(BOTTOM_BUTTON) < TOUCH_THRESHOLD));
}

int waitUntilPress(int timeout) {
    int start = millis();
    while (millis() - start < timeout) {
        delay(100);
        if (touchRead(TOP_BUTTON) < TOUCH_THRESHOLD) return 1;
        if (touchRead(BOTTOM_BUTTON) < TOUCH_THRESHOLD) return 2;
    }
    return 0;
}

void callback() {
    return;
}

void setup() {
//    Serial.begin(115200);
//    for (;;) {
//        Serial.print("TOP: ");
//        Serial.println(touchRead(TOP_BUTTON));
//        Serial.print("BOTTOM: ");
//        Serial.println(touchRead(BOTTOM_BUTTON));
//    }
    touchAttachInterrupt(TOP_BUTTON, callback, 25);
    touchAttachInterrupt(BOTTOM_BUTTON, callback, 25);
    esp_sleep_enable_touchpad_wakeup();
    initOled();
    Serial.begin(115200);
    switch(esp_sleep_get_wakeup_cause()) {
        case ESP_SLEEP_WAKEUP_TIMER:
            initBLE();
            runScan();
            displayStatus();
            delay(50);
            ESP.deepSleep(BLE_SLEEP_TIME * 1000000);
            break;
        case ESP_SLEEP_WAKEUP_TOUCHPAD:
            mac1found = EEPROM.read(0);
            mac2found = EEPROM.read(1);
            while(true) {
                displayRecordingMessage(true, 0);
                delay(1000);
                while(!buttonHeld()) {}
                uint32_t len = captureMessage(messageBuffer, buttonHeld, displayRecordingMessage);
                displayRecordingAskMessage();
                delay(500);
                if (waitUntilPress(10000) == 0) continue;
                displayRecordConfirmMessage();
                delay(500);
                int resp = waitUntilPress(10000);
                if (resp == 1) {
                    sendWaveFile(messageBuffer, len);
                    delay(500);
                    displayRecordingSentMessage();
                    delay(500);
                    displayStatus();
                    delay(50);
                    ESP.deepSleep(BLE_SLEEP_TIME * 1000000);
                    break;
                } else {
                    displayStatus();
                    delay(50);
                    ESP.deepSleep(BLE_SLEEP_TIME * 1000000);
                }

            }

        default:
            initBLE();
            runScan();
            displayStatus();
            delay(50);
            ESP.deepSleep(BLE_SLEEP_TIME * 1000000);
    };
//    initOled();
//    while (!buttonHeld()) delay(50);
//    uint32_t len = captureMessage(messageBuffer, buttonHeld, displayRecordingMessage);
//    shutdownBLE();
//    play(messageBuffer, len);
////    sendWaveFile(messageBuffer, len);

}

void loop() {
//    Serial.println(touchRead(27));
//    delay(50);
}

void initOled() {
    oled.begin(SSD1306_SWITCHCAPVCC, 0x3D);
    oled.setRotation(1);
    oled.setCursor(0,0);
    oled.setTextColor(SSD1306_WHITE);
    oled.clearDisplay();
    delay(500);
    Serial.println("OLED init'd!");
}

void displayStatus() {
    oled.clearDisplay();
    oled.setCursor(0,20);
    oled.setTextColor(SSD1306_WHITE);
    if (mac1found) {
        oled.setTextSize(4);
        oled.print("IN\n\n");
    } else {
        oled.setTextSize(3);
        oled.print("OUT\n\n");
    }
    if (mac2found) {
        oled.setTextSize(4);
        oled.print("IN\n");
    } else {
        oled.setTextSize(3);
        oled.print("OUT\n");
    }
    oled.display();
}

void displayRecordingMessage(bool animate, uint8_t progress) {
    if (progress == 0)
        oled.clearDisplay();
    oled.setCursor(0, 0);
    oled.setTextSize(2);
    oled.print("HOLD\n");
    if (animate) {
        oled.display();
        delay(100);
    }
    oled.setTextSize(4);
    oled.print("2\n");
    if (animate) {
        oled.display();
        delay(100);
    }
    oled.setTextSize(2);
    oled.print("REC\n");
    if (animate) {
        oled.display();
        delay(50);
    }
    oled.print("ORD\n");
    if (animate) {
        oled.display();
        delay(50);
    }
    oled.print("MSG");
    if (animate) {
        oled.display();
        delay(50);
    }
    oled.drawRoundRect(0, 118, 63, 9, 3, SSD1306_WHITE);
    oled.fillRoundRect(0, 118, map(progress, 0, 100, 0, 63), 9, 3, SSD1306_WHITE);
    oled.display();
}
void displayRecordConfirmMessage() {
    oled.clearDisplay();
    oled.setTextSize(2);
    oled.setCursor(5, 40);
    oled.print("SEND\n");
    oled.setTextSize(4);
    oled.print(" ?");
    oled.setTextSize(3);
    oled.setCursor(45, 0);
    oled.print("Y");
    oled.setCursor(45, 105);
    oled.print("N");
    oled.display();
}

void displayRecordingAskMessage() {
    oled.clearDisplay();
    oled.setCursor(0, 35);
    oled.setTextSize(2);
    oled.print("  TO\n WHOM\n");
    oled.setTextSize(2);
    oled.print("  ?");
    oled.setTextSize(1);
    oled.setCursor(45, 0);
    oled.print("TIM");
    oled.setCursor(20, 120);
    oled.print("ALBERTO");
    oled.display();
}
void displayRecordingSendingMessage() {
    oled.clearDisplay();
    oled.setCursor(0,0);
    oled.setTextSize(3);
    oled.print("SE\nND\nING");
    oled.display();
}
void displayRecordingSentMessage() {
    oled.clearDisplay();
    oled.setCursor(0,0);
    oled.setTextSize(5);
    char statement[] = "SENT";
    for (int i = 0; i < 4; i++) {
        oled.print(statement[i]);
        if (i == 1) oled.print("\n");
        oled.display();
        delay(100);
    }
    oled.display();
}

void initBLE() {
    BLEDevice::init("");
    pBleScan = BLEDevice::getScan();
    pBleScan->setAdvertisedDeviceCallbacks(new Callback());
    pBleScan->setInterval(BLE_SCAN_INTERVAL);
    pBleScan->setWindow(BLE_SCAN_WINDOW);
    pBleScan->setActiveScan(true);
    Serial.println("BLE init'd!");
    initd = true;
}

void shutdownBLE() {
    BLEDevice::deinit(true);
    delete pBleScan;
}

void runScan() {
    scanStarted = true;
    mac1currfound = false;
    mac2currfound = false;
    pBleScan->start(BLE_SCAN_TIME);
    scanFinished = true;
    mac1found = mac1currfound;
    mac2found = mac2currfound;
    EEPROM.write(0, mac1found);
    EEPROM.write(1, mac1found);
    EEPROM.commit();
}

int captureMessage(uint8_t* &buf, bool(*f)(), void(*g)(bool, uint8_t)) {
    free(buf);
    int bufSize = SAMPLE_CHUNK_SIZE;
    analogReadResolution(8);
    pinMode(MIC_PIN, INPUT);
    buf = static_cast<uint8_t*>(malloc(bufSize*sizeof(uint8_t)));
    g(false, 0); // update progress bar;
    while (f() && bufSize < MAX_SAMPLES) {
        for (int i = bufSize - SAMPLE_CHUNK_SIZE; i < bufSize; i++) {
            buf[i] = analogRead(MIC_PIN);
            delayMicroseconds(1000000 / SAMPLE_RATE);
        }
        g(false, bufSize / 1000); // update progress bar;
        if (bufSize + SAMPLE_CHUNK_SIZE > MAX_SAMPLES || ESP.getFreeHeap() < SAMPLE_CHUNK_SIZE) break;
        buf = static_cast<uint8_t*>(realloc(buf, (bufSize + SAMPLE_CHUNK_SIZE) * sizeof(uint8_t)));
        bufSize += SAMPLE_CHUNK_SIZE;
    }
    if (bufSize > MAX_SAMPLES) bufSize -= SAMPLE_CHUNK_SIZE;
    Serial.println("returning!");
    return bufSize;
}

void sendWaveFile(uint8_t* buf, uint32_t len) {
    displayRecordingSendingMessage();
    buf = WaveHandler::createWavFile(buf, len, SAMPLE_RATE * .80);
    initHttp();
    http.begin(URL);
    http.addHeader("Content-Type", "audio/wav");
    http.POST(buf, len);
    free(buf);
}

void initHttp() {
    int start = millis();
    if (WiFi.status() != WL_CONNECTED)
        WiFi.begin(SSID, PASS);
    while (WiFi.status() != WL_CONNECTED && (millis() - start < 20000)) {
        delay(100);
    }
    Serial.println("connected!");
}
void shutdownHTTP() {
    WiFi.disconnect(false, true);
}

void play(uint8_t* buf, uint32_t len) {
    for (int i = 0; i < len; i++) {
        dacWrite(25, buf[i]);
        delayMicroseconds(1000000 / SAMPLE_RATE);
    }
}

