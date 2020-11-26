/* Source code for the Under Cabinet Lighting. */
/* The sketch generated will run on the arduino connected to the TFT. */
#include <Wire.h>
#include "Nextion.h"

#define INSTRUCTION_PIN 3
#define READY_PIN 10

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

// A list of all available colors
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

/***************************** BEGIN GLOBALS *********************************/
/* Note that in order for the nextion library to update variables they must be declared globally. */

// global variables for the light settings: action, color, brightness and speed
led_action_t G_action             = LED_OFF;
int          G_chosen             = 0; // for alternating we will need to wait for two colors to be G_chosen
boolean      G_haveUpdate         = false;
boolean      G_onTime             = false;
uint32_t     G_brightnessPosition = 127;
uint32_t     G_speedPosition      = 50;
color_t      G_color;
color_t      G_color2;

// global variables for tracking how much time has passed
unsigned long G_currentTime;
unsigned long G_previousTime = 0;

// global variables for the auto on and off feature
char         G_timeBuf[30]       = {0};
led_action_t G_defaultAction     = LED_SOLID;
color_t      G_defaultColor      = CYAN;
color_t      G_defaultColor2     = GREEN;
uint32_t     G_defaultBrightness = 255;
uint32_t     G_defaultSpeed      = 99;
char         G_turnOn[4]         = {'1', '9', '0', '0'}; // 7pm
char         G_turnOff[4]        = {'0', '1', '0', '0'}; // 1am

// The pointer for button clicks
NexGauge pointer  = NexGauge(0, 1, "pointer");

// The pages that the arduino will need access to
NexPage pAction           = NexPage(0, 0, "HomeScreen");
NexPage pColor            = NexPage(1, 0, "ColorScreen");
NexPage pBrightness       = NexPage(2, 0, "Brightness");
NexPage pSpeed            = NexPage(3, 0, "Speed");
NexPage pBrightnessSpeed  = NexPage(4, 0, "BrightSpeed");

// The buttons for the first page where mode is selected
NexButton bSolid    = NexButton(0, 2, "bSolid");
NexButton bPulse    = NexButton(0, 3, "bPulse");
NexButton bSnake    = NexButton(0, 4, "bSnake");
NexButton bAlt      = NexButton(0, 5, "bAlternating");
NexButton bRainbow  = NexButton(0, 6, "bRainbow");
NexButton bDance    = NexButton(0, 7, "bDance");
NexButton bOff      = NexButton(0, 8, "bOff");

// The buttons for the second page where colors are selected
NexButton bAll    = NexButton(1, 2, "bAll");
NexButton bGreen  = NexButton(1, 3, "bGreen");
NexButton bBlue   = NexButton(1, 4, "bBlue");
NexButton bPurple = NexButton(1, 5, "bPurple");
NexButton bWhite  = NexButton(1, 6, "bWhite");
NexButton bCyan   = NexButton(1, 7, "bCyan");

// The text for the second page where colors are selected
NexText tChoose = NexText(1, 9, "t0");

// The button for setting the default G_color settings
NexButton bSetDefaults = NexButton(9, 2, "bYes");

// The Slider for the page with just brightness
NexSlider hBrightness = NexSlider(2, 2, "hBrightness");

// The Slider for the page with just speed
NexSlider hSpeed = NexSlider(3, 2, "hSpeed");

// The sliders for the page with brightness and speed
NexSlider hBrightness2 = NexSlider(4, 2, "hBrightness");
NexSlider hSpeed2      = NexSlider(4, 6, "hSpeed");

// The buttons for the clock when auto times are set
NexButton b1 = NexButton(8, 4, "b1");
NexButton b2 = NexButton(8, 5, "b2");
NexButton b3 = NexButton(8, 6, "b3");
NexButton b4 = NexButton(8, 7, "b4");
NexButton b5 = NexButton(8, 8, "b5");
NexButton b6 = NexButton(8, 9, "b6");
NexButton b7 = NexButton(8, 10, "b7");
NexButton b8 = NexButton(8, 11, "b8");
NexButton b9 = NexButton(8, 12, "b9");
NexButton b0 = NexButton(8, 14, "b1");

