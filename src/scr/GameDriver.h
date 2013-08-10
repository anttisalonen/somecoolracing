#ifndef SCR_GAMEDRIVER_H
#define SCR_GAMEDRIVER_H

#include "Renderer.h"

#include "common/DriverFramework.h"

#include "GameWorld.h"

class GameDriver : public Common::Driver {
	public:
		GameDriver(unsigned int screenWidth, unsigned int screenHeight, const char* caption);
		bool init() override;
		bool prerenderUpdate(float frameTime) override;
		void drawFrame() override;

		bool handleKeyDown(float frameTime, SDLKey key) override;
		bool handleKeyUp(float frameTime, SDLKey key) override;

	private:
		Renderer mRenderer;
		GameWorld mWorld;

		float mThrottle = 0.0f;
		float mBrake = 0.0f;
		float mSteering = 0.0f;
		float mZoomSpeed = 0.0f;
		float mZoom = 0.015f;
};

#endif

