#pragma once
#include "oxygine-framework.h"
#include "Simmulation\CelestialBody.h"
#include "Primitives\Polyline.h"
#include <bitset>
#include <atomic>
#include "AnimationConic.h"

class AnimationSystem;

class AnimationBody
{
public:
	AnimationBody(std::shared_ptr<const simmulation::CelestialBody> & body, AnimationSystem * system);
	~AnimationBody();

	void Rebuild();

	const std::shared_ptr<const simmulation::CelestialBody> & GetCelestialBody() const;

	oxygine::spActor & GetActor();

	void Update(const oxygine::VectorD2 & position, const oxygine::VectorD2 * parentPosition);

	void ShowInfo(bool show);
	bool IsInfoShown();

	float GetMaximumCameraScale();

private:
	oxygine::Actor * GetParentActor();

	AnimationSystem * m_system;
	oxygine::spActor m_actor; // system

	primitive::spCircle m_planet;
	primitive::spText m_name;

	void CreatePlanet();
	spAnimationConic CreateApproximatedOrbit();
	spAnimationConic CreateKeplerOrbit();
	primitive::spPolyline CreateOrbitTrajectory();

	struct OrbitPrimitives
	{
		spAnimationConic approximatedOrbit; // parent
		spAnimationConic keplerOrbit; // parent
		primitive::spPolyline orbitTrajectory; // system
	};

	struct OrbitData
	{
		OrbitPrimitives currentPrimitives;
		OrbitPrimitives newPrimitives;

		enum class Rebuild : size_t
		{
			ellipse,
			trajectory
		};
		std::bitset<2> rebuild;

		size_t currentPeriod = 1;
		double topMostParentPeriodTime;

	} m_orbitData;
	std::mutex m_orbitDataLock;
	void HandleOrbitDataRebuildFlags();

	std::shared_ptr<const simmulation::CelestialBody> m_body;

	std::thread m_rebuildTrajectoryThread;
	std::atomic<bool> m_rebuildTrajectoryRunning;
	void AsyncRebuildTrajectory();

	size_t GetCurrentPeriod(double celestialBodyPeriodTime);
	double GetTopMostParentPeriodTime();

	// this holds information about registration in AnimationBodyInfoUpdater
	// if body is registered/unregistered this value must be updated
	bool m_showInfo = false;
};
