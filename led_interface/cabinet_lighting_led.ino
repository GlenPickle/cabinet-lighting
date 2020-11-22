/* Source code for the Under Cabinet Lighting. */
 
#include <Wire.h>
#include "FastLED.h"

#define LED_DATA_PIN 9
#define INSTRUCTION_PIN 3
#define READY_PIN 10
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS 256
#define BRIGHTNESS 96

// A list of all possible LED Modes
typedef enum
{
    LED_SOLID,
    LED_PULSE,
    LED_SNAKE,
    LED_ALTERNATING,
    LED_RAINBOW,
    LED_DANCEPARTY,
    LED_OFF
} led_action_t;

// A list of all available G_colors
typedef enum
{
    GREEN,
    BLUE,
    PURPLE,
    WHITE,
    CYAN,
    ALL,
    RED
} color_t;

class colorInfo
{
    public:
    uint32_t h;
    uint32_t s;
    uint32_t v;
};

/**************************** Begin Globals ********************************/

// Global variables used by multiple modes
led_action_t G_action    = LED_OFF;
led_action_t G_tmpAction = LED_OFF; // for some modes don't want to update G_action until after a G_color is chosen

// set defaults for the G_colors so that variables aren't referenced before assignment
color_t      G_color     = GREEN;
color_t      G_color2    = GREEN;
color_t      G_tmpColor  = GREEN;
color_t      G_tmpColor2 = GREEN;

colorInfo    G_currentColor;
colorInfo    G_currentColor2;

uint32_t     G_currentHue;
uint32_t     G_currentSat;
uint32_t     G_currentVal;

long         G_delayTime = 250L;

uint32_t     G_brightnessPosition;
uint32_t     G_SpeedPosition;

// Global variables used to alternate G_colors and pulse
uint32_t     G_altHue;
uint32_t     G_altSat;
float        G_altVal;
uint32_t     G_altHueDelta;
float        G_delta; 
float        G_pulseSpeed = 1.0; // default G_altValue that will be changed by the slider
float        G_altValueMin;
float        G_altValueMax;
uint32_t     G_altHueA;
uint32_t     G_altSatA;
uint32_t     G_altHueB;
uint32_t     G_altSatB;
float        G_dv;

// variable used by just snake
uint32_t     G_snakeTail = 0;
uint32_t     G_snakeQuarter = NUM_LEDS / 4;

// variables used just by rainbow
uint32_t     G_rainbowIndex = 0;
uint32_t     G_rainbowHue = 0;

// set up array for LEDs
CRGB         leds[NUM_LEDS];

/****************************** End Globals **********************************/


/******************************************************************************
**
**  Function Name: solidHSV
**
**  Purpose: Change the color of all lights to a single solid color
**
**  Globals Used: leds
**
**  Globals Set: none
**
******************************************************************************/
void solidHSV(uint32_t H, uint32_t S, uint32_t V) {
    for (int i = 0; i < NUM_LEDS; i++)
    {
        leds[i] = CHSV(H, S, V);
    }
    FastLED.show();
}


