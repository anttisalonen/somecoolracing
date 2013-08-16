#include <cassert>

#include <algorithm>
#include <fstream>
#include <sstream>

#include <jsoncpp/json/json.h>

#include "Car.h"

#include "common/Math.h"

using Common::Vector2;


TyreForce::TyreForce(const Vector2& attachpos)
	: mAttachPos(attachpos)
{
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
		auto slipAngle = velDir.cross2d(tyreDir);
		Vector2 latForce, rollingFriction;

		latForce = Common::Math::rotate2D(tyreDir, HALF_PI);
		latForce = latForce * slipAngle * mTyreConfig.mCorneringForceCoefficient;

		// self aligning torque - force in the direction of the tyre
		Vector2 selfAlign = tyreDir * speed * mTyreConfig.mSelfAligningTorqueCoefficient;

		// rolling friction - force opposite to velocity
		rollingFriction = velDir * -speed * mTyreConfig.mRollingFrictionCoefficient;
		auto ang = 0.0f;
		if(speed > 0.5)
			ang = std::max<float>(mBrake, slipAngle);
		rollingFriction = rollingFriction * (1.0f + ang * mTyreConfig.mBrakeCoefficient);

		assert(!isnan(latForce.x));
		assert(!isnan(selfAlign.x));
		assert(!isnan(rollingFriction.x));
		force = latForce + selfAlign + rollingFriction;
	}

	// throttle
	if(mThrottle)
		force = force + tyreDir * mThrottle;

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
	mThrottle = f;
}

void TyreForce::setBrake(float f)
{
	mBrake = f;
}

void TyreForce::setTyreConfig(const TyreConfig& tc)
{
	mTyreConfig = tc;
}

const Common::Vector2& TyreForce::getAttachPosition() const
{
	return mAttachPos;
}

DragForce::DragForce(float k1, float k2)
	: mK1(k1),
	mK2(k2)
{
}

void DragForce::updateForce(Abyss::RigidBody* body, Abyss::Real duration)
{
	Common::Vector2 force = body->velocity;
	Abyss::Real dragCoeff = force.length();
	dragCoeff = mK1 * dragCoeff + mK2 * dragCoeff * dragCoeff;

	force.normalize();
	force *= -dragCoeff;
	body->addForce(force);
}

Car::Car(const CarConfig* carconf, Abyss::World* world, const Track* track)
	: mCarConfig(*carconf),
	mWidth(carconf->Width),
	mLength(carconf->Length),
	mPhysicsWorld(world),
	mLBTyreForce(TyreForce(Vector2(-carconf->Wheelbase * 0.5f, -mWidth * 0.5f))),
	mRBTyreForce(TyreForce(Vector2(carconf->Wheelbase * 0.5f, -mWidth * 0.5f))),
	mLFTyreForce(TyreForce(Vector2(-carconf->Wheelbase * 0.5f, mWidth * 0.5f))),
	mRFTyreForce(TyreForce(Vector2(carconf->Wheelbase * 0.5f, mWidth * 0.5f))),
	mDragForce(DragForce(carconf->DragCoefficient, carconf->DragCoefficient2)),
	mTrack(track)
{
	mRigidBody.setMass(mCarConfig.Mass);
	mRigidBody.setInertiaTensor(mCarConfig.Mass * (1.0 / 12.0) * ((mWidth * mWidth) + (mLength * mLength)));
	mRigidBody.angularDamping = mCarConfig.AngularDamping;

	mPhysicsWorld->addBody(&mRigidBody);
	mPhysicsWorld->getForceRegistry()->add(&mRigidBody, &mLBTyreForce);
	mPhysicsWorld->getForceRegistry()->add(&mRigidBody, &mRBTyreForce);
	mPhysicsWorld->getForceRegistry()->add(&mRigidBody, &mLFTyreForce);
	mPhysicsWorld->getForceRegistry()->add(&mRigidBody, &mRFTyreForce);
	mPhysicsWorld->getForceRegistry()->add(&mRigidBody, &mDragForce);

	mLBTyreForce.setTyreConfig(mCarConfig.AsphaltTyres);
	mRBTyreForce.setTyreConfig(mCarConfig.AsphaltTyres);
	mLFTyreForce.setTyreConfig(mCarConfig.AsphaltTyres);
	mRFTyreForce.setTyreConfig(mCarConfig.AsphaltTyres);
}

Car::~Car()
{
	mPhysicsWorld->getForceRegistry()->remove(&mRigidBody, &mLBTyreForce);
	mPhysicsWorld->getForceRegistry()->remove(&mRigidBody, &mRBTyreForce);
	mPhysicsWorld->getForceRegistry()->remove(&mRigidBody, &mLFTyreForce);
	mPhysicsWorld->getForceRegistry()->remove(&mRigidBody, &mRFTyreForce);
	mPhysicsWorld->getForceRegistry()->remove(&mRigidBody, &mDragForce);
	mPhysicsWorld->removeBody(&mRigidBody);
}

