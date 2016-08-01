#ifndef GAME_OF_LIFE_H
#define GAME_OF_LIFE_H

#include "../menus.h"

class GameOfLife: public StateBase {
public:
	GameOfLife();
	virtual ~GameOfLife();
public:
	static const int width = 64;
	static const int height = 64;
protected:
	virtual ErrorType onInit();
	virtual ReturnStateContext onRun(QKeyboard &kb);
	virtual ErrorType onShutdown();
	void initGame();
	void life(unsigned int *array, char choice, short width, short height, unsigned int *temp);
private:
	uint16_t Generations;
	uint16_t CurrentGeneration;
	uint8_t Neighborhood;
	unsigned int gol[height];
	char UtilityBuf[64];
};

#endif