/******************************************************************************
**
**  Function Name: updateLights
**
**  Purpose: If an animation is running, update to the next step
**
**  Globals Used: leds, G_action, G_color, G_altSat, G_altVal, G_AltValueMin,
**                G_altValueMax, G_altHueA, G_AltHue_B, G_delta, G_dv,
**                G_snakeTail, G_snakeQuarter, G_dv, G_currentHue,
**                G_currentSat, G_currentVal, G_rainbowIndex
**
**  Globals Set: G_dv, G_altVal, G_altHue, G_altSat, leds, G_snakeTail,
**               G_rainbowIndex, G_rainbowHue
**
******************************************************************************/
void updateLights()
{
    switch(G_action)
    {
        case LED_SOLID:
        case LED_OFF:
            break; // these cases need no updating, the G_color is set in the button callback

        case LED_PULSE:
            // For pulse we update by changing the brightness of all the lights by one step
            G_dv = ((exp(sin(G_pulseSpeed * millis()/2000.0*PI)) -0.36787944) * G_delta);
            G_altVal = G_altValueMin + G_dv;
            G_altHue = map(G_altVal, G_altValueMin, G_altValueMax, G_altHueA, G_altHueB);  // Map G_altHue based on current G_altVal
            G_altSat = map(G_altVal, G_altValueMin, G_altValueMax, G_altSatA, G_altSatB);  // Map G_altSat based on current G_altVal
    
            for (uint32_t i = 0; i < NUM_LEDS; i++)
            {
                if (G_color == ALL)
                {
                    G_altHue = i;
                }
                leds[i] = CHSV(G_altHue, G_altSat, G_altVal);
            }
            FastLED.show();
            break;

        case LED_SNAKE:
            // For snake we update by lighting up one more light and turning off one at the beginning
            leds[G_snakeTail] = CRGB(0, 0, 0);
            if (G_snakeTail + G_snakeQuarter >= NUM_LEDS)
            {
                if (G_color == ALL)
                {
                    G_currentHue = G_snakeTail + G_snakeQuarter - NUM_LEDS;
                }
                leds[G_snakeTail + G_snakeQuarter - NUM_LEDS] = CHSV(G_currentHue, G_currentSat, G_currentVal);
            }
            else
            {
                if (G_color == ALL)
                {
                    G_currentHue = G_snakeTail + G_snakeQuarter;
                }
                leds[G_snakeTail + G_snakeQuarter] = CHSV(G_currentHue, G_currentSat, G_currentVal);
            }
            G_snakeTail++;
            if (G_snakeTail == NUM_LEDS)
            {
                G_snakeTail = 0;
            }
            FastLED.show();
            delay(G_delayTime);
            break;

        case LED_RAINBOW:
            // for rainbow we increment the G_altHue of each light by one, resetting if the G_altHue is over 255
            for (uint32_t i = 0; i < NUM_LEDS; i++)
            {
                if (G_rainbowIndex + i > 255)
                {
                   G_rainbowHue = G_rainbowIndex + i - 255;
                }
                else
                {
                    G_rainbowHue = G_rainbowIndex + i;
                }
                leds[i] = CHSV(G_rainbowHue, 255, G_currentVal);
            }
            G_rainbowIndex++;
            FastLED.show();
            delay(G_delayTime);
            break;
    
        case LED_ALTERNATING:
            // for alternating we update by changing the G_altHue of all the lights by one step
            // if the G_color is all we just cycle through all G_colors
            if (G_color == ALL || G_color2 == ALL)
            {
                G_currentHue++;
                if (G_currentHue >= 255)
                {
                    G_currentHue = 0;
                }
                solidHSV(G_currentHue, 255, G_currentVal);
                delay(G_delayTime);
            }
            // otherwise we find a step between the two chosen G_colors, and change each light to this G_altHue
            else
            {
                G_dv = ((exp(sin(G_pulseSpeed * millis()/2000.0*PI)) -0.36787944) * G_delta);
                G_altVal = G_altValueMin + G_dv;
                G_altHue = map(G_altVal, G_altValueMin, G_altValueMax, G_altHueA, G_altHueB);  // Map G_altHue based on current G_altVal
                G_altSat = map(G_altVal, G_altValueMin, G_altValueMax, G_altSatA, G_altSatB);  // Map G_altSat based on current G_altVal
    
                for (uint32_t i = 0; i < NUM_LEDS; i++)
                {
                    leds[i] = CHSV(G_altHue, G_altSat, G_altVal);
                }
                FastLED.show();
            }
            break;

        case LED_DANCEPARTY:
            // for Dance Party we just go totally random for every light, with max brightness
            for (int i=0; i<NUM_LEDS; i++)
            {
                int h = random() % 256;
                leds[i] = CHSV(h, 255, 255);
            }
            FastLED.show();
            delay(G_delayTime);
            break;
    } 
}


