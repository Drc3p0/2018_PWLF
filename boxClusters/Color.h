#include <stdint.h>
#include <math.h>

#define ATTRACT_MODE_SPEED    (5.0)

#define lerp(a,b,v)    ((a) + ((b) - (a)) * (v))

#ifndef randf
#define randf()    (random(0xffffff) * (1.0f / 0xffffff))
#endif
//For TX: need to speed up animations to account for added LEDs per unit.
// From PWLF: Stations should be arranged in this order (as viewed from inside the arc, facing out):
//
//              6,7
//             -----
//      2,3/    4,5     \10,11
//        /0,1        8,9\.
//       /                \.

/*
const uint8_t PROGMEM gamma8[] = {
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
		0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
		1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
		2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
		5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
	 10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
	 17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
	 25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
	 37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
	 51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
	 69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
	 90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
	115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
	144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
	177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
	215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };
*/

const uint8_t PROGMEM gamma8[] = {
	0,   0,   0,   0,   0,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
	1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
	1,   2,   2,   2,   2,   2,   2,   2,   2,   3,   3,   3,   3,   3,   4,   4,
	4,   4,   4,   5,   5,   5,   5,   6,   6,   6,   6,   7,   7,   7,   7,   8,
	8,   8,   9,   9,   9,  10,  10,  10,  11,  11,  12,  12,  12,  13,  13,  14,
 14,  15,  15,  15,  16,  16,  17,  17,  18,  18,  19,  19,  20,  20,  21,  22,
 22,  23,  23,  24,  25,  25,  26,  26,  27,  28,  28,  29,  30,  30,  31,  32,
 33,  33,  34,  35,  36,  36,  37,  38,  39,  40,  40,  41,  42,  43,  44,  45,
 46,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,
 61,  62,  63,  64,  65,  67,  68,  69,  70,  71,  72,  73,  75,  76,  77,  78,
 80,  81,  82,  83,  85,  86,  87,  89,  90,  91,  93,  94,  95,  97,  98,  99,
101, 102, 104, 105, 107, 108, 110, 111, 113, 114, 116, 117, 119, 121, 122, 124,
125, 127, 129, 130, 132, 134, 135, 137, 139, 141, 142, 144, 146, 148, 150, 151,
153, 155, 157, 159, 161, 163, 165, 166, 168, 170, 172, 174, 176, 178, 180, 182,
184, 186, 189, 191, 193, 195, 197, 199, 201, 204, 206, 208, 210, 212, 215, 217,
219, 221, 224, 226, 228, 231, 233, 235, 238, 240, 243, 245, 248, 250, 253, 255 };

// RGB: Range of each channel is 0.0...1.0
struct RGB {
	float r;  // red
	float g;  // green
	float b;  // blue
};

struct RGB8 {
	uint8_t r;
	uint8_t g;
	uint8_t b;
};

// Ranges: 0.0...1.0
struct HSV {
	float h;  // hue. Range: 0..1
	float s;  // saturation
	float v;  // value (brightness)
};

HSV cells[8];

inline float saturate(const float f) {
	return constrain(f, 0.0f, 1.0f);
}

inline RGB addRGBclamped(const RGB * c0, const RGB * c1) {
	return (RGB){
		.r = saturate(c0->r + c1->r),
		.g = saturate(c0->g + c1->g),
		.b = saturate(c0->b + c1->b)
	};
}

inline RGB lerpRGB(const RGB * c0, const RGB * c1, float v) {
	return (RGB) {
		.r = lerp(c0->r, c1->r, v),
		.g = lerp(c0->g, c1->g, v),
		.b = lerp(c0->b, c1->b, v),
	};
}

