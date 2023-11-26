// MIT License

// Copyright (c) 2023 Jacob Preston

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// This firmware checks for water presence for the purposes of ensuring that
// a toilet reservoir fills completely after a flush.

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
