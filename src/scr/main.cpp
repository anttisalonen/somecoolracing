#include <iostream>
#include <vector>
#include <cassert>
#include <algorithm>
#include <cstring>

#include "common/Vector2.h"

#include "abyss/RigidBody.h"

#include "Game.h"

void test_abyss_rigid_bodies()
{
	using namespace Abyss;
	using namespace Common;

	World world;

	Real boatMass = 50.0;
	Real boatDepth = 2.0;
	Real boatDensity = 100.0;

	RigidBody boat;
	boat.setMass(boatMass);

	Gravity gravity(Vector2(0.0, -9.8));
	Buoyancy buoyancy(Vector2(0.0, 0.0), boatDepth, boatMass / boatDensity, 0.0, 1000.0);
	Drag drag(10.0, 0.0);

	world.addBody(&boat);
	world.getForceRegistry()->add(&boat, &gravity);
	world.getForceRegistry()->add(&boat, &buoyancy);
	world.getForceRegistry()->add(&boat, &drag);

	for(int i = 0; i < 5000; i++) {
		world.startFrame();
		if(i < 50 || i % 100 == 0) {
			std::cout << i << " Boat position: " << boat.position << "; velocity: " << boat.velocity << "\n";
			if(boat.position.y < -boatDepth) {
				std::cout << "Boat sank!\n";
				break;
			} else if(boat.position.y > boatDepth * 2.0) {
				std::cout << "Boat floats in the air!\n";
				break;
			}
		}
		boat.addForce(Vector2(50.0, 0.0));
		world.runPhysics(0.01);
	}
}

int run_game(const char* carname)
{
	Game g;
	g.run(carname);
	return 0;
}

int main(int argc, char** argv)
{
	const char* carname = "stock_car";
	for(int i = 0; i < argc; i++) {
		if(!strcmp(argv[i], "-t")) {
			test_abyss_rigid_bodies();
			return 0;
		} else if(!strcmp(argv[i], "--car")) {
			i++;
			if(i == argc) {
				std::cerr << "--car requires an argument.\n";
				return 1;
			}
			carname = argv[i];
		}
	}

	run_game(carname);

	return 0;
}