// The Text for the auto on off clock
NexText tHour1   = NexText(8, 19, "t5");
NexText tHour2   = NexText(8, 18, "t4");
NexText tMinute1 = NexText(8, 17, "t3");
NexText tMinute2 = NexText(8, 16, "t2");

// The variable for whether we are updating G_turnOn or G_turnOff
NexVariable vOnOff = NexVariable(8, 24, "onOrOff");

// The RTC module
NexRtc rtc;

/******************************** END GLOBALS ********************************/

// This is the list that the Nextion lib uses to get serial input from the GUI
NexTouch *nex_listen_list[] = 
{
    &pAction,
    &pColor,
    &pBrightness,
    &pSpeed,
    &pBrightnessSpeed,
    &bSolid,
    &bPulse,
    &bSnake,
    &bAlt,
    &bRainbow,
    &bDance,
    &bOff,
    &bAll,
    &bGreen,
    &bBlue,
    &bPurple,
    &bWhite,
    &bCyan,
    &b1,
    &b2,
    &b3,
    &b4,
    &b5,
    &b6,
    &b7,
    &b8,
    &b9,
    &b0,
    &bSetDefaults,
    &hBrightness,
    &hSpeed,
    &hBrightness2,
    &hSpeed2,
    &vOnOff,
    NULL
};

/******************************************************************************
**
**  Function Name: showErrorLights
**
**  Purpose: To light up all the lights red when an error is detected
**
**  Globals Used: none
**
**  Globals Set: G_action, G_color, G_haveUpdate
**
******************************************************************************/
void showErrorLights()
{
    G_action = LED_SOLID;
    G_color = RED;
    G_haveUpdate = true; // no G_color selection, go ahead and update
}


/******************************************************************************
**
**  Function Name: buttonActionPopCallback
**
**  Purpose: To check which G_action button was pressed and decide if 
**           the LEDs need instruction
**
**  Globals Used: none
**
**  Globals Set: G_action, G_haveUpdate, G_brightnessPosition, G_speedPosition
**
******************************************************************************/
void buttonActionPopCallback(void *ptr)
{
    NexButton *button = (NexButton *) ptr;
    char buffer[100] = {0};

    if (button->getText(buffer, sizeof(buffer)) <= 0)
    {
        showErrorLights();
        return;
    }

    if (strcmp(buffer, "Solid Color") == 0)
    {
        G_action = LED_SOLID;
        pColor.show();
        return;
    }
    else if (strcmp(buffer, "Pulse") == 0)
    {
        G_action = LED_PULSE;
        pColor.show();
        return;
    }
    else if (strcmp(buffer, "Snake") == 0)
    {
        G_action = LED_SNAKE;
        pColor.show();
        return;
    }
    else if (strcmp(buffer, "Alternating") == 0)
    {
        G_action = LED_ALTERNATING;
        pColor.show();
        return;
    }
    else if (strcmp(buffer, "Rainbow") == 0)
    {
        G_action = LED_RAINBOW;
        G_haveUpdate = true; // no G_color selection, go ahead and update

        // set brightness and speed to defaults
        G_brightnessPosition = 127;
        G_speedPosition = 50;
        pBrightnessSpeed.show();
        return;
    }
    else if (strcmp(buffer, "Dance Party") == 0)
    {
        G_action = LED_DANCEPARTY;
        G_haveUpdate = true; // no G_color selection, go ahead and update

        // set brightness and speed to defaults
        G_brightnessPosition = 127;
        G_speedPosition = 50;
        pBrightnessSpeed.show();
        return;
    }
    else if (strcmp(buffer, "Off") == 0)
    {
        G_action = LED_OFF;
        G_haveUpdate = true; // no G_color selection for Off, update now

        // set brightness and speed to defaults
        G_brightnessPosition = 127;
        G_speedPosition = 50;
        return;
    }
    else
    {
        // unexpected value, error and return
        showErrorLights();
        return;
    }
}


