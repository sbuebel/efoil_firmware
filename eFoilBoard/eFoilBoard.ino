#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(7, 8); // CE, CSN

const byte address[6] = "00001";

// TODO: this belongs in a shared file...
struct eFoilCommandPacket {
  uint16_t motor_speed;
  uint8_t state;
  uint16_t remoteBattery;  // milliVolts
  uint8_t rssi;  // somehow express this... hm
};


void setup() {
  Serial.begin(9600);
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();
}

void loop() {
  if (radio.available()) {
    eFoilCommandPacket cmd;
    radio.read(&cmd, sizeof(cmd));
    Serial.print("speed: ");
    Serial.print(cmd.motor_speed);
  }
}