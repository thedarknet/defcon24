/*
 * ir.c
 *
 *  Created on: Jul 12, 2016
 *      Author: alvaro
 */

#include "stm32f1xx_hal.h"
#include "ir.h"

typedef enum {
  IR_RX_IDLE = 0,
  IR_RX_START = 1,
  IR_RX_MARK_START = 2,
  IR_RX_MARK = 3,
  IR_RX_SPACE = 4,
  IR_RX_DONE = 5,
  IR_RX_ERR = -1,
  IR_RX_ERR_TIMEOUT = -2
} IRState_t;

typedef enum {
 IR_RX = 0,
 IR_TX
} IRMode_t;

static volatile IRState_t IRState;
static volatile IRMode_t IRMode;
static volatile uint8_t irRxBuff[128];
static volatile uint32_t irRxBits;

TIM_HandleTypeDef htim3;

// Number of TIM3 ticks for mark/space/start pulses
#define TICK_BASE (400)
#define MARK_TICKS (TICK_BASE)
#define START_TICKS (TICK_BASE * 2)
#define SPACE_ZERO_TICKS (TICK_BASE)
#define SPACE_ONE_TICKS (TICK_BASE * 3)

// Margin to account for time delay in measuring input pulses during receive
// I tried 50 which didn't always work, 100 seems pretty stable
#define RX_MARGIN 100

//
// Configure timer 3 to measure incoming IR pulse widths
// 48MHz clock divided by 32 gives us 1.5MHz timer clock
// With a 4096 period, timer overflows in ~2.73ms
//
void TIM3_Init() {

  TIM_ClockConfigTypeDef sClockSourceConfig;

  __HAL_RCC_TIM3_CLK_ENABLE();

  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 32;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 4096;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  HAL_TIM_Base_Init(&htim3);

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig);

  __HAL_TIM_CLEAR_FLAG(&htim3, TIM_SR_UIF);

  HAL_NVIC_SetPriority(TIM3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(TIM3_IRQn);
}

void delayTicks(uint32_t ticks) {
  IRMode = IR_TX;
  HAL_TIM_Base_Stop_IT(&htim3);
  TIM3->CNT = 0;
  TIM3->ARR = ticks;
  __HAL_TIM_CLEAR_FLAG(&htim3, TIM_SR_UIF);
  HAL_TIM_Base_Start_IT(&htim3);

  while(IRMode == IR_TX) {
      __WFI();
  }
}

void startIRPulseTimer() {
  TIM3->CNT = 0;
  __HAL_TIM_CLEAR_FLAG(&htim3, TIM_SR_UIF);
  HAL_TIM_Base_Start_IT(&htim3);
}

void stopIRPulseTimer() {
  HAL_TIM_Base_Stop_IT(&htim3);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  if (htim == &htim3) {
      stopIRPulseTimer();
      if(IRMode == IR_RX) {
        // Timed out :(
        IRState = IR_RX_ERR_TIMEOUT;
      } else if (IRMode == IR_TX) {
          IRMode = IR_RX;
      }
  }
}

void IRInit(void) {
  // IR Transmit GPIO configuration
  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.Pin = IR_UART2_TX_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(IR_UART2_TX_GPIO_Port, &GPIO_InitStruct);

  // Turn off IR LED by default
  HAL_GPIO_WritePin(IR_UART2_TX_GPIO_Port, IR_UART2_TX_Pin, GPIO_PIN_RESET);

  // IR Receive GPIO configuration
  GPIO_InitStruct.Pin = IR_UART2_RX_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(IR_UART2_RX_GPIO_Port, &GPIO_InitStruct);

  // Receive interrupt
  HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 0);
  HAL_NVIC_DisableIRQ(EXTI3_IRQn);

  // Pulse measuring timer for receive
  TIM3_Init();

}

// Transmit start pulse
void IRStart(void) {
  HAL_GPIO_WritePin(IR_UART2_TX_GPIO_Port, IR_UART2_TX_Pin, GPIO_PIN_SET);
  delayTicks(START_TICKS);
  HAL_GPIO_WritePin(IR_UART2_TX_GPIO_Port, IR_UART2_TX_Pin, GPIO_PIN_RESET);
  delayTicks(START_TICKS);
}

void IRStop(void) {
  HAL_GPIO_WritePin(IR_UART2_TX_GPIO_Port, IR_UART2_TX_Pin, GPIO_PIN_SET);
  delayTicks(START_TICKS);
  HAL_GPIO_WritePin(IR_UART2_TX_GPIO_Port, IR_UART2_TX_Pin, GPIO_PIN_RESET);
  delayTicks(START_TICKS);
}

