#include <SPI.h>
#include "RF24.h"

RF24 radio(10,9);
byte addresses[][6] = {"Receiver","Transmitter"};

typedef struct{
  bool state1;
  bool state2;
  bool state3;
  bool state4;
  bool state5;
  bool state6;
  bool state7;
  bool state8;
} 
message ;

void setup() {
  delay(3000);
  //start keyboard here

  //or start serial here
  Serial.begin(115200);
  Serial.println("start");
  
  radio.begin();
  //low power mode
  radio.setPALevel(RF24_PA_LOW);
  
  // Open a writing and reading pipe on each radio, with opposite addresses
  radio.openWritingPipe(addresses[0]);
  radio.openReadingPipe(1,addresses[1]);
  radio.startListening();
}

void loop() {
    message receivedmessage;
    
    if( radio.available()){
                                                                    // Variable for the received timestamp
      while (radio.available()) {                                   // While there is data ready
        radio.read( &receivedmessage, sizeof(message) );             // Get the payload
      }
      Serial.print("rec: ");
      Serial.print(receivedmessage.state1);
      Serial.print(" - ");
      Serial.print(receivedmessage.state2);
      Serial.print(" - ");
      Serial.print(receivedmessage.state3);
      Serial.print(" - ");
      Serial.print(receivedmessage.state4);
      Serial.print(" - ");
      Serial.print(receivedmessage.state5);
      Serial.print(" - ");
      Serial.print(receivedmessage.state6);
      Serial.print(" - ");      
      Serial.print(receivedmessage.state7);
      Serial.print(" - ");
      Serial.println(receivedmessage.state8);
   }
 

}
