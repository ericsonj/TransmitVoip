/**
 * File:   rtp.h
 * Author: Ericson Joseph
 *
 * Created on July 28, 2018,  2:02 PM
 */

#ifndef __RTP_H
#define __RTP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"

void RTP_Init(void);

void RTP_AddHeader(uint8_t *frame);

void RTP_AddHeaderMark(uint8_t *frame);

void RTP_AddVarHeader(uint8_t *frame);
    
#ifdef __cplusplus
}
#endif

#endif /* __RTP_H */