/******************************************************************************
**
**  Function Name: updateColor
**
**  Purpose: If the color selection from the screen has changed, set the globals
**
**  Globals Used: G_tmpAction, G_action, G_tmpColor, G_color, G_currentColor,
**                
**
**  Globals Set: G_color, G_color2, G_currentColor, G_currentHue, G_currentSat,
**               G_currentVal, G_snakeTail
**
******************************************************************************/
void updateColor()
{
    if (G_tmpAction == G_action && G_tmpColor == G_color)
    {
        return; // no buttons have been pressed, so just exit
    }
    G_color = G_tmpColor;
    G_color2 = G_tmpColor2;
    if (G_color == ALL || G_color == RED)
    {
        G_currentColor.h = 0; // start with Red
        G_currentColor.s = 255;
        G_currentColor.v = 127;
    }
    else if (G_color == GREEN)
    {
        G_currentColor.h = 96;
        G_currentColor.s = 255;
        G_currentColor.v = 127;
    }
    else if (G_color == CYAN)
    {
        G_currentColor.h = 128;
        G_currentColor.s = 255;
        G_currentColor.v = 127;
    }
    else if (G_color == BLUE)
    {
        G_currentColor.h = 160;
        G_currentColor.s = 255;
        G_currentColor.v = 127;
    }
    else if (G_color == PURPLE)
    {
        G_currentColor.h = 192;
        G_currentColor.s = 255;
        G_currentColor.v = 127;
    }
    else if (G_color == WHITE)
    {
        G_currentColor.h = 0;
        G_currentColor.s = 0;
        G_currentColor.v = 127;
    }

    // set up G_color2
    if (G_color2 == ALL || G_color2 == RED)
    {
        G_currentColor2.h = 0;
        G_currentColor2.s = 255;
        G_currentColor2.v = 127;
    }
    else if (G_color2 == GREEN)
    {
        G_currentColor2.h = 96;
        G_currentColor2.s = 255;
        G_currentColor2.v = 127;
    }
    else if (G_color2 == CYAN)
    {
        G_currentColor2.h = 128;
        G_currentColor2.s = 255;
        G_currentColor2.v = 127;
    }
    else if (G_color2 == BLUE)
    {
        G_currentColor2.h = 160;
        G_currentColor2.s = 255;
        G_currentColor2.v = 127;
    }
    else if (G_color2 == PURPLE)
    {
        G_currentColor2.h = 192;
        G_currentColor2.s = 255;
        G_currentColor2.v = 127;
    }
    else if (G_color2 == WHITE)
    {
        G_currentColor2.h = 0;
        G_currentColor2.s = 0;
        G_currentColor2.v = 127;
    }


    if (G_tmpAction == LED_SOLID)
    {
        G_currentHue = G_currentColor.h;
        G_currentSat = G_currentColor.s;
        G_currentVal = G_currentColor.v;
        if (G_color == ALL)
        {
            // each light gets it's own G_altHue
            for (uint32_t i = 0; i < NUM_LEDS; ++i) 
            { 
                leds[i] = CHSV(i,G_currentSat,G_currentVal); 
            } 
            FastLED.show();
        }
        else
        { 
            // each light gets the same G_altHue
            solidHSV(G_currentHue, G_currentSat, G_currentVal);
        }
        G_action = G_tmpAction;
    }
    else if (G_tmpAction == LED_PULSE)
    {
        // set up G_altHues so we can calculate the steps between them
        G_altHueA = G_currentColor.h;
        G_altSatA = G_currentColor.s;
        G_altValueMin = 40.0;

        G_altHueB = G_currentColor.h;
        G_altSatB = G_currentColor.s;
        G_altValueMax = 127.0;  // Go to half brightness by default

        G_altHue = G_altHueA;
        G_altSat = G_altSatA;
        G_altVal = G_altValueMin;
        G_altHueDelta = G_altHueA - G_altHueB;
        G_delta = (G_altValueMax - G_altValueMin) / 2.35040238;  // Do Not Edit
        G_action = G_tmpAction;
    }
    else if (G_tmpAction == LED_ALTERNATING)
    {
	G_altHueA = G_currentColor.h;  // Start G_altHue at G_altValueMin.
	G_altSatA = G_currentColor.s;  // Start G_altSaturation at G_altValueMin.
	G_altValueMin = 120.0;  // Pulse minimum G_altValue (Should be less then G_altValueMax).

        G_altHueB = G_currentColor2.h;  // End G_altHue at G_altValueMax.
        G_altSatB = G_currentColor2.s;  // End G_altSaturation at G_altValueMax.
        G_altValueMax = 255.0;  // Pulse maximum G_altValue (Should be larger then G_altValueMin).
        G_delta = (G_altValueMax - G_altValueMin) / 2.35040238;  // Do Not Edit

        G_action = G_tmpAction;
    }
    else if (G_tmpAction == LED_SNAKE)
    {
        G_currentHue = G_currentColor.h;
        G_currentSat = G_currentColor.s;
        G_currentVal = G_currentColor.v;

        FastLED.clear();
        FastLED.show();
        G_snakeTail = 0; // we'll start at the beginning of the strand
        // turn on the first part of the snake
        for (uint32_t i = 0; i <= G_snakeQuarter; i++)
        {
            leds[i] = CHSV(G_currentHue, G_currentSat, G_currentVal);
        }
        FastLED.show();

        G_action = G_tmpAction;
    }
}


/******************************************************************************
**
**  Function Name: updateAction
**
**  Purpose: If the action selection has changed, update globals
**
**  Globals Used: G_tmp_Action, G_action, G_tmpColor, G_color
**
**  Globals Set: G_delayTime, G_action, G_currentVal
**
******************************************************************************/
void updateAction()
{
    if (G_tmpAction == G_action && G_tmpColor == G_color)
    {
        return; // no buttons have been pressed, so just exit
    }
    switch (G_tmpAction)
    {
        case LED_PULSE:
            break;
        case LED_SNAKE:
            G_delayTime = 250L;
            break;
        case LED_RAINBOW:
            G_action = LED_RAINBOW; // no G_color selection so start this mode
            break;
        case LED_DANCEPARTY:
            G_currentVal = 255;
            G_delayTime = 250L;
            G_action = LED_DANCEPARTY; // no G_color selection so start this mode
            break;
        case LED_OFF:
            FastLED.clear();
            FastLED.show();
            G_action = LED_OFF; // no G_color selection so start this mode
            break;
        default:
            break;
    }
}


