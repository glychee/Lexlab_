#include <SPI.h>
#include "RF24.h"

RF24 radio(10, 9);
byte addresses[][6] = {"Receiver", "Transmitter"};

//define flip action
const int FLIP = 2;

// Define the number of rows and columns in the button matrix
const int numRows = 3;
const int numCols = 4;

// Define the pin mappings for rows and columns
const int rowPins[numRows] = {A6, A5, A7};
const int colPins[numCols] = {A1, A2, A3, A4};

//use this set if you need serial debugging
//const int numPins = 8;
//const int OutputPins[numPins] = {2, 3, 4, 5, 6, 7, 8, A0};
//use this set for production, it will also do draining and main faucet
const int numPins = 10;
const int OutputPins[numPins] = {2, 3, 4, 5, 6, 7, 8, A0, 0, 1};

// Define an array to store the state of the solenoids
bool solenoidStates[10] = {LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, HIGH}; // main faucet starts high

// random variables
unsigned long previousMillis = 0;
long interval = 1000;

// Define a matrix to store the state of the buttons
int buttonMatrix[numRows][numCols] = {
  {LOW, LOW, LOW, LOW},
  {LOW, LOW, LOW, LOW},
  {LOW, LOW, LOW, LOW}
};


// Variable to track if any value was HIGH previous loop
bool previousValueHigh = false;

//Variable to track if random spouts are enabled
bool randomMode = false;

typedef struct {
  int target = 0;
  int state = 0;
} message;

void toggleSolenoids() {
  //use button matrix to toggle all normal solenoids
  for (int row = 0; row < 2; row++) {
    for (int col = 0; col < numCols; col++) {
      // Check if the value in buttonMatrix is HIGH
      if (buttonMatrix[row][col] == HIGH) {
        // Flip the corresponding element in solenoidStates
        solenoidStates[col + (row * 4)] = !solenoidStates[col + (row * 4)];

      }
    }
  }
  if (buttonMatrix[2][1]) {
    //emergency shut off
    solenoidStates[10] = LOW;
    //digitalWrite(OutputPins[10], LOW);
    //Serial.println("shut off");
    
    //delay(3000);
  }
}

void setup() {
  // Initialize the serial communication
  //Serial.begin(115200);
  //delay for startup
  //delay(5000);
  for (int i = 0; i < numPins; i++) {
    pinMode(OutputPins[i], OUTPUT);
  }
  // Set the row pins as output
  for (int row = 0; row < numRows; row++) {
    pinMode(rowPins[row], INPUT_PULLUP);
  }

  // Set the column pins as input with internal pull-up resistors enabled
  for (int col = 0; col < numCols; col++) {
    pinMode(colPins[col], OUTPUT);
    digitalWrite(colPins[col], HIGH);
  }
  radio.begin();
  //low power mode
  radio.setPALevel(RF24_PA_HIGH);

  // Open a writing and reading pipe on each radio, with opposite addresses
  radio.openWritingPipe(addresses[0]);
  radio.openReadingPipe(1, addresses[1]);
  radio.startListening();

}

void loop() {
  // Read the state of the buttons and update the matrix
  for (int col = 0; col < numCols; col++) {
    for (int row = 0; row < numRows; row++) {
      // Activate the column
      digitalWrite(colPins[col], LOW);

      // Read the state of the button
      if (rowPins[row] == A6 || rowPins[row] == A7) {
        buttonMatrix[row][col] = !(analogRead(rowPins[row]) > 512);
        //Serial.print(analogRead(rowPins[row])) ;
        delay(2);
      } else {
        buttonMatrix[row][col] = !digitalRead(rowPins[row]);
      }

      // Deactivate the row
      digitalWrite(colPins[col], HIGH);
    }
  }

  bool anyValueHigh = false;
  // Iterate over all values in buttonMatrix
  for (int row = 0; row < numRows; row++) {
    for (int col = 0; col < numCols; col++) {
      // Check if the current value is HIGH
      if (buttonMatrix[row][col] == HIGH && !(row == 2 && col == 2)) { //skip our toggle button
//        Serial.print(String(row)+String(col));
        anyValueHigh = true;
        // Exit the loops since we found a HIGH value
        break;
      }
    }
    // Exit the outer loop if we found a HIGH value
    if (anyValueHigh) {
      break;
    }
  }

  //check if a button is actually toggled, we don't want constant toggling
  if (previousValueHigh == false && anyValueHigh == true) {
    previousValueHigh = true;
    //Random mode
    if (buttonMatrix[2][0]) {
      randomMode = !randomMode;
    }
    toggleSolenoids();
  }
  //reset the global check
  if (previousValueHigh == true && anyValueHigh == false) {
    previousValueHigh = false;
  }
/*
  //drain feature
  if (buttonMatrix[2][2]) {
    solenoidStates[10] = HIGH;
    digitalWrite(OutputPins[10], HIGH);//toggle is an exclusion on updates too..

//    Serial.println("draining");
  }
  else {
    solenoidStates[10] = LOW;
    digitalWrite(OutputPins[10], LOW);//toggle is an exclusion on updates too..

//    Serial.println("not draining");
  }
*/
    //drain feature
  if (buttonMatrix[2][1]) {
    solenoidStates[9] = LOW;
    digitalWrite(OutputPins[9], LOW);//toggle is an exclusion on updates too..
    solenoidStates[9] = LOW;
    //Serial.println("emergency shutoff");
    //delay(3000);
  }

  // Print the state of the buttons, for debugging
  
//    for (int row = 0; row < numRows; row++) {
//    for (int col = 0; col < numCols; col++) {
//    Serial.print(buttonMatrix[row][col]);
//    Serial.print('\t');
//    }
//    Serial.println();
//    }
//    Serial.println();
  
  // Print the state of solenoids, for debugging
  /*
    for (int x = 0; x <= 7; x++) {
    Serial.print(solenoidStates[x]);
    }
    Serial.print('\n');
  */
  //receive radio communication
  message receivedmessage;
  if ( radio.available()) {
    //Serial.println("rec");
    while (radio.available()) {                                   // While there is data ready
      radio.read( &receivedmessage, sizeof(message) );             // Get the payload
    }
    receivedmessage.state;
    receivedmessage.target;
    if (receivedmessage.state == FLIP) {
      solenoidStates[receivedmessage.target] = !solenoidStates[receivedmessage.target];
    } else {
      solenoidStates[receivedmessage.target] = (bool)receivedmessage.state;
    }
    //Serial.println("Received: " + String(receivedmessage.state) + ", " + String(receivedmessage.target));
  }

  //write states to motors
  if (!randomMode) {
    //toggleSolenoids();
    for (int x = 0; x < 10; x++) {
      digitalWrite(OutputPins[x], solenoidStates[x]);
      
//        Serial.print(OutputPins[x]);
//        Serial.print(" - ");
//        Serial.println(solenoidStates[x]);
//      
    }
  } else if (millis() - previousMillis >= interval) {
    previousMillis = millis();
    for (int x = 0; x <= 7; x++) { //randomise the 8 valves
      bool state = random(0, 2);
      digitalWrite(OutputPins[x], state);
      
      //Serial.println(state);
   }
   //interval = random(8,20)*1000;
  }


  // Wait for a short duration before reading the buttons again
  delay(100);
}
