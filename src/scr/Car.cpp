#include <cassert>

#include <algorithm>
#include <fstream>
#include <sstream>

#include "Car.h"

#include "common/Math.h"

using Common::Vector2;


static TyreConfig tyreconfig;

static void init_TyreConfig()
{
	using namespace std;
	static bool init = false;
	if(!init) {
		init = true;
		ifstream source;
		source.open("share/tyre.conf", ios_base::in);

		string line;
		while(getline(source, line)) {
			istringstream in(line);
			in >> tyreconfig.mCorneringForceCoefficient >>
				tyreconfig.mSelfAligningTorqueCoefficient >>
				tyreconfig.mRollingFrictionCoefficient;
		}
	}
}

TyreForce::TyreForce(const Vector2& attachpos)
	: mAttachPos(attachpos)
{
	init_TyreConfig();
}

void TyreForce::updateForce(Abyss::RigidBody* body, Abyss::Real duration)
{
	Vector2 force;
	Vector2 spinDir = body->orientation;
	Vector2 tyreDir = Common::Math::rotate2D(spinDir, mAngle);
	Vector2 velDir = body->velocity.normalized();
	float speed = body->velocity.length();
	if(speed) {
		// cornering force - lateral
		static const float coefficient = tyreconfig.mCorneringForceCoefficient;
		auto slipAngle = velDir.cross2d(tyreDir);
		Vector2 latForce, rollingFriction;
		// slipAngle may be nan when the magnitude of velDir and/or tyreDir
		// is slightly above 1 (floating point)
		if(!isnan(slipAngle)) {
			latForce = Common::Math::rotate2D(tyreDir, HALF_PI);
			latForce = latForce * slipAngle * coefficient;

			// rolling friction - force opposite to velocity
			rollingFriction = velDir * -speed * tyreconfig.mRollingFrictionCoefficient;
			auto ang = std::max<float>(mBrake, slipAngle);
			rollingFriction = rollingFriction * (1.0f + ang * 100.0f);
		}

		// self aligning torque - force in the direction of the tyre
		Vector2 selfAlign = tyreDir * speed * tyreconfig.mSelfAligningTorqueCoefficient;

		assert(!isnan(latForce.x));
		assert(!isnan(selfAlign.x));
		assert(!isnan(rollingFriction.x));
		force = latForce + selfAlign + rollingFriction;
	}

	// throttle
	if(mThrottle)
		force = force + tyreDir * mThrottle * 1000.0f;

	if(!force.null()) {
		Vector2 lws = body->getPointInWorldSpace(mAttachPos);
		body->addForceAtPoint(force, lws);
	}
}

void TyreForce::setAngle(float f)
{
	using namespace Common;
	// 0.8f rad = 45 degrees
	assert(f >= -0.7f && f <= 0.7f);
	mAngle = f;
}

void TyreForce::setThrottle(float f)
{
	assert(f >= 0.0f && f <= 1.0f);
	mThrottle = f;
}

void TyreForce::setBrake(float f)
{
	assert(f >= 0.0f && f <= 1.0f);
	mBrake = f;
}

Car::Car(float w, float l, Abyss::World* world)
	: mWidth(w),
	mLength(l),
	mPhysicsWorld(world),
	mLBTyreForce(TyreForce(Vector2(-mLength * 0.5f, -mWidth * 0.45f))),
	mRBTyreForce(TyreForce(Vector2(mLength * 0.5f, -mWidth * 0.45f))),
	mLFTyreForce(TyreForce(Vector2(-mLength * 0.5f, mWidth * 0.45f))),
	mRFTyreForce(TyreForce(Vector2(mLength * 0.5f, mWidth * 0.45f)))
{
	static const float mass = 1000.0f;
	mRigidBody.setMass(mass);
	mRigidBody.setInertiaTensor(mass * (1.0 / 12.0) * ((mWidth * mWidth) + (mLength * mLength)));
	mRigidBody.angularDamping = 0.2;

	mPhysicsWorld->addBody(&mRigidBody);
	mPhysicsWorld->getForceRegistry()->add(&mRigidBody, &mLBTyreForce);
	mPhysicsWorld->getForceRegistry()->add(&mRigidBody, &mRBTyreForce);
	mPhysicsWorld->getForceRegistry()->add(&mRigidBody, &mLFTyreForce);
	mPhysicsWorld->getForceRegistry()->add(&mRigidBody, &mRFTyreForce);
}

Car::~Car()
{
	mPhysicsWorld->getForceRegistry()->remove(&mRigidBody, &mLBTyreForce);
	mPhysicsWorld->getForceRegistry()->remove(&mRigidBody, &mRBTyreForce);
	mPhysicsWorld->getForceRegistry()->remove(&mRigidBody, &mLFTyreForce);
	mPhysicsWorld->getForceRegistry()->remove(&mRigidBody, &mRFTyreForce);
	mPhysicsWorld->removeBody(&mRigidBody);
}

const Common::Vector2& Car::getPosition() const
{
	return mRigidBody.position;
}

float Car::getOrientation() const
{
	return atan2(mRigidBody.orientation.x, mRigidBody.orientation.y);
}

float Car::getSpeed() const
{
	return mRigidBody.velocity.length();
}

void Car::setThrottle(float value)
{
	assert(value >= 0.0f && value <= 1.0f);
	mLBTyreForce.setThrottle(value);
	mRBTyreForce.setThrottle(value);
}

Abyss::RigidBody* Car::getBody()
{
	return &mRigidBody;
}

void Car::setBrake(float value)
{
	assert(value >= 0.0f && value <= 1.0f);
	mLBTyreForce.setBrake(value);
	mRBTyreForce.setBrake(value);
}

void Car::setSteering(float value)
{
	assert(value >= -1.0f && value <= 1.0f);
	mSteering = -value;
	mLFTyreForce.setAngle(mSteering * 0.2f);
	mRFTyreForce.setAngle(mSteering * 0.2f);
}


