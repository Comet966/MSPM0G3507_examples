#include "ti_msp_dl_config.h"
#include "EncoderExti.h"

extern Car_t Car;

/*
 * Dual-port encoder edge counter (11_PID_car wiring).
 *
 *   Left  wheel: A = PA21 (interrupt, rising), B = PA22 (dir level)   -> GPIOA
 *   Right wheel: A = PB19 (interrupt, rising), B = PB20 (dir level)   -> GPIOB
 *
 * On MSPM0G3507 both GPIOA and GPIOB feed the shared INT_GROUP1 vector, so a
 * single GROUP1_IRQHandler dispatches both wheels.
 *
 * Dispatch uses DL_GPIO_getPendingInterrupt() (the CPU_INT.IIDX register), which
 * returns the highest-priority pending pin index AND clears that flag as a
 * read side-effect — the same scheme the known-good 11_PID_car firmware uses.
 * We loop per port until IIDX reports "no more pending" (0), so multiple pins
 * pending at once are all serviced. This avoids the MIS&pins matching issue seen
 * when the earlier getEnabledInterruptStatus()-based handler entered the vector
 * but both branches read status==0.
 *
 * Left  forward convention: on A rising, B low  -> ++, B high -> --
 * Right forward convention: on A rising, B low  -> --, B high -> ++  (mirrored side)
 * Signs are validated by the on-hardware wheel-spin test — if a wheel counts the
 * wrong way, swap the ++/-- in that wheel's branch below.
 */
void GROUP1_IRQHandler(void)
{
    /* GPIOA — left wheel (A-phase PA21 interrupt, B-phase PA22 level). */
    for (;;) {
        uint32_t iidx = DL_GPIO_getPendingInterrupt(GPIO_Encoder_EncoderL_A_PORT);
        if (iidx == GPIO_Encoder_EncoderL_A_IIDX) {
            if (DL_GPIO_readPins(GPIO_Encoder_EncoderL_B_PORT, GPIO_Encoder_EncoderL_B_PIN)) {
                Car.Motors->EncoderLeft->EncoderCount--;
                Car.Motors->EncoderLeft->TotalCount--;
            } else {
                Car.Motors->EncoderLeft->EncoderCount++;
                Car.Motors->EncoderLeft->TotalCount++;
            }
        } else {
            break;   /* 0 = no more GPIOA pins pending */
        }
    }

    /* GPIOB — right wheel (A-phase PB19 interrupt, B-phase PB20 level). */
    for (;;) {
        uint32_t iidx = DL_GPIO_getPendingInterrupt(GPIO_Encoder_EncoderR_A_PORT);
        if (iidx == GPIO_Encoder_EncoderR_A_IIDX) {
            if (DL_GPIO_readPins(GPIO_Encoder_EncoderR_B_PORT, GPIO_Encoder_EncoderR_B_PIN)) {
                Car.Motors->EncoderRight->EncoderCount++;
                Car.Motors->EncoderRight->TotalCount++;
            } else {
                Car.Motors->EncoderRight->EncoderCount--;
                Car.Motors->EncoderRight->TotalCount--;
            }
        } else {
            break;   /* 0 = no more GPIOB pins pending */
        }
    }
}

void EncoderExtiInit(void)
{
    /* Left wheel A-phase is on GPIOA, right wheel A-phase on GPIOB — enable both. */
    NVIC_EnableIRQ(GPIO_Encoder_GPIOA_INT_IRQN);
    NVIC_EnableIRQ(GPIO_Encoder_GPIOB_INT_IRQN);
}
