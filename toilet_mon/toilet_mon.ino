#include <Button.h>

Button stop_alarm_btn(5);

const unsigned int ENERGIZE_PIN             = 10;
const unsigned int ALARM_PIN                = 9;
const unsigned int LED_PIN                  = 8;
const unsigned int LOW_WATER_COUNTS         = 1000;
const unsigned int WATER_PRESENT_LOOP_MS    = 30000;    // 30 second interval while water detected

typedef enum
{
    STATE_WATER_PRESENT,    // 0
    STATE_FIRST_NO_WATER,   // 1
    STATE_NO_WATER_ALARM,   // 2
    STATE_NO_WATER_SILENCE  // 3
} state_t;

state_t state = STATE_WATER_PRESENT;

unsigned int loop_delay_ms = 1000;

void energize()
{
    // Energize the pin to check for voltage
    digitalWrite(ENERGIZE_PIN, 1);
    pinMode(ENERGIZE_PIN, OUTPUT);
}

void denergize()
{
    // Denergize the pin to minimize electrolysis
    digitalWrite(ENERGIZE_PIN, 0);
    pinMode(ENERGIZE_PIN, INPUT_PULLDOWN);
}

void setup()
{
    // 12-bit ADC
    analogReadResolution(12);

    // GPIO Setup
    pinMode(ENERGIZE_PIN, INPUT_PULLDOWN);
    pinMode(ALARM_PIN, OUTPUT);
    pinMode(LED_PIN, OUTPUT);
    pinMode(PIN_LED_13, OUTPUT);

    digitalWrite(ALARM_PIN, 0);
    digitalWrite(LED_PIN, 0);
    digitalWrite(PIN_LED_13, 0);

    stop_alarm_btn.begin();

    state = STATE_WATER_PRESENT;
    loop_delay_ms = WATER_PRESENT_LOOP_MS;
}

void loop()
{
    energize();
    delay(1);
    unsigned int a0_counts = analogRead(A0);
    denergize();

    switch (state)
    {
        case STATE_WATER_PRESENT:
            if (a0_counts < LOW_WATER_COUNTS)
            {
                state = STATE_FIRST_NO_WATER;
                digitalWrite(LED_PIN, 1);
            }
            break;
        case STATE_FIRST_NO_WATER:
            if (a0_counts < LOW_WATER_COUNTS)
            {
                state = STATE_NO_WATER_ALARM;
                digitalWrite(ALARM_PIN, 1);
                loop_delay_ms = 0;
            }
            else
            {
                state = STATE_WATER_PRESENT;
                digitalWrite(LED_PIN, 0);
            }
            break;
        case STATE_NO_WATER_ALARM:
            if (a0_counts > LOW_WATER_COUNTS)
            {
                state = STATE_WATER_PRESENT;
                digitalWrite(LED_PIN, 0);
                digitalWrite(ALARM_PIN, 0);
                loop_delay_ms = WATER_PRESENT_LOOP_MS;
            }

            if (stop_alarm_btn.pressed())
            {
                // Turn off audible alarm
                state = STATE_NO_WATER_SILENCE;
                digitalWrite(ALARM_PIN, 0);
            }
            break;
        case STATE_NO_WATER_SILENCE:
            if (a0_counts > LOW_WATER_COUNTS)
            {
                state = STATE_WATER_PRESENT;
                digitalWrite(LED_PIN, 0);
                loop_delay_ms = WATER_PRESENT_LOOP_MS;
            }
            break;
        default:
            state = STATE_WATER_PRESENT;
    }

    delay(loop_delay_ms);
}
