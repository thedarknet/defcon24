#include "GameOfLife.h"
#include <ssd1306.h>

GameOfLife::GameOfLife() :
		Generations(0), CurrentGeneration(0), Neighborhood(0) {
}

GameOfLife::~GameOfLife() {

}

ErrorType GameOfLife::onInit() {
	initGame();
	return ErrorType();
}

static uint32_t displayMessageUntil = 0;
bool ReInitGame = false;

ReturnStateContext GameOfLife::onRun(QKeyboard &kb) {
	uint32_t now = HAL_GetTick();
	if (now < displayMessageUntil) {
		gui_lable_multiline(&UtilityBuf[0], 0, 10, 128, 64, 0, 0);
	} else if (ReInitGame) {
		initGame();
	} else {
		uint16_t count = 0;
		uint8_t bitToCheck = CurrentGeneration%32;
		for (uint16_t j = 1; j < height - 1; j++) {
			for (uint16_t k = 1; k < width - 1; k++) {
				if ((gol[j] & (k << bitToCheck)) != 0) {
					SSD1306_DrawPixel(k, j, SSD1306_COLOR_WHITE);
					count++;
				}
			}
		}
		if (0 == count) {
			sprintf(&UtilityBuf[0], "ALL DEAD\nAfter %d\ngenerations", CurrentGeneration);
			displayMessageUntil = now + 3000;
			ReInitGame = true;
		} else {
			unsigned int tmp[sizeof(gol)];
			life(&gol[0], Neighborhood, width, height, &tmp[0]);
		}
		if (now % 3 == 0) {
			CurrentGeneration++;
			if (CurrentGeneration >= Generations) {
				ReInitGame = true;
			}
		}
	}
	if (kb.getLastPinSeleted() == QKeyboard::NO_PIN_SELECTED) {
		return ReturnStateContext(this);
	} else {
		return ReturnStateContext(StateFactory::getMenuState());
	}
}

ErrorType GameOfLife::onShutdown() {
	return ErrorType();
}

void GameOfLife::initGame() {
	ReInitGame = false;
	uint32_t start = HAL_GetTick();
	displayMessageUntil = start + 3000;
	CurrentGeneration = 0;
	Neighborhood = (start & 1) == 0 ? 'm' : 'v';
	srand(start);
	short chanceToBeAlive = rand() % 25;
	memset(&gol[0], 0, sizeof(gol));
	unsigned int tmp[height];
	for (int j = 1; j < height - 1; j++) {
		for (int i = 1; i < width - 1; i++) {
			if ((rand() % chanceToBeAlive) == 0) {
				gol[j] |= (1 << i);
			} else {
				//gol[j] |= (1<<i);
			}
		}
	}
	Generations = 50 + (rand() % 75);
	gui_lable_multiline((const char*) "Max Generations: ", 0, 10, 128, 64, 0, 0);
	sprintf(&UtilityBuf[0], "Max\nGenerations:\n%d", Generations);
}
//The life function is the most important function in the program.
//It counts the number of cells surrounding the center cell, and
//determines whether it lives, dies, or stays the same.
void GameOfLife::life(unsigned int *array, char choice, short width, short height, unsigned int *temp) {
	//Copies the main array to a temp array so changes can be entered into a grid
	//without effecting the other cells and the calculations being performed on them.
	memcpy(&temp[0], &array[0], sizeof(temp));
	for (int j = 1; j < height - 1; j++) {
		for (int i = 1; i < width - 1; i++) {
			if (choice == 'm') {
				//The Moore neighborhood checks all 8 cells surrounding the current cell in the array.
				int count = 0;
				count = ((array[j - 1] & (1 << i)) > 0 ? 1 : 0) + ((array[j - 1] & (1 << (i - 1))) > 0 ? 1 : 0)
						+ ((array[j] & (1 << (i - 1))) > 0 ? 1 : 0) + ((array[j + 1] & (1 << (i - 1))) > 0 ? 1 : 0)
						+ ((array[j + 1] & (1 << i)) > 0 ? 1 : 0) + ((array[j + 1] & (1 << (i + 1))) > 0 ? 1 : 0)
						+ ((array[j] & (1 << (i + 1))) > 0 ? 1 : 0) + ((array[j - 1] & (1 << (i + 1))) > 0 ? 1 : 0);
				if (count < 2 || count > 3)
					temp[j] &= ~(1 << i);
				if (count == 3)
					temp[j] |= (1 << i);
			} else if (choice == 'v') {
				//The Von Neumann neighborhood checks only the 4 surrounding cells in the array, (N, S, E, and W).
				int count = 0;
				count = ((array[j - 1] & (1 << i)) > 0 ? 1 : 0) + ((array[j] & (1 << (i - 1))) > 0 ? 1 : 0)
						+ ((array[j + 1] & (1 << i)) > 0 ? 1 : 0) + ((array[j] & (1 << (i + 1))) > 0 ? 1 : 0);
				//The cell dies.
				if (count < 2 || count > 3)
					temp[j] &= ~(1 << i);
				//The cell either stays alive, or is "born".
				if (count == 3)
					temp[j] |= (1 << i);
			}
		}
	}
	//Copies the completed temp array back to the main array.
	memcpy(&array[0], &temp[0], sizeof(temp));
}
