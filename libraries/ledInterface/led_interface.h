#include "shared.h"

#define LED_DATA_PIN 9
#define INSTRUCTION_PIN 3
#define READY_PIN 10
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS 4
#define BRIGHTNESS 96
#define SMOOTH_DELTA 2.35040238

// a structure for the HSV color values
typedef struct
{
    uint32_t hue;
    uint32_t sat;
    uint32_t val;
} HsvVals;

// structure for general color and action settings
typedef struct
{
    LedAction action    = LED_OFF;
    LedAction tmpAction = LED_OFF; // for some modes don't want to update until a color is chosen
    ColorVal  color     = GREEN;
    ColorVal  color2    = GREEN;
    ColorVal  tmpColor  = GREEN;
    ColorVal  tmpColor2 = GREEN;
    
    HsvVals   currentColor;
    HsvVals   currentColor2;
    
    long      delayTime = 250L;
    
    uint32_t  brightness;
    uint32_t  speed;
} ColorSettings;

// structure for the Alternating and Pulse modes
typedef struct
{
    uint32_t altHue;
    uint32_t altSat;
    float    altVal;
    uint32_t altHueDelta;
    float    delta; 
    float    pulseSpeed = 1.0; // default G_altValue that will be changed by the slider
    float    altValueMin;
    float    altValueMax;
    uint32_t altHueA;
    uint32_t altSatA;
    uint32_t altHueB;
    uint32_t altSatB;
    float    dv;
} AlternatingPulse;

// structure for the snake mode
typedef struct
{
    uint32_t     tail = 0;
    uint32_t     quarter = NUM_LEDS / 4;
    uint32_t     head;
} SnakePosition;

// structure used by the rainbow mode
typedef struct
{
    uint32_t     index = 0;
    uint32_t     hue = 0;
} Rainbow;
