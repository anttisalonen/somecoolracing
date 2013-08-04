#include <cassert>

#include "Car.h"

using Common::Vector2;

ThrottleForce::ThrottleForce(const Vector2& attachpos, float power)
	: mAttachPos(attachpos),
	mPower(power)
{
}

void ThrottleForce::updateForce(Abyss::RigidBody* body, Abyss::Real duration)
{
	if(!mThrottle)
		return;

	body->addForceAtBodyPoint(Vector2(0.0, 1.0) * mPower * mThrottle, mAttachPos);
}

void ThrottleForce::setThrottle(float f)
{
	assert(f >= 0.0f && f <= 1.0f);
	mThrottle = f;
}

Car::Car(float w, float l, Abyss::World* world)
	: mWidth(w),
	mLength(l),
	mPhysicsWorld(world),
	mLBThrottleForce(ThrottleForce(Vector2(-mWidth * 0.45f, -mLength * 0.45f), 10000.0f)),
	mRBThrottleForce(ThrottleForce(Vector2(mWidth * 0.45f, -mLength * 0.45f), 10000.0f))
{
	mRigidBody.setMass(1000.0);
	mPhysicsWorld->addBody(&mRigidBody);
	mPhysicsWorld->getForceRegistry()->add(&mRigidBody, &mLBThrottleForce);
	mPhysicsWorld->getForceRegistry()->add(&mRigidBody, &mRBThrottleForce);
}

Car::~Car()
{
	mPhysicsWorld->getForceRegistry()->remove(&mRigidBody, &mLBThrottleForce);
	mPhysicsWorld->getForceRegistry()->remove(&mRigidBody, &mRBThrottleForce);
	mPhysicsWorld->removeBody(&mRigidBody);
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
	mLBThrottleForce.setThrottle(value);
	mRBThrottleForce.setThrottle(value);
}

Abyss::RigidBody* Car::getBody()
{
	return &mRigidBody;
}

void Car::setBrake(float value)
{
}


