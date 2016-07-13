
#ifndef __IR_H__
#define __IR_H__

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdbool.h>

#define IR_TX 0

#define CYCLES_PER_LOOP 5

inline void wait_cycles( uint32_t n ) {
    uint32_t l = n/CYCLES_PER_LOOP;
    asm volatile( "0:" "SUBS %[count], 1;" "BNE 0b;" :[count]"+r"(l) );
}

void IRInit(void);
void IRStart(void);
void IRZero(void);
void IROne(void);
void IRTxByte(uint8_t byte);
void IRTxBuff(uint8_t *buff, size_t len);

void IRRXThing();

int32_t IRBytesAvailable();
uint8_t *IRGetBuff();
bool IRDataReady();
void IRStartRx();

#ifdef __cplusplus
}
#endif

#endif
