#include "Vision.h"
#include "System.h"
#include "headfile.h"

/*
 * Byte-fed frame assembler for the K230 link. The state machine resynchronises on
 * the two fixed header bytes (0xAA, 0x0D), buffers a full 13-byte frame, verifies
 * the checksum, then publishes a decoded Vision_t. Sign bytes are restored so
 * dx/dy carry the "move in this direction to close the error" convention.
 *
 * Runs entirely in ISR context (called per received byte); keep it allocation-free
 * and branch-light. Only whole, valid frames mutate the published struct, so a
 * reader in the main loop always sees a consistent snapshot.
 */

static Vision_t s_vision;

static uint8_t  s_buf[VISION_FRAME_LEN];
static uint8_t  s_idx = 0;

void Vision_Init(Vision_t **v)
{
    memset(&s_vision, 0, sizeof(s_vision));
    s_vision.Flag = VISION_FLAG_LOST;
    s_idx = 0;
    if (v) *v = &s_vision;
}

/* Restore a signed pixel value from a (sign, abs) byte pair.
 * sign byte: 0 = positive, 1 = negative. */
static inline int16_t signed_px(uint8_t sign, uint8_t abs)
{
    return sign ? -(int16_t)abs : (int16_t)abs;
}

void Vision_Feed(uint8_t byte)
{
    /* Header resync: byte 0 must be 0xAA, byte 1 must be 0x0D. */
    if (s_idx == 0) {
        if (byte != VISION_FRAME_HEAD) return;       /* wait for head */
    } else if (s_idx == 1) {
        if (byte != VISION_FRAME_LEN2) {             /* bad length; maybe this is a fresh head */
            s_idx = (byte == VISION_FRAME_HEAD) ? 1 : 0;
            if (s_idx == 1) s_buf[0] = VISION_FRAME_HEAD;
            return;
        }
    }

    s_buf[s_idx++] = byte;

    if (s_idx < VISION_FRAME_LEN) return;
    s_idx = 0;                                       /* frame complete; reset for next */

    /* Checksum: low 8 bits of the sum of bytes 0..11. */
    uint8_t chk = 0;
    for (int i = 0; i < VISION_FRAME_LEN - 1; i++) chk += s_buf[i];
    if (chk != s_buf[VISION_FRAME_LEN - 1]) return;  /* drop corrupt frame */

    s_vision.Flag      = s_buf[2];
    s_vision.DxCenter  = signed_px(s_buf[3], s_buf[4]);
    s_vision.DyCenter  = signed_px(s_buf[5], s_buf[6]);
    s_vision.BaseIndex = s_buf[7];
    s_vision.DxBase    = signed_px(s_buf[8],  s_buf[9]);
    s_vision.DyBase    = signed_px(s_buf[10], s_buf[11]);
    s_vision.FrameCount++;
    s_vision.LastRxMs  = millis();
}

bool Vision_IsFresh(void)
{
    if (s_vision.FrameCount == 0) return false;
    return (millis() - s_vision.LastRxMs) <= VISION_STALE_MS;
}
