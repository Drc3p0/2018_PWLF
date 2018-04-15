// ****************************************************************************


// 3) Connect 2 wires from the UNO to the WAV Trigger's serial connector:
//
//    Uno           WAV Trigger
//    ===           ===========
//    GND  <------> GND
//    tx <------> RX
//
//    If you want to power the WAV Trigger from the Uno, then close the 5V
//    solder jumper on the WAV Trigger and connect a 3rd wire:
//
//    5V   <------> 5V

#include <AltSoftSerial.h>    // edited out softSerial... I think
//softSerial wasn't edited out...threw errors when commeneted out so pin 9 <---> RX on wavtrigger

#include <wavTrigger.h>  //https://github.com/robertsonics/WAV-Trigger-Arduino-Serial-Library

#define LED 13                // our LED

wavTrigger wTrig;             // Our WAV Trigger object


// ****************************************************************************
void setup() {
  
  // Serial monitor
  Serial.begin(9600);
 
  // Initialize the LED pin
  pinMode(LED,OUTPUT);
  
  // WAV Trigger startup at 57600
  wTrig.start();
  
  // If the Uno is powering the WAV Trigger, we should wait for the WAV Trigger
  //  to finish reset before trying to send commands.
  delay(1000);
  
  // If we're not powering the WAV Trigger, send a stop-all command in case it
  //  was already playing tracks. If we are powering the WAV Trigger, it doesn't
  //  hurt to do this.
  wTrig.stopAllTracks();
  
}


// ****************************************************************************

void loop() {
  
int i;
 wTrig.trackGain(1, 0);                 //sets track 1 gain
 wTrig.trackPlayPoly(1);  // Start Track 1 poly
 digitalWrite(LED, HIGH);
 delay(1000);
 //wTrig.trackStop(1);                   // Stop Track 1
 digitalWrite(LED, LOW);
 delay(1000);
 wTrig.trackPlayPoly(2);               // Start Track 2 poly
 digitalWrite(LED, HIGH);
 delay(1000);
 //wTrig.trackStop(2); // Stop Track 2
 digitalWrite(LED, LOW);
 delay(1000);
 wTrig.trackPlayPoly(3);               // Start Track 3 poly
 digitalWrite(LED, HIGH);
 delay(1000);
 //wTrig.trackStop(3); 
 digitalWrite(LED, LOW);
 delay(500);    
      
}
           
 


