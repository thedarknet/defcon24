#include "badge.h"
#include "stdint.h"
#include "gui.h"
#include "ir.h"
#include <tim.h>
#include <usart.h>
#include <string.h>
#include <stdio.h>

#define ONE_TIME 0

int state = 0;
int tmp = 0;

void uartPrint(const char* buff) {
    HAL_UART_Transmit(&huart3, (uint8_t *)buff, strlen((const char *)buff), 500);
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

void ledInit() {
  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
}

void startBadge() {

  initUARTIR();
  IRInit();
  ledInit();

#if IR_TX
  uartPrint("Transmit Mode!\n");
#else
  uartPrint("Receive Mode!\n");
  IRStartRx();
#endif

}

int counter = 0;

void checkStateTimer(int nextState, int timeToNextSwitch) {
  if (nextStateSwitchTime < HAL_GetTick()) {
    state = nextState;
    nextStateSwitchTime = HAL_GetTick() + timeToNextSwitch;
  }
}

uint32_t lastSendTime = 0;
uint32_t txCount = 0;
void loopBadge() {
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
  HAL_Delay(500);

#if IR_TX
  uint8_t buff[128];
  snprintf((char *)buff, sizeof(buff), "DCDN! %ld", txCount++);
  IRTxBuff(buff, strlen((const char *)buff));
  uartPrint((const char *)buff);
#else // IR_RX
  uint8_t buff[128];
  if(IRDataReady() == true) {
      snprintf((char *)buff, sizeof(buff), "Data Received! %ld \"%s\"\n", IRBytesAvailable(), IRGetBuff());
      uartPrint((const char *)buff);

      IRStartRx(); // Prepare for next RX
  }
#endif

  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
  HAL_Delay(500);
}

