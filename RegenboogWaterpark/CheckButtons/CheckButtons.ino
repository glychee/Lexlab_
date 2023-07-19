// Define the number of rows and columns in the button matrix
const int numRows = 3;
const int numCols = 4;

// Define the pin mappings for rows and columns
const int rowPins[numRows] = {A5, A6, A7};
const int colPins[numCols] = {A1, A2, A3, A4};

// Define a matrix to store the state of the buttons
int buttonMatrix[numRows][numCols] = {
  {LOW, LOW, LOW, LOW},
  {LOW, LOW, LOW, LOW},
  {LOW, LOW, LOW, LOW}
};

void setup() {
  // Initialize the serial communication
  Serial.begin(115200);

  // Set the row pins as output
  for (int row = 0; row < numRows; row++) {
    pinMode(rowPins[row], INPUT_PULLUP);
  }

  // Set the column pins as input with internal pull-up resistors enabled
  for (int col = 0; col < numCols; col++) {
    pinMode(colPins[col], OUTPUT);
    digitalWrite(colPins[col], HIGH);
  }
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
        delay(5);
      } else {
        buttonMatrix[row][col] = !digitalRead(rowPins[row]);
      }

      // Deactivate the row
      digitalWrite(colPins[col], HIGH);
    }
  }

  // Print the state of the buttons
  for (int row = 0; row < numRows; row++) {
    for (int col = 0; col < numCols; col++) {
      Serial.print(buttonMatrix[row][col]);
      Serial.print('\t');
    }
    Serial.println();
  }
  Serial.println();

  // Wait for a short duration before reading the buttons again
  delay(100);
}