/******************************************************************************
**
**  Function Name: buttonColorPopCallback
**
**  Purpose: To check which G_color button was pressed and decide if 
**           the LEDs need instruction
**
**  Globals Used: G_chosen
**
**  Globals Set: color, G_haveUpdate, G_brightnessPosition, G_speedPosition
**               G_chosen
**
******************************************************************************/
void buttonColorPopCallback(void *ptr)
{
    NexButton *button = (NexButton *) ptr;
    char buffer[100] = {0};
    color_t tmpColor;

    if (button->getText(buffer, sizeof(buffer)) <= 0)
    {
        showErrorLights();
        return;
    }

    if (strcmp(buffer, "All") == 0)
    {
        tmpColor = ALL;
    }
    else if (strcmp(buffer, "Green") == 0)
    {
        tmpColor = GREEN;
    }
    else if (strcmp(buffer, "Blue") == 0)
    {
        tmpColor = BLUE;
    }
    else if (strcmp(buffer, "Purple") == 0)
    {
        tmpColor = PURPLE;
    }
    else if (strcmp(buffer, "White") == 0)
    {
        tmpColor = WHITE;
    }
    else if (strcmp(buffer, "Aqua") == 0)
    {
        tmpColor = CYAN;
    }
    else
    {
        // unexpected value, show error and return
        showErrorLights();
        return;
    }

    if (G_chosen == 0)
    {
        G_color = tmpColor;
        if (G_action == LED_ALTERNATING && G_color != ALL)
        {
            G_chosen = 1;
            tChoose.setText("Choose other color");
            return;
        }
    }
    else
    {
        G_color2 = tmpColor;
        G_chosen = 0;
    }

    G_haveUpdate = true;

    // set brightness and speed to defaults
    G_brightnessPosition = 127;
    G_speedPosition = 50;

    // go to the next page
    switch (G_action)
    {
        case LED_SOLID:
            pBrightness.show();
            break;
        case LED_PULSE:
        case LED_ALTERNATING:
            pSpeed.show();
            break;
        case LED_SNAKE:
        case LED_RAINBOW:
        case LED_DANCEPARTY:
            pBrightnessSpeed.show();
            break;
        default:
            showErrorLights();
            break;
    }
}


/******************************************************************************
**
**  Function Name: buttonAutoClockPopCallback
**
**  Purpose: To update the time that the lights automatically turn on or off
**
**  Globals Used: none
**
**  Globals Set: G_turnOn, G_turnOff
**
******************************************************************************/
void buttonAutoClockPopCallback(void *ptr)
{
    uint32_t onOrOff;
    char newTime[4] = {0};
    char buffer[100] = {0};

    if (!vOnOff.getValue(&onOrOff))
    {
        // failed to get variable, light up red and return
        showErrorLights();
        return;
    }

    // get the times currently displayed on the screen
    if (tHour1.getText(buffer, sizeof(buffer)) < 0)
    {
        showErrorLights();
        return;
    }
    else
    {
        newTime[0] = buffer[0];
    }

    if (tHour2.getText(buffer, sizeof(buffer)) < 0)
    {
        showErrorLights();
        return;
    }
    else
    {
        newTime[1] = buffer[0];
    }

    if (tMinute1.getText(buffer, sizeof(buffer)) < 0)
    {
        showErrorLights();
        return;
    }
    else
    {
        newTime[2] = buffer[0];
    }

    if (tMinute2.getText(buffer, sizeof(buffer)) < 0)
    {
        showErrorLights();
        return;
    }
    else
    {
        newTime[3] = buffer[0];
    }
  
    if (onOrOff == 0) // update auto on time
    {
        memcpy(G_turnOn, newTime, sizeof(G_turnOn));
    }
    else if (onOrOff == 1) // update auto off time
    {
        memcpy(G_turnOff, newTime, sizeof(G_turnOn));
    }
    else
    {
        // unexpected value, light up red and return
        showErrorLights();
        return;
    }
}


