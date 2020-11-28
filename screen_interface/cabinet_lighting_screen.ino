/* Source code for the Under Cabinet Lighting. */
/* The sketch generated will run on the arduino connected to the TFT. */
#include <Wire.h>
#include "Nextion.h"
#include "screen_interface.h"

/***************************** BEGIN GLOBALS *********************************/
/* Note that in order for the nextion library to update variables they must be declared globally. */
// global variables for the light settings: action, color, brightness and speed
Settings G_settings;

// global variables for the auto on and off feature
Defaults G_defaults;

// Note, the Nextion Library expects all TFT objects to be defined globally
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

// The button for setting the default color settings
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

// The variable for whether we are updating turnOn or turnOff
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
**  Globals Set: G_settings
**
******************************************************************************/
void showErrorLights()
{
    Settings *settings = &G_settings;
    settings->action = LED_SOLID;
    settings->color = RED;
    settings->haveUpdate = true;
}


/******************************************************************************
**
**  Function Name: buttonActionPopCallback
**
**  Purpose: To check which action button was pressed and decide if 
**           the LEDs need instruction
**
**  Globals Used: none
**
**  Globals Set: G_settings
**
******************************************************************************/
void buttonActionPopCallback(void *ptr)
{
    Settings *settings = &G_settings;
    NexButton *button = (NexButton *) ptr;
    char buffer[100] = {0};

    if (button->getText(buffer, sizeof(buffer)) <= 0)
    {
        showErrorLights();
        return;
    }

    if (strcmp(buffer, "Solid Color") == 0)
    {
        settings->action = LED_SOLID;
        pColor.show();
        return;
    }
    else if (strcmp(buffer, "Pulse") == 0)
    {
        settings->action = LED_PULSE;
        pColor.show();
        return;
    }
    else if (strcmp(buffer, "Snake") == 0)
    {
        settings->action = LED_SNAKE;
        pColor.show();
        return;
    }
    else if (strcmp(buffer, "Alternating") == 0)
    {
        settings->action = LED_ALTERNATING;
        pColor.show();
        return;
    }
    else if (strcmp(buffer, "Rainbow") == 0)
    {
        settings->action = LED_RAINBOW;
        settings->haveUpdate = true; // no color selection, go ahead and update

        // set brightness and speed to defaults
        settings->brightnessPosition = 127;
        settings->speedPosition = 50;
        pBrightnessSpeed.show();
        return;
    }
    else if (strcmp(buffer, "Dance Party") == 0)
    {
        settings->action = LED_DANCEPARTY;
        settings->haveUpdate = true; // no color selection, go ahead and update

        // set brightness and speed to defaults
        settings->brightnessPosition = 127;
        settings->speedPosition = 50;
        pBrightnessSpeed.show();
        return;
    }
    else if (strcmp(buffer, "Off") == 0)
    {
        settings->action = LED_OFF;
        settings->haveUpdate = true; // no color selection for Off, update now

        // set brightness and speed to defaults
        settings->brightnessPosition = 127;
        settings->speedPosition = 50;
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
**  Purpose: To check which color button was pressed and decide if 
**           the LEDs need instruction
**
**  Globals Used: G_settings
**
**  Globals Set: G_settings
**
******************************************************************************/
void buttonColorPopCallback(void *ptr)
{
    Settings *settings = &G_settings;

    NexButton *button = (NexButton *) ptr;
    char buffer[100] = {0};
    ColorVal tmpColor;

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

    if (settings->chosen == 0)
    {
        settings->color = tmpColor;
        if (settings->action == LED_ALTERNATING && settings->color != ALL)
        {
            settings->chosen = 1;
            tChoose.setText("Choose other color");
            return;
        }
    }
    else
    {
        settings->color2 = tmpColor;
        settings->chosen = 0;
    }

    settings->haveUpdate = true;

    // set brightness and speed to defaults
    settings->brightnessPosition = 127;
    settings->speedPosition = 50;

    // go to the next page
    switch (settings->action)
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
**  Globals Set: G_defaults
**
******************************************************************************/
void buttonAutoClockPopCallback(void *ptr)
{
    Defaults *defaults = &G_defaults;
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
        memcpy(defaults->turnOn, newTime, sizeof(defaults->turnOn));
    }
    else if (onOrOff == 1) // update auto off time
    {
        memcpy(defaults->turnOff, newTime, sizeof(defaults->turnOff));
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
**  Purpose: To update the G_settings->color and animation for when the lights 
**           automatically turn on
**
**  Globals Used: none
**
**  Globals Set: G_defaults
**
******************************************************************************/
void buttonSetDefaultsPopCallback(void *ptr)
{
    Settings *settings = &G_settings;
    Defaults *defaults = &G_defaults;

    defaults->action     = settings->action;
    defaults->color      = settings->color;
    defaults->color2     = settings->color2;
    defaults->brightness = settings->brightnessPosition;
    defaults->speed      = settings->speedPosition;
}


/******************************************************************************
**
**  Function Name: sliderBrightnessPopCallback
**
**  Purpose: To update the brightness when the slider is moved on the screen
**
**  Globals Used: none
**
**  Globals Set: G_settings
**
******************************************************************************/
void sliderBrightnessPopCallback(void *ptr)
{
    Settings *settings = &G_settings;
    // use the ptr passed so this function works for both brightness sliders
    NexSlider *slider = (NexSlider *) ptr;
    slider->getValue(&settings->brightnessPosition);
    settings->haveUpdate = true;
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
**  Globals Set: G_settings
**
******************************************************************************/
void sliderSpeedPopCallback(void *ptr)
{
    Settings *settings = &G_settings;
    // use the ptr passed so this function works for both speed sliders
    NexSlider *slider = (NexSlider *) ptr;
    slider->getValue(&settings->speedPosition); // position between 0 and 100
    settings->haveUpdate = true;
}


/******************************************************************************
**
**  Function Name: sendInstructions
**
**  Purpose: To send instructions to the other arduino that is 
**           controlling the LEDs
**
**  Globals Used: G_settings
**
**  Globals Set: none
**
******************************************************************************/
void sendInstructions()
{
    Settings *settings = &G_settings;
    Wire.beginTransmission(INSTRUCTION_PIN);
    Wire.write(settings->action);
    Wire.write(settings->color);
    Wire.write(settings->color2);
    Wire.write(settings->brightnessPosition);
    Wire.write(settings->speedPosition);
    Wire.endTransmission();
}

/******************************************************************************
**
**  Function Name: autoOn
**
**  Purpose: To automatically turn the lights on at a time set by the user
**
**  Globals Used: G_defaults
**
**  Globals Set: G_settings
**
******************************************************************************/
void autoOn(char timeBuf[30])
{
    Settings *settings = &G_settings;
    Defaults *defaults = &G_defaults;

    // if it's after the auto on time and we haven't turned on, turn on
    if (timeBuf[11] == defaults->turnOn[0] && 
        timeBuf[12] == defaults->turnOn[1] &&
        timeBuf[14] == defaults->turnOn[2] &&
        timeBuf[15] == defaults->turnOn[3] &&
        defaults->onTime == false)
    {
        if (settings->action == LED_OFF) // only do this if the lights are already off
        {
            // set the default action
            settings->action = defaults->action;

            // set the default colors
            settings->color = defaults->color;
            settings->color2 = defaults->color2;

            // set brightness and speed to defaults
            settings->brightnessPosition = defaults->brightness;
            settings->speedPosition = defaults->speed;

            settings->haveUpdate = true; // defaults set, let's update
        }
        defaults->onTime = true;
    }
}


/******************************************************************************
**
**  Function Name: autoOff
**
**  Purpose: To automatically turn the lights off at a time set by the user
**
**  Globals Used: G_defaults
**
**  Globals Set: G_settings
**
******************************************************************************/
void autoOff(char timeBuf[30])
{
    Settings *settings = &G_settings;
    Defaults *defaults = &G_defaults;

    // if it's after the auto off time and lights are on, turn off
    if (timeBuf[11] == defaults->turnOff[0] &&
        timeBuf[12] == defaults->turnOff[1] &&
        timeBuf[14] == defaults->turnOff[2] &&
        timeBuf[15] == defaults->turnOff[3] &&
        defaults->onTime == true)
    {
        settings->action = LED_OFF;
        settings->haveUpdate = true;
        defaults->onTime = false;
        settings->brightnessPosition = 127;
        settings->speedPosition = 50;
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
    // variables for tracking how much time has passed.
    unsigned long currentTime;
    unsigned long previousTime = 0;
    char          timeBuf[30]  = {0};

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

    // Attach callback functions to G_settings->color selection buttons
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

    Settings *settings = &G_settings;

    // the main loop to update the follower arduino
    while (1)
    {
        nexLoop(nex_listen_list);
    
        // if we have an update and the other arduino is ready, send the update
        if (settings->haveUpdate && digitalRead(READY_PIN) == HIGH)
        {
            sendInstructions();
            settings->haveUpdate = false;
        }
    
        // check if it's time to automatically turn off every 45 seconds
        currentTime = millis();
    
        // the second case happens when millis() rolls over to 0
        if (currentTime - previousTime > 45000 ||
            previousTime > currentTime)
        {
            rtc.read_rtc_time(timeBuf, 30);
            autoOn(timeBuf);
            autoOff(timeBuf);
            previousTime = currentTime;
        }
    }
}
