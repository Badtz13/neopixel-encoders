#include "FastLED.h"

FASTLED_USING_NAMESPACE

// setup leds
#define LED_PIN 5
#define COLOR_ORDER GRB
#define NUM_LEDS 12
CRGB leds[NUM_LEDS];

// knob turning
static int aPin = 2;
static int bPin = 3;
volatile byte aFlag = 0;
volatile byte bFlag = 0;
volatile byte encoderPos = 0;
volatile byte oldEncPos = 0;
volatile byte reading = 0;

// knob pressing
#define freezePin 4
int solidVal;

// mode button
#define modeButton 6
int val;

// system config
volatile uint8_t BRIGHTNESS = 20;
int MODE = 4;
bool SOLID_ENABLED = false;

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

// setup
void setup()
{
    // setup led ring
    FastLED.addLeds<WS2812B, LED_PIN>(leds, NUM_LEDS);
    FastLED.setBrightness(BRIGHTNESS);

    // pinmode for knob
    pinMode(aPin, INPUT_PULLUP);
    pinMode(bPin, INPUT_PULLUP);
    attachInterrupt(0, PinA, RISING);
    attachInterrupt(1, PinB, RISING);
    pinMode(freezePin, INPUT_PULLUP);

    // mode change button
    pinMode(modeButton, INPUT_PULLUP);
}

// handler for mode knob down
void PinA()
{
    cli();
    reading = PIND & 0xC;
    if (reading == B00001100 && aFlag)
    {
        encoderPos--;
        bFlag = 0;
        aFlag = 0;
    }
    else if (reading == B00000100)
        bFlag = 1;
    sei();
}

// handler for mode knob down
void PinB()
{
    cli();
    reading = PIND & 0xC;
    if (reading == B00001100 && bFlag)
    {
        encoderPos++;
        bFlag = 0;
        aFlag = 0;
    }
    else if (reading == B00001000)
        aFlag = 1;
    sei();
}

// setup mode list
typedef void (*modeList[])();
modeList gPatterns = {fill, glitter, confetti, sinelon, white};

uint8_t gHue = 0; // rotating "base color" used by many of the patterns

void loop()
{
    solidVal = digitalRead(freezePin);
    val = digitalRead(modeButton);

    // check for mode button pressed
    if (solidVal == LOW)
    {
        SOLID_ENABLED = !SOLID_ENABLED;
        delay(1000);
    }
    // check for mode button pressed
    if (val == LOW)
    {
        MODE = (MODE + 1) % ARRAY_SIZE(gPatterns);
        delay(1000);
    }

    // check for mode knob turned
    if (oldEncPos != encoderPos)
    {
        FastLED.setBrightness(encoderPos);
        oldEncPos = encoderPos;
    }

    // call current mode
    gPatterns[MODE]();
    FastLED.show();

    // if solid, disable hue change
    if (!SOLID_ENABLED)
    {
        EVERY_N_MILLISECONDS(20) { gHue++; }
    }
}

void fill()
{
    if (!SOLID_ENABLED)
    {
        fill_rainbow(leds, NUM_LEDS, gHue, 14);
    }
    else
    {
        fill_solid(leds, NUM_LEDS, CHSV(gHue, 255, 192));
    }
}

void glitter()
{
    fill();
    addGlitter(80);
}

void addGlitter(fract8 chanceOfGlitter)
{
    if (random8() < chanceOfGlitter)
    {
        leds[random16(NUM_LEDS)] += CRGB::White;
    }
}

void confetti()
{
    fadeToBlackBy(leds, NUM_LEDS, 10);
    int pos = random16(NUM_LEDS);
    leds[pos] += CHSV(gHue + random8(64), 200, 255);
}

void sinelon()
{
    fadeToBlackBy(leds, NUM_LEDS, 20);
    int pos = beatsin16(13, 0, NUM_LEDS);
    leds[pos] += CHSV(gHue, 255, 192);
}

void white()
{
    fill_solid(leds, NUM_LEDS, CRGB::White);
}