RGB hsv2rgb( float hF, float sF, float vF ) {
	// Expand H by 6 times for the 6 color regions. Wrap the remainder to 0..1
	float hWrap = (hF*6.0);
	hWrap = hWrap - floor(hWrap);  // floating point remainder. 0..1 for each of the 6 color regions

	float v = vF;  // top (max) value
	float p = vF * (1.0-sF);  // bottom (min) value
	float q = vF * (1.0-(hWrap*sF));  // falling (yellow->green: the red channel falls)
	float t = vF * (1.0 - ((1.0-hWrap)*sF));  // rising (red->yellow: the green channel rises)

	// Need to find the correct color region that the hue belongs to.
	// Hue can have a negative value, so need to step it to positive space, so the modulus % operator behaves as expected.
	float hF_pos = hF;
	if( hF_pos < 0.0 ) {
		hF_pos += ceil(-hF_pos);
	}
	uint8_t hue_i = (uint8_t)(floor( hF_pos * 6.0 )) % 6;

	switch( hue_i ) {
		case 0:  return (RGB){v,t,p};  // red -> yellow
		case 1:  return (RGB){q,v,p};  // yellow -> green
		case 2:  return (RGB){p,v,t};  // green -> cyan
		case 3:  return (RGB){p,q,v};  // cyan -> blue
		case 4:  return (RGB){t,p,v};  // blue -> magenta
		case 5:  return (RGB){v,p,q};  // magenta -> red
	}

	return (RGB){0,0,0};  // sanity escape
};

// Using a sawtooth signal: Generate a triangle wave with variable pulse width / skew.
// v:           assumes range [0..1]
// pulseWidth:  assumes range [0..1], 0.5 is average
// Output is in range [0..1..0]
inline float tri(float v, float pulseWidth) {
	return (v < pulseWidth) ? (v / pulseWidth) : ((1.0f - v) / (1.0f - pulseWidth));
}

inline float easeSine(float v) {
	return sin(v * M_PI_2);
}

inline float easeSineBlend(float v, float blend) {
	return lerp(v, easeSine(v), blend);
}

inline float easeQuad(float v) {
	float iv = 1.0f - v;
	return 1.0f - (iv * iv);
}

float attractDiagonal(uint8_t cluster, uint8_t box, float elapsed)
{
	// Clusters: Waves should appear to emit from center.
	// Odd-numbered clusters: reverse the box order.
	if (cluster & 0x1) {
		box = (box <= 3) ? (box + 4) : (box - 4);
	}

	//uint8_t step = (box <= 3) ? (box * 2) : ((7 - box) * 2 + 1);  // zip zag
	const uint8_t ar[] = {0,3,6,1, 4,7,2,5};
	uint8_t step = ar[box];

	float saw = (step / 8.0f) + elapsed;
	return saw - floor(saw);
}

float attractWalk(uint8_t box, float elapsed)
{
	//uint8_t step = (box <= 3) ? (box * 2) : ((7 - box) * 2 + 1);  // zip zag
	const uint8_t ar[] = {0,1,2,3,4,5,6,7};
	uint8_t step = ar[box];

	float saw = (step / 8.0f) + elapsed;
	return saw - floor(saw);
}

bool isAnimDrums(uint8_t animType) {
	return (animType == 0);
}

void boxColorWasHit(uint8_t cluster, uint8_t animType, uint8_t box) {
	if (isAnimDrums(animType)) {
		bool greenish = (random(999) % 2);

		// Pick a warm hue
		float h = randf();
		if (greenish) {
			h = lerp(0.46f, 0.5f, randf());
		} else {
			h = (h - 0.15f) * (1.0f / 6.0f);
		}

		float s = 0.0f;
		if (greenish) {
			s = lerp(0.6f, 0.8f, randf());
		} else {
			s = lerp(0.7f, 0.9f, randf());
		}

		cells[box] = (HSV){
			.h = h,
			.s = s * 0.5f,
			.v = 1.0f
		};
	}
}

