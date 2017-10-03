#include "AnimationBody.h"
#include "Common.h"
#include "Simmulation\SolarSystem.h"
#include "MainActor.h"
#include "AnimationSystem.h"
#include <mutex>

#define NAME_MINIMUM_DISTANCE_SQUARED 400.0f
#define TRAJECTORY_MAXIMUM_SCALE 10000.0f
#define PLANET_DEFAULT_RADIUS 3.0f
#define PLANET_RADIUS_MAXIMUM_FOR_WHITE_NAME 25.0f
#define PLANET_MAXIMUM_RADIUS(screenSize) (screenSize*0.40f) 

AnimationBody::AnimationBody(std::shared_ptr<const simmulation::CelestialBody> & body, AnimationSystem * system)
	: m_body(body), m_system(system), m_actor(new oxygine::Actor), m_rebuildTrajectoryRunning(false)
{
	std::lock_guard<const simmulation::CelestialBody> lock(*m_body.get());

	CreatePlanet();

	m_actor->setPosition(m_body->GetPositions()[0]);
}

AnimationBody::~AnimationBody()
{
	if (m_rebuildTrajectoryThread.joinable())
		m_rebuildTrajectoryThread.join();
}

spAnimationConic AnimationBody::CreateApproximatedOrbit()
{
	if (m_body->GetParentRelativeData().conic.GetType() != simmulation::Conic::ellipse)
		return nullptr;

	size_t periodPointCount = (size_t)(m_body->GetParentRelativeData().period / SIMMULATION_DT);
	if (periodPointCount > m_body->GetParentRelativeData().positions.size())
		periodPointCount = m_body->GetParentRelativeData().positions.size();

	simmulation::Conic conic = simmulation::ApproximateConic(m_body->GetParentRelativeData().positions.cbegin(), 
		m_body->GetParentRelativeData().positions.cbegin() + periodPointCount);
	
	return AnimationConic::Create(conic, oxygine::Color(oxygine::Color::Red, 30), primitive::DrawType::nanovg);
}

spAnimationConic AnimationBody::CreateKeplerOrbit()
{
	if (m_body->GetParent() == nullptr)
		return nullptr;

	double parentMass = m_body->GetParent()->GetMass();

	m_body->GetParent()->lock();
	oxygine::VectorD2 parentPosition = m_body->GetParent()->GetPositions()[0];
	oxygine::VectorD2 parentVelocity = m_body->GetParent()->GetMomenta()[0] / parentMass;
	m_body->GetParent()->unlock();

	double mass = m_body->GetMass();
	oxygine::VectorD2 position = m_body->GetPositions()[0];
	oxygine::VectorD2 velocity = m_body->GetMomenta()[0] / mass;

	simmulation::Conic conic = simmulation::ComputeConic(parentMass, mass, position - parentPosition, velocity - parentVelocity);

	return AnimationConic::Create(conic, oxygine::Color(oxygine::Color::Orange, 120), primitive::DrawType::nanovg);
}

primitive::spPolyline AnimationBody::CreateOrbitTrajectory()
{
	size_t pointStart = 0;
	size_t pointEnd = m_body->GetPositions().size();

	if (m_body->GetParent() && m_body->GetParentRelativeData().conic.GetType() == simmulation::Conic::ellipse)
	{
		double periodTime = GetTopMostParentPeriodTime();
		size_t periodPointCount = (size_t)(periodTime / SIMMULATION_DT);
		size_t currentPeriod = GetCurrentPeriod(GetTopMostParentPeriodTime());

		pointStart = periodPointCount * currentPeriod;
		pointEnd = pointStart + periodPointCount;

		if (pointStart >= m_body->GetPositions().size())
		{
			pointStart = 0;
			pointEnd = m_body->GetPositions().size();
		}
		
		if (pointEnd > m_body->GetPositions().size())
			pointEnd = m_body->GetPositions().size();
	}

	std::vector<oxygine::Vector2> linePoints;
	for (size_t i = pointStart; i < pointEnd; ++i)
		linePoints.push_back(m_body->GetPositions()[i]);

	primitive::spPolyline ret = new primitive::Polyline(linePoints);
	ret->SetLineColor(oxygine::Color(oxygine::Color::Green, 120));

	return ret;
}

