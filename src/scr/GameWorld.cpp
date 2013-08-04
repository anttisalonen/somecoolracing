#include "GameWorld.h"

GameWorld::GameWorld()
	: mCar(Car(3.0f, 7.0f))
{
	mPhysicsWorld.addBody(mCar.getBody());
}

void GameWorld::updatePhysics(float time)
{
	mPhysicsWorld.startFrame();
	mPhysicsWorld.runPhysics(time);
}

const Car* GameWorld::getCar() const
{
	return &mCar;
}

