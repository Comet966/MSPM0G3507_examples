#include "ti_msp_dl_config.h"
#include "Task.h"

/*
 * Racetrack line-follower state machine (北邮 2026: 2 straights + 2 semicircle
 * arcs, smooth closed loop — NO right angles to count).
 *
 * Strategy (user-confirmed):
 *   - Constant cruise speed the whole lap (CAR_CRUISE_SPEED).
 *   - Lap completion judged purely by accumulated centerline odometry:
 *     when EncoderTotalLengthGet() >= LAP_LENGTH, one lap is done → beep,
 *     clear the odometer, increment the lap counter.
 *   - After LAP_COUNT laps, stop the car.
 *   - Steering is pure grayscale closed loop (GraySensorToTurnAngle). On a
 *     lost line (all-white or all-black) we HOLD the last steering command
 *     rather than snapping to zero, so a momentary dropout doesn't jerk the car.
 */

void TaskInit(Task_t** Task)
{
    static Task_t task;
    *Task = &task;

    task.AverageSpeed        = CAR_CRUISE_SPEED;
    task.Deltayaw            = 0;
    task.CarStartFlag        = 0;
    task.KeyPressFlag        = 0;
    task.LastKeyPressFlag    = 0;
    task.KeyPressCount       = 0;
    task.CircleCount         = 0;
    task.RightAngleCount     = 0;
    task.CarBeginLengthCountFlag = 0;
    task.CarBeginLengthCount = 0;
}

void Task(Car_t Car)
{
    /* ---- Steering: grayscale closed loop, hold last command when line lost ---- */
    if (Car.GraySensor->BinaryData != 0x00 && Car.GraySensor->BinaryData != 0xFF) {
        Car.Tasks->Deltayaw = GraySensorToTurnAngle(Car.GraySensor);
    }
    /* else: keep previous Deltayaw (line temporarily out of view) */

    /* ---- Constant cruise speed ---- */
    Car.Tasks->AverageSpeed = CAR_CRUISE_SPEED;

    /* ---- Lap counting by accumulated odometry ---- */
    if (EncoderTotalLengthGet(Car.Motors) >= LAP_LENGTH) {
        EncoderLengthClear(Car.Motors);
        Car.Tasks->CircleCount++;
        BuzzerBee(Car.Buzzer, 1);

        if (Car.Tasks->CircleCount >= LAP_COUNT) {
            CarStop(Car);
            BuzzerBee(Car.Buzzer, 2);
        }
    }
}

void CarStart(Car_t car)
{
    car.Tasks->CircleCount = 0;
    EncoderLengthClear(car.Motors);
    car.Tasks->CarStartFlag = 1;
    BuzzerBee(car.Buzzer, 1);
}

void CarStop(Car_t car)
{
    car.Tasks->CarStartFlag = 0;
    BuzzerBee(car.Buzzer, 1);
}
