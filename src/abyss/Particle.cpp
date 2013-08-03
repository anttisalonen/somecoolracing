#include "Particle.h"

#include <cassert>

namespace Abyss {

	void Particle::integrate(Real time)
	{
		assert(time > 0.0);
		position += velocity * time;

		Common::Vector2 resultAcc = acceleration + forceAccum * inverseMass;

		velocity += resultAcc * time;

		velocity *= pow(damping, time);
	}

	void Particle::clearAccumulator()
	{
		forceAccum = Common::Vector2();
	}

	void Particle::addForce(const Common::Vector2& f)
	{
		forceAccum += f;
	}

	ParticleDrag::ParticleDrag(Real k1, Real k2)
		: mk1(k1),
		mk2(k2)
	{
	}

	void ParticleDrag::updateForce(Particle* p, Real duration)
	{
		Common::Vector2 force = p->velocity;
		Real dragCoeff = force.length();
		dragCoeff = mk1 * dragCoeff + mk2 * dragCoeff * dragCoeff;

		force.normalize();
		force *= -dragCoeff;
		p->addForce(force);
	}

	ParticleSpring::ParticleSpring(Particle* other, Real springConstant, Real restLength)
		: mOther(other),
		mSpringConstant(springConstant),
		mRestLength(restLength)
	{
	}

	void ParticleSpring::updateForce(Particle* p, Real duration)
	{
		Common::Vector2 force = p->position - mOther->position;
		Real mag = force.length();
		mag = fabs(mag - mRestLength) * mSpringConstant;

		force.normalize();
		p->addForce(force * -mag);
	}

	AnchoredParticleSpring::AnchoredParticleSpring(const Common::Vector2 anchorPos, Real springConstant, Real restLength)
		: mAnchorPos(anchorPos),
		mSpringConstant(springConstant),
		mRestLength(restLength)
	{
	}

	void AnchoredParticleSpring::updateForce(Particle* p, Real duration)
	{
		Common::Vector2 force = p->position - mAnchorPos;
		Real mag = force.length();
		mag = fabs(mag - mRestLength) * mSpringConstant;

		force.normalize();
		p->addForce(force * -mag);
	}

	ParticleBungee::ParticleBungee(Particle* other, Real springConstant, Real restLength)
		: mOther(other),
		mSpringConstant(springConstant),
		mRestLength(restLength)
	{
	}

	void ParticleBungee::updateForce(Particle* p, Real duration)
	{
		Common::Vector2 force = p->position - mOther->position;
		Real mag = force.length();
		if(mag <= mRestLength)
			return;

		mag = (mRestLength - mag) * mSpringConstant;

		force.normalize();
		p->addForce(force * -mag);
	}

	AnchoredBungee::AnchoredBungee(const Common::Vector2 anchorPos, Real springConstant, Real restLength)
		: mAnchorPos(anchorPos),
		mSpringConstant(springConstant),
		mRestLength(restLength)
	{
	}

	void AnchoredBungee::updateForce(Particle* p, Real duration)
	{
		Common::Vector2 force = p->position - mAnchorPos;
		Real mag = force.length();
		if(mag <= mRestLength)
			return;

		mag = (mRestLength - mag) * mSpringConstant;

		force.normalize();
		p->addForce(force * -mag);
	}

	void ParticleForceRegistry::add(Particle* particle, ParticleForceGenerator* fg)
	{
		assert(particle);
		assert(fg);

		ParticleForceRegistration reg;
		reg.particle = particle;
		reg.fg = fg;
		registrations.push_back(reg);
	}

	void ParticleForceRegistry::remove(Particle* particle, ParticleForceGenerator* fg)
	{
		for(auto it = registrations.begin(); it != registrations.end(); ++it) {
			if(it->particle == particle &&
					it->fg == fg) {
				registrations.erase(it);
				return;
			}
		}
	}

	void ParticleForceRegistry::clear()
	{
		registrations.clear();
	}

	void ParticleForceRegistry::updateForces(Real duration)
	{
		for(auto& reg : registrations) {
			reg.fg->updateForce(reg.particle, duration);
		}
	}

	void ParticleContact::resolve(Real duration)
	{
		resolveVelocity(duration);
		resolveInterpenetration(duration);
	}

