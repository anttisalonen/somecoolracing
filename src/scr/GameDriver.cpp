#include "GameDriver.h"

GameDriver::GameDriver(unsigned int screenWidth, unsigned int screenHeight, const char* caption)
	: Driver(screenWidth, screenHeight, caption)
{
}

bool GameDriver::init()
{
	return mRenderer.init();
}

void GameDriver::drawFrame()
{
	mRenderer.drawFrame(&mWorld);
}

bool GameDriver::prerenderUpdate(float frameTime)
{
	mWorld.updatePhysics(frameTime);
	return false;
}

