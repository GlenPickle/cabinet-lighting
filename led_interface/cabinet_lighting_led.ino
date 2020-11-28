/* Source code for the Under Cabinet Lighting. */
 
#include <Wire.h>
#include "FastLED.h"
#include "led_interface.h"

/************************** Begin Globals ************************************/
// The Arduino Wire library requires these vars to be global
ColorSettings    G_settings;
AlternatingPulse G_altPulse;
SnakePosition    G_snakePos;
Rainbow          G_rainbow;

// array for LEDs
CRGB         leds[NUM_LEDS];
/*************************** End Globals *************************************/


/******************************************************************************
**
**  Function Name: solidHSV
**
**  Purpose: Change the color of all lights to a single solid color
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
**  Function Name: showErrorLights
**
**  Purpose: To light up all the lights red when an error is detected
**
******************************************************************************/
void showErrorLights(ColorSettings *colorSettings)
{
    colorSettings->action = LED_SOLID;
    colorSettings->color = RED;
    colorSettings->currentColor.hue = 0;
    colorSettings->currentColor.sat = 255;
    colorSettings->currentColor.val = 127;
    solidHSV(colorSettings->currentColor.hue,
             colorSettings->currentColor.sat,
             colorSettings->currentColor.val);
}


/******************************************************************************
**
**  Function Name: updateLights
**
**  Purpose: If an animation is running, update to the next step
**
******************************************************************************/
void updateLights(ColorSettings *colorSettings, AlternatingPulse *altPulse,
                  SnakePosition *snakePos, Rainbow *rainbow)
{
    switch(colorSettings->action)
    {
        case LED_SOLID:
        case LED_OFF:
            break; // these cases need no updating, the color is set in the button callback

        case LED_PULSE:
            // For pulse we update by changing the brightness of all the lights by one step
            altPulse->dv = ((exp(sin(altPulse->pulseSpeed * millis()/2000.0*PI)) -0.36787944) * altPulse->delta);
            altPulse->altVal = altPulse->altValueMin + altPulse->dv;

            // Map altHue based on current altVal
            altPulse->altHue = map(altPulse->altVal, altPulse->altValueMin, altPulse->altValueMax,
                                   altPulse->altHueA, altPulse->altHueB);

            // Map altSat based on current altVal
            altPulse->altSat = map(altPulse->altVal, altPulse->altValueMin, altPulse->altValueMax,
                                   altPulse->altSatA, altPulse->altSatB);
    
            // for the "all" color selection we set each individual light it's own color and pulse to black
            for (uint32_t i = 0; i < NUM_LEDS; i++)
            {
                if (colorSettings->color == ALL)
                {
                    altPulse->altHue = i;
                }
                leds[i] = CHSV(altPulse->altHue, altPulse->altSat, altPulse->altVal);
            }
            FastLED.show();
            break;

        case LED_SNAKE:
            snakePos->tail = snakePos->tail++ % NUM_LEDS;
            snakePos->head = snakePos->tail + snakePos->quarter % NUM_LEDS;

            // For snake we update by lighting up one more light and turning off one at the beginning
            leds[snakePos->tail] = CRGB(0, 0, 0);
            if (colorSettings->color == ALL)
            {
                colorSettings->currentColor.hue = snakePos->head;
            }
            leds[snakePos->head] = CHSV(colorSettings->currentColor.hue,
                                        colorSettings->currentColor.sat,
                                        colorSettings->currentColor.val);
            FastLED.show();
            delay(colorSettings->delayTime);
            break;

        case LED_RAINBOW:
            // for rainbow we increment the hue of each light by one, rolling over to 0 if the hue is over 255
            for (uint32_t i = 0; i < NUM_LEDS; i++)
            {
                rainbow->hue = (rainbow->index + i > 255) ? rainbow->index + i - 255 : rainbow->index + i;
                leds[i] = CHSV(rainbow->hue, 255, colorSettings->currentColor.val);
            }
            rainbow->index++;
            FastLED.show();
            delay(colorSettings->delayTime);
            break;
    
        case LED_ALTERNATING:
            // for alternating we update by changing the altHue of all the lights by one step
            // if the color is all we just cycle through all colors
            if (colorSettings->color == ALL || colorSettings->color2 == ALL)
            {
                colorSettings->currentColor.hue++;
                if (colorSettings->currentColor.hue >= 255)
                {
                    colorSettings->currentColor.hue = 0;
                }
                solidHSV(colorSettings->currentColor.hue, 255, colorSettings->currentColor.val);
                delay(colorSettings->delayTime);
            }
            // otherwise we find a step between the two chosen colors, and change each light to this altHue
            else
            {
                // For pulse we update by changing the brightness of all the lights by one step
                altPulse->dv = ((exp(sin(altPulse->pulseSpeed * millis()/2000.0*PI)) -0.36787944) * altPulse->delta);
                altPulse->altVal = altPulse->altValueMin + altPulse->dv;

                // Map altHue based on current altVal
                altPulse->altHue = map(altPulse->altVal, altPulse->altValueMin, altPulse->altValueMax,
                                       altPulse->altHueA, altPulse->altHueB);

                // Map altSat based on current altVal
                altPulse->altSat = map(altPulse->altVal, altPulse->altValueMin, altPulse->altValueMax,
                                       altPulse->altSatA, altPulse->altSatB);

                solidHSV(altPulse->altHue, altPulse->altSat, altPulse->altVal);
            }
            break;

        case LED_DANCEPARTY:
            // for Dance Party we just go totally random for every light, with max brightness and saturation
            for (int i=0; i<NUM_LEDS; i++)
            {
                int h = random() % 256;
                leds[i] = CHSV(h, 255, 255);
            }
            FastLED.show();
            delay(colorSettings->delayTime);
            break;
    } 
}


