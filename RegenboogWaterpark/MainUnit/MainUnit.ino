#include <SPI.h>
#include "RF24.h"

RF24 radio(10, 9);
byte addresses[][6] = {"Receiver", "Transmitter"};

//define flip action
const int FLIP = 2;
const int RANDOM = 3;
const int ALLOFF = 4;

// Define the number of rows and columns in the button matrix
const int numRows = 3;
const int numCols = 4;

// Define the pin mappings for rows and columns
const int rowPins[numRows] = {A5, A6, A7};
const int colPins[numCols] = {A1, A2, A3, A4};

//use this set if you need serial debugging
//const int numPins = 8;
//const int OutputPins[numPins] = {2, 3, 4, 5, 6, 7, 8, A0};
//use this set for production, it will also do draining and main faucet
const int numPins = 10;
const int OutputPins[numPins] = {2, 3, 4, 5, 6, 7, 8, A0, 0, 1};

// Define an array to store the state of the solenoids
bool solenoidStates[10] = {LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW, HIGH}; // main faucet starts high
bool solenoidWorksDuringRandomMode[10] = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, LOW, LOW}; // if HIGH, random will influence the corresponding solenoid
int maximumSimultaneousSolenoids = 4; //sets how many solenoids will be on at most during random mode

// random variables
unsigned long previousMillis = 0;
long interval = 1000;
int minimalRandomSeconds = 40;
int maximumRandomSeconds = 60;

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

//debug settings:
#define debugInputMatrix  true
#define debugRadio        false
#define debugRandom       false
#define debugOutputs      true //caveat, if debugging, valves 9 and  10 will also read high, since they're writing serial data :)
#define debugOutputIntent false

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
  if (debugInputMatrix or debugOutputs or debugRandom or debugRadio or debugOutputIntent) {
    Serial.begin(115200);
  }
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
    //All off
    if (buttonMatrix[2][0]) {
      randomMode = false;
      for (int x = 0; x < 9; x++) {
        solenoidStates[x] = LOW;
      }
    }
    toggleSolenoids();
  }
  //reset the global check
  if (previousValueHigh == true && anyValueHigh == false) {
    previousValueHigh = false;
  }

  //drain feature
  if (buttonMatrix[2][1]) {
    solenoidStates[9] = LOW;
    digitalWrite(OutputPins[9], LOW);//toggle is an exclusion on updates too..
    solenoidStates[9] = LOW;
    //Serial.println("emergency shutoff");
    //delay(3000);
  }

  // Print the state of the buttons, for debugging
  if (debugInputMatrix) {
    for (int row = 0; row < numRows; row++) {
      for (int col = 0; col < numCols; col++) {
        Serial.print(buttonMatrix[row][col]);
        Serial.print('\t');
      }
      Serial.println();
    }
    Serial.println();
  }

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
    }
    else if (receivedmessage.state == RANDOM) {
      randomMode = !randomMode;
    }
    else if (receivedmessage.state == ALLOFF) {
      randomMode = !randomMode;
    }
    else {
      solenoidStates[receivedmessage.target] = (bool)receivedmessage.state;
    }
    if (debugRadio) {
      Serial.println("Received: " + String(receivedmessage.state) + ", " + String(receivedmessage.target));
    }
  }

  //write states to motors
  if (!randomMode) {
    //toggleSolenoids();
    for (int x = 0; x < 10; x++) {
      digitalWrite(OutputPins[x], solenoidStates[x]);

    }
  } else if (millis() - previousMillis >= interval) {
    previousMillis = millis();
    int toggledSolenoids = 0;
    for (int x = 0; (x <= 7) && (toggledSolenoids < maximumSimultaneousSolenoids); x++) { //randomise the 8 valves
      bool state = random(0, 2);
      if (solenoidWorksDuringRandomMode[x]) {
        digitalWrite(OutputPins[x], state);
        if (state) {
          toggledSolenoids++;
        };
      }
    }
    interval = random(minimalRandomSeconds, maximumRandomSeconds) * 1000;
  }
  if (debugOutputIntent) {
    Serial.print("Output intents: ");
    for (int x = 0; x < 10; x++) {
      Serial.print(solenoidStates[x]);
    }
    Serial.print('\n');
  }
  if (debugOutputs) {
    Serial.print("Actual outputs: ");
    for (int x = 0; x < 10; x++) {
      Serial.print(digitalRead(OutputPins[x]));
    }
    Serial.print('\n');
  }

  Serial.println("\n--------------------------------------------------");
  // Wait for a short duration before reading the buttons again
  delay(100);
}
