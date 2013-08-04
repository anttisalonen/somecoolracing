#include "Game.h"
#include "GameDriver.h"

bool Game::run()
{
	GameDriver driver(800, 600, "Some Cool Racing");
	driver.run();
	return true;
}