void AnimationBody::CreatePlanet()
{
	m_planet = new primitive::Circle(PLANET_DEFAULT_RADIUS);
	m_planet->SetNotZoomable(true);
	m_planet->SetFillColor(oxygine::Color::White);
	m_actor->addChild(m_planet);

	m_name = new primitive::Text(m_body->GetName().c_str());
	m_name->SetNotZoomable(true);
	// name should be in front of planet circle
	m_name->setPriority(1);
	m_actor->addChild(m_name);

	m_system->addChild(m_actor);

	if (m_body->GetParent())
	{
		if (m_body->GetParentRelativeData().conic.GetType() == simmulation::Conic::ellipse)
		{
			double R = m_body->GetParentRelativeData().positions[0].length();
			double m = m_body->GetMass();
			double M = m_body->GetParent()->GetMass();

			double hillSphereRadius = simmulation::ComputeHillSphereRadius(R, m, M);

			primitive::spCircle hillSphere = new primitive::Circle(float(hillSphereRadius));
			hillSphere->SetLineColor(oxygine::Color(oxygine::Color::White, 60));
			m_actor->addChild(hillSphere);

			//primitive::spCircle soi = new primitive::Circle(float(0.9431*a*pow(m/M, 2.0/5.0)));
			//soi->SetLineColor(oxygine::Color(oxygine::Color::Blue, 60));
			//m_actor->addChild(soi);
		}
	}
}

void AnimationBody::Rebuild()
{
	double periodTime = 0.0;
	spAnimationConic approximatedOrbit;
	spAnimationConic keplerOrbit;
	primitive::spPolyline trajectory;

	{
		std::lock_guard<const simmulation::CelestialBody> lock(*m_body.get());

		approximatedOrbit = CreateApproximatedOrbit();
		trajectory = CreateOrbitTrajectory();
		keplerOrbit = CreateKeplerOrbit();
		periodTime = GetTopMostParentPeriodTime();
	}

	{
		std::lock_guard<std::mutex> lock(m_orbitDataLock);
		m_orbitData.newPrimitives.approximatedOrbit = approximatedOrbit;
		m_orbitData.newPrimitives.keplerOrbit = keplerOrbit;
		m_orbitData.newPrimitives.orbitTrajectory = trajectory;
		m_orbitData.rebuild[(size_t)OrbitData::Rebuild::ellipse] = true;
		m_orbitData.rebuild[(size_t)OrbitData::Rebuild::trajectory] = true;

		m_orbitData.topMostParentPeriodTime = periodTime;
		m_orbitData.currentPeriod = GetCurrentPeriod(periodTime);
	}
}

void SwapPrimitives(oxygine::spActor & oldPrim, oxygine::spActor & newPrim, oxygine::Actor * parent)
{
	if (oldPrim && oldPrim->getParent())
		oldPrim->getParent()->removeChild(oldPrim);
	oldPrim = newPrim;
	newPrim = nullptr;

	if (oldPrim)
		parent->addChild(oldPrim);
}

float AnimationBody::GetMaximumCameraScale()
{
	float maximumPlanetRadius = PLANET_MAXIMUM_RADIUS(std::min(oxygine::getStage()->getSize().x, oxygine::getStage()->getSize().y));
	return maximumPlanetRadius / (float)m_body->GetRadius();
}

void AnimationBody::Update(const oxygine::VectorD2 & position, const oxygine::VectorD2 * parentPosition)
{
	// --- update position  ---
	m_actor->setPosition(position);
	//DebugActor::addDebugString("%s %.3f %.3f", m_name->getText().c_str(), position.x, position.y);

	// swap primitives
	HandleOrbitDataRebuildFlags();

	// TODO possible optimization to handle some of those only if body is followed

	// --- planet radius ---
	double planetRadius = g_main->GetCamera().GetScaleCamera() * m_body->GetRadius();
	if (planetRadius < PLANET_DEFAULT_RADIUS)
		planetRadius = PLANET_DEFAULT_RADIUS;
	if (!sigmaCompare((float)planetRadius, m_planet->GetRadius(), 1.0))
		m_planet->SetRadius((float)planetRadius);

	// --- hide name if too close to parent ---
	if (m_showInfo)
	{
		m_name->setVisible(false);
	}
	else if (parentPosition)
	{
		float shownDistanceFromParentSquared = (float)(*parentPosition - position).sqlength() * g_main->GetCamera().GetScaleCamera() * g_main->GetCamera().GetScaleCamera();
		m_name->setVisible(shownDistanceFromParentSquared >= NAME_MINIMUM_DISTANCE_SQUARED);
	}

	// --- change name color ---
	if (m_name->getVisible())
	{
		m_name->SetColor(planetRadius > PLANET_RADIUS_MAXIMUM_FOR_WHITE_NAME ? oxygine::Color::Black : oxygine::Color::White);
	}

	// --- hide trajectory on big scale ---
	if (m_orbitData.currentPrimitives.orbitTrajectory)
		m_orbitData.currentPrimitives.orbitTrajectory->setVisible(g_main->GetCamera().GetScaleCamera() < TRAJECTORY_MAXIMUM_SCALE);

	// --- rebuild trajectory if current time is in next period ---
	{
		size_t currentPeriod = GetCurrentPeriod(m_orbitData.topMostParentPeriodTime);

		if (currentPeriod > m_orbitData.currentPeriod)
			AsyncRebuildTrajectory();
	}
}

