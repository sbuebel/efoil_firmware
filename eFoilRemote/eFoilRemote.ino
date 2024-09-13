#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define LOOP_DELAY_US 4000  // 250Hz
#define RADIO_LOOP_HZ 10

/*

State machine and transitions

0. off
  n->0: switch off by holding a button - reed switch
  0->1: switch on by pressing a button?
2. active
  2->3: issue detected w/ anything, low voltage, bad link
3. fault/safe
  3->1: all conditions met, return to idle
4. charging
  4->0: charger removed
  4->5: charge complete
5. charge complete
  5->0: charger removed


   [ off ]
      |
      |   
      |   
      |   
      ------> [ on ] --> [ fault ]
                |  ^                       
                |  |                       
                |  |                       
                v  |                       
            [ charging ]

*/

struct eFoilCommandPacket {
  uint16_t speed;
  uint8_t state;
  uint16_t remoteBattery;  // milliVolts
  uint8_t rssi;  // somehow express this... hm
};

// output - used to drive charge FET
int CHARGE_ENABLE_PIN = 2;

// input - used to detect charge voltage
int CHARGE_DETECT_PIN = 6;

// CE, CSN
RF24 radio(7, 8);

const byte address[6] = "00001";

// Create an instance of the display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
  pinMode(5, OUTPUT);

  // Initialize the display
  if(!display.begin(0x3C, 0x3C)) { // Check if the display is connected
    Serial.println("SSD1306 allocation failed");
    for(;;); // TODO: flash an LED here or something
  }

  // // todo: kill this
  display.display(); // Show initial splash screen
  delay(2000); // Pause for 2 seconds

  display.clearDisplay(); // Clear the display buffer
  
  Serial.begin(115200);

  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);  // TODO: change to high if needed
  radio.stopListening();
}

void loop() {
  /*

  Timing:

  0us      1000us     2000us     3000us     4000us (end)
  |----------|----------|----------|----------|
  |          |          |          |          |
  [] <- motor start
            [] <- min pulse
                        [] <- max pulse
   [         ] <- free time (read from radio?)
             [**********] <- block for motor command?
                        [                      ] <- free time (control display)

  */
  uint32_t _start = micros();
  uint16_t speedCmd = 1000 + map(analogRead(A0), 500, 600, 0, 1000);

  eFoilCommandPacket cmd;
  cmd.speed = speedCmd;
  cmd.state = 1;  // TODO: state machine
  cmd.remoteBattery = 4110;  // analogRead(A4); map it!
  cmd.rssi = 100;
  radio.write(&cmd, sizeof(cmd));


  // display.clearDisplay(); // Clear the display buffer

  // Draw text
  // display.setTextSize(1);      // Set text size
  // display.setTextColor(SSD1306_WHITE); // Set text color
  // display.setCursor(0, 0);     // Set cursor to the top-left corner
  // const char speed[100];
  // sprintf(speed, "%d", speedCmd);
  // display.println(speedCmd); // Print text
  // display.display(); // Update the display with the drawn content

  // simplest possible write to speed controller
  digitalWrite(5, HIGH);
  Serial.println(speedCmd);

  while (micros() - _start < speedCmd)
    delayMicroseconds(1);
  
  digitalWrite(5, LOW);

  while (micros() - _start < LOOP_DELAY_US)
    delayMicroseconds(1);
}