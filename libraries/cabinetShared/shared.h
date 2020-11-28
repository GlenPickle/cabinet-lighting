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
} LedAction;

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
} ColorVal;

