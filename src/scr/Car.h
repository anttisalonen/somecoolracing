#ifndef SCR_CAR_H
#define SCR_CAR_H

#include "common/Vector2.h"

#include "abyss/RigidBody.h"

#include "Track.h"

struct TyreConfig {
	float mCorneringForceCoefficient = 100.0f;
	float mSelfAligningTorqueCoefficient = 100.0f;
	float mRollingFrictionCoefficient = 100.0f;
	float mBrakeCoefficient = 10.0f;
};

class TyreForce : public Abyss::ForceGenerator {
	public:
		TyreForce(const Common::Vector2& attachpos);
		virtual void updateForce(Abyss::RigidBody* body, Abyss::Real duration) override;
		void setAngle(float f);
		void setThrottle(float f);
		void setBrake(float f);
		void setTyreConfig(const TyreConfig& tc);
		const Common::Vector2& getAttachPosition() const;

	private:
		Common::Vector2 mAttachPos;
		float mThrottle = 0.0f;
		float mAngle = 0.0f;
		float mBrake = 0.0f;
		TyreConfig mTyreConfig;
};

struct CarConfig {
	float Width = 2.0f;
	float Length = 5.0f;
	float Mass = 1000.0f;
	float AngularDamping = 0.9f;
	float Wheelbase = 3.5f;
	TyreConfig AsphaltTyres;
	TyreConfig GrassTyres;
	float ThrottleCoefficient = 100.0f;
	float SteeringCoefficient = 0.2f;
};

class Car {
	public:
		Car(const CarConfig* carconf, Abyss::World* world, const Track* track);
		~Car();
		Car(const Car&) = delete;
		Car& operator=(const Car&) = delete;
		const Common::Vector2& getPosition() const;
		void setPosition(const Common::Vector2& pos);
		float getOrientation() const;
		float getSpeed() const;
		void setThrottle(float value);
		void setBrake(float value);
		void setSteering(float value);
		Abyss::RigidBody* getBody();
		void moved();
		bool isOffroad() const;
		float getWidth() const;
		float getLength() const;

		static CarConfig readCarConfig(const char* filename);

	private:
		CarConfig mCarConfig;
		float mWidth;
		float mLength;
		float mSteering = 0.0f;
		Abyss::RigidBody mRigidBody;
		Abyss::World* mPhysicsWorld;
		TyreForce mLBTyreForce;
		TyreForce mRBTyreForce;
		TyreForce mLFTyreForce;
		TyreForce mRFTyreForce;
		const Track* mTrack;
		bool mOffroad = false;
};

#endif

