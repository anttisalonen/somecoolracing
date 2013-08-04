#include "GameWorld.h"

GameWorld::GameWorld()
	: mCar(3.0f, 7.0f, &mPhysicsWorld)
{
}

GameWorld::~GameWorld()
{
}

void GameWorld::updatePhysics(float time)
{
	mPhysicsWorld.startFrame();
	mPhysicsWorld.runPhysics(time);
}

Car* GameWorld::getCar()
{
	return &mCar;
}

const Car* GameWorld::getCar() const
{
	return &mCar;
}

