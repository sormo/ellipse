#include "SimmulationStepper.h"
#include <boost/numeric/odeint.hpp>
#include "Common.h"

namespace oxygine
{
	VectorD2 operator*(double d, VectorD2 o)
	{
		return o * d;
	}
}

namespace simmulation
{

	SimmulationStepper::SimmulationStepper(bool considerFirstObjectStatic) : m_considerFirstObjectStatic(considerFirstObjectStatic) {}

//[ coordinate_function
struct CoordinateFnc
{
	const std::vector<SimmulationStepper::SimmulationObject> & m_objects;

	CoordinateFnc(const std::vector<SimmulationStepper::SimmulationObject> & objects) : m_objects(objects) { }

	void operator()(const std::vector<oxygine::VectorD2> & p, std::vector<oxygine::VectorD2> & dqdt) const
	{
		for (size_t i = 0; i < m_objects.size(); ++i)
			dqdt[i] = p[i] / m_objects[i].mass;
	}
};
//]


//[ momentum_function
struct MomentumFnc
{
	const std::vector<SimmulationStepper::SimmulationObject> & m_objects;
	const bool m_isFirstStatic;

	MomentumFnc(const std::vector<SimmulationStepper::SimmulationObject> & objects, bool isFirstStatic) : m_objects(objects), m_isFirstStatic(isFirstStatic) { }

	void operator()(const std::vector<oxygine::VectorD2> & q, std::vector<oxygine::VectorD2> & dpdt) const
	{
		for (size_t i = 0; i < m_objects.size(); ++i)
		{
			dpdt[i] = { 0.0, 0.0 };
			for (size_t j = 0; j<i; ++j)
			{
				oxygine::VectorD2 diff = q[j] - q[i];
				double d = diff.length();
				diff *= (scaleGravityConstant(GRAVITY_CONSTANT) * m_objects[i].mass * m_objects[j].mass / d / d / d);
				if (!m_isFirstStatic || i != 0)
					dpdt[i] += diff;
				if (!m_isFirstStatic || j != 0)
					dpdt[j] -= diff;

			}
		}
	}
};
//]

//[ streaming_observer
struct ObserverFnc
{
	std::vector<SimmulationStepper::SimmulationObject> m_objects;

	ObserverFnc(std::vector<SimmulationStepper::SimmulationObject> objects) : m_objects(objects) { }

	template< class State >
	void operator()(const State &x, double t) const
	{
		std::vector<oxygine::VectorD2> &q = x.first;
		std::vector<oxygine::VectorD2> &p = x.second;
		
		for (size_t i = 0; i < m_objects.size(); ++i)
		{
			m_objects[i].positions.push_back(q[i]);
			m_objects[i].momenta.push_back(p[i]);
		}
	}
};
//]

/* Boost https://github.com/headmyshoulder/odeint-v2/blob/master/examples/solar_system.cpp
	Copyright 2010-2012 Karsten Ahnert
	Copyright 2011 Mario Mulansky
	Solar system example for Hamiltonian stepper
	Distributed under the Boost Software License, Version 1.0.
	(See accompanying file LICENSE_1_0.txt or
	copy at http://www.boost.org/LICENSE_1_0.txt)
*/
void SimmulationStepper::Simmulate(std::vector<SimmulationObject> objects, size_t steps, double dt)
{
	//[ integration_solar_system
	typedef boost::numeric::odeint::symplectic_rkn_sb3a_mclachlan<std::vector<oxygine::VectorD2>> StepperType;

	std::vector<oxygine::VectorD2> q(objects.size());
	std::vector<oxygine::VectorD2> p(objects.size());

	for (size_t i = 0; i < objects.size(); ++i)
	{
		q[i] = objects[i].positions.back();
		p[i] = objects[i].momenta.back();

		objects[i].positions.pop_back();
		objects[i].momenta.pop_back();
	}

	boost::numeric::odeint::integrate_const(
		StepperType(),
		std::make_pair(CoordinateFnc(objects), MomentumFnc(objects, m_considerFirstObjectStatic)),
		std::make_pair(boost::ref(q), boost::ref(p)),
		0.0, (double)steps * dt, dt, ObserverFnc(objects));
	//]
}

// ---

void MakeStep(std::vector<oxygine::VectorD2> & previousForces, std::vector<oxygine::VectorD2> & actualForces, 
	std::vector<SimmulationStepper::SimmulationObject> & objects, double dt, bool isFirstStatic)
{
	memcpy(previousForces.data(), actualForces.data(), actualForces.size() * sizeof(oxygine::VectorD2));
	memset(actualForces.data(), 0, actualForces.size() * sizeof(oxygine::VectorD2));

	for (size_t i = 0; i < objects.size(); ++i)
	{
		for (size_t j = i + 1; j < objects.size(); ++j)
		{
			const SimmulationStepper::SimmulationObject & o1 = objects[i];
			const SimmulationStepper::SimmulationObject & o2 = objects[j];

			oxygine::VectorD2 posVector = o2.positions.back() - o1.positions.back();
			double posVectorLenght = posVector.length();

			oxygine::VectorD2 f = posVector.normalized()*(scaleGravityConstant(GRAVITY_CONSTANT)*(o1.mass*o2.mass) / pow(posVectorLenght, 2));

			// drop small forces beacuse of stability
			//if (f.length() < 1.0e-4)
			//	continue;

			if (!isFirstStatic || i != 0)
				actualForces[i] += f;
			if (!isFirstStatic || j != 0)
				actualForces[j] -= f;
		}
	}

	for (size_t i = 0; i < actualForces.size(); ++i)
	{
		SimmulationStepper::SimmulationObject & o = objects[i];

		// leapfrog
		oxygine::VectorD2 acceleration = actualForces[i] / o.mass;
		oxygine::VectorD2 prevaccel = previousForces[i] / o.mass;

		oxygine::VectorD2 velocity = o.momenta.back() / o.mass + (acceleration + prevaccel)*0.5*dt;
		oxygine::VectorD2 position = o.positions.back() + velocity*dt + prevaccel*pow(dt, 2)*0.5;

		o.positions.push_back(position);
		o.momenta.push_back(velocity * o.mass);
	}
}

void SimmulationStepper::SimmulateNaive(std::vector<SimmulationObject> objects, size_t steps, double dt)
{
	std::vector<oxygine::VectorD2> previousForces(objects.size(), { 0.0, 0.0 });
	std::vector<oxygine::VectorD2> actualForces(objects.size(), { 0.0, 0.0 });

	for (size_t i = 0; i < steps; ++i)
		MakeStep(previousForces, actualForces, objects, dt, m_considerFirstObjectStatic);
}

}
