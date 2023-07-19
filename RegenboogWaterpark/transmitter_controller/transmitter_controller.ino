#include <SPI.h>
#include "RF24.h"
#include <Keypad.h>

const int FLIP = 2;

//user configurable
int tileNumber = 1;

int switchPin = 3;
int stripPin = 6;

//do not change
RF24 radio(10, 9);
byte addresses[][6] = {"Receiver", "Transmitter"};

typedef struct {
  int target = 0;
  int state = 0;
} message;

const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
//define the cymbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
  {'0', '1', '2', '3'},
  {'4', '5', '6', '7'},
  {'8', '9', 'A', 'B'},
  {'C', 'D', 'E', 'F'}
};
byte rowPins[ROWS] = {6, 7, 8, A0}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {2, 3, 4, 5}; //connect to the column pinouts of the keypad

//initialize an instance of class NewKeypad
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

void setup() {
  Serial.begin(115200);
  Serial.println("start");
  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.openWritingPipe(addresses[1]);
  radio.openReadingPipe(1, addresses[0]);
}

bool lastPin1State = 0;

void loop() {
  char customKey = customKeypad.getKey();
  message transmitmessage;
  if (customKey) {
    Serial.println(customKey);
    switch (customKey) {
      case '0':
        transmitmessage.target = 0;
        transmitmessage.state = FLIP;
        radio.write( &transmitmessage , sizeof(message));
        break;
      case '1':
        transmitmessage.target = 1;
        transmitmessage.state = FLIP;
        radio.write( &transmitmessage , sizeof(message));
        break;
      case '2':
        transmitmessage.target = 2;
        transmitmessage.state = FLIP;
        radio.write( &transmitmessage , sizeof(message));
        break;
      case '3':
        transmitmessage.target = 3;
        transmitmessage.state = FLIP;
        radio.write( &transmitmessage , sizeof(message));
        break;
      case '4':
        transmitmessage.target = 4;
        transmitmessage.state = FLIP;
        radio.write( &transmitmessage , sizeof(message));
        break;
      case '5':
        transmitmessage.target = 5;
        transmitmessage.state = FLIP;
        radio.write( &transmitmessage , sizeof(message));
        break;
      case '6':
        transmitmessage.target = 6;
        transmitmessage.state = FLIP;
        radio.write( &transmitmessage , sizeof(message));
        break;
      case '7':
        transmitmessage.target = 7;
        transmitmessage.state = FLIP;
        radio.write( &transmitmessage , sizeof(message));
        break;
      case '8': //flips state of solenoid 0
        transmitmessage.target = 0;
        transmitmessage.state = FLIP;
        radio.write( &transmitmessage , sizeof(message));
        break;
      case '9': //attempts to make 1 the state this node thinks it should be
        //to do this, it uses a variable set globally to remember what it was
        transmitmessage.target = 1;
        transmitmessage.state = lastPin1State;
        lastPin1State = !lastPin1State;
        radio.write( &transmitmessage , sizeof(message));
        break;
      case 'A': //turns on 2 for a specific amount of time
        transmitmessage.target = 2;
        transmitmessage.state = true;
        radio.write( &transmitmessage , sizeof(message));
        delay(1000);
        transmitmessage.state = false;
        radio.write( &transmitmessage , sizeof(message));
        break;
      case 'B': //turns on 2 and 3 for a specific amount of time
        //turn on
        transmitmessage.target = 2;
        transmitmessage.state = true;
        radio.write( &transmitmessage , sizeof(message));
        delay(150);
        transmitmessage.target = 3;
        transmitmessage.state = true;
        radio.write( &transmitmessage , sizeof(message));
        //wait
        delay(1000);
        //turn off
        transmitmessage.target = 2;
        transmitmessage.state = false;
        radio.write( &transmitmessage , sizeof(message));
        delay(150);
        transmitmessage.target = 3;
        transmitmessage.state = false;
        radio.write( &transmitmessage , sizeof(message));
        break;
      case 'C': //C and D are used here to toggle one motor together
        transmitmessage.target = 3;
        transmitmessage.state = true;
        radio.write( &transmitmessage , sizeof(message));
        break;
      case 'D':
        transmitmessage.target = 3; //couple button D to motor 3
        transmitmessage.state = false;
        radio.write( &transmitmessage , sizeof(message));
        break;

      default:
        //if behaviour not defined, don't send anything.
        break;
    }
    Serial.println("Sent: " + String(transmitmessage.state) + ", " + String(transmitmessage.target));
  }
}
