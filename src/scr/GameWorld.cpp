#include "GameWorld.h"

GameWorld::GameWorld()
{
	auto carConfig = Car::readCarConfig("share/car.conf");
	mCar = new Car(&carConfig, &mPhysicsWorld, &mTrack);
}

GameWorld::~GameWorld()
{
	delete mCar;
}

void GameWorld::updatePhysics(float time)
{
	mPhysicsWorld.startFrame();
	mPhysicsWorld.runPhysics(time);
	mCar->moved();

	{
		auto carpos = mCar->getPosition();
		Common::Vector2 bl, tr;
		mTrack.getLimits(bl, tr);
		if(carpos.x < bl.x || carpos.y < bl.y ||
				carpos.x > tr.x || carpos.y > tr.y) {
			mCar->setPosition(Common::Vector2());
		}
	}

}

Car* GameWorld::getCar()
{
	return mCar;
}

const Car* GameWorld::getCar() const
{
	return mCar;
}

const Track* GameWorld::getTrack() const
{
	return &mTrack;
}


