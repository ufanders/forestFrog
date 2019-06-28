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
#define NUM_LEDS 32
#define DATA_PIN_1 15
#define DATA_PIN_2 14
#define DATA_PIN_3 13
#define DATA_PIN_4 12
CRGB leds[NUM_LEDS];
CRGB leds2[NUM_LEDS];
CRGB leds3[NUM_LEDS];
CRGB leds4[NUM_LEDS];
#define FRAMES_PER_SECOND  60
uint8_t gHue = 0; // rotating "base color" used by many of the patterns

int period[2] = {500, 500};
unsigned long time_now[2] = {0, 0};
char toggle = 0;

// ==== Direct PWM stuff
const int bigLedPins[] = {2, 0, 4, 17, 5, 18}; //we don't use pin 5 because of high-output erratum upon POR.
const int motorPins[] = {16, 19};
const int pwmFreq = 25000; //above the audible frequency band so we can't hear any EMI through the speaker.
const int pwmRes = 8;
CRGB ledsDirect[2];
int currentLed;

// ==== Button stuff
#include <Debounce.h>
const int buttonPins[] = {33, 32, 35, 34};
bool button1Down = false;
bool button2Down = false;
bool button3Down = false;
bool button4Down = false;

Debounce Button1(buttonPins[0], 100); // Button1 debounced, default 50ms delay.
Debounce Button2(buttonPins[1], 100); // Button2 debounced, default 50ms delay.
Debounce Button3(buttonPins[2], 100); // Button1 debounced, default 50ms delay.
Debounce Button4(buttonPins[3], 100); // Button2 debounced, default 50ms delay.

//color stuff
byte coolingValue;
byte sparkingValue;
byte brightnessValue;

CRGBPalette16 gPal0;
CRGBPalette16 gPal1;
CRGBPalette16 gPal2;
CRGBPalette16 gPal3;
CRGBPalette16* gPalCurrent;

// control stuff
byte colorMode = 0;
bool gReverseDirection = false;
bool motorToggle = false;

void setup() 
{
  delay(1000); //power-up settle delay.
  
  Serial.begin(115200);

  //==== FastLED stuff
  FastLED.addLeds<LED_TYPE, DATA_PIN_1, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, DATA_PIN_2, COLOR_ORDER>(leds2, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, DATA_PIN_3, COLOR_ORDER>(leds3, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, DATA_PIN_4, COLOR_ORDER>(leds4, NUM_LEDS).setCorrection(TypicalLEDStrip);
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

  //button pins as GPIO
  for(i=0; i<(sizeof(buttonPins)/sizeof(const int)); i++)
  {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }

  // color stuff
  gPal0 = CRGBPalette16( CRGB::Black, CRGB::White);
  gPal1 = CRGBPalette16( CRGB::Black, CRGB::Red, CRGB::Yellow, CRGB::White);
  gPal2 = CRGBPalette16( CRGB::Black, CRGB::Purple, CRGB::Aqua,  CRGB::White);

  coolingValue = 55;
  sparkingValue = 120;
  colorMode = 1;

}

void loop() 
{
  //==== XTC DAC sound stuff
  DacAudio.FillBuffer();

  //if(CROAK.Playing == false)
  if(!digitalRead(buttonPins[0]) && CROAK.Playing == false)
  {
    CROAK.Speed = random(50, 125) / 100.0;
    DacAudio.Play(&CROAK);
  }

  //==== light control stuff
  /*
  if(!digitalRead(buttonPins[1]))
  {
    gHue = 0;
  }

  if(!digitalRead(buttonPins[2]))
  {
    gHue = 63;
  }

  if(!digitalRead(buttonPins[3]))
  {
    gHue = 127;
  }
  
  
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
  fill_rainbow( leds2, NUM_LEDS, gHue, 7);
  fill_rainbow( leds3, NUM_LEDS, gHue, 7);
  fill_rainbow( leds4, NUM_LEDS, gHue, 7);
  */
  fill_rainbow( ledsDirect, 2, gHue, 7);
 

  //TODO: read pin states.
  if(!Button4.read() && button4Down == false)
  {
    //LED mode button pushed.
    button4Down = true;
    
    colorMode++;
    if(colorMode > 3) colorMode = 0;

    switch(colorMode)
    {
      case 0: //LEDs off.
      coolingValue = 55;
      sparkingValue = 192;
      brightnessValue = 0;
      FastLED.setBrightness(brightnessValue);

      break;

      case 1: //Fire.
      coolingValue = 55;
      sparkingValue = 100;
      brightnessValue = 96;
      FastLED.setBrightness(brightnessValue);
      break;

      case 2: //MDMA fire.
      coolingValue = 55;
      sparkingValue = 160;
      brightnessValue = 192;
      FastLED.setBrightness(brightnessValue);
      break;

      case 3: //FREAK OUT
      coolingValue = 55;
      sparkingValue = 192;
      brightnessValue = 255;
      FastLED.setBrightness(brightnessValue);
      break;
    }
  }

  if(Button4.read())
  {
    button4Down = false;
  }

  if(!Button3.read() && button3Down == false) //CHANGE EYES
  {
    motorToggle ^= 1;
    
    digitalWrite(motorPins[0], motorToggle);
    digitalWrite(motorPins[1], motorToggle);
  }

  if(Button3.read())
  {
    button3Down = false;
  }

  //TODO: draw video frame.
  random16_add_entropy( random8());
  
  if(colorMode == 3)
  {
    static uint8_t hue = 0;
    hue++;
    CRGB darkcolor  = CHSV(hue,255,192); // pure hue, three-quarters brightness
    CRGB lightcolor = CHSV(hue,128,255); // half 'whitened', full brightness
    gPal3 = CRGBPalette16( CRGB::Black, darkcolor, lightcolor, CRGB::White);
  }
  
  Fire2012WithPalette(); // run simulation frame, using palette colors
  
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

void Fire2012WithPalette()
{
// Array of temperature readings at each simulation cell
  static byte heat[NUM_LEDS];

  // Step 1.  Cool down every cell a little
    for( int i = 0; i < NUM_LEDS; i++) {
      heat[i] = qsub8( heat[i],  random8(0, ((coolingValue * 10) / NUM_LEDS) + 2));
    }
  
    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for( int k= NUM_LEDS - 1; k >= 2; k--) {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    }
    
    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if( random8() < sparkingValue ) {
      int y = random8(7);
      heat[y] = qadd8( heat[y], random8(160,255) );
    }

    // Step 4.  Map from heat cells to LED colors
    for( int j = 0; j < NUM_LEDS; j++) {
      // Scale the heat value from 0-255 down to 0-240
      // for best results with color palettes.
      byte colorindex = scale8( heat[j], 240);

      CRGB color;

      switch(colorMode)
      {
        case 0: //all black.
          color = ColorFromPalette( gPal0, 0);
        break;

        case 1: //fire.
          color = ColorFromPalette( gPal1, colorindex);
        break;

        case 2: //MDMA fire.
          color = ColorFromPalette( gPal2, colorindex);
        break;

        case 3: //FREAK OUT
          color = ColorFromPalette( gPal3, colorindex);
        break;
      }
      
      int pixelnumber;
      if( gReverseDirection ) {
        pixelnumber = (NUM_LEDS-1) - j;
      } else {
        pixelnumber = j;
      }
      
      leds[pixelnumber] = color;
      leds2[pixelnumber] = color; //we hack in a copy of the first strip data into the second strip.
      leds3[pixelnumber] = color;
      leds4[pixelnumber] = color;
    }
}
