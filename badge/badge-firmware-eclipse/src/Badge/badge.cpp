#include "badge.h"
#include "stdint.h"
#include "gui.h"
#include <tim.h>
#include <usart.h>
#include <string.h>

#define ONE_TIME 0

#define IR_TX 1

int state = 0;
int tmp = 0;

void IRInit(void) {
  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.Pin = IR_UART2_TX_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(IR_UART2_TX_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = IR_UART2_RX_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(IR_UART2_RX_GPIO_Port, &GPIO_InitStruct);

  HAL_GPIO_WritePin(IR_UART2_TX_GPIO_Port, IR_UART2_TX_Pin, GPIO_PIN_RESET);
}

#define CYCLES_PER_LOOP 5
inline void wait_cycles( uint32_t n ) {
    uint32_t l = n/CYCLES_PER_LOOP;
    asm volatile( "0:" "SUBS %[count], 1;" "BNE 0b;" :[count]"+r"(l) );
}

void IRStart(void) {
  HAL_GPIO_WritePin(IR_UART2_TX_GPIO_Port, IR_UART2_TX_Pin, GPIO_PIN_SET);
  wait_cycles(37895); // 30 cycles
  HAL_GPIO_WritePin(IR_UART2_TX_GPIO_Port, IR_UART2_TX_Pin, GPIO_PIN_RESET);
  wait_cycles(37895); // 30 cycles
}

void IRZero(void) {
  HAL_GPIO_WritePin(IR_UART2_TX_GPIO_Port, IR_UART2_TX_Pin, GPIO_PIN_SET);
  wait_cycles(18947); // 15 cycles
  HAL_GPIO_WritePin(IR_UART2_TX_GPIO_Port, IR_UART2_TX_Pin, GPIO_PIN_RESET);
  wait_cycles(18947); // 15 cycles
}

void IROne(void) {
  HAL_GPIO_WritePin(IR_UART2_TX_GPIO_Port, IR_UART2_TX_Pin, GPIO_PIN_SET);
  wait_cycles(18947); // 15 cycles
  HAL_GPIO_WritePin(IR_UART2_TX_GPIO_Port, IR_UART2_TX_Pin, GPIO_PIN_RESET);
  wait_cycles(18947); // 15 cycles
  wait_cycles(18947); // 15 cycles
  wait_cycles(18947); // 15 cycles
}

void IRTxByte(uint8_t byte) {
  for (int8_t bit = 7; bit >= 0; bit--){
      if((byte & (0x01 << bit)) == 0x00) {
          IRZero();
      } else {
          IROne();
      }
  }
}

void IRTxBuff(uint8_t *buff, size_t len) {
  IRStart();
  for(uint8_t byte = 0; byte < len; byte++) {
      IRTxByte(buff[byte]);
  }
}

void delay(uint32_t time) {
  HAL_Delay(time);
}

void initUARTIR() {
  HAL_TIM_OC_Start(&htim2, TIM_CHANNEL_2);
}

void stopUARTIR() {
  HAL_TIM_OC_Stop(&htim2, TIM_CHANNEL_2);
}

uint32_t nextStateSwitchTime = 0;

void startBadge() {

  initUARTIR();
  IRInit();

}

int counter = 0;

void checkStateTimer(int nextState, int timeToNextSwitch) {
  if (nextStateSwitchTime < HAL_GetTick()) {
    state = nextState;
    nextStateSwitchTime = HAL_GetTick() + timeToNextSwitch;
  }
}

uint32_t lastSendTime = 0;

void loopBadge() {
  char buf[128];

  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
  HAL_Delay(500);

#if IR_TX
  uint8_t str[] = "tx!\n";
  IRTxBuff(str, strlen((const char *)str));
  HAL_UART_Transmit(&huart3, str, strlen((const char *)str), 500);

#else // IR_RX

#endif

  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
  HAL_Delay(500);
}

