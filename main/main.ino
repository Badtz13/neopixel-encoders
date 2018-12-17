#include "FastLED.h"

FASTLED_USING_NAMESPACE

// setup leds
#define LED_PIN 5
#define COLOR_ORDER GRB
#define NUM_LEDS 12
CRGB leds[NUM_LEDS];

// mode knob turning
static int modePinA = 2;
static int modePinB = 3;
volatile byte aFlag = 0;
volatile byte bFlag = 0;
volatile byte encoderPos = 0;
volatile byte oldEncPos = 0;
volatile byte reading = 0;

// mode knob pressing
#define buttonPin 8

// system defaults
bool SOLID_ENABLED = true;
uint8_t BRIGHTNESS = 10;
uint8_t MODE = 2;

// setup
void setup()
{
    // setup led ring
    FastLED.addLeds<WS2812B, LED_PIN>(leds, NUM_LEDS);
    FastLED.setBrightness(BRIGHTNESS);

    // pinmode for mode knob
    pinMode(modePinA, INPUT_PULLUP);
    pinMode(modePinB, INPUT_PULLUP);
    attachInterrupt(0, PinA, RISING);
    attachInterrupt(1, PinB, RISING);
    pinMode(buttonPin, INPUT_PULLUP);

    // start serial for debug
    Serial.begin(115200);
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
modeList gPatterns = {fill, glitter, confetti, sinelon, juggle};

uint8_t gHue = 0; // rotating "base color" used by many of the patterns

void loop()
{
    // check for mode button pressed
    if (digitalRead(buttonPin) == LOW)
    {
        Serial.println("PRESSED");
        SOLID_ENABLED = !SOLID_ENABLED;
        delay(500);
    }
    // check for mode knob turned
    if (oldEncPos != encoderPos)
    {
        Serial.println(encoderPos);
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

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern()
{
    // add one to the current pattern number, and wrap around at the end
    MODE = (MODE + 1) % ARRAY_SIZE(gPatterns);
}

void fill()
{
    fill_rainbow(leds, NUM_LEDS, gHue, 14);
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

void juggle()
{
    // eight colored dots, weaving in and out of sync with each other
    fadeToBlackBy(leds, NUM_LEDS, 20);
    byte dothue = 0;
    for (int i = 0; i < 3; i++)
    {
        leds[beatsin16(i + 7, 0, NUM_LEDS)] |= CHSV(dothue, 200, 255);
        dothue += 32;
    }
}
