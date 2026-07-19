#include "headfile.h"
#include "Encoder.h"
#include "EncoderExti.h"

/*
 * Two-wheel odometry. The encoders are allocated here (static) and hung off the
 * Motors_t aggregate, then EncoderExti wires the ISR to their EncoderCount.
 */

static void encoder_zero(Encoder_t* e)
{
    e->EncoderCount = 0;
    e->TotalCount   = 0;
    e->sample       = 0;
    e->vel          = 0.0f;
    e->V            = 0.0f;
    e->X            = 0.0f;
}

void EncodersInit(Motors_t* Motors)
{
    static Encoder_t encoderLeft;
    static Encoder_t encoderRight;

    Motors->EncoderLeft  = &encoderLeft;
    Motors->EncoderRight = &encoderRight;

    encoder_zero(Motors->EncoderLeft);
    encoder_zero(Motors->EncoderRight);

    EncoderExtiInit();   /* enable GPIOB interrupt now that counters exist */
}

void EncoderDataUpdate(Motors_t* Motors)
{
    Encoder_t* L = Motors->EncoderLeft;
    Encoder_t* R = Motors->EncoderRight;

    /* Snapshot + clear the ISR counters (brief; ISR only ticks these). */
    L->sample = L->EncoderCount;  L->EncoderCount = 0;
    R->sample = R->EncoderCount;  R->EncoderCount = 0;

    /* counts/window -> rad/s : (counts * f_ctrl / lines_per_rev) * 2*PI */
    const float k = (ControlFrequency / EncoderLines) * 2.0f * PI;

    /* Low-pass (0.1 new / 0.9 old, ~50 ms tau) to tame the coarse quantization
     * at 200 Hz: 1 count = ~14.5 cm/s, so raw V jumps in big steps. Heavy
     * filtering keeps the PID feedback smooth enough not to reverse-swing. */
    L->vel = (L->sample * k) * 0.1f + L->vel * 0.9f;
    R->vel = (R->sample * k) * 0.1f + R->vel * 0.9f;

    L->V = L->vel * TireRadius;   /* cm/s */
    R->V = R->vel * TireRadius;

    L->X += L->V / ControlFrequency;  /* cm */
    R->X += R->V / ControlFrequency;
}

float EncoderTotalLengthGet(Motors_t* Motors)
{
    return (Motors->EncoderLeft->X + Motors->EncoderRight->X) / 2.0f;
}

float EncoderDeltaLengthGet(Motors_t* Motors)
{
    return (Motors->EncoderLeft->X - Motors->EncoderRight->X) / 2.0f;
}

void EncoderLengthClear(Motors_t* Motors)
{
    Motors->EncoderLeft->X  = 0.0f;
    Motors->EncoderRight->X = 0.0f;
}
