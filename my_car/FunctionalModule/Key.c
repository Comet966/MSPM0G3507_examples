#include "ti_msp_dl_config.h"
#include "Key.h"
#include "Buzzer.h"

/*
 * Button on PB21 (SysConfig GPIO group GPIO_KEY, pin START), input with pull-up.
 * Pressed = pin LOW (active-low). Debounced by counting held 200 Hz samples;
 * the action fires on release (falling of the held state).
 *
 * Phase 2 scope: short press toggles CarStartFlag + one beep.
 * Long press hook kept for Phase 6 (currently mirrors short press).
 */

#define KEY_PRESSED()  (DL_GPIO_readPins(GPIO_KEY_PORT, GPIO_KEY_START_PIN) == 0)
#define SHORT_MIN_SAMPLES  4    /* ~20 ms debounce at 200 Hz */
#define LONG_SAMPLES       100  /* ~500 ms held = long press */

void KeyShortPressProcess(Car_t* Car)
{
    Car->Tasks->CarStartFlag = !Car->Tasks->CarStartFlag;  /* toggle run state */
    BuzzerBee(Car->Buzzer, 1);
}

void KeyLongPressProcess(Car_t* Car)
{
    /* Phase 6 will add lap/reset semantics; for now: stop + double beep. */
    Car->Tasks->CarStartFlag = 0;
    BuzzerBee(Car->Buzzer, 2);
}

void KeyDataUpdate(Car_t* Car)
{
    Task_t* T = Car->Tasks;

    T->KeyPressFlag = KEY_PRESSED() ? 1 : 0;

    if (T->KeyPressFlag) {
        T->KeyPressCount++;
    }

    /* Act on release. */
    if (!T->KeyPressFlag && T->LastKeyPressFlag) {
        if (T->KeyPressCount >= LONG_SAMPLES) {
            KeyLongPressProcess(Car);
        } else if (T->KeyPressCount >= SHORT_MIN_SAMPLES) {
            KeyShortPressProcess(Car);
        }
        T->KeyPressCount = 0;
    }

    T->LastKeyPressFlag = T->KeyPressFlag;
}
