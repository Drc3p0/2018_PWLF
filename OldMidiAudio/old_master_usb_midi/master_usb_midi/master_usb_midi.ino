
/*
changed from teensy to lenoardo: https://www.arduino.cc/en/Tutorial/MidiDevice
*/

#include "PitchToNote.h"

#include "MIDIUSB.h" //for lenoardo

#include <wavTrigger.h>  //https://github.com/robertsonics/WAV-Trigger-Arduino-Serial-Library
wavTrigger wTrig;             // Our WAV Trigger object




#define BAUD_RATE  (57600)

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

// Box->pitch order can be changed here:
const byte notePitches[8] = {0, 1, 2, 3, 4, 5, 6, 7};


int inByte = 0;
int lastInByte = 0;

void setup() {

  pinMode(5,OUTPUT);   //receive / transmit pin... pull low to receive for Master
  digitalWrite(5, LOW);

  // initialize both serial ports:
  Serial.begin(BAUD_RATE);
  Serial1.begin(BAUD_RATE);

//Wav info:
  wTrig.start();   // WAV Trigger startup at 57600
    // If the Uno is powering the WAV Trigger, we should wait for the WAV Trigger
  //  to finish reset before trying to send commands.
  delay(1000);
  // If we're not powering the WAV Trigger, send a stop-all command in case it
  //  was already playing tracks. If we are powering the WAV Trigger, it doesn't
  //  hurt to do this.
  wTrig.stopAllTracks();

  Serial.println("are you here");
  
}

void loop() {

  serialEvent(); //RS485 incoming

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
    int pitch = (boxNumber * 8) + notePitches[pixelNumber];

    Serial.println(boxNumber);
    Serial.println(pixelNumber);
    Serial.print("pitch: ");
    Serial.println(pitch - 7);
    
    wTrig.trackGain(1, 0);                 //sets track 1 gain
    wTrig.trackPlayPoly(pitch - 7);

// First parameter is the event type (0x09 = note on, 0x08 = note off).
// Second parameter is note-on/note-off, combined with the channel.
// Channel can be anything between 0-15. Typically reported to the user as 1-16.
// Third parameter is the note number (48 = middle C).
// Fourth parameter is the velocity (64 = normal, 127 = fastest).


  midiEventPacket_t noteOn = {0x09, 0x90 | 0, pitch, 99}; //added for lenoardo
  MidiUSB.sendMIDI(noteOn); //added for leonardo
  MidiUSB.flush();

    //usbMIDI.sendNoteOn(pitch, 99, 1);  //for teensy

   // reset things for the next lot.
   newInput = false;
   inputIndex = 0;
   inputSentence [0] = '\0';
 }
}



void serialEvent ()  // build the input string.
{
 while (Serial1.available() ) //read from hardware serial
 {
   char readChar = Serial1.read ();
   Serial.println(readChar);
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
