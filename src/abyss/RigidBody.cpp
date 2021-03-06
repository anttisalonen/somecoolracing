#include "RigidBody.h"

#include <math.h>
#include <iostream>
#include <algorithm>

#include "common/Math.h"

namespace Abyss {
	Common::Matrix22 getRotationMatrix(const Common::Vector2& orientation)
	{
		assert(fabs(orientation.length() - 1.0) < 0.001);
		Real theta = atan2(orientation.x, orientation.y);
		Real s = sin(theta);
		Real c = cos(theta);
		return Common::Matrix22(c, s, -s, c);
	}

	Common::Vector2 localToWorld(const Common::Vector2& local, const Common::Vector2& pos,
			const Common::Matrix22& rotationMatrix)
	{
		// TODO: verify this
		return pos + rotationMatrix * local;
	}

	Common::Vector2 worldToLocal(const Common::Vector2& world, const Common::Vector2& pos,
			const Common::Matrix22& rotationMatrix)
	{
		// TODO: verify this
		return rotationMatrix.inverse() * world - pos;
	}

	void RigidBody::addForce(const Common::Vector2& force)
	{
		forceAccum += force;
	}

	void RigidBody::addTorque(Real t)
	{
		torqueAccum += t;
	}

	void RigidBody::integrate(Real duration)
	{
		Common::Vector2 lastFrameAcceleration = forceAccum * inverseMass;
		Real angularAcceleration = torqueAccum * inverseInertiaTensor;

		velocity += lastFrameAcceleration * duration;
		rotation += angularAcceleration * duration;

		velocity *= pow(damping, duration);
		rotation *= pow(angularDamping, duration);

		position += velocity * duration;
		orientation = Common::Math::rotate2D(orientation, rotation * duration);

		calculateDerivedData();
		clearAccumulators();
	}

	bool RigidBody::hasFiniteMass() const
	{
		return inverseMass != 0.0;
	}

	Real RigidBody::getMass() const
	{
		assert(inverseMass);
		return 1.0 / inverseMass;
	}

	void RigidBody::setMass(Real m)
	{
		assert(m);
		inverseMass = 1.0 / m;
	}

	void RigidBody::setInertiaTensor(Real i)
	{
		assert(i);
		inverseInertiaTensor = 1.0 / i;
	}

	void RigidBody::clearAccumulators()
	{
		forceAccum.zero();
		torqueAccum = 0.0;
	}

	void RigidBody::addForceAtBodyPoint(const Common::Vector2& force, const Common::Vector2& point)
	{
		Common::Vector2 pt = getPointInWorldSpace(point);
		addForceAtPoint(force, pt);
	}

	void RigidBody::addForceAtPoint(const Common::Vector2& force, const Common::Vector2& worldpoint)
	{
		assert(!isnan(force.x));
		assert(!isnan(force.y));
		assert(!isnan(worldpoint.x));
		assert(!isnan(worldpoint.y));

		forceAccum += force;

		Common::Vector2 momentArm = worldpoint - position;
		Real torque = momentArm.cross2d(force);

		torqueAccum += torque;
	}

	void RigidBody::calculateDerivedData()
	{
		calculateRotationMatrix();
	}

	void RigidBody::calculateRotationMatrix()
	{
		rotationMatrix = Abyss::getRotationMatrix(orientation);
	}

	Common::Vector2 RigidBody::getPointInWorldSpace(const Common::Vector2& p) const
	{
		return localToWorld(p, position, rotationMatrix);
	}

	const Common::Matrix22& RigidBody::getRotationMatrix() const
	{
		return rotationMatrix;
	}

	Gravity::Gravity(const Common::Vector2& gravity)
		: mGravity(gravity)
	{
	}

	void Gravity::updateForce(RigidBody* body, Real duration)
	{
		if(!body->hasFiniteMass())
			return;

		body->addForce(mGravity * body->getMass());
	}

