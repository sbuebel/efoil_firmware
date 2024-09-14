
#include <SPI.h>
#include "RF24.h"
 
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

RF24 radio(7, 8);
 
// Let these addresses be used for the pair
uint8_t address[6] = "tx069";

struct eFoilCommandPacket {
  uint16_t speed;
  uint8_t state;
  uint16_t remoteBattery;  // milliVolts
  uint8_t rssi;  // somehow express this... hm
};

#define LOOP_DELAY_US 20000  // 50Hz

void setup() {
  Serial.begin(115200);
  while (!Serial);

  // this is our PPM pin
  pinMode(5, OUTPUT);

  if (!radio.begin()) {
    Serial.println(F("radio hardware is not responding!!"));
    while (1) {}  // hold in infinite loop
  }
  radio.setPALevel(RF24_PA_LOW);  // RF24_PA_MAX is default.
  radio.setAutoAck(false);
  radio.setPayloadSize(sizeof(eFoilCommandPacket));
  radio.openReadingPipe(0, address);  // using pipe 1
  radio.startListening();  // put radio in RX mode
 
}  // setup
 
void loop() {
  /*

  Timing:

  0us      1000us     2000us     3000us     20,000us (end)
  |----------|----------|----------|----...----|
  |          |          |          |           |
  [] <- motor start
            [] <- min pulse
                        [] <- max pulse
   [         ] <- free time (read from radio?)
             [**********] <- block for motor command?
                        [                      ] <- free time (control display)

  */
  static uint8_t loop_count = 0;
  uint32_t _start = micros();

  static uint32_t _last_packet = 0;
   eFoilCommandPacket cmd;

    // if (radio.available(0)) {
    if (radio.available()) {
      Serial.println("started to read");
      radio.read(&cmd, sizeof(cmd));
      Serial.print("Read speed: ");
      Serial.println(cmd.speed);
      _last_packet = millis();
      radio.flush_rx();  // in case we lose connection
    }
    else if (millis() - _last_packet > 3000)
    {
      Serial.println("No TX found, cycle power on both?");
    }
    else
    {
      Serial.println("Alive but unwell");
    }
 
  bool fast = 0;
  while (micros() - _start < LOOP_DELAY_US) {
    delayMicroseconds(1);
    fast = 1;
  }
  if (!fast)
    Serial.println("too slow gee");
  
  loop_count ++;
}  // loop