/******************************************************************************
**
**  Function Name: buttonSetDefaultsPopCallback
**
**  Purpose: To update the G_color and animation for when the lights 
**           automatically turn on
**
**  Globals Used: none
**
**  Globals Set: G_defaultAction, DEFAULT_color_t, DEFAULT_color_t2, 
**               G_defaultBrightness, G_defaultSpeed
**
******************************************************************************/
void buttonSetDefaultsPopCallback(void *ptr)
{
    G_defaultAction     = G_action;
    G_defaultColor      = G_color;
    G_defaultColor2     = G_color2;
    G_defaultBrightness = G_brightnessPosition;
    G_defaultSpeed      = G_speedPosition;
}


/******************************************************************************
**
**  Function Name: sliderBrightnessPopCallback
**
**  Purpose: To update the brightness when the slider is moved on the screen
**
**  Globals Used: none
**
**  Globals Set: G_brightnessPosition
**
******************************************************************************/
void sliderBrightnessPopCallback(void *ptr)
{
    // use the ptr passed so this function works for both brightness sliders
    NexSlider *slider = (NexSlider *) ptr;
    slider->getValue(&G_brightnessPosition);
    G_haveUpdate = true;
}


/******************************************************************************
**
**  Function Name: sliderBrightnessPopCallback
**
**  Purpose: To update the speed of animations when the slider is 
**           moved on the screen
**
**  Globals Used: none
**
**  Globals Set: G_speedPosition
**
******************************************************************************/
void sliderSpeedPopCallback(void *ptr)
{
    // use the ptr passed so this function works for both speed sliders
    NexSlider *slider = (NexSlider *) ptr;
    slider->getValue(&G_speedPosition); // position between 0 and 100
    G_haveUpdate = true;
}


/******************************************************************************
**
**  Function Name: sendInstructions
**
**  Purpose: To send instructions to the other arduino that is 
**           controlling the LEDs
**
**  Globals Used: action, color, color2, G_brightnessPosition, G_speedPosition
**
**  Globals Set: none
**
******************************************************************************/
void sendInstructions()
{
    Wire.beginTransmission(INSTRUCTION_PIN);
    Wire.write(G_action);
    Wire.write(G_color);
    Wire.write(G_color2);
    Wire.write(G_brightnessPosition);
    Wire.write(G_speedPosition);
    Wire.endTransmission();
}

/******************************************************************************
**
**  Function Name: autoOn
**
**  Purpose: To automatically turn the lights on at a time set by the user
**
**  Globals Used: G_turnOn, G_turnOff, G_onTime
**
**  Globals Set: action, color, color2, G_brightnessPosition, 
**               G_speedPosition, G_haveUpdate, G_onTime
**
******************************************************************************/
void autoOn(char timeBuf[30])
{
    // if it's after the auto on time and we haven't turned on, turn on
    if (timeBuf[11] == G_turnOn[0] && timeBuf[12] == G_turnOn[1] &&
        timeBuf[14] == G_turnOn[2] && timeBuf[15] == G_turnOn[3] &&
        G_onTime == false)
    {
        if (G_action == LED_OFF) // only do this if the lights are already off
        {
            // set the default action
            G_action = G_defaultAction;

            // set the default colors
            G_color = G_defaultColor;
            G_color2 = G_defaultColor2;

            // set brightness and speed to defaults
            G_brightnessPosition = G_defaultBrightness;
            G_speedPosition = G_defaultSpeed;

            G_haveUpdate = true; // defaults set, let's update
        }
        G_onTime = true;
    }
}


/******************************************************************************
**
**  Function Name: autoOff
**
**  Purpose: To automatically turn the lights off at a time set by the user
**
**  Globals Used: G_turnOn, G_turnOff
**
**  Globals Set: action, G_haveUpdate, G_onTime, G_brightnessPosition, 
**               G_speedPosition
**
******************************************************************************/
void autoOff(char timeBuf[30])
{
    // if it's after the auto off time and lights are on, turn off
    if (timeBuf[11] == G_turnOff[0] && timeBuf[12] == G_turnOff[1] &&
        timeBuf[14] == G_turnOff[2] && timeBuf[15] == G_turnOff[3] &&
        G_onTime == true)
    {
        G_action = LED_OFF;
        G_haveUpdate = true;
        G_onTime = false;
        G_brightnessPosition = 127;
        G_speedPosition = 50;
    }
}


