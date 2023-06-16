#include <SPI.h>
#include "RF24.h"

//user configurable
int tileNumber = 1;

int switchPin = 3;
int stripPin = 6;

//do not change
RF24 radio(10,9);
byte addresses[][6] = {"Receiver","Transmitter"};

typedef struct{
  bool state1 = 0;
  bool state2 = 0;
  bool state3 = 0;
  bool state4 = 0;
  bool state5 = 0;
  bool state6 = 0;
  bool state7 = 0;
  bool state8 = 0;
} 
message ;

void setup() {
 Serial.begin(115200);
  Serial.println("start");
  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.openWritingPipe(addresses[1]);
  radio.openReadingPipe(1,addresses[0]);
}

void loop() {
  
      message transmitmessage;
      Serial.println(F("Now sending"));
      transmitmessage.state1 = 1;
      transmitmessage.state4 = 1;
      transmitmessage.state5 = 1;
      transmitmessage.state8 = 1;
      
      radio.write( &transmitmessage , sizeof(message));
      delay(1000);
  
}
