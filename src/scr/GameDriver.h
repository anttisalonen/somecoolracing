#ifndef SCR_GAMEDRIVER_H
#define SCR_GAMEDRIVER_H

#include "common/DriverFramework.h"

class GameDriver : public Common::Driver {
	public:
		GameDriver(unsigned int screenWidth, unsigned int screenHeight, const char* caption);
		virtual bool init();
		virtual void render();
		virtual void drawFrame();
};

#endif

