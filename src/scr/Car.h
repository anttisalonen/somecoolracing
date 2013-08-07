#ifndef SCR_CAR_H
#define SCR_CAR_H

#include "common/Vector2.h"

#include "abyss/RigidBody.h"

struct TyreConfig {
	float mCorneringForceCoefficient = 0.0f;
	float mSelfAligningTorqueCoefficient = 0.0f;
	float mRollingFrictionCoefficient = 0.0f;
};

class TyreForce : public Abyss::ForceGenerator {
	public:
		TyreForce(const Common::Vector2& attachpos);
		virtual void updateForce(Abyss::RigidBody* body, Abyss::Real duration) override;
		void setAngle(float f);
		void setThrottle(float f);
		void setBrake(float f);

	private:
		Common::Vector2 mAttachPos;
		float mThrottle = 0.0f;
		float mAngle = 0.0f;
		float mBrake = 0.0f;
};

class Car {
	public:
		Car(float w, float l, Abyss::World* world);
		~Car();
		Car(const Car&) = delete;
		Car& operator=(const Car&) = delete;
		const Common::Vector2& getPosition() const;
		float getOrientation() const;
		float getSpeed() const;
		void setThrottle(float value);
		void setBrake(float value);
		void setSteering(float value);
		Abyss::RigidBody* getBody();

	private:
		float mWidth;
		float mLength;
		float mSteering = 0.0f;
		Abyss::RigidBody mRigidBody;
		Abyss::World* mPhysicsWorld;
		TyreForce mLBTyreForce;
		TyreForce mRBTyreForce;
		TyreForce mLFTyreForce;
		TyreForce mRFTyreForce;
};

#endif

