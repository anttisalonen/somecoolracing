#ifndef SCR_CAR_H
#define SCR_CAR_H

#include "common/Vector2.h"

#include "abyss/RigidBody.h"

class Car {
	public:
		Car(float w, float l);
		const Common::Vector2& getPosition() const;
		float getOrientation() const;
		void setThrottle(float value);
		Abyss::RigidBody* getBody();

	private:
		float mWidth;
		float mLength;
		float mThrottle = 0.0f;
		Abyss::RigidBody mRigidBody;
};

#endif

