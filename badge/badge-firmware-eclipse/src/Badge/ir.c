/*
 * ir.c
 *
 *  Created on: Jul 12, 2016
 *      Author: alvaro
 */

#include "stm32f1xx_hal.h"
#include "ir.h"

void IRInit(void) {
  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.Pin = IR_UART2_TX_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(IR_UART2_TX_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = IR_UART2_RX_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(IR_UART2_RX_GPIO_Port, &GPIO_InitStruct);

  HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);

  HAL_GPIO_WritePin(IR_UART2_TX_GPIO_Port, IR_UART2_TX_Pin, GPIO_PIN_RESET);
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

void IRIRQ() {
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if( GPIO_Pin & IR_UART2_RX_Pin) {
      IRIRQ();
  }
}