/******************************************************************************
**
**  Function Name: updateBrightness
**
**  Purpose: If the brightness instruction on the screen has changed, update
**           the global variables that track brightness
**
**  Globals Used: G_brightnessPosition, G_action, G_color, G_currentHue,
**                G_currentSat, G_currentVal, G_altValueMin
**
**  Globals Set: G_currentVal, G_altValueMax, G_delta
**
******************************************************************************/
void updateBrightness()
{
    G_currentVal = G_brightnessPosition;
    if (G_action == LED_SOLID)
    {
        if (G_color == ALL)
        {
            for (uint32_t i = 0; i < NUM_LEDS; i++)
            {
                leds[i] = CHSV(i, G_currentSat, G_currentVal);
            }
            FastLED.show();
        }
        else
        {
            solidHSV(G_currentHue, G_currentSat, G_currentVal);
        }
    }
    else if (G_action == LED_PULSE)
    {
        G_altValueMax = G_brightnessPosition;
        G_delta = (G_altValueMax - G_altValueMin) / 2.35040238;  // Do Not Edit
    }
    else if (G_action == LED_ALTERNATING)
    {
        G_altValueMax = G_brightnessPosition;
        if (G_altValueMin >= G_altValueMax)
        {
            G_altValueMin = G_altValueMax / 2;
        }
        G_delta = (G_altValueMax - G_altValueMin) / 2.35040238;  // Do Not Edit
    }
    return;
}


/******************************************************************************
**
**  Function Name: updateSpeed
**
**  Purpose: If the speed instruction on the screen has changed, update
**           the global variables that track brightness
**
**  Globals Used: G_speedPosition, G_action
**
**  Globals Set: G_pulseSpeed, G_delayTime
**
******************************************************************************/
void updateSpeed()
{
    if (G_action == LED_ALTERNATING)
    {
         G_pulseSpeed = ((float) G_SpeedPosition / 250.0) + 1.0;
    }
    else if (G_action == LED_PULSE)
    {
         G_pulseSpeed = ((float) G_SpeedPosition / 250.0) + 1.0;
    }
    else if (G_action == LED_RAINBOW)
    {
         G_delayTime = ((100.0 -(float) G_SpeedPosition) / 100.0) * 100.0;
    }
    else if (G_action == LED_SNAKE)
    {
         G_delayTime = ((100.0 - (float) G_SpeedPosition) / 100.0) * 150.0;
    }
    else if (G_action == LED_DANCEPARTY)
    {
         G_delayTime = ((100.0 -(float) G_SpeedPosition) / 100.0) * 600.0;
    }
    return;
}


/******************************************************************************
**
**  Function Name: receiveEvent
**
**  Purpose: Reads instructions from the other arduino using the Wire library
**
**  Globals Used: none
**
**  Globals Set: G_tmpAction, G_tmpColor, G_tmpColor2,
**               G_brightnessPosition, G_speedPosition
**
******************************************************************************/
void receiveEvent(int bytes)
{
    G_tmpAction = (led_action_t) Wire.read();
    G_tmpColor = (color_t) Wire.read();
    G_tmpColor2 = (color_t) Wire.read();
    G_brightnessPosition = (uint32_t) Wire.read();
    G_SpeedPosition = (uint32_t) Wire.read();
}


/******************************************************************************
**
**  Function Name: setup
**
**  Purpose: An initialization function that is standard to arduinos
**
**  Globals Used: leds
**
**  Globals Set: none
**
******************************************************************************/
void setup() {
    // initialize LEDs
    FastLED.addLeds<LED_TYPE, LED_DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip); // initializes LED strip
    FastLED.setBrightness(BRIGHTNESS);// global brightness
    
    // initialize communication
    Serial1.begin(9600);
    Wire.begin(INSTRUCTION_PIN); 
    pinMode(READY_PIN, OUTPUT);

    // Attach a function to trigger when something is received from i2c
    Wire.onReceive(receiveEvent);
}


/******************************************************************************
**
**  Function Name: loop
**
**  Purpose: The main loop for the arduino. Listens for instructions from
**           the other arduino and controls the LEDs
**
**  Globals Used: none
**
**  Globals Set: none
**
******************************************************************************/
void loop() {
    updateAction();
    updateSpeed();
    digitalWrite(READY_PIN, LOW); //signal that we aren't ready for messages because the next 3 functions all have blocking function calls
    updateColor();
    updateBrightness();
    updateLights();
    digitalWrite(READY_PIN, HIGH); //signal that we are ready for messages again because the first two functions have no blocking calls
}
