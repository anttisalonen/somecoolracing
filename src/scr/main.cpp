#include <iostream>
#include <vector>
#include <cassert>
#include <algorithm>

#include "common/Vector2.h"

typedef double Real;


// Physics base

namespace Environment {
	static Real Gravity = 9.81f; // m/s^2
}

struct PointMass {
	Real mass = 0.0f;
	Common::Vector2 position;
	mutable Common::Vector2 relPosition; // relative to centre of mass of the rigid body
	Real momentOfInertia = 0.0f; // unit: kg*m^2

	enum class Shape {
		Box,
	};

	Shape shape;

	union {
		struct {
			Real width;
			Real height;
		} Box;
	} shapeData;

	PointMass(Real m, const Common::Vector2& pos, Real w, Real h)
		: mass(m), position(pos),
		shape(Shape::Box)
	{
		shapeData.Box.width = w;
		shapeData.Box.height = h;
		setMomentOfInertia();
	}

	private:
		void setMomentOfInertia()
		{
			switch(shape) {
				case Shape::Box:
					momentOfInertia = (mass / 12.0) *
						(shapeData.Box.width * shapeData.Box.width +
						 shapeData.Box.height * shapeData.Box.height);
					return;
			}
			assert(0);
		}
};

class RigidBody {
	public:
		virtual ~RigidBody() { }
		void addPointMass(const PointMass& p) { mPointMass.push_back(p); invalidateCache(); }
		Real getMass() const;
		const Common::Vector2& getCentreOfMass() const;
		Real getMassMomentOfInertia() const;

	private:
		void invalidateCache() const;
		static Real MomentOfInertiaRel(Real moi, Real m,
				const Common::Vector2& com, const Common::Vector2& pos);

		std::vector<PointMass> mPointMass;
		mutable Real mMass;
		mutable bool mMassCached = false;
		mutable Common::Vector2 mCOM;
		mutable bool mCOMCached = false;
		mutable Real mMassMomentOfInertia;
		mutable bool mMMOICached = false;
};

void RigidBody::invalidateCache() const
{
	mMassCached = false;
	mCOMCached = false;
	mMMOICached = false;
}

Real RigidBody::getMass() const
{
	if(mMassCached)
		return mMass;

	mMassCached = true;
	mMass = std::accumulate(mPointMass.begin(), mPointMass.end(),
			0, [](int total, const PointMass& p) { return total + p.mass; });
	return mMass;
}

const Common::Vector2& RigidBody::getCentreOfMass() const
{
	if(mCOMCached)
		return mCOM;

	mCOMCached = true;

	mCOM = std::accumulate(mPointMass.begin(), mPointMass.end(),
			Common::Vector2(), [](const Common::Vector2& total, const PointMass& p) {
				return total + p.position * p.mass; });

	mCOM /= getMass();

	for(auto& p : mPointMass) {
		p.relPosition = p.position - mCOM;
	}

	return mCOM;
}

Real RigidBody::getMassMomentOfInertia() const
{
	if(mMMOICached)
		return mMassMomentOfInertia;

	mMMOICached = true;

	// calculate centre of mass to ensure p.relPosition is set
	auto com = getCentreOfMass();
	mMassMomentOfInertia = std::accumulate(mPointMass.begin(), mPointMass.end(),
			0.0, [&](Real total, const PointMass& p) {
				auto v = total + MomentOfInertiaRel(p.momentOfInertia,
					p.mass, com, p.position);
				return v;
			});

	return mMassMomentOfInertia;
}

Real BoxMass(Real x, Real y, Real z, Real density)
{
	return x * y * z * density;
}

Real MassToWeight(Real m)
{
	return m * Environment::Gravity;
}

// relative to the neutral axis of the body
Real RigidBody::MomentOfInertiaRel(Real moi, Real m,
		const Common::Vector2& com, const Common::Vector2& pos)
{
	Real dx = com.x - pos.x;
	Real dy = com.y - pos.y;
	Real d2 = dx * dx + dy * dy;

	Real val = moi + m * d2;
	return val;
}

// Physics base tests

void testReal(const char* descr, Real have, Real expected, Real tolerance = 0.01)
{
	if(fabs(have - expected) > tolerance) {
		std::cerr << descr << ": have " << have << ", expected " << expected << "\n";
	}
}

void test_rigidBody()
{
	RigidBody car;

	Real FuelDensity = 750.0f; // kg/m^3
	Real fuel_w = 0.9;
	Real fuel_h = 0.5;
	Real fuel_l = 0.3;

	Real fuel_mass = BoxMass(fuel_w, fuel_h, fuel_l, FuelDensity);

	PointMass carbody(1783.8939857, Common::Vector2(30.5, 30.5), 1.8, 4.7);
	PointMass driver(86.6462793, Common::Vector2(31.5, 31.0), 0.5, 0.9);
	PointMass fueltank(fuel_mass, Common::Vector2(28.0, 30.5), fuel_w, fuel_h);

	car.addPointMass(carbody);
	car.addPointMass(driver);
	car.addPointMass(fueltank);

	Real carMass = car.getMass();
	Common::Vector2 carCOM = car.getCentreOfMass();
	Real carMassMomentOfInertia = car.getMassMomentOfInertia();

	testReal("Car mass", carMass, 1972.0, 5);
	testReal("Car centre of mass X", carCOM.x, 30.42, 0.1);
	testReal("Car centre of mass X", carCOM.y, 30.53, 0.1);
	testReal("Car mass moment of inertia", carMassMomentOfInertia, 4538.68, 30);

	std::cout << "Finished rigid body test.\n";
}

int main(void)
{
	std::cout << "Hello world!\n";

	test_rigidBody();
	return 0;
}

