#include "Car.h"

Car::Car(float w, float l)
	: mWidth(w),
	mLength(l)
{
	mRigidBody.setMass(1000.0);
}

const Common::Vector2& Car::getPosition() const
{
	return mRigidBody.position;
}

float Car::getOrientation() const
{
	return mRigidBody.orientation;
}

void Car::setThrottle(float value)
{
	assert(value >= 0.0f && value <= 1.0f);
	mThrottle = value;
}

Abyss::RigidBody* Car::getBody()
{
	return &mRigidBody;
}

