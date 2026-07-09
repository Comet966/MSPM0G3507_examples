#include "ti_msp_dl_config.h"
#include "EncoderExti.h"

extern Car_t Car;

/*
 * Shared GPIOB interrupt handler. Both encoder B-phase pins (PB13 left, PB7 right)
 * raise GPIOB_INT_IRQn; we read the pending status and, for each fired B edge,
 * sample that wheel's A-phase to pick the count direction.
 *
 * Left  forward convention: on B rising, A low  -> ++, A high -> --
 * Right forward convention: on B rising, A low  -> --, A high -> ++  (mirrored side)
 * (Signs get validated by the Phase-4 wheel-spin test; adjust here if reversed.)
 */
void GROUP1_IRQHandler(void)
{
    uint32_t status = DL_GPIO_getEnabledInterruptStatus(
        GPIO_Encoder_PORT,
        GPIO_Encoder_EncoderL_B_PIN | GPIO_Encoder_EncoderR_B_PIN);

    if (status & GPIO_Encoder_EncoderL_B_PIN) {
        if (DL_GPIO_readPins(GPIO_Encoder_PORT, GPIO_Encoder_EncoderL_A_PIN)) {
            Car.Motors->EncoderLeft->EncoderCount--;
        } else {
            Car.Motors->EncoderLeft->EncoderCount++;
        }
        DL_GPIO_clearInterruptStatus(GPIO_Encoder_PORT, GPIO_Encoder_EncoderL_B_PIN);
    }

    if (status & GPIO_Encoder_EncoderR_B_PIN) {
        if (DL_GPIO_readPins(GPIO_Encoder_PORT, GPIO_Encoder_EncoderR_A_PIN)) {
            Car.Motors->EncoderRight->EncoderCount++;
        } else {
            Car.Motors->EncoderRight->EncoderCount--;
        }
        DL_GPIO_clearInterruptStatus(GPIO_Encoder_PORT, GPIO_Encoder_EncoderR_B_PIN);
    }
}

void EncoderExtiInit(void)
{
    NVIC_EnableIRQ(GPIO_Encoder_INT_IRQN);
}
