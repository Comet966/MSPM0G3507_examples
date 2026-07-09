#include "headfile.h"
#include "Key.h"

/* Pull-up, active-low: pin LOW when pressed → 1 = pressed */
#define read_key   ((GPIO_KEY_PORT->DIN31_0 & GPIO_KEY_START_PIN) ? 0x00 : 0x01)

/*
 * Single START button (S2). Debounced press toggles the run state:
 *   stopped → press → CarStart (reset lap counter + odometer, run line-follow)
 *   running → press → CarStop
 *
 * Called at 200 Hz from duty_200hz(). KeyPressCount counts consecutive
 * "held" samples; we act on RELEASE (falling edge of the pressed state) once
 * the press has lasted a few ticks, which rejects contact bounce.
 * NOTE: no blocking delays here — this runs inside the control ISR path.
 */

#define KEY_DEBOUNCE_TICKS   5   /* ~25 ms at 200 Hz */

static void KeyPressProcess(Car_t* Car)
{
    if (Car->Tasks->CarStartFlag)
        CarStop(*Car);
    else
        CarStart(*Car);
}

void KeyDataUpdate(Car_t* Car)
{
    Car->Tasks->KeyPressFlag = read_key;

    if (Car->Tasks->KeyPressFlag) {
        Car->Tasks->KeyPressCount++;
    }

    /* Act on release, only if the press was held past the debounce window. */
    if (!Car->Tasks->KeyPressFlag && Car->Tasks->LastKeyPressFlag) {
        if (Car->Tasks->KeyPressCount >= KEY_DEBOUNCE_TICKS) {
            KeyPressProcess(Car);
        }
        Car->Tasks->KeyPressCount = 0;
    }

    Car->Tasks->LastKeyPressFlag = Car->Tasks->KeyPressFlag;
}