/******************************************************************************
**
**  Function Name: updateColor
**
**  Purpose: If the color selection from the screen has changed, set the globals
**
******************************************************************************/
void updateColor(ColorSettings *colorSettings, AlternatingPulse *altPulse,
                 SnakePosition *snakePos)
{
    if (colorSettings->tmpAction == colorSettings->action && colorSettings->tmpColor == colorSettings->color)
    {
        return; // no buttons have been pressed, so just exit
    }
    colorSettings->color = colorSettings->tmpColor;
    colorSettings->color2 = colorSettings->tmpColor2;

    // by default set to full saturation and medium brightness
    colorSettings->currentColor.sat = 255;
    colorSettings->currentColor.val = 127;
    switch (colorSettings->color)
    {
        case ALL:
        case RED:
            colorSettings->currentColor.hue = 0;
            break;
        case GREEN:
            colorSettings->currentColor.hue = 96;
            break;
        case CYAN:
            colorSettings->currentColor.hue = 128;
            break;
        case BLUE:
            colorSettings->currentColor.hue = 160;
            break;
        case PURPLE:
            colorSettings->currentColor.hue = 192;
            break;
        case WHITE:
            colorSettings->currentColor.hue = 0;
            colorSettings->currentColor.sat = 0; // White has no saturation
            break;
        default:
            showErrorLights(colorSettings);
            break;
    }

    // set up color2
    colorSettings->currentColor2.sat = 255;
    colorSettings->currentColor2.val = 127;
    switch (colorSettings->color2)
    {
        case ALL:
        case RED:
            colorSettings->currentColor2.hue = 0;
            break;
        case GREEN:
            colorSettings->currentColor2.hue = 96;
            break;
        case CYAN:
            colorSettings->currentColor2.hue = 128;
            break;
        case BLUE:
            colorSettings->currentColor2.hue = 160;
            break;
        case PURPLE:
            colorSettings->currentColor2.hue = 192;
            break;
        case WHITE:
            colorSettings->currentColor2.hue = 0;
            colorSettings->currentColor2.sat = 0; // White has no saturation
            break;
        default:
            showErrorLights(colorSettings);
            break;
    }

    switch (colorSettings->tmpAction)
    {
        case LED_SOLID:
            if (colorSettings->color == ALL)
            {
                // each light gets it's own altHue
                for (uint32_t i = 0; i < NUM_LEDS; ++i) 
                { 
                    leds[i] = CHSV(i, colorSettings->currentColor.sat, colorSettings->currentColor.val); 
                } 
                FastLED.show();
            }
            else
            { 
                // each light gets the same altHue
                solidHSV(0,
                         colorSettings->currentColor.sat,
                         colorSettings->currentColor.val);
            }
            colorSettings->action = colorSettings->tmpAction;
            break;
        case LED_PULSE:
            // set up altHues so we can calculate the steps between them
            altPulse->altHueA = colorSettings->currentColor.hue;
            altPulse->altSatA = colorSettings->currentColor.sat;
            altPulse->altValueMin = 40.0; // pulse is just alternating between color and its dimmer self
    
            altPulse->altHueB = colorSettings->currentColor.hue;
            altPulse->altSatB = colorSettings->currentColor.sat;
            altPulse->altValueMax = 127.0;  // Go to half brightness by default
    
            altPulse->altHue = altPulse->altHueA;
            altPulse->altSat = altPulse->altSatA;
            altPulse->altVal = altPulse->altValueMin;
            altPulse->altHueDelta = altPulse->altHueA - altPulse->altHueB;
            altPulse->delta = (altPulse->altValueMax - altPulse->altValueMin) / 2.35040238;  // Do Not Edit
            colorSettings->action = colorSettings->tmpAction;
            break;
        case LED_ALTERNATING:
    	    altPulse->altHueA = colorSettings->currentColor.hue;
    	    altPulse->altSatA = colorSettings->currentColor.sat;
    	    altPulse->altValueMin = 120.0;  // Pulse minimum altValue (Should be less then altValueMax).
    
            altPulse->altHueB = colorSettings->currentColor2.hue;
            altPulse->altSatB = colorSettings->currentColor2.sat; 
            altPulse->altValueMax = 255.0;  // Pulse maximum altValueMax (Should be larger then altValueMin).
            altPulse->delta = (altPulse->altValueMax - altPulse->altValueMin) / SMOOTH_DELTA;
    
            colorSettings->action = colorSettings->tmpAction;
            break;
        case LED_SNAKE:
            FastLED.clear();
            snakePos->tail = 0; // we'll start at the beginning of the strand
            // turn on the first part of the snake
            for (uint32_t i = 0; i <= snakePos->quarter; i++)
            {
                leds[i] = CHSV(colorSettings->currentColor.hue,
                               colorSettings->currentColor.sat,
                               colorSettings->currentColor.val);
            }
            FastLED.show();
    
            colorSettings->action = colorSettings->tmpAction;
            break;
        default:
            showErrorLights(colorSettings);
            break;
    }
}


