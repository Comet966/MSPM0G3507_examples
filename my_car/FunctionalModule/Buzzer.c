#include "ti_msp_dl_config.h"
#include "Buzzer.h"

/*
 * Buzzer tone = PWM on TIMG6 (SysConfig instance PWM_Buzzer -> PWM_Buzzer_INST).
 * We gate the tone by starting/stopping the timer counter. Per the SDK TIMER guide,
 * silencing means stop the counter AND drive the pin low (CC=0 is unreliable).
 *
 * Beat pattern (advanced at 10 Hz by BuzzerDataUpdate): for each queued beep,
 * ON for BEEP_ON_TICKS, then OFF for BEEP_OFF_TICKS.
 */

#define BEEP_ON_TICKS   2   /* 2 * 100ms = 200ms tone */
#define BEEP_OFF_TICKS  2   /* 2 * 100ms = 200ms gap  */

static void buzzer_on(void)
{
    DL_TimerG_startCounter(PWM_Buzzer_INST);
}

static void buzzer_off(void)
{
    DL_TimerG_stopCounter(PWM_Buzzer_INST);
    DL_GPIO_clearPins(GPIO_PWM_Buzzer_C0_PORT, GPIO_PWM_Buzzer_C0_PIN);
}

void BuzzerInit(Buzzer_t** Buzzer)
{
    static Buzzer_t buzzer;
    *Buzzer = &buzzer;
    buzzer.BuzzerFlag     = 1;              /* 1 = in ON phase */
    buzzer.BuzzerCount    = BEEP_ON_TICKS;
    buzzer.BuzzerBeeCount = 0;
    buzzer_off();
}

void BuzzerDataUpdate(Buzzer_t* Buzzer)
{
    if (Buzzer->BuzzerBeeCount != 0) {
        if (Buzzer->BuzzerFlag) {          /* ON phase */
            buzzer_on();
            if (--Buzzer->BuzzerCount == 0) {
                Buzzer->BuzzerFlag  = 0;
                Buzzer->BuzzerCount = BEEP_OFF_TICKS;
            }
        } else {                            /* OFF phase */
            buzzer_off();
            if (--Buzzer->BuzzerCount == 0) {
                Buzzer->BuzzerFlag  = 1;
                Buzzer->BuzzerCount = BEEP_ON_TICKS;
                Buzzer->BuzzerBeeCount--;   /* one beep completed */
            }
        }
    } else {
        buzzer_off();
    }
}

void BuzzerBee(Buzzer_t* Buzzer, uint8_t times)
{
    Buzzer->BuzzerBeeCount += times;
}
