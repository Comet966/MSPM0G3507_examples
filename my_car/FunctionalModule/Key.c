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

/* IR remote receiver on PA28 (SysConfig group GPIO_IR, pin IR_DAT), input pull-up.
 * A 3-pin (VCC/GND/DAT) wireless module — no encoding, DAT just asserts while the
 * paired transmitter is active. Treated as a *second* start/stop button that runs
 * the identical KeyShortPressProcess() (toggle CarStartFlag + one beep), so S2 keeps
 * working untouched and either input toggles the car.
 *
 * Polarity: default active-LOW (asserted = DAT low), matching S2 and the pull-up's
 * idle-high rest state. If on-board the remote toggles inversely (idle low, pressed
 * high), flip this one macro to `!= 0`. */
#define IR_PRESSED()      (DL_GPIO_readPins(GPIO_IR_PORT, GPIO_IR_IR_DAT_PIN) == 0)
#define IR_STABLE_SAMPLES  30   /* ~150 ms of a CONTINUOUS clean level to accept (rejects EMI/chatter) */

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

/* IR remote start/stop. Hardened against the noise/chatter a cheap 3-pin wireless
 * receiver emits on DAT when idle (worse once the motors run and couple EMI into
 * this pull-up line — that was the cause of the car entering STOP on its own).
 *
 * Instead of a short debounce + edge, this requires the asserted level to hold
 * CONTINUOUSLY for IR_STABLE_SAMPLES (~150 ms): any single glitch resets the counter,
 * so brief noise never accumulates. Fires the toggle exactly ONCE when the threshold
 * is first crossed, then latches until DAT returns to idle (one press = one toggle,
 * no matter how long it is held). Call at 200 Hz alongside KeyDataUpdate(). */
void IrDataUpdate(Car_t* Car)
{
    static uint16_t irStable  = 0;   /* consecutive asserted samples */
    static uint8_t  irLatched = 0;   /* 1 = already toggled for this press */

    if (IR_PRESSED()) {
        if (irStable < 0xFFFF) irStable++;

        if (irStable >= IR_STABLE_SAMPLES && !irLatched) {
            KeyShortPressProcess(Car);   /* same toggle + beep as S2 */
            irLatched = 1;               /* one toggle per press */
        }
    } else {
        /* Idle level → require a clean release before re-arming. A lone idle sample
         * mid-press also resets the stability counter, so noise can't sneak through. */
        irStable  = 0;
        irLatched = 0;
    }
}
