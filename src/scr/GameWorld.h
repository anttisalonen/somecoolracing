#ifndef SCR_GAMEWORLD_H
#define SCR_GAMEWORLD_H

#include "Car.h"

#include "abyss/RigidBody.h"

class GameWorld {
	public:
		GameWorld();
		void updatePhysics(float time);
		const Car* getCar() const;

	private:
		Abyss::World mPhysicsWorld;
		Car mCar;
};

#endif

