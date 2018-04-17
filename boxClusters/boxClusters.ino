//TX version: 7 sensors per board, 4 boards controlling 28 keys, sending serial to WAV trigger.  All unused piezo inputs grounded out.
//Need to understand how to speed up and slow down animations, as well as how to modify code to work with the 7 sensors and to create a seamless animation.
//(right now, when you tap one, it has a ripple effect that makes the boxes next to it light up temporarily.  This does not appear to be seamless between the boards.
//I have left ?? comments next to the lines I have been working on.


// MODIFY THESE when flashing Arduinos:
#define boxClusterNumber (1)
#define animationType    (2)  // Range is [0...5]

// Animation types:
//      0 == Purple drum pads
//      1 == Washed out (white -> fade to pastels)
//      2 == 80's sweater
//      3 == Fast rainbow
//      4 == Slow rainbow, white pulsar
//      5 == Lava

// Cluster count:
#define CLUSTERS  (12)

// Boxes per cluster: (2018 PWLF was 8. TX version is 7.)
#define BOXES     (7)

#include "Keyboard.h"
#include <stdint.h>
#include <Adafruit_NeoPixel.h>
#include "Color.h"

#define PIN 2
//#define BAUD_RATE     (460800)
//#define BAUD_RATE     (115200) //was 115200, had to change for wav
#define BAUD_RATE     (57600) //was 115200, had to change for wav


// DEBUG_SERIAL and the rest must be __false__ for production!
#define DEBUG_SERIAL      (false)
#define DEBUG_PIEZO       (false)
#define DEBUG_HIT_VALUES  (false)

#define randf()    (random(0xffffff) * (1.0f / 0xffffff))

//#define ATTRACTOR_FADE_OUT_TIME       (1.6f)
//#define ATTRACTOR_HIDE_TIME           (4.5f)
//#define ATTRACTOR_FADE_IN_TIME        (3.5f)
#define ATTRACTOR_FADE_OUT_TIME       (0.3f) // how quickly the animation fades out after being triggered
#define ATTRACTOR_HIDE_TIME           (0.3f)  // amount of time the animation waits before coming back in ??
#define ATTRACTOR_FADE_IN_TIME        (0.7f)  //how long it takes to fade back in.  Speed of animations are controlled in color.h

// When hit: Effects fade out at this rate:
#define BRIGHTNESS_DECAY_RATE    (100.0f)   // Lower == faster decay

//uint16_t ledArray[]={0,15,30,45,60,75,90,105};  // PWLF boxes
uint16_t ledArray[]={0,62,124,186,248,310,372,434};  //for TX final designed board: this specifies the number of LEDs in each strip.
//uint16_t ledArray[]={0,1,2,3,4,5,6,7};  //test board: this specifies the number of LEDs in each strip.

// LED strip length: Match the number of LEDs specified in ledArray[]..
Adafruit_NeoPixel strip = Adafruit_NeoPixel(ledArray[BOXES], PIN, NEO_GRB + NEO_KHZ800); //was 521 for test board strips

// this fixes all the piezo locations for each box. They were randomly connected and have to be asigned to the correct box
int box[CLUSTERS][BOXES] = {
  {3,2,1,0,9,8,7}, //cluster 0  ground out pin 9, just to keep code logic straight
  {3,2,1,0,9,8,7},  //cluster 1  //only 4 boards used in the TX version.  will ground out pin 9 ??
  {0,1,2,3,9,8,7},  //cluster 2
  {0,1,2,3,9,8,7},  //cluster 3
  {0,1,2,3,9,8,7},  //cluster 4
  {0,1,2,3,9,8,7},  //cluster 5  //should I comment out the unused boards/hit values/etc.?  or just leave it be.. ??
  {0,1,2,3,9,8,7},  //cluster 6
  {0,1,2,3,9,8,7},  //cluster 7
  {0,1,2,3,6,7,8},  //cluster 8
  {0,1,2,3,6,7,8},  //cluster 9
  {0,1,2,3,6,7,8},  //cluster 10
  {0,1,2,3,6,7,8}   //cluster 11
};

// Default hit thresholds
#define HT0     ( 70)
#define HT1     ( 70)
#define HT2     ( 70)
#define HT3     ( 70)
#define HT4     ( 70)
#define HT5     ( 70)
#define HT6     ( 70)
#define HT7     ( 70)
#define HT8     ( 70)
#define HT9     ( 70)
#define HT10    ( 70)
#define HT11    ( 70)

