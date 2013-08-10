#ifndef SCR_GAMEWORLD_H
#define SCR_GAMEWORLD_H

#include "Car.h"

#include "abyss/RigidBody.h"

class GameWorld {
	public:
		GameWorld();
		~GameWorld();
		void updatePhysics(float time);
		const Car* getCar() const;
		Car* getCar();
		const Track* getTrack() const;

	private:
		Abyss::World mPhysicsWorld;
		Track mTrack;
		Car mCar;
};

#endif