const Common::Vector2& Car::getPosition() const
{
	return mRigidBody.position;
}

void Car::setPosition(const Common::Vector2& pos)
{
	mRigidBody.position = pos;
}

void Car::setVelocity(const Common::Vector2& vel)
{
	mRigidBody.velocity = vel;
}

void Car::setOrientation(float o)
{
	mRigidBody.orientation = Common::Math::rotate2D(Vector2(1.0f, 0.0f), o);
}

void Car::setAngularVelocity(float o)
{
	mRigidBody.rotation = o;
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
	mLBTyreForce.setThrottle(value * mCarConfig.ThrottleCoefficient);
	mRBTyreForce.setThrottle(value * mCarConfig.ThrottleCoefficient);
}

Abyss::RigidBody* Car::getBody()
{
	return &mRigidBody;
}

void Car::setBrake(float value)
{
	assert(value >= 0.0f && value <= 1.0f);
	mLBTyreForce.setBrake(value * mCarConfig.BrakeCoefficient);
	mRBTyreForce.setBrake(value * mCarConfig.BrakeCoefficient);
}

void Car::setSteering(float value)
{
	value = Common::clamp(-1.0f, value, 1.0f);
	mSteering = -value;
	mLFTyreForce.setAngle(mSteering * mCarConfig.SteeringCoefficient);
	mRFTyreForce.setAngle(mSteering * mCarConfig.SteeringCoefficient);
}

void Car::moved()
{
	int offroad = 0;
	const auto& pos = mRigidBody.position;
	if(mTrack->onTrack(pos + mLBTyreForce.getAttachPosition())) {
		mLBTyreForce.setTyreConfig(mCarConfig.AsphaltTyres);
	} else {
		mLBTyreForce.setTyreConfig(mCarConfig.GrassTyres);
		offroad++;
	}
	if(mTrack->onTrack(pos + mRBTyreForce.getAttachPosition())) {
		mRBTyreForce.setTyreConfig(mCarConfig.AsphaltTyres);
	} else {
		mRBTyreForce.setTyreConfig(mCarConfig.GrassTyres);
		offroad++;
	}
	if(mTrack->onTrack(pos + mLFTyreForce.getAttachPosition())) {
		mLFTyreForce.setTyreConfig(mCarConfig.AsphaltTyres);
	} else {
		mLFTyreForce.setTyreConfig(mCarConfig.GrassTyres);
		offroad++;
	}
	if(mTrack->onTrack(pos + mRFTyreForce.getAttachPosition())) {
		mRFTyreForce.setTyreConfig(mCarConfig.AsphaltTyres);
	} else {
		mRFTyreForce.setTyreConfig(mCarConfig.GrassTyres);
		offroad++;
	}

	mOffroad = offroad == 4;
}

bool Car::isOffroad() const
{
	return mOffroad;
}

float Car::getWidth() const
{
	return mWidth;
}

float Car::getLength() const
{
	return mLength;
}

CarConfig Car::readCarConfig(const char* filename)
{
	Json::Reader reader;
	Json::Value root;

	std::ifstream input(filename, std::ifstream::binary);
	bool parsingSuccessful = reader.parse(input, root, false);
	if (!parsingSuccessful) {
		throw std::runtime_error(reader.getFormatedErrorMessages());
	}

	CarConfig cc;
	cc.Width = root["width"].asDouble();
	cc.Length = root["length"].asDouble();
	cc.Mass = root["mass"].asDouble();
	cc.AngularDamping = root["angularDamping"].asDouble();
	cc.Wheelbase = root["wheelbase"].asDouble();
	cc.ThrottleCoefficient = root["throttle"].asDouble();
	cc.BrakeCoefficient = root["brake"].asDouble();
	cc.SteeringCoefficient = root["steeringCoeff"].asDouble();
	cc.DragCoefficient = root["dragCoeff"].asDouble();
	cc.DragCoefficient2 = root["dragCoeff2"].asDouble();

	const auto& at = root["tyres"]["asphalt"];
	cc.AsphaltTyres.mCorneringForceCoefficient      = at["corneringCoeff"].asDouble();
	cc.AsphaltTyres.mSelfAligningTorqueCoefficient  = at["selfAligningCoeff"].asDouble();
	cc.AsphaltTyres.mRollingFrictionCoefficient     = at["rollingFrictionCoeff"].asDouble();
	cc.AsphaltTyres.mBrakeCoefficient               = at["brakeCoeff"].asDouble();

	const auto& gt = root["tyres"]["grass"];
	cc.GrassTyres.mCorneringForceCoefficient      = gt["corneringCoeff"].asDouble();
	cc.GrassTyres.mSelfAligningTorqueCoefficient  = gt["selfAligningCoeff"].asDouble();
	cc.GrassTyres.mRollingFrictionCoefficient     = gt["rollingFrictionCoeff"].asDouble();
	cc.GrassTyres.mBrakeCoefficient               = gt["brakeCoeff"].asDouble();

	return cc;
}

