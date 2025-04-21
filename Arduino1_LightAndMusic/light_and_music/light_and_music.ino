#include <Wire.h>

/*************************************************
     PITCHES
  *************************************************/
#define NOTE_B0  31
#define  NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define  NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define  NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define  NOTE_C2  65
#define NOTE_CS2 69qa
#define NOTE_D2  73
#define NOTE_DS2 78
#define  NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define  NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define  NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3  185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define  NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4  277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define  NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4  415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define  NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5  622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define  NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5  932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define  NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define  NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define  NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define  NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978

//I2C Communication
#define SLAVE_ADDRESS1 9
#define ANSWERSIZE 8

int jingle_bells_melody[] = {
  NOTE_E5, NOTE_E5, NOTE_E5,
  NOTE_E5, NOTE_E5, NOTE_E5,
  NOTE_E5, NOTE_G5, NOTE_C5, NOTE_D5,
  NOTE_E5,
  NOTE_F5, NOTE_F5, NOTE_F5, NOTE_F5,
  NOTE_F5, NOTE_E5, NOTE_E5, NOTE_E5, NOTE_E5,
  NOTE_E5, NOTE_D5, NOTE_D5, NOTE_E5,
  NOTE_D5, NOTE_G5
};

int jingle_bells_tempo[] = {
  8, 8, 4,
  8, 8, 4,
  8, 8, 8, 8,
  2,
  8, 8, 8, 8,
  8, 8, 8, 16, 16,
  8, 8, 8, 8,
  4, 4
};

int  wish_melody[] = {
  NOTE_B3, 
  NOTE_F4, NOTE_F4, NOTE_G4, NOTE_F4, NOTE_E4,
  NOTE_D4, NOTE_D4, NOTE_D4,
  NOTE_G4, NOTE_G4, NOTE_A4, NOTE_G4, NOTE_F4,
  NOTE_E4, NOTE_E4, NOTE_E4,
  NOTE_A4, NOTE_A4, NOTE_B4, NOTE_A4, NOTE_G4,
  NOTE_F4, NOTE_D4, NOTE_B3, NOTE_B3,
  NOTE_D4, NOTE_G4, NOTE_E4,
  NOTE_F4
};

int  wish_tempo[] = {
  4,
  4, 8, 8, 8, 8,
  4, 4, 4,
  4, 8, 8, 8, 8,
  4, 4, 4,
  4, 8, 8, 8, 8,
  4, 4, 8, 8,
  4, 4, 4,
  2
};

#define melodyPin 12
#define lights1_pin 2
#define lights2_pin 3

unsigned long prevNoteTime = 0;  // Time for the last note
unsigned long prevLEDTime = 0;   // Time for the last LED toggle
bool light1State = false;        // Current state of LED 1
int currentNote = 0;             // Current note being played
unsigned long currentTime;
int song = 1;

// System state management
bool isSystemOn = false;    // Tracks the state of the system
bool isPlaying = false;     // Tracks if music is currently playing


void setup() {
  Serial.begin(9600);
  Wire.begin(SLAVE_ADDRESS1);
  Wire.onReceive(receiveEvent);

  pinMode(lights1_pin, OUTPUT);
  pinMode(lights2_pin, OUTPUT);
  pinMode(melodyPin, OUTPUT);
  
  // Initialize all outputs to OFF state
  stopEverything();
}

void loop() {
  currentTime = millis();
  
  // Only play if system is ON
  if (isSystemOn) {
    play_song(song);
  }
}

// Function to immediately stop all outputs
void stopEverything() {
  // Stop the buzzer
  digitalWrite(melodyPin, LOW);
  // Turn off both lights
  digitalWrite(lights1_pin, HIGH);
  digitalWrite(lights2_pin, HIGH);
  // Reset playback variables
  currentNote = 0;
  prevNoteTime = currentTime;
  prevLEDTime = currentTime;
  light1State = false;
  isPlaying = false;
}

void play_song(int song_num) {
  int size;
  int* melody;
  int* tempo;
  
  // Select the appropriate song arrays
  if (song_num == 1) {
    size = sizeof(jingle_bells_melody) / sizeof(int);
    melody = jingle_bells_melody;
    tempo = jingle_bells_tempo;
  } else {
    size = sizeof(wish_melody) / sizeof(int);
    melody = wish_melody;
    tempo = wish_tempo;
  }

  if (currentNote < size) {
    isPlaying = true;
    int noteDuration = 2000 / tempo[currentNote];

    // Play the current note
    if (currentTime - prevNoteTime < noteDuration) {
      buzz(melodyPin, melody[currentNote], noteDuration);
    } else {
      buzz(melodyPin, 0, 0); // Turn off the buzzer
      currentNote++;
      prevNoteTime = currentTime;
    }

    // Handle LED effects
    if (currentTime - prevLEDTime >= noteDuration / 2) {
      prevLEDTime = currentTime;
      light1State = !light1State;
      digitalWrite(lights1_pin, light1State ? LOW : HIGH);
      digitalWrite(lights2_pin, light1State ? HIGH : LOW);
    }
  } else {
    // Reset when song completes
    currentNote = 0;
    prevNoteTime = currentTime;
    if (isPlaying) {
      stopEverything();
    }
  }
}

void buzz(int targetPin, long frequency, long length) {
  // Only produce sound if system is ON
  if (!isSystemOn) {
    digitalWrite(targetPin, LOW);
    return;
  }
  
  if (frequency > 0) {
    long delayValue = 1000000 / frequency / 2;
    long numCycles = frequency * length / 1000;
    for (long i = 0; i < numCycles; i++) {
      // Check if system turned OFF during long notes
      if (!isSystemOn) {
        digitalWrite(targetPin, LOW);
        return;
      }
      digitalWrite(targetPin, HIGH);
      delayMicroseconds(delayValue);
      digitalWrite(targetPin, LOW);
      delayMicroseconds(delayValue);
    }
  } else {
    digitalWrite(targetPin, LOW);
  }
}

void receiveEvent(int bytes) {
  char received[ANSWERSIZE];
  int i = 0;

  while (Wire.available() && i < sizeof(received)) {
    received[i++] = Wire.read();
  }
  received[i] = '\0';

  if (strcmp(received, "ON") == 0) {
    Serial.println("System turning ON");
    isSystemOn = true;
    // Don't reset currentNote so it can continue from where it was
  } 
  else if (strcmp(received, "OFF") == 0) {
    Serial.println("System turning OFF");
    isSystemOn = false;
    stopEverything();  // Immediately stop all outputs
  } 
  else if (strcmp(received, "Change") == 0) {
    Serial.print("Changing song from ");
    Serial.print(song);
    song = (song == 1) ? 2 : 1;
    Serial.print(" to ");
    Serial.println(song);
    
    // Reset playback state for new song
    currentNote = 0;
    prevNoteTime = currentTime;
    prevLEDTime = currentTime;
  }
}