// Transmit a zero
void IRZero(void) {
  HAL_GPIO_WritePin(IR_UART2_TX_GPIO_Port, IR_UART2_TX_Pin, GPIO_PIN_SET);
  delayTicks(MARK_TICKS);
  HAL_GPIO_WritePin(IR_UART2_TX_GPIO_Port, IR_UART2_TX_Pin, GPIO_PIN_RESET);
  delayTicks(SPACE_ZERO_TICKS);
}

// Transmit a one
void IROne(void) {
  HAL_GPIO_WritePin(IR_UART2_TX_GPIO_Port, IR_UART2_TX_Pin, GPIO_PIN_SET);
  delayTicks(MARK_TICKS);
  HAL_GPIO_WritePin(IR_UART2_TX_GPIO_Port, IR_UART2_TX_Pin, GPIO_PIN_RESET);
  delayTicks(SPACE_ONE_TICKS);
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
  IRStop();
}

// Shift bits into rx buffer
void IRRxBit(uint8_t newBit) {
  uint32_t byte = irRxBits >> 3;
  uint32_t bit = irRxBits & 0x07;
  if(newBit == 0) {
    irRxBuff[byte] &= ~(1 << (7-bit));
  } else {
    irRxBuff[byte] |= (1 << (7-bit));
  }

  irRxBits++;
}

int32_t IRBytesAvailable() {
  // TODO: Check state to make sure we're not in error
  return (irRxBits >> 3);
}

void IRStartRx() {
  irRxBits = 0;
  IRState = IR_RX_IDLE;
  __HAL_GPIO_EXTI_CLEAR_IT(IR_UART2_RX_Pin);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);
}

int32_t IRRxBlocking(uint32_t timeout_ms) {
  uint32_t timeout = HAL_GetTick() + timeout_ms;
  IRStartRx();
  while((IRState != IR_RX_DONE) && !(IRState < 0) && (HAL_GetTick() < timeout)) {
      // Error condition
      if(IRState < 0) {
          return IRState;
      }
      __WFI();
  }

  if (HAL_GetTick() >= timeout) {
      return IR_RX_ERR_TIMEOUT;
  } else {
      return IRBytesAvailable();
  }
}

int32_t IRGetState() {
  return IRState;
}

bool IRDataReady() {
  if (IRState == IR_RX_DONE) {
      return true;
  } else {
      return false;
  }
}

uint8_t *IRGetBuff() {
  return (uint8_t *)irRxBuff;
}

// Receive GPIO state machine
void IRStateMachine() {
  uint32_t count = TIM3->CNT; // Save timer value as soon as possible
  uint32_t pinState = HAL_GPIO_ReadPin(IR_UART2_RX_GPIO_Port, IR_UART2_RX_Pin);

  // Stop timer to prevent overflow
  stopIRPulseTimer();

  // Add margin to account for measurement delays (interrupt latency, etc)
  count += RX_MARGIN;

  switch(IRState) {
    // Idle, waiting for a start pulse
    case IR_RX_IDLE: {
      if(pinState == 0) {
          startIRPulseTimer(); // Start counting
          IRState = IR_RX_START;
      }
      break;
    }

    // Waiting for start pulse to finish
    case IR_RX_START: {
      // Start pulse received! Start getting bits
      if((pinState == 1) && (count > START_TICKS)) {
          irRxBits = 0;
          IRState = IR_RX_MARK_START;
      } else {
          // Doesn't look like a start pulse, go back to waiting
          IRState = IR_RX_IDLE;
      }
      break;
    }

    case IR_RX_MARK_START: {
      if(pinState == 0) {
        startIRPulseTimer(); // Start timing mark
        IRState = IR_RX_MARK;
      } else {
          IRState = IR_RX_ERR;
      }
      break;
    }

    case IR_RX_MARK: {
      if(pinState == 0) {
        IRState = IR_RX_ERR;
        break;
      }

      if(count > START_TICKS) {
          IRState = IR_RX_DONE;
          HAL_NVIC_DisableIRQ(EXTI3_IRQn);
      } else if(count > MARK_TICKS) {
          startIRPulseTimer(); // Start timing space
          IRState = IR_RX_SPACE;
        } else {
          IRState = IR_RX_ERR;
        }
      break;
    }

    case IR_RX_SPACE: {
      if(pinState == 0) {
        startIRPulseTimer(); // Start timing next mark
        IRState = IR_RX_MARK;
        if (count > SPACE_ONE_TICKS) {
            IRRxBit(1);
        } else if (count > SPACE_ZERO_TICKS) {
            IRRxBit(0);
        } else {
            // Something bad happened
            IRState = IR_RX_ERR;
        }
      }
      break;
    }

    case IR_RX_DONE: {
      // check CRC if there is one?
      break;
    }

    default: {
      break;
    }
  }

  // Disable interrupts if an error occurred (until user resets it)
  if (IRState < 0) {
    HAL_NVIC_DisableIRQ(EXTI3_IRQn);
  }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if(GPIO_Pin & IR_UART2_RX_Pin) {
      IRStateMachine();
  }
}