	Buoyancy::Buoyancy(const Common::Vector2& center,
			Real maxDepth,
			Real volume,
			Real waterHeight,
			Real liquidDensity)
		: mCenter(center),
		mMaxDepth(maxDepth),
		mVolume(volume),
		mWaterHeight(waterHeight),
		mLiquidDensity(liquidDensity)
	{
	}

	void Buoyancy::updateForce(RigidBody* body, Real duration)
	{
		Common::Vector2 pointInWorld = body->getPointInWorldSpace(mCenter);
		Real depth = pointInWorld.y;

		// check if out of water
		if(depth >= mWaterHeight + mMaxDepth) {
			return;
		}

		Common::Vector2 force;
		if(depth <= mWaterHeight - mMaxDepth) {
			// maximum depth
			force.y = mLiquidDensity * mVolume;
			body->addForceAtBodyPoint(force, mCenter);
			return;
		}

		force.y = mLiquidDensity * mVolume *
			-(depth - mMaxDepth - mWaterHeight) * 0.5 * mMaxDepth;
		body->addForceAtBodyPoint(force, mCenter);
	}

	Drag::Drag(Real k1, Real k2)
		: mk1(k1),
		mk2(k2)
	{
	}

	void Drag::updateForce(RigidBody* p, Real duration)
	{
		Common::Vector2 force = p->velocity;
		Real dragCoeff = force.length();
		dragCoeff = mk1 * dragCoeff + mk2 * dragCoeff * dragCoeff;

		force.normalize();
		force *= -dragCoeff;
		p->addForce(force);
	}

	Spring::Spring(const Common::Vector2& localConnectionPoint,
			RigidBody* other,
			const Common::Vector2& otherConnectionPoint,
			Real springConstant,
			Real restLength)
		: mLocalConnectionPoint(localConnectionPoint),
		mOther(other),
		mOtherConnectionPoint(otherConnectionPoint),
		mSpringConstant(springConstant),
		mRestLength(restLength)
	{
	}

	void Spring::updateForce(RigidBody* body, Real duration)
	{
		Common::Vector2 lws = body->getPointInWorldSpace(mLocalConnectionPoint);
		Common::Vector2 ows = mOther->getPointInWorldSpace(mOtherConnectionPoint);

		Common::Vector2 force = lws - ows;

		Real mag = fabs(force.length() - mRestLength) * mSpringConstant;

		force.normalize();

		force *= -mag;
		body->addForceAtPoint(force, lws);
	}

	void ForceRegistry::add(RigidBody* body, ForceGenerator* fg)
	{
		ForceRegistration reg;
		reg.body = body;
		reg.fg = fg;
		registrations.push_back(reg);
	}

	void ForceRegistry::remove(RigidBody* body, ForceGenerator* fg)
	{
		for(auto it = registrations.begin(); it != registrations.end(); ++it) {
			if(it->body == body &&
					it->fg == fg) {
				registrations.erase(it);
				return;
			}
		}
	}

	void ForceRegistry::clear()
	{
		registrations.clear();
	}

	void ForceRegistry::updateForces(Real duration)
	{
		for(auto& reg : registrations) {
			reg.fg->updateForce(reg.body, duration);
		}
	}

	ForceRegistry* World::getForceRegistry()
	{
		return &mRegistry;
	}

	void World::addBody(RigidBody* b)
	{
		mBodyRegistration.push_back(b);
	}

	void World::removeBody(RigidBody* b)
	{
		mBodyRegistration.erase(std::remove(mBodyRegistration.begin(),
					mBodyRegistration.end(), b),
				mBodyRegistration.end());
	}

	void World::startFrame()
	{
		for(auto& b : mBodyRegistration) {
			b->clearAccumulators();
			b->calculateDerivedData();
		}
	}

	void World::runPhysics(Real duration)
	{
		mRegistry.updateForces(duration);

		for(auto& b : mBodyRegistration) {
			b->integrate(duration);
		}
	}

}

