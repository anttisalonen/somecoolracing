#include "GameDriver.h"

GameDriver::GameDriver(unsigned int screenWidth, unsigned int screenHeight, const char* caption)
	: Driver(screenWidth, screenHeight, caption),
	mRenderer(screenWidth, screenHeight)
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
	auto car = mWorld.getCar();
	car->setThrottle(mThrottle);
	car->setBrake(mBrake);
	mWorld.updatePhysics(frameTime);
	return false;
}

bool GameDriver::handleKeyDown(float frameTime, SDLKey key)
{
	switch(key) {
		case SDLK_ESCAPE:
			return true;

		case SDLK_w:
			mThrottle = 1.0f;
			break;

		case SDLK_s:
			mBrake = 1.0f;
			break;

		default:
			break;
	}

	return false;
}

bool GameDriver::handleKeyUp(float frameTime, SDLKey key)
{
	switch(key) {
		case SDLK_w:
			mThrottle = 0.0f;
			break;

		case SDLK_s:
			mBrake = 0.0f;
			break;

		default:
			break;
	}

	return false;
}


