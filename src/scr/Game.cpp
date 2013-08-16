#include "Game.h"
#include "GameDriver.h"

bool Game::run(const char* carname)
{
	GameDriver driver(800, 600, "Some Cool Racing", carname);
	driver.run();
	return true;
}


