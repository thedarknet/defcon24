#ifndef __IR_H__
#define __IR_H__

#ifdef __cplusplus
extern "C" {
#endif

void IRInit(void);
void IRStop();
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