int hitThreshold[CLUSTERS][BOXES] = {      //calibration if needed.. default was 20
	{ HT0, HT0, HT0, HT0, HT0, HT0, HT0}, //cluster 0
	{ HT1, HT1, HT1, HT1, HT1, HT1, HT1},  //cluster 1
	{ HT2, HT2, HT2, HT2, HT2, HT2, HT2},  //cluster 2
	{ HT3, HT3, HT3, HT3, HT3, HT3, HT3},  //cluster 3
	{ HT4, HT4, HT4, HT4, HT4, HT4, HT4},  //cluster 4
	{ HT5, HT5, HT5, HT5, HT5, HT5, HT5},  //cluster 5
	{ HT6, HT6, HT6, HT6, HT6, HT6, HT6},  //cluster 6
	{ HT7, HT7, HT7, HT7, HT7, HT7, HT7},  //cluster 7
	{ HT8, HT8, HT8, HT8, HT8, HT8, HT8},  //cluster 8
	{ HT9, HT9, HT9, HT9, HT9, HT9, HT9},  //cluster 9
	{HT10,HT10,HT10,HT10,HT10,HT10,HT10},  //cluster 10
	{HT11,HT11,HT11,HT11,HT11,HT11,HT11}  //cluster 11
};

int sensorValue = 0;        // value read from the pot

HSV hsvRandoms[BOXES];
float brights[BOXES] = {0.0f};

long piezoLastHit[BOXES] = {0};
long currentTime = 0;
const int waitBetweenHits = 100;

float elapsed = 0.0f;
float attractor = 1.0f; // Attractor animation fades in/out as appropriate. 0==off, 1==on.
float attractorFadeOut = 0.0f;  // When a piezo is hit: Set this to fade out attractor.

void setup() {

	pinMode(0, OUTPUT);
	digitalWrite(0,LOW);

	pinMode(5,OUTPUT);   //receive / transmit pin... pull high to send
	digitalWrite(5, LOW);

	if (DEBUG_SERIAL || DEBUG_PIEZO || DEBUG_HIT_VALUES) {
		Serial.begin(BAUD_RATE);
	}

	Serial1.begin(BAUD_RATE);

	strip.begin();
	strip.show(); // Initialize all pixels to 'off'
}