/******************************************************************************
**
**  Function Name: setup
**
**  Purpose: An initialization function that is standard to arduinos
**
**  Globals Used: none
**
**  Globals Set: none
**
******************************************************************************/
void setup(void)
{
    // The pro micro uses Serial1 to talk to the Nextion display
    // if you are using a different microcontroller this may need
    // to be updated
    Serial1.begin(9600);

    // initialize communication with the other arduino
    Wire.begin();

    pinMode(READY_PIN, INPUT);
    // Attach callback functions to mode selection buttons
    bSolid.attachPop(buttonActionPopCallback, &bSolid);
    bPulse.attachPop(buttonActionPopCallback, &bPulse);
    bSnake.attachPop(buttonActionPopCallback, &bSnake);
    bAlt.attachPop(buttonActionPopCallback, &bAlt);
    bRainbow.attachPop(buttonActionPopCallback, &bRainbow);
    bDance.attachPop(buttonActionPopCallback, &bDance);
    bOff.attachPop(buttonActionPopCallback, &bOff);

    // Attach callback functions to G_color selection buttons
    bAll.attachPop(buttonColorPopCallback, &bAll);
    bGreen.attachPop(buttonColorPopCallback, &bGreen);
    bBlue.attachPop(buttonColorPopCallback, &bBlue);
    bPurple.attachPop(buttonColorPopCallback, &bPurple);
    bWhite.attachPop(buttonColorPopCallback, &bWhite);
    bCyan.attachPop(buttonColorPopCallback, &bCyan);

    // Attach callback functions to sliders
    hBrightness.attachPop(sliderBrightnessPopCallback, &hBrightness);  
    hSpeed.attachPop(sliderSpeedPopCallback, &hSpeed);  
    hBrightness2.attachPop(sliderBrightnessPopCallback, &hBrightness2);  
    hSpeed2.attachPop(sliderSpeedPopCallback, &hSpeed2);

    // Attach callback functions to all number buttons for auto on/off
    b1.attachPop(buttonAutoClockPopCallback, &b1);
    b2.attachPop(buttonAutoClockPopCallback, &b2);
    b3.attachPop(buttonAutoClockPopCallback, &b3);
    b4.attachPop(buttonAutoClockPopCallback, &b4);
    b5.attachPop(buttonAutoClockPopCallback, &b5);
    b6.attachPop(buttonAutoClockPopCallback, &b6);
    b7.attachPop(buttonAutoClockPopCallback, &b7);
    b8.attachPop(buttonAutoClockPopCallback, &b8);
    b9.attachPop(buttonAutoClockPopCallback, &b9);
    b0.attachPop(buttonAutoClockPopCallback, &b1);

    // Attach callback function to default settings buttons
    bSetDefaults.attachPop(buttonSetDefaultsPopCallback, &bSetDefaults);
}


/******************************************************************************
**
**  Function Name: loop
**
**  Purpose: The main loop for the arduino. Sends instructions to the other 
**           arduino and checks if it's time to automatically turn on/off
**
**  Globals Used: G_currentTime, G_previousTime, G_haveUpdate
**
**  Globals Set: G_currentTime, G_previousTime, G_haveUpdate, G_timeBuf
**
******************************************************************************/
void loop()
{
    nexLoop(nex_listen_list);

    // if we have an update and the other arduino is ready, send the update
    if (G_haveUpdate && digitalRead(READY_PIN) == HIGH)
    {
        sendInstructions();
        G_haveUpdate = false;
    }

    // check if it's time to automatically turn off every 45 seconds
    G_currentTime = millis();

    // the second case happens when millis() rolls over to 0
    if (G_currentTime - G_previousTime > 45000 ||
        G_previousTime > G_currentTime)
    {
        rtc.read_rtc_time(G_timeBuf, 30);
        autoOn(G_timeBuf);
        autoOff(G_timeBuf);
        G_previousTime = G_currentTime;
    }
}
