#ifndef SCR_CAR_H
#define SCR_CAR_H

#include "common/Vector2.h"

#include "abyss/RigidBody.h"

class ThrottleForce : public Abyss::ForceGenerator {
	public:
		ThrottleForce(const Common::Vector2& attachpos, float power);
		virtual void updateForce(Abyss::RigidBody* body, Abyss::Real duration) override;
		void setThrottle(float f);

	private:
		Common::Vector2 mAttachPos;
		float mPower;
		float mThrottle = 0.0f;
};

class Car {
	public:
		Car(float w, float l, Abyss::World* world);
		~Car();
		Car(const Car&) = delete;
		Car& operator=(const Car&) = delete;
		const Common::Vector2& getPosition() const;
		float getOrientation() const;
		void setThrottle(float value);
		void setBrake(float value);
		Abyss::RigidBody* getBody();

	private:
		float mWidth;
		float mLength;
		Abyss::RigidBody mRigidBody;
		Abyss::World* mPhysicsWorld;
		ThrottleForce mLBThrottleForce;
		ThrottleForce mRBThrottleForce;
};

#endif

