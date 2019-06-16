// ==== XTC DAC sound stuff
#include "CROAK.h";
#include "XT_DAC_Audio.h";
XT_DAC_Audio_Class DacAudio(25,0); // Use GPIO 25, one of the 2 DAC pins and timer 0
XT_Wav_Class CROAK(CROAKWav);
uint32_t DemoCounter=0;

// ==== FastLED stuff
#include <FastLED.h>
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS 10
#define DATA_PIN 15
CRGB leds[NUM_LEDS];
#define FRAMES_PER_SECOND  60
uint8_t gHue = 0; // rotating "base color" used by many of the patterns

int period[2] = {500, 500};
unsigned long time_now[2] = {0, 0};
char toggle = 0;

// ==== Direct PWM stuff
const int bigLedPins[] = {2, 0, 4, 17, 18, 19}; //we don't use pin 5 because of high-output erratum upon POR.
const int motorPins[] = {16, 21};
const int pwmFreq = 25000; //above the audible frequency band so we can't hear any EMI through the speaker.
const int pwmRes = 8;
CRGB ledsDirect[2];
int currentLed;

void setup() 
{
  Serial.begin(115200);

  //==== FastLED stuff
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, 21, RGB>(ledsDirect, sizeof(ledsDirect)/sizeof(CRGB)).setCorrection(TypicalLEDStrip);
  pinMode(21, INPUT); //we're not using this pin so we don't need it toggling.

  //==== Direct PWM stuff
  int i;

  //LED pins as PWM outputs
  for(i=0; i<(sizeof(bigLedPins)/sizeof(const int)); i++)
  {
    ledcSetup(i, pwmFreq, pwmRes);
    ledcAttachPin(bigLedPins[i], i);
  }

  //motor pins as GPIO
  for(i=0; i<(sizeof(motorPins)/sizeof(const int)); i++)
  {
    digitalWrite(motorPins[i], 0);
    pinMode(motorPins[i], OUTPUT);
  }

  digitalWrite(motorPins[0], 1);
}

void loop() 
{
  //==== XTC DAC sound stuff
  DacAudio.FillBuffer();

  if(CROAK.Playing == false)
  {
    CROAK.Speed = random(50, 125) / 100.0;
    DacAudio.Play(&CROAK);
  }

  //==== light control stuff
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
  fill_rainbow( ledsDirect, 2, gHue, 7);

  FastLED.show();

  //show direct PWM output
  char* ledsDirectPtr = (char*)ledsDirect; //get pointer to RGB array.
  for(int i = 0; i<sizeof(ledsDirect); i++)
  {
    ledcWrite(i, ledsDirectPtr[i]); //red, green, blue in turn.
  }

  FastLED.delay(1000/FRAMES_PER_SECOND); 
  EVERY_N_MILLISECONDS( 20 ) { gHue++; }
}
