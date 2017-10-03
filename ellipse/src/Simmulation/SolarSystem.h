#pragma once
#include "oxygine-framework.h"
#include "CelestialBody.h"
#include <vector>

namespace simmulation
{
	class SolarSystem
	{
	public:
		~SolarSystem();

		void SetName(const char * name);
		const std::string & GetName();

		std::shared_ptr<CelestialBody> SetStar(const CelestialBodyDef & def);
		std::shared_ptr<CelestialBody> AddBody(const CelestialBodyDef & def, const std::shared_ptr<const CelestialBody> * parent = nullptr);

		void ClearData(size_t index);
		void Simmulate(size_t steps);
		void RebuildParents();

		std::vector<std::shared_ptr<CelestialBody>> & GetBodies();
		std::shared_ptr<CelestialBody> & GetSun();

		void LoadBodies(const char * name);
		bool LoadData();

		std::shared_ptr<CelestialBody> & GetBody(const char * name);

	private:
		void SaveData();
		void WriteBody(const CelestialBodyDef & def);

		std::string m_name;

		std::shared_ptr<CelestialBody> m_invalid;
		std::shared_ptr<CelestialBody> m_star;
		std::vector<std::shared_ptr<CelestialBody>> m_bodies;

	};

	extern std::unique_ptr<SolarSystem> g_system;

	double ComputeHillSphereRadius(double R, double m, double M);
}