void loop() {
	long lastTime = currentTime;
	currentTime = millis();

	float frameSeconds = (currentTime - lastTime) / 1000.0f;
	elapsed += frameSeconds;

	// There is a bug where `elapsed` accumulates so high, that it loses floating
	// point precision, and animations slow down, or freeze entirely :D
	// When elapsed is larger than some large-ish number of seconds: Flip back to 0.
	if (elapsed > (60 * 60 * 3.0f)) {	// OK to reset once every 3 hours?
		elapsed = 0.0f;
	}

	if (attractorFadeOut > 0.0f) {
		// Attractor is fading out / hiding
		attractor = max(0.0f, attractor - (frameSeconds / ATTRACTOR_FADE_OUT_TIME));
		attractorFadeOut -= frameSeconds;

	} else {
		// Attractor is fading in
		attractor = min(1.0f, attractor + (frameSeconds / ATTRACTOR_FADE_IN_TIME));
	}

	// For each box: Dim the colors according to the brights.
	// Update each LED color.
	boxColorsWillUpdate(boxClusterNumber, animationType, elapsed, attractor);

	for (uint8_t box = 0; box < BOXES; box++) {
		// Get the box color. See: Color.h
		RGB rgb = boxColor(boxClusterNumber, animationType, box, &hsvRandoms[box], brights[box], elapsed, attractor);

		RGB8 rgb8 = rgbToRGB8(rgb);

		//uint16_t stripNum = ledArray[box];
    //for (uint16_t stripIncrementer = 0; stripIncrementer < 65; stripIncrementer++){
		//strip.setPixelColor(stripIncrementer, rgb8.r, rgb8.g, rgb8.b); }
 //    strip.setPixelColor(stripNum, rgb8.r, rgb8.g, rgb8.b);
 //		strip.setPixelColor(stripNum + 1, rgb8.r, rgb8.g, rgb8.b);
 //   strip.setPixelColor(stripNum + 2, rgb8.r, rgb8.g, rgb8.b);
 //   strip.setPixelColor(stripNum + 3, rgb8.r, rgb8.g, rgb8.b);
 //   strip.setPixelColor(stripNum + 4, rgb8.r, rgb8.g, rgb8.b);
 //   strip.setPixelColor(stripNum + 5, rgb8.r, rgb8.g, rgb8.b);
 //   strip.setPixelColor(stripNum + 6, rgb8.r, rgb8.g, rgb8.b);
 //   strip.setPixelColor(stripNum + 7, rgb8.r, rgb8.g, rgb8.b);
 //   strip.setPixelColor(stripNum + 8, rgb8.r, rgb8.g, rgb8.b);
 //   strip.setPixelColor(stripNum + 9, rgb8.r, rgb8.g, rgb8.b);

		uint16_t stripNum = ledArray[box];
		uint16_t stripLast = ledArray[box+1]; // this enabled the 2nd light strip.  Repeat increment for each lightbox strip.
    for(uint16_t i = stripNum; i<stripLast; i++)
    {
      strip.setPixelColor(i, rgb8.r, rgb8.g, rgb8.b);
    }

		// Decay brightness
		brights[box] = max(0.0f, brights[box] - (1.0f / BRIGHTNESS_DECAY_RATE));
	}

	strip.show();

	if (DEBUG_PIEZO) {
		Serial.println(analogRead(box[boxClusterNumber][0]));
		return;
	}

	// Read values for each piezo sensor
	int hitValues[BOXES] = {0};
	int8_t bestBox = -1;
//3,0,7,9,6,8,2,1
	for (int8_t b = 0; b < BOXES; b++) {   //thomas changed this from 6 to 2, so its only scanning the first few analog inputs.  the leonardo might have a damaged analog pin?? pin 6 was freaking out for some reason
	//	for (uint8_t b = 7; b < 8; b++) {
		// Ensure enough time has passed between hits.
		if ((currentTime - piezoLastHit[b]) < waitBetweenHits) continue;

		hitValues[b] = analogRead(box[boxClusterNumber][b]);
    //delay(500);

		if (DEBUG_HIT_VALUES && (hitValues[b] >= 20)) {
			Serial.print(b);
			Serial.print(":");
			Serial.println(hitValues[b]);
		}

		if (hitValues[b] > hitThreshold[boxClusterNumber][b]) {
			if (bestBox < 0) {
				// This box is the first box with a strong hit.
				bestBox = b;
			} else if (hitValues[b] > hitValues[bestBox]) {
				// This box registered a stronger hit than the leading box.
				bestBox = b;
			}

			// Also: This box is disabled from receiving another hit for 100ms or so.
			piezoLastHit[b] = currentTime;
		}
	}

	if (bestBox >= 0) {
  Serial.println(bestBox);
		processHit(bestBox, hitValues[bestBox]);
	}

}

void processHit(uint8_t activeBox, int sensorValue)
{
	// Take control of the data line (RS485 differential pair)
	digitalWrite(5, HIGH);

	Serial1.print(boxClusterNumber + 10);
	Serial1.print(",");
	Serial1.println(activeBox);

	if (DEBUG_SERIAL) {
		Serial.print(boxClusterNumber + 10);
		Serial.print(",");
		Serial.println(activeBox);
		Serial.println(sensorValue);
	}

	// Relinquish control of the data line
	Serial1.flush();

	if (DEBUG_SERIAL) {
		Serial.flush();
	}

	digitalWrite(5, LOW);

	startBoxAnimation(activeBox);
}

// Fill the dots one after the other with a color
void startBoxAnimation(uint8_t b) {
	// Fade out the attractor. Pad with time for the attractor to stay hidden.
	attractorFadeOut = ATTRACTOR_FADE_OUT_TIME + ATTRACTOR_HIDE_TIME;

	//colors[b] = Wheel(random(0, 255));
	hsvRandoms[b] = (HSV){.h = randf(), .s = randf(), .v = randf()};
	brights[b] = 1.0f;  // Start at full brightness

	boxColorWasHit(boxClusterNumber, animationType, b);
}


// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
	WheelPos = 255 - WheelPos;
	if(WheelPos < 85) {
		return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
	}
	if(WheelPos < 170) {
		WheelPos -= 85;
		return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
	}
	WheelPos -= 170;
	return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
