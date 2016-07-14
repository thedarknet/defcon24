#include "badge.h"
#include "stdint.h"
#include "gui.h"
#include "ir.h"
#include <tim.h>
#include <usart.h>
#include <string.h>
#include <stdio.h>

#define ONE_TIME 0

#define IR_TX 0 // Set to 1 for transmitter, 0 for receiver

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

#define LED_BLINK_PERIOD (500)
uint32_t ledTimer;

void ledInit() {
  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

  ledTimer = HAL_GetTick() + LED_BLINK_PERIOD;
}

#define IR_MESSAGE_PERIOD (1000)
uint32_t irMessageTimer;
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

  irMessageTimer = HAL_GetTick() + IR_MESSAGE_PERIOD;
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

  if (HAL_GetTick() > ledTimer) {
      ledTimer += LED_BLINK_PERIOD;
      HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
  }

#if IR_TX
  if (HAL_GetTick() > irMessageTimer) {
      irMessageTimer += IR_MESSAGE_PERIOD;
    uint8_t buff[128];
    snprintf((char *)buff, sizeof(buff), "DCDN! %ld", txCount++);
    IRTxBuff(buff, strlen((const char *)buff));
    uartPrint((const char *)buff);
    uartPrint("\n");
  }
#else // IR_RX
  uint8_t buff[128];
  int32_t rxLen = IRRxBlocking(5000);
  if(rxLen > 0) {
      uint8_t *rxBuff = IRGetBuff();
      rxBuff[IRBytesAvailable()] = 0; // Make sure we have a null terminated string
      snprintf((char *)buff, sizeof(buff), "Data Received! %ld \"%s\"\n", IRBytesAvailable(), rxBuff);
      uartPrint((const char *)buff);
  } else {
    snprintf((char *)buff, sizeof(buff), "Timeout :(\n");
    uartPrint((const char *)buff);
  }
#endif
  __WFI();
}