	Real ParticleContact::calculateSeparatingVelocity() const
	{
		assert(particles[0] != nullptr);
		Common::Vector2 relVelocity = particles[0]->velocity;
		if(particles[1]) {
			relVelocity -= particles[1]->velocity;
		}
		return relVelocity.dot(contactNormal);
	}

	void ParticleContact::resolveVelocity(Real duration)
	{
		Real sepVelocity = calculateSeparatingVelocity();

		if(sepVelocity > 0.0)
			return;

		Real newSepVel = -sepVelocity * restitution;

		// handling for resting contact
		Common::Vector2 accCausedVel = particles[0]->acceleration;
		if(particles[1])
			accCausedVel += particles[1]->acceleration;
		// TODO: check the fixity is correct on this line
		Real accCausedSepVel = accCausedVel.dot(contactNormal) * duration;
		if(accCausedSepVel < 0.0) {
			newSepVel += restitution * accCausedSepVel;
			if(newSepVel < 0.0)
				newSepVel = 0.0;
		}

		Real deltaVel = newSepVel - sepVelocity;

		Real totInvMass = particles[0]->inverseMass;
		if(particles[1])
			totInvMass += particles[1]->inverseMass;

		if(totInvMass <= 0.0)
			return;

		Real impulse = deltaVel / totInvMass;
		Common::Vector2 impulsePerInvMass = contactNormal * impulse;

		particles[0]->velocity += impulsePerInvMass * particles[0]->inverseMass;
		if(particles[1]) {
			// opposite direction
			particles[1]->velocity += impulsePerInvMass * -particles[1]->inverseMass;
		}
	}

	void ParticleContact::resolveInterpenetration(Real duration)
	{
		if(penetration <= 0.0)
			return;

		Real totInvMass = particles[0]->inverseMass;
		if(particles[1])
			totInvMass += particles[1]->inverseMass;

		if(totInvMass <= 0.0)
			return;

		Common::Vector2 movePerInvMass = contactNormal * (-penetration / totInvMass);

		particles[0]->position += movePerInvMass * particles[0]->inverseMass;
		if(particles[1]) {
			particles[1]->velocity += movePerInvMass * -particles[1]->inverseMass;
		}
	}

	ParticleContactResolver::ParticleContactResolver(unsigned int iterations)
		: mIterations(iterations)
	{
	}

	void ParticleContactResolver::setIterations(unsigned int iterations)
	{
		mIterations = iterations;
	}

	void ParticleContactResolver::resolveContacts(ParticleContact* contactArray,
			unsigned int numContacts, Real duration)
	{
		mIterationsUsed = 0;
		while(mIterationsUsed < mIterations) {
			Real maxV = 0;
			unsigned int maxIndex = numContacts;
			for(unsigned int i = 0; i < numContacts; i++) {
				Real sepVel = contactArray[i].calculateSeparatingVelocity();
				if(sepVel > maxV) {
					maxV = sepVel;
					maxIndex = i;
				}
			}

			if(maxIndex == numContacts)
				break;

			contactArray[maxIndex].resolve(duration);
			mIterationsUsed++;
		}
	}

	Real ParticleLink::currentLength() const
	{
		assert(particles[0]);
		assert(particles[1]);
		Common::Vector2 relPos = particles[0]->position - particles[1]->position;
		return relPos.length();
	}

	unsigned int ParticleCable::fillContact(ParticleContact* contact,
			unsigned int limit) const
	{
		Real len = currentLength();

		if(len < maxLength)
			return 0;

		contact->particles[0] = particles[0];
		contact->particles[1] = particles[1];

		Common::Vector2 normal = particles[1]->position - particles[0]->position;
		normal.normalize();
		contact->contactNormal = normal;
		contact->penetration = len - maxLength;
		contact->restitution = restitution;

		return 1;
	}

	unsigned int ParticleRod::fillContact(ParticleContact* contact,
			unsigned int limit) const
	{
		Real currLen = currentLength();

		if(currLen == length) {
			return 0;
		}

		contact->particles[0] = particles[0];
		contact->particles[1] = particles[1];

		Common::Vector2 normal = particles[1]->position - particles[0]->position;
		normal.normalize();
		if(currLen > length) {
			contact->contactNormal = normal;
			contact->penetration = currLen - length;
		} else {
			contact->contactNormal = normal * -1.0;
			contact->penetration = length - currLen;
		}

		contact->restitution = 0.0;

		return 1;
	}
}


