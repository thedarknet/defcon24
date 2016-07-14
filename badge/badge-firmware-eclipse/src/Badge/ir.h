
#ifndef __IR_H__
#define __IR_H__

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdbool.h>

#define CYCLES_PER_LOOP 5

inline void wait_cycles( uint32_t n ) {
    uint32_t l = n/CYCLES_PER_LOOP;
    asm volatile( "0:" "SUBS %[count], 1;" "BNE 0b;" :[count]"+r"(l) );
}

void IRInit(void);
void IRTxBuff(uint8_t *buff, size_t len);
int32_t IRRxBlocking(uint32_t timeout_ms);

int32_t IRBytesAvailable();
uint8_t *IRGetBuff();
bool IRDataReady();
void IRStartRx();
int32_t IRGetState();

#ifdef __cplusplus
}
#endif

#endif
