#include "GameDriver.h"

GameDriver::GameDriver(unsigned int screenWidth, unsigned int screenHeight, const char* caption)
	: Driver(screenWidth, screenHeight, caption)
{
}

bool GameDriver::init()
{
	return true;
}

void GameDriver::render()
{
}

void GameDriver::drawFrame()
{
}


