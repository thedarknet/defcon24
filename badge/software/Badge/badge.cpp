
#include "badge.h"
#include "stdint.h"
#include "gui.h"
#include <RH_RF69.h>
#include <Keyboard.h>
#include <KeyStore.h>

GUI_TickerData td("HI FROM STM32",10,5,120,10);
char sendingBuf[64]={'\0'};
char receivingBuf[64]={'\0'};

int state = 0;

RH_RF69 Radio(RFM69_SPI_NSS_Pin,RFM69_Interrupt_DIO0_Pin);
//QKeyboard KB(QKeyboard::PinConfig(GPIOA,8),QKeyboard::PinConfig(GPIOA,9),QKeyboard::PinConfig(GPIOA,10)
//	,QKeyboard::PinConfig(GPIOB,12),QKeyboard::PinConfig(GPIOB,13),QKeyboard::PinConfig(GPIOB,14),QKeyboard::PinConfig(GPIOB,15));

//start at 45K for 19K
ContactStore MyContacts(0x800B000,0x4E00);
	
void delay(uint32_t time) {
	HAL_Delay(time);
}

void startBadge() {
	gui_init();
	Radio.init();
	Radio.setHeaderFrom(1);
	Radio.setHeaderTo(2);
	Radio.setHeaderId(0);
	ContactStore::MyInfo mi = MyContacts.getMyInfo();
	MyContacts.init();
	mi.setAgentname("Demetrius");
	mi.setUniqueID(1);
	uint8_t pubkey[ContactStore::PUBLIC_KEY_LENGTH] = {0};
	mi.setPublicKey(pubkey);
	//Radio.send((const uint8_t*)"HI",strlen("HI"));
	//printf("Here");
	//gui_ticker(&td);
	//td.bg
}

int counter = 0;
int nextStateSwitchTime = 0;

void loopBadge() {
	gui_ticker(&td);
	td.x++;
	if(td.x>127) td.x=0;
	gui_draw();
	
	gui_text(&sendingBuf[0],0,20,0);
	gui_text(&receivingBuf[0],0,30,0);
	//KB.scan();
	
	switch(state) {
		case 0:
			{
				sprintf(sendingBuf,"Case 0:");
				if(nextStateSwitchTime==0) {
					nextStateSwitchTime = HAL_GetTick() + 2000;
				}
				if(nextStateSwitchTime<HAL_GetTick()) {
					state = 1;
				}
			}
			break;
		case 1:
			sprintf(sendingBuf,"Sending 'Hi for the %d time'",counter++);
			//Radio.send((const uint8_t*)&sendingBuf[0],strlen(sendingBuf));
			nextStateSwitchTime = HAL_GetTick()+10000;
			state = 2;
			break;
		case 2:
		{
			if(Radio.available()>0) {
				uint8_t bufSize = 64;
				memset(&receivingBuf[0],0,64);
				//Radio.recv((uint8_t*)&receivingBuf[0],&bufSize);
				nextStateSwitchTime = HAL_GetTick()+2000;
				state = 0;
			} else {
				if(nextStateSwitchTime<HAL_GetTick()) {
					state = 0;
					nextStateSwitchTime = HAL_GetTick()+2000;
					sprintf(&receivingBuf[0],"timed out");
				}
				HAL_Delay(10);
			}
		}
		break;
		
	}
	//const char * str = "HI STM32";
	//gui_text(str,10,10,SSD1306_COLOR_WHITE);
	
}

