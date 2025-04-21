#include <LiquidCrystal.h>
#include <Wire.h>

//I2C Communication
#define SLAVE_ADDRESS2 7
#define ANSWERSIZE 4

// Initialize the library by associating the LCD pins with Arduino pins
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// System state management
bool isSystemOn = false;    // Tracks if the display should be active

// Array of Christmas-themed messages split into two lines
const char* messages[][2] = {
  {"Have a Holly", "Jolly Christmas!"},
  {"Tis the season", "to be jolly"},
  {"Wishing you a", "Merry Christmas!"},
  {"Happy Holidays", "Everyone!!"},
  {"Ho Ho Ho!", "Santa's Coming!"}
};
const int numMessages = sizeof(messages) / sizeof(messages[0]);

// Timing variables for switching messages
unsigned long previousMillis = 0;
const long interval = 10000; // 10 seconds interval for each message
int currentMessage = 0;      // Index to track the current message

void setup() {
  Serial.begin(9600);
  // Initialize I2C
  Wire.begin(SLAVE_ADDRESS2);  // Fixed the address to SLAVE_ADDRESS2
  Wire.onReceive(receiveEvent);

  // Initialize the LCD
  lcd.begin(16, 2);
  
  // Start with a blank display
  clearDisplay();
}

void loop() {
  if (isSystemOn) {
    updateDisplay();
  }
}

// Function to update the display with messages
void updateDisplay() {
  unsigned long currentMillis = millis();

  // Check if it's time to switch to the next message
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Move to the next message
    currentMessage++;
    if (currentMessage >= numMessages) {
      currentMessage = 0;
    }

    // Display the new message
    displayCurrentMessage();
  }
}

// Function to display the current message
void displayCurrentMessage() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(messages[currentMessage][0]);
  lcd.setCursor(0, 1);
  lcd.print(messages[currentMessage][1]);
}

// Function to clear the display and reset states
void clearDisplay() {
  lcd.clear();
  currentMessage = 0;
  previousMillis = millis();
}

// Function to turn the display on
void turnDisplayOn() {
  if (!isSystemOn) {
    isSystemOn = true;
    Serial.println("Display turning ON");
    displayCurrentMessage();  // Show the current message immediately
  }
}

// Function to turn the display off
void turnDisplayOff() {
  if (isSystemOn) {
    isSystemOn = false;
    Serial.println("Display turning OFF");
    clearDisplay();  // Clear the display when turning off
  }
}

void receiveEvent(int bytes) {
  char received[ANSWERSIZE];
  int i = 0;

  while (Wire.available() && i < sizeof(received)) {
    received[i++] = Wire.read();
  }
  received[i] = '\0';  // Null-terminate the string

  Serial.print("Received command: ");
  Serial.println(received);

  if (strcmp(received, "ON") == 0) {
    turnDisplayOn();
  } 
  else if (strcmp(received, "OFF") == 0) {
    turnDisplayOff();
  }
}