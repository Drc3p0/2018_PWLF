
/*
   This examples shows how to make a simple seven keys MIDI keyboard with volume control

   Created: 4/10/2015
   Author: Arturo Guadalupi <a.guadalupi@arduino.cc>
   
   http://www.arduino.cc/en/Tutorial/MidiDevice
*/

#include "MIDIUSB.h"
#include "PitchToNote.h"
#define NUM_BUTTONS  7


const int MAX_LEN = 10;
const char lineEnding = '\n'; // whatever marks the end of your input.
char inputSentence [MAX_LEN + 1];
int inputIndex;
bool newInput;

const byte MAX_TOKENS = 3;
const char* delimiters = ", "; // whatever characters delimit your input string
char* tokens [MAX_TOKENS + 1];
enum indexName {box, pixel};
#define PRINT_ITEM(x) printItem (x, #x)





const byte notePitches[][8] = {
  {C3, D3, E3, F3, G3, A3, B3, B4},
  {C4, D4, E4, F4, G4, A4, B4, B5}
};

uint8_t notesTime[NUM_BUTTONS];
uint8_t pressedButtons = 0x00;
uint8_t previousButtons = 0x00;
uint8_t intensity = 100;

int inByte = 0;
int lastInByte = 0;

void setup() {
  
  pinMode(5,OUTPUT);   //receive / transmit pin... pull low to receive
  digitalWrite(5, LOW);
  
  // initialize both serial ports:
  Serial.begin(9600);
  Serial1.begin(9600);
}


void loop() {

  serialEvent();

  if (newInput && strlen (inputSentence))
  {
    int tokenIndex = 0;
    //Serial.println (inputSentence); // tell 'em what you've got
    tokens [tokenIndex] = strtok (inputSentence, delimiters);
    while ((tokenIndex < MAX_TOKENS - 1) && tokens [tokenIndex])
    {
      tokenIndex++;
      tokens [tokenIndex] = strtok (NULL, delimiters);
    }
   
    PRINT_ITEM (box);
    PRINT_ITEM (pixel);


    int boxNumber = atoi(tokens[0]) - 10;
    int pixelNumber = atoi(tokens[1]);
    Serial.println(boxNumber);
    Serial.println(pixelNumber);
    Serial.println(notePitches[boxNumber][pixelNumber]);

    noteOn(1, notePitches[boxNumber][pixelNumber], intensity);

  


   

   //Serial.println(tokens[0]);
   //Serial.println(tokens[1]);


   // reset things for the next lot.
   newInput = false;
   inputIndex = 0;
   inputSentence [0] = '\0';
 }

 
  // read from port 1, send to port 0:

//  Serial.println("Sending note on");
//  noteOn(0, 48, 64);   // Channel 0, middle C, normal velocity
//  MidiUSB.flush();
//  delay(500);
//  Serial.println("Sending note off");
//  noteOff(0, 48, 64);  // Channel 0, middle C, normal velocity
//  MidiUSB.flush();
//  delay(1500);
  
//  if (Serial1.available()) {
//    inByte = Serial1.read();
//    Serial.write(inByte);
//    delay(1000);
//  }
//  
//
//  if(inByte != lastInByte)
//  {
//    lastInByte = inByte;
//    int number = random(0,8);
//    noteOn(1, notePitches[number], intensity);
//    MidiUSB.flush();
//  }
}

void serialEvent ()  // build the input string.
{
 while (Serial1.available() ) //read from hardware serial
 {
   char readChar = Serial1.read ();
   if (readChar == lineEnding)
   {
     newInput = true;
   }
   else
   {
     if (inputIndex < MAX_LEN)
     {
       inputSentence [inputIndex++] = readChar;
       inputSentence [inputIndex] = '\0';
     }
   }
 }
}

void printItem (int index, char* name)
{
 Serial.print (name);
 Serial.print (F(" "));
 Serial.println (tokens [index]);  
}


// First parameter is the event type (0x0B = control change).
// Second parameter is the event type, combined with the channel.
// Third parameter is the control number number (0-119).
// Fourth parameter is the control value (0-127).

void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
}


// First parameter is the event type (0x09 = note on, 0x08 = note off).
// Second parameter is note-on/note-off, combined with the channel.
// Channel can be anything between 0-15. Typically reported to the user as 1-16.
// Third parameter is the note number (48 = middle C).
// Fourth parameter is the velocity (64 = normal, 127 = fastest).

void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
}