/******************************************************************************
**
**  Function Name: updateAction
**
**  Purpose: If the action selection has changed, update globals
**
******************************************************************************/
void updateAction(ColorSettings *colorSettings)
{
    if (colorSettings->tmpAction == colorSettings->action && 
        colorSettings->tmpColor == colorSettings->color)
    {
        return; // no buttons have been pressed, so just exit
    }
    switch (colorSettings->tmpAction)
    {
        case LED_SOLID:
        case LED_PULSE:
            break;
        case LED_SNAKE:
            colorSettings->delayTime = 250L;
            break;
        case LED_RAINBOW:
            colorSettings->action = LED_RAINBOW; // no color selection so start this mode
            break;
        case LED_DANCEPARTY:
            colorSettings->currentColor.val = 255;
            colorSettings->delayTime = 250L;
            colorSettings->action = LED_DANCEPARTY; // no color selection so start this mode
            break;
        case LED_OFF:
            FastLED.clear();
            FastLED.show();
            colorSettings->action = LED_OFF; // no color selection so start this mode
            break;
        default:
            showErrorLights(colorSettings);
            break;
    }
}


/******************************************************************************
**
**  Function Name: updateBrightness
**
**  Purpose: If the brightness instruction on the screen has changed, update
**           the variables that track brightness
**
******************************************************************************/
void updateBrightness(ColorSettings *colorSettings, AlternatingPulse *altPulse)
{
    colorSettings->currentColor.val = colorSettings->brightness;
    if (colorSettings->action == LED_SOLID)
    {
        if (colorSettings->color == ALL)
        {
            for (uint32_t i = 0; i < NUM_LEDS; i++)
            {
                leds[i] = CHSV(i, colorSettings->currentColor.sat, colorSettings->currentColor.val);
            }
            FastLED.show();
        }
        else
        {
            solidHSV(colorSettings->currentColor.hue,
                     colorSettings->currentColor.sat,
                     colorSettings->currentColor.val);
        }
    }
    else if (colorSettings->action == LED_PULSE)
    {
        altPulse->altValueMax = colorSettings->brightness;
        altPulse->delta = (altPulse->altValueMax - altPulse->altValueMin) / SMOOTH_DELTA;
    }
    else if (colorSettings->action == LED_ALTERNATING)
    {
        altPulse->altValueMax = colorSettings->brightness;
        if (altPulse->altValueMin >= altPulse->altValueMax)
        {
            altPulse->altValueMin = altPulse->altValueMax / 2;
        }
        altPulse->delta = (altPulse->altValueMax - altPulse->altValueMin) / SMOOTH_DELTA;
    }
    return;
}


