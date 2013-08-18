#include "Game.h"
#include "GameDriver.h"

bool Game::run(const char* carname, const char* trackname)
{
	GameDriver driver(800, 600, "Some Cool Racing", carname, trackname);
	driver.run();
	return true;
}


