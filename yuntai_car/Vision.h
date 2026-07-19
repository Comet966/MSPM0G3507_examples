#ifndef __VISION_H
#define __VISION_H

#include <stdint.h>
#include <stdbool.h>
#include "datatype.h"

/*
 * K230 vision link parser (UART2, 115200 8N1).
 *
 * Frame: 13 bytes, one every ~10 ms (see profiles/下位机通信协议_副本.md):
 *   [0]=0xAA head  [1]=0x0D len  [2]=flag(0xBB track/0xCC lost)
 *   [3..6]=center dx/dy (sign,abs)  [7]=base_index  [8..11]=base dx/dy (sign,abs)
 *   [12]=checksum = low 8 bits of sum(bytes 0..11)
 *
 * This milestone: parse only. Vision_Feed() is driven byte-by-byte from the UART2
 * RX ISR; a completed, checksum-valid frame updates the shared Vision_t. Nothing
 * here drives the gimbal yet — that is Gimbal_AimVision (reserved).
 */

#define VISION_FRAME_LEN   13
#define VISION_FRAME_HEAD  0xAA
#define VISION_FRAME_LEN2  0x0D
#define VISION_FLAG_TRACK  0xBB
#define VISION_FLAG_LOST   0xCC

void Vision_Init(Vision_t **v);      /* zero state, publish pointer */
void Vision_Feed(uint8_t byte);      /* call from UART2 RX ISR, one byte per call */
bool Vision_IsFresh(void);           /* true if a valid frame arrived within VISION_STALE_MS */

#endif /* __VISION_H */
