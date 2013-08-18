#ifndef SCR_GAMEDRIVER_H
#define SCR_GAMEDRIVER_H

#include "Renderer.h"

#include "common/DriverFramework.h"
#include "common/Clock.h"
#include "common/Vector2.h"

#include "GameWorld.h"

class GameDriver : public Common::Driver {
	public:
		GameDriver(unsigned int screenWidth, unsigned int screenHeight,
				const char* caption, const char* carname, const char* trackname);
		bool init() override;
		bool prerenderUpdate(float frameTime) override;
		void drawFrame() override;

		bool handleKeyDown(float frameTime, SDLKey key) override;
		bool handleKeyUp(float frameTime, SDLKey key) override;
		bool handleMouseMotion(float frameTime, const SDL_MouseMotionEvent& ev) override;
		bool handleMousePress(float frameTime, Uint8 button) override;

	private:
		GameWorld mWorld;
		Renderer mRenderer;

		float mThrottle = 0.0f;
		float mBrake = 0.0f;
		float mSteering = 0.0f;
		float mSteeringVelocity = 0.0f;
		float mZoomSpeed = 0.0f;
		float mZoom = 0.15f;

		Common::SteadyTimer mDebugDisplay;

		bool mSteeringWithMouse = false;
		bool mCamOrientation = false;
};

#endif

