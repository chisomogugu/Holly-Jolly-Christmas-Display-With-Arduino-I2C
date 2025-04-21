#include <Arduino.h>
#include <IRremote.hpp>
#include <Wire.h>

// Define Slave Setup
#define SLAVE_ADDRESS 8
#define SLAVE_ADDRESS1 9
#define SLAVE_ADDRESS2 7
#define ANSWERSIZE 4

// Button setup
#define BUTTON_PIN1 2
volatile byte buttonState = LOW;

// IR receiver setup  
const int RECV_PIN = 4;
IRrecv irrecv(RECV_PIN);
bool isSystemOn = false;         // Current system state
bool lastSentState = false;      // Last sent state to the slave
unsigned long received_val = 0;
int song = 1;

// Photoresistor setup
int pResistor = A0;  // Photoresistor input
unsigned long p_value;

// Timer setup
int interval = 1000;  // 1-second interval
unsigned long start = 0;
bool auto_light = true;

// New variables for handling countdown completion
bool countdownComplete = false;
bool songStarted = false;
unsigned long lastCheckTime = 0;
const int CHECK_INTERVAL = 1000;  // Check countdown status every second

void setup() {
    Serial.begin(9600);
    Wire.begin();  // Join I2C bus as master
    irrecv.enableIRIn();  // Start the IR receiver
}

void loop() {
    unsigned long currentTime = millis();
    
    // Always process IR signals first
    IRremote_switch();
    
    // Check status less frequently (every 5 seconds instead of every second)
    if (isSystemOn && !songStarted && (currentTime - lastCheckTime >= 5000)) {
        String response = slaveResponse();
        
        if (response == "Done") {
            Serial.println("Countdown complete, starting song playback");
            send_song_message("ON");
            send_display_message("ON");
            songStarted = true;
            Serial.println("End on ON");
        }
        lastCheckTime = currentTime;
    }

    // Check photoresistor at its interval
    if (auto_light && (currentTime - start >= interval)) {
        p_value = analogRead(pResistor);
        check_lights(p_value);
        start = currentTime;
    }
    
    // Reduced delay to make system more responsive
    delay(50);
}

// Function to handle light-based system control
void check_lights(unsigned long value) {
    if(value < 600){
        if(!isSystemOn){  // System turns on at Dark
            Serial.print("Photoresistor value: ");
            Serial.println(p_value);
            isSystemOn = !isSystemOn;
            system_state_changed();  // Use new function to handle state change
        }
    }
}

// New function to handle system state changes
void system_state_changed() {
    if (isSystemOn) {
        send_message("ON");
        songStarted = false;  // Reset song state when system turns on
        countdownComplete = false;  // Reset countdown state
    } else {
        send_message("OFF");
        send_song_message("OFF");  // Turn off song Arduino
        send_display_message("OFF"); // Turn off display Arduino
        Serial.println("End Of OFF");
        songStarted = false;  // Reset song state
    }
}

void IRremote_switch(){
    if (irrecv.decode()) {
        unsigned long value = irrecv.decodedIRData.decodedRawData;
        Serial.println(value);

        if (value == 0xFFFFFFFF) {
            value = received_val;
            return;
        }

        switch (value) {
            case 0xFA05EF00:  // ON/OFF button
                isSystemOn = !isSystemOn;
                Serial.println(isSystemOn ? "System is ON" : "System is OFF");
                system_state_changed();  // Use new function to handle state change
                break;

            case 0xEC13EF00:  // Song 1 button
                if(isSystemOn && song != 1){
                    Serial.println("Play Song 1");
                    send_song_message("Change");
                    song = 1;
                }
                break;

            case 0xED12EF00:  // Song 2 button
                if(isSystemOn && song != 2){
                    Serial.println("Play Song 2");
                    send_song_message("Change");
                    song = 2;
                }
                break;

            case 0xFD02EF00:  // Auto_light setting
                auto_light = !auto_light;
                Serial.println(auto_light ? "Auto_Light is On" : "Auto_Light is off");
                break;

            default:
                break;
        }
        received_val = value;
        irrecv.resume();
    }
}

//Sends message to SLAVE_ADDRESS
void send_message(String text) {
    Serial.println("Sending to Countdown Slave: " + text);
    Wire.beginTransmission(SLAVE_ADDRESS);
    Wire.write(text.c_str());
    Wire.endTransmission();
}

//Sends message to SLAVE_ADDRESS1
void send_song_message(String text) {
    Serial.println("Sending to Song Slave: " + text);
    Wire.beginTransmission(SLAVE_ADDRESS1);
    Wire.write(text.c_str());
    Wire.endTransmission();
}

//Sends message to SLAVE_ADDRESS2
void send_display_message(String text) {
    Serial.println("Sending to Display Slave: " + text);
    Wire.beginTransmission(SLAVE_ADDRESS2);
    Wire.write(text.c_str());
    Wire.endTransmission();
}

String slaveResponse() {
    String response = "";
    Wire.requestFrom(SLAVE_ADDRESS, ANSWERSIZE);
    while (Wire.available()) {
        char c = Wire.read();
        response += c;
    }
    return response;
}