/******************************************************************************
**
**  Function Name: updateSpeed
**
**  Purpose: If the speed instruction on the screen has changed, update
**           the variables that track brightness by scaling them according to
**           the range of the slider values
**
******************************************************************************/
void updateSpeed(ColorSettings *colorSettings, AlternatingPulse *altPulse)
{
    if (colorSettings->action == LED_ALTERNATING)
    {
         altPulse->pulseSpeed = ((float) colorSettings->speed / 250.0) + 1.0;
    }
    else if (colorSettings->action == LED_PULSE)
    {
         altPulse->pulseSpeed = ((float) colorSettings->speed / 250.0) + 1.0;
    }
    else if (colorSettings->action == LED_RAINBOW)
    {
         colorSettings->delayTime = ((100.0 -(float) colorSettings->speed) / 100.0) * 100.0;
    }
    else if (colorSettings->action == LED_SNAKE)
    {
         colorSettings->delayTime = ((100.0 - (float) colorSettings->speed) / 100.0) * 150.0;
    }
    else if (colorSettings->action == LED_DANCEPARTY)
    {
         colorSettings->delayTime = ((100.0 -(float) colorSettings->speed) / 100.0) * 600.0;
    }
    return;
}


/******************************************************************************
**
**  Function Name: receiveEvent
**
**  Purpose: Reads instructions from the other arduino using the Wire library
**
******************************************************************************/
void receiveEvent(int bytes)
{
    ColorSettings *settings = &G_settings;
    settings->tmpAction = (LedAction) Wire.read();
    settings->tmpColor = (ColorVal) Wire.read();
    settings->tmpColor2 = (ColorVal) Wire.read();
    settings->brightness = (uint32_t) Wire.read();
    settings->speed = (uint32_t) Wire.read();
}


/******************************************************************************
**
**  Function Name: setup
**
**  Purpose: An initialization function that is standard to arduinos
**
******************************************************************************/
void setup() 
{
    // initialize LEDs
    FastLED.addLeds<LED_TYPE, LED_DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(BRIGHTNESS);// global brightness
    
    // initialize communication
    Serial1.begin(9600);
    Wire.begin(INSTRUCTION_PIN); 
    pinMode(READY_PIN, OUTPUT);

    // Attach a function to trigger when something is received from i2c
    Wire.onReceive(receiveEvent);

    // begin the main loop to receive instructions and update the leds
    while (1)
    {
        //signal that we are ready for messages
        digitalWrite(READY_PIN, HIGH);
        updateAction(&G_settings);
        updateSpeed(&G_settings, &G_altPulse);

        //signal that we aren't ready for messages because the next 3 functions block
        digitalWrite(READY_PIN, LOW);
        updateColor(&G_settings, &G_altPulse, &G_snakePos);
        updateBrightness(&G_settings, &G_altPulse);
        updateLights(&G_settings, &G_altPulse, &G_snakePos, &G_rainbow);
    }
}
