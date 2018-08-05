/**
 * File:   rtp.c
 * Author: Ericson Joseph
 *
 * Created on July 28, 2018,  2:08 PM
 */

#include "rtp.h"

#define PL_PCMA 0x08 

static uint16_t seqNum;
static uint32_t timestamp;
static uint32_t ssrc;

static void setRTPValue16ToBuffer(uint8_t *buffer, uint32_t index, uint16_t value) {
    uint16_t hight = value & 0xFF00;
    hight >>= 8;
    *(buffer + index)     = (uint8_t)hight;
    uint16_t low          = value & 0x00FF;
    *(buffer + index + 1) = (uint8_t)low;
}


static void setRTPValue32ToBuffer(uint8_t *buffer, uint32_t index, uint32_t value) {
    uint32_t a = value & 0xFF000000;
    a >>= 24;
    *(buffer + index) = (uint8_t) a;
    a = value & 0x00FF0000;
    a >>= 16;
    *(buffer + index + 1) = (uint8_t) a;
    a = value & 0x000000FF00;
    a >>= 8;
    *(buffer + index + 2) = (uint8_t) a;
    a = value & 0x00000000FF;
    *(buffer + index + 3) = (uint8_t) a;
}

void RTP_Init(void) {
    seqNum    = 1;
    timestamp = 20;
    ssrc      = 16384;
}

void RTP_AddHeader(uint8_t *frame) {
    frame[0]     = 0x80;
    frame[1]     = PL_PCMA;
    setRTPValue32ToBuffer(frame, 8, ssrc);
}

void RTP_AddHeaderMark(uint8_t *frame) {
    frame[0]     = 0x80;
    frame[1]     = PL_PCMA | 0x80;
    setRTPValue32ToBuffer(frame, 8, ssrc);
}

void RTP_AddVarHeader(uint8_t *frame) {
    setRTPValue16ToBuffer(frame, 2, seqNum);
    setRTPValue32ToBuffer(frame, 4, timestamp);
    seqNum++;
    timestamp += 20;
}
