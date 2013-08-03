#ifndef ABYSS_PARTICLE_H
#define ABYSS_PARTICLE_H

#include <vector>

#include "common/Vector2.h"

namespace Abyss {
	typedef double Real;

	class Particle {
		public:
			Common::Vector2 position;
			Common::Vector2 velocity;
			Common::Vector2 acceleration;

			Common::Vector2 forceAccum;

			// damping removes energy to cope with
			// numerical instability
			Real damping = 1.0;
			Real inverseMass = 0.0;

			void integrate(Real time);
			void clearAccumulator();
			void addForce(const Common::Vector2& f);
	};

	class ParticleForceGenerator {
		public:
			~ParticleForceGenerator() { }
			virtual void updateForce(Particle* p, Real duration) = 0;
	};

	class ParticleDrag : public ParticleForceGenerator {
		public:
			ParticleDrag(Real k1, Real k2);
			virtual void updateForce(Particle* p, Real duration) override;

		private:
			Real mk1;
			Real mk2;
	};

	class ParticleSpring : public ParticleForceGenerator {
		public:
			ParticleSpring(Particle* other, Real springConstant, Real restLength);
			virtual void updateForce(Particle* p, Real duration) override;

		private:
			Particle* mOther;
			Real mSpringConstant;
			Real mRestLength;
	};

	class AnchoredParticleSpring : public ParticleForceGenerator {
		public:
			AnchoredParticleSpring(const Common::Vector2 anchorPos, Real springConstant, Real restLength);
			virtual void updateForce(Particle* p, Real duration) override;

		private:
			Common::Vector2 mAnchorPos;
			Real mSpringConstant;
			Real mRestLength;
	};

	class ParticleBungee : public ParticleForceGenerator {
		public:
			ParticleBungee(Particle* other, Real springConstant, Real restLength);
			virtual void updateForce(Particle* p, Real duration) override;

		private:
			Particle* mOther;
			Real mSpringConstant;
			Real mRestLength;
	};

	class AnchoredBungee : public ParticleForceGenerator {
		public:
			AnchoredBungee(const Common::Vector2 anchorPos, Real springConstant, Real restLength);
			virtual void updateForce(Particle* p, Real duration) override;

		private:
			Common::Vector2 mAnchorPos;
			Real mSpringConstant;
			Real mRestLength;
	};

	class ParticleForceRegistry {
		protected:
			struct ParticleForceRegistration {
				Particle* particle;
				ParticleForceGenerator* fg;
			};

			std::vector<ParticleForceRegistration> registrations;

		public:
			void add(Particle* particle, ParticleForceGenerator* fg);
			void remove(Particle* particle, ParticleForceGenerator* fg);
			void clear();
			void updateForces(Real duration);
	};

	class ParticleContact {
		public:
			void resolve(Real duration);
			Real calculateSeparatingVelocity() const;

			// particles[1] may be nullptr (contact with scenery)
			Particle* particles[2];
			Real restitution;
			Common::Vector2 contactNormal;
			Real penetration;

		private:
			void resolveVelocity(Real duration);
			void resolveInterpenetration(Real duration);
	};

	class ParticleContactResolver {
		public:
			ParticleContactResolver(unsigned int iterations);
			void setIterations(unsigned int iterations);
			void resolveContacts(ParticleContact* contactArray,
					unsigned int numContacts, Real duration);

		protected:
			unsigned int mIterations;
			unsigned int mIterationsUsed;
	};

	class ParticleLink {
		public:
			virtual ~ParticleLink() { }
			virtual unsigned int fillContact(ParticleContact* contact,
					unsigned int limit) const = 0;
			Particle* particles[2];

		protected:
			Real currentLength() const;
	};

	class ParticleCable : public ParticleLink {
		public:
			virtual unsigned int fillContact(ParticleContact* contact,
					unsigned int limit) const override;

			Real maxLength;
			Real restitution;
	};

	class ParticleRod : public ParticleLink {
		public:
			virtual unsigned int fillContact(ParticleContact* contact,
					unsigned int limit) const override;

			Real length;
	};
}

#endif

