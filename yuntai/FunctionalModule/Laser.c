#include "ti_msp_dl_config.h"
#include "Laser.h"

/* LASER is a plain push-pull output (GPIO_STEP group, PB12), active HIGH. */

static bool s_on = false;

void Laser_Init(void)
{
    DL_GPIO_clearPins(GPIO_STEP_PORT, GPIO_STEP_LASER_PIN);
    s_on = false;
}

void Laser_On(void)
{
    DL_GPIO_setPins(GPIO_STEP_PORT, GPIO_STEP_LASER_PIN);
    s_on = true;
}

void Laser_Off(void)
{
    DL_GPIO_clearPins(GPIO_STEP_PORT, GPIO_STEP_LASER_PIN);
    s_on = false;
}

void Laser_Set(bool on)
{
    if (on) Laser_On();
    else    Laser_Off();
}

bool Laser_IsOn(void)
{
    return s_on;
}