void AnimationBody::HandleOrbitDataRebuildFlags()
{
	std::lock_guard<std::mutex> lock(m_orbitDataLock);

	if (m_orbitData.rebuild[(size_t)OrbitData::Rebuild::ellipse])
	{
		m_orbitData.rebuild[(size_t)OrbitData::Rebuild::ellipse] = false;

		std::lock_guard<const simmulation::CelestialBody> lock(*m_body.get());
		oxygine::spActor parent = GetParentActor();

		SwapPrimitives((oxygine::spActor&)m_orbitData.currentPrimitives.approximatedOrbit, (oxygine::spActor&)m_orbitData.newPrimitives.approximatedOrbit, parent.get());
		SwapPrimitives((oxygine::spActor&)m_orbitData.currentPrimitives.keplerOrbit, (oxygine::spActor&)m_orbitData.newPrimitives.keplerOrbit, parent.get());
	}

	if (m_orbitData.rebuild[(size_t)OrbitData::Rebuild::trajectory])
	{
		m_orbitData.rebuild[(size_t)OrbitData::Rebuild::trajectory] = false;

		SwapPrimitives((oxygine::spActor&)m_orbitData.currentPrimitives.orbitTrajectory, (oxygine::spActor&)m_orbitData.newPrimitives.orbitTrajectory, m_system);
	}
}

oxygine::Actor * AnimationBody::GetParentActor()
{
	if (m_body->GetParent())
	{
		auto parent = m_body->GetParent();
		return m_system->GetAnimationBody(parent)->m_actor.get();
	}
	return m_system;
}

const std::shared_ptr<const simmulation::CelestialBody> & AnimationBody::GetCelestialBody() const
{
	return m_body;
}

oxygine::spActor & AnimationBody::GetActor()
{
	return m_actor;
}

void AnimationBody::AsyncRebuildTrajectory()
{
	if (m_rebuildTrajectoryRunning)
		return;
	m_rebuildTrajectoryRunning = true;

	if (m_rebuildTrajectoryThread.joinable())
		m_rebuildTrajectoryThread.join();

	m_rebuildTrajectoryThread = std::thread([this]()
	{
		std::lock_guard<const simmulation::CelestialBody> lock(*m_body.get());

		auto approximatedOrbit = CreateApproximatedOrbit();
		auto keplerOrbit = CreateKeplerOrbit(); // TODO this is not needed

		{
			std::lock_guard<std::mutex> lock(m_orbitDataLock);
			m_orbitData.newPrimitives.approximatedOrbit = approximatedOrbit;
			m_orbitData.newPrimitives.keplerOrbit = keplerOrbit;
			m_orbitData.rebuild[(size_t)OrbitData::Rebuild::ellipse] = true;

			m_orbitData.topMostParentPeriodTime = GetTopMostParentPeriodTime();
			m_orbitData.currentPeriod = GetCurrentPeriod(m_orbitData.topMostParentPeriodTime);
		}

		m_rebuildTrajectoryRunning = false;
	});
}

double AnimationBody::GetTopMostParentPeriodTime()
{
	double periodTime = m_body->GetParentRelativeData().period;
	const simmulation::CelestialBody * body = m_body.get();

	while (body)
	{
		std::lock_guard<const simmulation::CelestialBody> lockBody(*body);
		if (!body->GetParent())
			break;
		std::lock_guard<const simmulation::CelestialBody> lockParent(*body->GetParent().get());

		// iterate while parent of parent of parent is star
		if (!body->GetParent()->GetParent())
		{
			periodTime = body->GetParentRelativeData().period;
			break;
		}

		body = body->GetParent().get();
	}

	return periodTime;
}

size_t AnimationBody::GetCurrentPeriod(double celestialBodyPeriodTime)
{
	if (celestialBodyPeriodTime == 0.0)
		return 0;

	return (size_t)((m_system->GetRunningTime()) / celestialBodyPeriodTime);
}

void AnimationBody::ShowInfo(bool show)
{
	m_showInfo = show;
}

bool AnimationBody::IsInfoShown()
{
	return m_showInfo;
}
