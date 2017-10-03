#pragma once
#include <vector>
#include "oxygine-framework.h"

namespace simmulation
{
class SimmulationStepper
{
public:
	struct SimmulationObject
	{
		std::vector<oxygine::VectorD2> & positions;
		std::vector<oxygine::VectorD2> & momenta;
		double mass;
	};

	// ! ATTENTION ! first object is static like sun
	SimmulationStepper(bool considerFirstObjectStatic);

	void Simmulate(std::vector<SimmulationObject> objects, size_t steps, double dt);
	void SimmulateNaive(std::vector<SimmulationObject> objects, size_t steps, double dt);
private:
	const bool m_considerFirstObjectStatic;
};
}
