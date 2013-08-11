#include "GameDriver.h"

GameDriver::GameDriver(unsigned int screenWidth, unsigned int screenHeight, const char* caption)
	: Driver(screenWidth, screenHeight, caption),
	mRenderer(screenWidth, screenHeight),
	mDebugDisplay(0.2f)
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
	if(!mBrake)
		car->setThrottle(mThrottle);
	if(!mThrottle)
		car->setBrake(mBrake);
	car->setSteering(mSteering);
	mWorld.updatePhysics(frameTime);
	mZoom += mZoomSpeed * frameTime;
	mZoom = mRenderer.setZoom(mZoom);

	if(mDebugDisplay.check(frameTime)) {
		mRenderer.updateDebug(&mWorld);
	}
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

		case SDLK_a:
			mSteering = -1.0f;
			break;

		case SDLK_d:
			mSteering = 1.0f;
			break;

		case SDLK_SPACE:
			std::cout << mWorld.getCar()->getSpeed() << "\n";
			break;

		case SDLK_PLUS:
		case SDLK_KP_PLUS:
			mZoomSpeed = -1.0f;
			break;

		case SDLK_MINUS:
		case SDLK_KP_MINUS:
			mZoomSpeed = 1.0f;
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

		case SDLK_a:
			mSteering = 0.0f;
			break;

		case SDLK_d:
			mSteering = 0.0f;
			break;

		case SDLK_PLUS:
		case SDLK_KP_PLUS:
		case SDLK_MINUS:
		case SDLK_KP_MINUS:
			mZoomSpeed = 0.0f;
			break;

		default:
			break;
	}

	return false;
}


