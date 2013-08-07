#ifndef ABYSS_RIGIDBODY_H
#define ABYSS_RIGIDBODY_H

#include <list>
#include <vector>
#include <cassert>

#include "common/Vector2.h"
#include "common/Matrix22.h"

#include "Prereq.h"

namespace Abyss {
	Common::Matrix22 getRotationMatrix(const Common::Vector2& orientation);
	Common::Vector2 localToWorld(const Common::Vector2& local, const Common::Vector2& pos,
			const Common::Matrix22& rotationMatrix);
	Common::Vector2 worldToLocal(const Common::Vector2& world, const Common::Vector2& pos,
			const Common::Matrix22& rotationMatrix);

	class RigidBody {
		public:
			void addForce(const Common::Vector2& force);
			void addForceAtBodyPoint(const Common::Vector2& force, const Common::Vector2& point);
			void addForceAtPoint(const Common::Vector2& force, const Common::Vector2& worldpoint);
			void addTorque(Real t);
			void integrate(Real duration);

			bool hasFiniteMass() const;
			Real getMass() const;
			void setMass(Real m);
			void setInertiaTensor(Real i);

			Common::Vector2 getPointInWorldSpace(const Common::Vector2& p) const;
			const Common::Matrix22& getRotationMatrix() const;
			void calculateDerivedData();
			void clearAccumulators();

			Real inverseMass = 0.0;
			Real inverseInertiaTensor = 0.0;
			Common::Vector2 position;
			Common::Vector2 orientation = Common::Vector2(1.0, 0.0);

			Common::Vector2 velocity;
			Real rotation;

			Common::Vector2 forceAccum;
			Real torqueAccum = 0.0;
			Real damping = 1.0;
			Real angularDamping = 1.0;


		private:
			void calculateRotationMatrix();

			// derived
			Common::Matrix22 rotationMatrix;
	};

	class ForceGenerator {
		public:
			virtual ~ForceGenerator() { }
			virtual void updateForce(RigidBody* body, Real duration) = 0;
	};

	class Gravity : public ForceGenerator {
		public:
			Gravity(const Common::Vector2& gravity);
			virtual void updateForce(RigidBody* body, Real duration) override;

		private:
			Common::Vector2 mGravity;
	};

	class Buoyancy : public ForceGenerator {
		public:
			Buoyancy(const Common::Vector2& center,
					Real maxDepth,       // depth of the body
					Real volume,         // body volume
					Real waterHeight,    // water level
					Real liquidDensity); // e.g. 1000.0 for water
			virtual void updateForce(RigidBody* body, Real duration) override;

		private:
			Common::Vector2 mCenter;
			Real mMaxDepth;
			Real mVolume;
			Real mWaterHeight;
			Real mLiquidDensity;
	};

	class Drag : public ForceGenerator {
		public:
			Drag(Real k1, Real k2);
			virtual void updateForce(RigidBody* p, Real duration) override;

		private:
			Real mk1;
			Real mk2;
	};

	class Spring : public ForceGenerator {
		public:
			Spring(const Common::Vector2& localConnectionPoint,
					RigidBody* other,
					const Common::Vector2& otherConnectionPoint,
					Real springConstant,
					Real restLength);
			virtual void updateForce(RigidBody* body, Real duration) override;

		private:
			Common::Vector2 mLocalConnectionPoint;
			RigidBody* mOther;
			Common::Vector2 mOtherConnectionPoint;
			Real mSpringConstant;
			Real mRestLength;
	};

	class ForceRegistry {
		protected:
			struct ForceRegistration {
				RigidBody* body;
				ForceGenerator* fg;
			};

			std::vector<ForceRegistration> registrations;

		public:
			void add(RigidBody* body, ForceGenerator* fg);
			void remove(RigidBody* body, ForceGenerator* fg);
			void clear();
			void updateForces(Real duration);
	};

	class World {
		public:
			void startFrame();
			void runPhysics(Real duration);
			void addBody(RigidBody* b);
			void removeBody(RigidBody* b);
			ForceRegistry* getForceRegistry();

		private:
			std::list<RigidBody*> mBodyRegistration;
			ForceRegistry mRegistry;
	};
}

#endif

