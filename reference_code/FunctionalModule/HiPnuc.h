#ifndef __HiPnuc_H
#define __HiPnuc_H

#include "headfile.h"

#define CH_HDR_SIZE             (0x06)              /* 协议头部长度（包含同步字） */
#define HiPnuc_PACKET_SIZE 				80

typedef enum {
	Frame_header  = 0,
	Data 					= 1
} PackData_State;

void pack_data(uint8_t ch);
void HipnucDataUpdate(HiPnuc_t* hipnuc);
void HiPnucInit(HiPnuc_t** HiPnuc);

static int pack_check(uint8_t* data);
static void pack_decode(uint8_t* data,HiPnuc_t* hipnuc);
static void hipnuc_crc16(uint16_t *initial, const uint8_t *buf, uint32_t len);
#endif
