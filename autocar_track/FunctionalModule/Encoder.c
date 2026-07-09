#include "headfile.h"
#include "Encoder.h"
#include "EncoderExti.h"

static void encoder_zero(Encoder_t* e)
{
    e->EncoderCount = 0;
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

    EncoderExtiInit();
}

void EncoderDataUpdate(Motors_t* Motors)
{
    Encoder_t* L = Motors->EncoderLeft;
    Encoder_t* R = Motors->EncoderRight;

    L->sample = L->EncoderCount;  L->EncoderCount = 0;
    R->sample = R->EncoderCount;  R->EncoderCount = 0;

    const float k = (ControlFrequency / EncoderLines) * 2.0f * PI;

    L->vel = (L->sample * k) * 0.1f + L->vel * 0.9f;
    R->vel = (R->sample * k) * 0.1f + R->vel * 0.9f;

    L->V = L->vel * TireRadius;
    R->V = R->vel * TireRadius;

    L->X += L->V / ControlFrequency;
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
