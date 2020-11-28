#include "shared.h"

typedef struct
{
    LedAction action             = LED_OFF;
    int       chosen             = 0; // for alternating we will need to wait for two colors to be G_chosen
    boolean   haveUpdate         = false;
    boolean   onTime             = false;
    uint32_t  brightnessPosition = 127;
    uint32_t  speedPosition      = 50;
    ColorVal  color;
    ColorVal  color2;
} Settings;

typedef struct
{
    LedAction action      = LED_SOLID;
    ColorVal  color       = CYAN;
    ColorVal  color2      = GREEN;
    uint32_t  brightness  = 255;
    uint32_t  speed       = 99;
    char      turnOn[4]   = {'1', '9', '0', '0'}; // 7pm
    char      turnOff[4]  = {'0', '1', '0', '0'}; // 1am
    bool      onTime      = false;
} Defaults;
