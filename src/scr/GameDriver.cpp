#include "GameDriver.h"

#include "common/Math.h"

GameDriver::GameDriver(unsigned int screenWidth, unsigned int screenHeight,
		const char* caption, const char* carname)
	: Driver(screenWidth, screenHeight, caption),
	mWorld(carname),
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

	if((mSteeringVelocity > 0.0f && mSteering < 0.0f) ||
			(mSteeringVelocity < 0.0f && mSteering > 0.0f))
		mSteering = 0.0f;

	if(mSteeringVelocity) {
		mSteering += mSteeringVelocity * frameTime;
		mSteering = Common::clamp(-1.0f, mSteering, 1.0f);
	}

	if(!mSteeringWithMouse && mSteeringVelocity == 0.0f && mSteering != 0.0f) {
		if(fabs(mSteering) < 4.0f * frameTime)
			mSteering = 0.0f;
		else if(mSteering < 0.0f)
			mSteering += 4.0f * frameTime;
		else if(mSteering > 0.0f)
			mSteering -= 4.0f * frameTime;
	}

	car->setSteering(mSteering);
	mWorld.updatePhysics(frameTime);
	mZoom += mZoomSpeed * frameTime;
	mZoom = mRenderer.setZoom(mZoom);
	mRenderer.setSteering(mThrottle, mBrake, mSteering);

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
			if(!mSteeringWithMouse)
				mThrottle = 1.0f;
			break;

		case SDLK_s:
			if(!mSteeringWithMouse)
				mBrake = 1.0f;
			break;

		case SDLK_a:
			if(!mSteeringWithMouse)
				mSteeringVelocity = -2.0f;
			break;

		case SDLK_d:
			if(!mSteeringWithMouse)
				mSteeringVelocity = 2.0f;
			break;

		case SDLK_c:
			mCamOrientation = !mCamOrientation;
			mRenderer.setCamOrientation(mCamOrientation);
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

		case SDLK_r:
			if(SDL_GetModState() & KMOD_SHIFT) {
				mWorld.resetCar();
			}
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
			if(!mSteeringWithMouse)
				mThrottle = 0.0f;
			break;

		case SDLK_s:
			if(!mSteeringWithMouse)
				mBrake = 0.0f;
			break;

		case SDLK_a:
			if(!mSteeringWithMouse)
				mSteeringVelocity = 0.0f;
			break;

		case SDLK_d:
			if(!mSteeringWithMouse)
				mSteeringVelocity = 0.0f;
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

bool GameDriver::handleMouseMotion(float frameTime, const SDL_MouseMotionEvent& ev)
{
	if(mSteeringWithMouse) {
#if 0
		mSteering += ev.xrel * 0.01f;
		mThrottle -= mBrake;
		mThrottle -= ev.yrel * 0.01f;
		if(mThrottle < 0.0f) {
			mBrake = -mThrottle;
			mThrottle = 0.0f;
		}
#else
		mSteering = (ev.x / (float)getScreenWidth() * 2.0f) - 1.0f;
		float y = ((1.0f - ev.y / (float)getScreenHeight()) * 2.0f) - 1.0f;
		mThrottle = std::max(0.0f, y);
		mBrake = std::max(0.0f, -y);
#endif

		mSteering = Common::clamp(-1.0f, mSteering, 1.0f);
		mThrottle = Common::clamp(0.0f, mThrottle, 1.0f);
		mBrake = Common::clamp(0.0f, mBrake, 1.0f);
	}

	return false;
}

bool GameDriver::handleMousePress(float frameTime, Uint8 button)
{
	if(button == SDL_BUTTON_LEFT) {
		mSteeringWithMouse = !mSteeringWithMouse;
		if(mSteeringWithMouse) {
			mSteering = 0.0f;
			mSteeringVelocity = 0.0f;
			mThrottle = 0.0f;
			mBrake = 0.0f;
			SDL_WarpMouse(getScreenWidth() / 2, getScreenHeight() / 2);
		}
	}

	if(button == SDL_BUTTON_WHEELUP) {
		mZoom -= 0.02f;
	} else if(button == SDL_BUTTON_WHEELDOWN) {
		mZoom += 0.02f;
	}

	return false;
}