void boxColorsWillUpdate(uint8_t cluster, uint8_t animType, float elapsed, float attractor) {
	if (isAnimDrums(animType)) {

		const float DECAY = 0.05f; //.003f

		for (uint8_t c = 0; c < 8; c++) {
			cells[c].v = max(0.0f, cells[c].v - DECAY);
		}
	}
}

// cluster:    ID number of this cluster. Determines which animation is shown.
// box:        ID number of this box. Always [0-7].
// rando:      Random HSV value. Rerolled per piezo hit per box.
// bright:     Decaying envelope, triggered by piezo hit. No hits == 0.
// elapsed:    Global elapsed time, in seconds.
// attractor:  0..1 value indicating if the attractor is being shown.
RGB boxColor(uint8_t cluster, uint8_t animType, uint8_t box, HSV * rando, float bright, float elapsed, float attractor)
{
	//cluster = 99;	// DEBUG

	// Attempting to debug cluster==0, why no active animation? Hmm
	RGB rgbActive = {.r = 0, .g = 0, .b = 0};
	RGB rgbAttractor = {.r = 0, .g = 0, .b = 0};

	switch (animType) {

		case 0:
		{
			//
			//  2x2 drum pads
			//
			HSV * hsv = &cells[box];

			// The other cells in the 2x2 group: add some noise to the brightness
			float sizzle = hsv->v;
			if (hsv->s >= 0.66f) {
				sizzle *= lerp(0.93f, 1.0f, randf());
			}

			rgbActive = hsv2rgb(hsv->h, hsv->s, sizzle);

			// Slow walk?
			rgbAttractor = (RGB){.r = 0, .g = 0, .b = 0};
			if (hsv->v <= 0.0f) {
				float walk = attractWalk(box, elapsed * 0.45f * ATTRACT_MODE_SPEED);
				walk = saturate(walk * 2.0f - 1.0f);
				walk = tri(walk, 0.2f);
				walk = easeSine(walk);
				if (walk < 0.25f) walk = 0.0f;  // Prevent ugly artifacts at bottom of gamma curve
				rgbAttractor = hsv2rgb(0.0f, 1.0f, 0.4f * attractor * walk);
			}

			return addRGBclamped(&rgbActive, &rgbAttractor);
		}
		break;

		case 1:
		{
			//
			//  Washed out (white -> fade to pastels)
			//
			rgbActive = hsv2rgb(rando->h, 1.0 - bright, bright);

			// Attractor
			float attractHue = (elapsed / 40.0f) + (box ^ 0b11) * 0.08f;

			float wave = attractDiagonal(cluster, box, (elapsed / 4.0f) * ATTRACT_MODE_SPEED);
			wave = saturate(wave * 2.0f - 1.0f);
			wave = tri(wave, 0.05f);
			wave = easeSine(wave);
			if (wave < 0.25f) wave = 0.0f;

			rgbAttractor = hsv2rgb(attractHue, (1.0f - wave) * 0.5f, wave * 0.45f * attractor);

			return addRGBclamped(&rgbActive, &rgbAttractor);
		}
		break;

		case 2:
		{
			//
			//  80's sweater: Lime green, cyan, dark blue, magenta
			//
			float speedUp = lerp(0.65, 0.85f, rando->s);
			float fastFade = max(0.0f, bright * (speedUp + 1.0f) - speedUp);

			float h = lerp(0.84f, 0.667f, rando->h * rando->h);
			if (rando->v < 0.15f) {
				h = lerp(0.40f, 0.46f, rando->h);
			}

			// Reroll if it's cyan
			if (abs(h - 0.5f) < 0.09) {
				float hBig = rando->h * 54.3764362;
				float h = lerp(0.44f, 0.86f, hBig - floor(hBig) );
			}

			// Let's try a weird attack: Start at ~0.5 brightness, then decay.
			float b2 = fastFade * fastFade;
			float b4 = b2 * b2;

			// If the color is pink then it's LASER TIME
			const float LASER_TIME = 0.97f;
			float laser = 1.0f;
			if ((h >= 0.785f) && (fastFade >= LASER_TIME)) {
				float v_wrap = (1.0f - fastFade) / (1.0f - LASER_TIME);
				v_wrap *= 3.0f;
				v_wrap -= floor(v_wrap);
				laser = 1.0f - tri(v_wrap, 0.5f);
				laser *= laser;
				laser = lerp(0.45f, 1.0f, laser);
			}

			h += b4 * 0.075f;

			// I want some warble too
			float warble = (cos((rando->v + 0.9f) * (1.0f - bright) * 80.0f) + 1.0f) * 0.5f;
			float v = lerp(fastFade, warble, 0.12f);

			rgbActive = hsv2rgb(h, 1.0f, v * fastFade * laser);

			// Attractor
			float elapsedRando = (cluster & 0x1) ? (elapsed * 1.29f) : elapsed;
			float ch = cos(elapsedRando + (box ^ 57));
			float attractHue = 0.85f + (ch * ch) * 0.09f;  // green towards blue

			float wave = attractDiagonal(cluster, box, (elapsed / 4.0f) * ATTRACT_MODE_SPEED);
			wave = saturate(wave * 2.0f - 1.0f);
			wave = tri(wave, 0.05f);
			wave = easeSine(wave);
			if (wave < 0.25f) wave = 0.0f;

			rgbAttractor = hsv2rgb(
				attractHue,
				1.0f,
				wave * 0.45f * attractor * (1.0f - fastFade)
			);

			return addRGBclamped(&rgbActive, &rgbAttractor);
		}
		break;

		case 3:
		{
			//
			//  Fast rainbow-
			//
			float h = rando->h + bright * 1.6f;

			const float ATTACK_AMT = 0.955f;
			if (bright > ATTACK_AMT) {
				float attack = (bright - ATTACK_AMT) * (1.0f / (1.0f - ATTACK_AMT));
				attack *= attack; // easing
				h -= attack * 0.5f;
			}
			rgbActive = hsv2rgb(h, 1.0, bright);

			/*
			// Attractor: Checkerboard pattern
			float wave = attractDiagonal(cluster, (box & 0x1) ? 11 : 0, elapsed * 0.75f);
			wave = saturate(easeSineBlend(wave, 0.5f) * 2.0f - 0.8f);

			// Prevent ugly gamma artifacts at the bottom
			if (wave < 0.4f) wave = 0.0f;
			*/

			// Slow walk?
			float walk = attractWalk(box, elapsed * 0.42f * ATTRACT_MODE_SPEED);
			walk = saturate(walk * 2.0f - 1.0f);
			walk = tri(walk, 0.2f);
			walk = easeSine(walk);
			if (walk < 0.25f) walk = 0.0f;

			float attractHue = (elapsed / 13.0f);

			rgbAttractor = hsv2rgb(
				attractHue,
				0.50f,
				walk * attractor * 0.375f
			);

			return addRGBclamped(&rgbActive, &rgbAttractor);
		}
		break;

		case 4:
		{
			//
			//  Slow rainbow with glowing white LFO
			//
			float glow = (cos((1.0f - bright) * rando->s * 60.0f) + 1.0f) * 0.5f;
			float h = rando->h * 0.1f + (0.6667f - 0.05f) - glow * 0.1f + elapsed / 120.0f + sin(elapsed / 10.0f) * 0.5f; // Who knows what this does?? lol

			// Envelope should be shorter
			float env = constrain(bright * 2.0f - 1.0f, 0.0f, 1.0f);

			// More motion: Sync with the h changes?
			float glowCos = (cos(bright * rando->s * 120.0f) + 3.0f) * 0.25f;
			float v = glowCos * env;

			rgbActive = hsv2rgb(
				h,
				lerp(1.0f, 0.3f, glow),
				v
			);

			// Attractor
			float attractHue = h + (box ^ 0b11) * 0.08f;

			float wave = attractDiagonal(cluster, box, (elapsed / 4.0f) * ATTRACT_MODE_SPEED);
			wave = saturate(wave * 2.0f - 1.0f);
			wave = tri(wave, 0.15f);
			wave = easeSine(wave);
			if (wave < 0.25f) wave = 0.0f;

			rgbAttractor = hsv2rgb(attractHue, (1.0f - wave) * 0.5f, wave * 0.4f * attractor);

			return addRGBclamped(&rgbActive, &rgbAttractor);
		}
		break;

		case 5:
		{
			//
			//  Red/orange/yellow fire and lava.
			//

			//  Hue is kinda perlin noise, sum of harmonics.
			const float AMP0 = 1.0f;
			const float AMP1 = 0.7f;
			const float AMP2 = 0.5f;
			float p = bright * 200.0f;
			float n = sin(rando->h * p) * AMP0 +
								sin(rando->s * p) * AMP1 +
								sin(rando->v * p) * AMP2;
			n = (n * (0.5f / (AMP0 + AMP1 + AMP2))) + 0.5f;

			float speedUp = lerp(1.0, 3.0f, rando->h);
			float fastFade = max(0.0f, bright * (speedUp + 1.0f) - speedUp);
			float decay = lerp(fastFade, fastFade * fastFade, 0.5f + rando->v * 0.5f);

			rgbActive = hsv2rgb(
				constrain(n * 0.16667f, 0.0f, 1.0f/6),
				1.0f,
				decay
			);

			// Attractor: Don't fade out when there are active boxes.
			// Lava should always be active.
			if (cluster & 0x1) elapsed -= 668.0f;	// the neighbor of the beast
			float timeGnar = (cluster & 0x1) ? 135.2f : 139.0f;
			timeGnar *= ATTRACT_MODE_SPEED;
			uint8_t boxGnar = (box ^ 37);
			float p2 = boxGnar * timeGnar + elapsed * 4.2f;

			float n2 = sin(p2) * AMP0 +
								 sin(p2 * 1.12f) * AMP2 +
								 sin(p2 * 1.31f) * AMP1;
			n2 = (n2 * (0.5f / (AMP0 + AMP1 + AMP2))) + 0.5f;

			rgbAttractor = hsv2rgb(
				constrain((1.0f - n2) * 0.05, 0.0f, 1.0f/12),
				lerp(0.75, 1.0f, n2),
				constrain((0.1f + n2 * 0.35f) * (1.0f - decay), 0.0f, 1.0f)
			);

			return addRGBclamped(&rgbActive, &rgbAttractor);
		}
		break;

		default:
		{
			// Debug: Shows the box order
			RGB attract = (RGB){
				.r = (box & 0x1) ? 1.0f : 0.0f,
				.g = (box & 0x2) ? 1.0f : 0.0f,
				.b = (box & 0x4) ? 1.0f : 0.0f
			};

			// Active: Show a rainbow
			RGB active = hsv2rgb(elapsed * 0.2f, 1.0f, 1.0f);

			return lerpRGB(&attract, &active, bright);
		}
		break;

	}

	return (RGB){0.0f, 0.0f, 0.0f};
}

// Yes, applies gamma correction
RGB8 rgbToRGB8(RGB rgb) {
	float r = constrain(rgb.r, 0.0f, 1.0f);
	float g = constrain(rgb.g, 0.0f, 1.0f);
	float b = constrain(rgb.b, 0.0f, 1.0f);

	uint8_t rx = (uint8_t)(r * 0xff);
	uint8_t gx = (uint8_t)(g * 0xff);
	uint8_t bx = (uint8_t)(b * 0xff);

	return (RGB8){
		.r = pgm_read_byte(&gamma8[rx]),
		.g = pgm_read_byte(&gamma8[gx]),
		.b = pgm_read_byte(&gamma8[bx])
	};
}
