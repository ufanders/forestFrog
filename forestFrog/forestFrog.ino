// ==== XTC DAC sound stuff
#include "SoundData.h";
#include "XT_DAC_Audio.h";
XT_DAC_Audio_Class DacAudio(25,0); // Use GPIO 25, one of the 2 DAC pins and timer 0
XT_Wav_Class StarWars(StarWarsWav);
uint32_t DemoCounter=0;

// ==== FastLED stuff
#include <FastLED.h>
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS 10
#define DATA_PIN 15
CRGB leds[NUM_LEDS];

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
  //==== XTC DAC sound stuff
  Serial.begin(115200);
  StarWars.RepeatForever=true; // Keep on playing sample forever!!!
  DacAudio.Play(&StarWars); // Set to play

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
}


void loop() {
  DacAudio.FillBuffer();              // Fill the sound buffer with data
  Serial.println(DemoCounter++);      // Showing that the sound will play as well as your code running here.

  //==== FastLED stuff
  if(millis() > time_now[0] + period[0])
  {
    time_now[0] = millis();
    
    // Move a single white led 
    for(int whiteLed = 0; whiteLed < NUM_LEDS; whiteLed++) 
    {
      if(toggle)
      {
        // Turn our current led on to white, then show the leds
        leds[whiteLed] = CRGB::White;
      }
      else
      {
        // Turn our current led back to black for the next loop around
        leds[whiteLed] = CRGB::Black;
      }
      
      // Show the leds (only one of which is set to white, from above)
      FastLED.show();
    }
    
    toggle ^= 1; //toggle the state.
    
  }

  //==== Direct PWM stuff 
  if(millis() > time_now[1] + period[1])
  {
    time_now[1] = millis();

    if(toggle)
    {
      ledsDirect[0] = CRGB::White;
    }
    else
    {
      ledsDirect[0] = CRGB::Black;
    }

    //show direct PWM output
    char* ledsDirectPtr = (char*)ledsDirect; //get pointer to RGB array.
    for(int i = 0; i<sizeof(ledsDirect); i++)
    {
      ledcWrite(i, ledsDirectPtr[i]); //red, green, blue in turn.
    }
  }
}
