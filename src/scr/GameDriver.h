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

	private:
		Renderer mRenderer;
		GameWorld mWorld;
};

#endif

