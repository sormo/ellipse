#include "AnimationBodyInfoUpdater.h"
#include "Common.h"
#include "MainActor.h"

float fixedVelocityAngle(float previous, float current, float next)
{
	// fix transition from -PI to PI or vice versa
	float ret;

	if (current > 0.0f && next < 0.0f)
	{
		if (previous < current)
			ret = next - current + (float)(2.0*PI);
		else
			ret = next - current;
	}
	else if (current < 0.0f && next > 0.0f)
	{
		if (previous < current)
			ret = next - current;
		else
			ret = next - current - (float)(2.0*PI);
	}
	else
		ret = next - current;

	return ret;
}

AnimationBodyInfoUpdater::AnimationBodyInfoUpdater()
{
	m_infoWorker = std::thread(&AnimationBodyInfoUpdater::InfoWorkerCallback, this);
}

AnimationBodyInfoUpdater::~AnimationBodyInfoUpdater()
{
	m_infoWorkerRunning = false;
	if (m_infoWorker.joinable())
	{
		m_infoDataReady = true;
		m_infoWorkerCV.notify_one();
		m_infoWorker.join();
	}
}

AnimationBodyInfoUpdater::InfoActor::InfoActor()
{
	static const float VELOCITY_VECTOR_LENGTH = 60.0f;

	m_distance = new primitive::DirectedTextLine({ 0.0f, 0.0f }, { 0.0f, 10.0f });
	m_distance->SetLineColor({ 125, 125, 125, 50 });

	m_distance->GetCenterText()->SetText("10.0e20 m");
	m_distance->GetCenterText()->SetNotZoomable(true);

	m_velocity = new primitive::DirectedTextLine({ 0.0f, 0.0f }, { VELOCITY_VECTOR_LENGTH, 0.0f });
	m_velocity->SetLineColor({ 125, 125, 125, 50 });
	m_velocity->SetRightArrow(true);
	m_velocity->GetRightText()->SetText("10.0e20 m/s");
	m_velocity->SetNotZoomable(true);

	m_dataValue = new primitive::Text("");
	m_dataValue->SetNotZoomable(true);

	addChild(m_distance);
	addChild(m_velocity);
	addChild(m_dataValue);
}

void AnimationBodyInfoUpdater::InfoActor::UpdateIndex(size_t index, AnimationBody * animBody, AnimationBody * animParent)
{
	char buffer[64];

	// read data
	oxygine::VectorD2 position, parentPosition,
		parentMomentum, momentum, nextParentMomentum, nextMomentum, prevParentMomentum, prevMomentum;
	double mass, parentMass;
	std::string name;

	{
		const simmulation::CelestialBody * body = animBody->GetCelestialBody().get();
		std::lock_guard<const simmulation::CelestialBody> celestialBodyLock(*body);
		const simmulation::CelestialBody * parent = animParent->GetCelestialBody().get();
		std::lock_guard<const simmulation::CelestialBody> celestialParentLock(*parent);

		if (body->GetPositions().size() <= index)
			return;

		position = body->GetPositions()[index];
		parentPosition = parent->GetPositions()[index];

		momentum = body->GetMomenta()[index];
		nextMomentum = index + 1 >= body->GetMomenta().size() ? momentum : body->GetMomenta()[index + 1];
		prevMomentum = index == 0 ? momentum : body->GetMomenta()[index - 1];

		parentMomentum = parent->GetMomenta()[index];
		nextParentMomentum = index + 1 >= parent->GetMomenta().size() ? parentMomentum : parent->GetMomenta()[index + 1];
		prevParentMomentum = index == 0 ? parentMomentum : parent->GetMomenta()[index - 1];

		mass = body->GetMass();
		parentMass = parent->GetMass();

		name = body->GetName();
	}

	// update primitives

	sprintf(buffer, "%.2e m", scalePositionInv(position - parentPosition).length());
	m_distance->GetCenterText()->SetText(buffer);
	m_distance->GetCenterText()->SetNotZoomable(true);

	oxygine::VectorD2 velocity = momentum / mass - parentMomentum / parentMass;
	oxygine::VectorD2 nextVelocity = nextMomentum / mass - nextParentMomentum / parentMass;
	oxygine::VectorD2 prevVelocity = prevMomentum / mass - prevParentMomentum / parentMass;

	velocity = scaleVelocityInv(velocity);

	float currentAngle = (float)getAngle(velocity);
	float nextAngle = (float)getAngle(nextVelocity);
	float prevAngle = (float)getAngle(prevVelocity);

	sprintf(buffer, "%.2e m/s", velocity.length());
	m_velocity->GetRightText()->SetText(buffer);
	//m_newInfo[i].velocityValue->setRotation((float)-getAngle(prevVelocity));

	m_velocityInitialAngle = currentAngle;
	m_velocityAngleStepUpdate = fixedVelocityAngle(prevAngle, currentAngle, nextAngle);

	if (m_dataValue->GetText().empty())
	{
		mass = scaleKilogramsInv(mass);

		sprintf(buffer, "%s\n%.2e kg", name.c_str(), mass);
		m_dataValue->SetText(buffer);
	}
}

void AnimationBodyInfoUpdater::InfoWorkerCallback()
{
	auto & ready = m_infoDataReady;

	while (m_infoWorkerRunning)
	{
		std::unique_lock<std::mutex> infoLock(m_infoLock);
		m_infoWorkerCV.wait(infoLock, [&ready] { return ready.load(); });
		ready = false;

		// prepare new primitives
		{
			std::shared_lock<std::shared_timed_mutex> bodiesLock(m_bodiesLock);
			for (size_t i = 0; i < m_bodies.size(); ++i)
			{
				m_newInfo[i]->UpdateIndex(m_newIndex, m_bodies[i].body, m_bodies[i].parent);
			}
		}
	}
}

void AnimationBodyInfoUpdater::Register(AnimationBody * body, AnimationBody * parent)
{
	std::unique_lock<std::shared_timed_mutex> lockBodies(m_bodiesLock);
	for (auto b : m_bodies)
		if (b.body == body)
			return; // already registered

	BodyRegistration registration{ body, parent };
	m_bodies.push_back(registration);
	
	spInfoActor currentInfo = new InfoActor();
	spInfoActor newInfo = new InfoActor();

	currentInfo->UpdateIndex(m_newIndex - 1, body, parent);
	
	std::lock_guard<std::mutex> lockInfo(m_infoLock);
	m_currentInfo.push_back(currentInfo);
	m_newInfo.push_back(newInfo);

	body->GetActor()->addChild(currentInfo);
}

void AnimationBodyInfoUpdater::Unregister(AnimationBody * body)
{
	std::unique_lock<std::shared_timed_mutex> lockBodies(m_bodiesLock);
	std::lock_guard<std::mutex> lockInfo(m_infoLock);

	for (size_t i = 0; i < m_bodies.size(); ++i)
	{
		if (m_bodies[i].body != body)
			continue;

		if (m_currentInfo[i]->getParent())
			m_currentInfo[i]->getParent()->removeChild(m_currentInfo[i]);

		if (m_newInfo[i]->getParent())
			m_newInfo[i]->getParent()->removeChild(m_newInfo[i]);


		m_bodies.erase(std::begin(m_bodies) + i);
		m_currentInfo.erase(std::begin(m_currentInfo) + i);
		m_newInfo.erase(std::begin(m_newInfo) + i);

		break;
	}
}

void AnimationBodyInfoUpdater::UpdateInfo(size_t currentIndex, float stepProgress)
{
	bool newIndexChange = false;

	if (m_newIndex <= currentIndex)
	{
		std::lock_guard<std::mutex> lock(m_infoLock);

		SwapPrimitives();
		newIndexChange = true;
	}
	else
	{
		// this is special case, index has been returned back m_newInfo is useless
		std::lock_guard<std::mutex> lock(m_infoLock);
		std::shared_lock<std::shared_timed_mutex> bodiesLock(m_bodiesLock);

		for (size_t i = 0; i < m_bodies.size(); ++i)
			m_currentInfo[i]->UpdateIndex(currentIndex, m_bodies[i].body, m_bodies[i].parent);
		newIndexChange = true;
	}

	if (newIndexChange)
	{
		m_newIndex = currentIndex + 1;
		m_infoDataReady = true;
		m_infoWorkerCV.notify_one();
	}

	// this is hack, position is interpolated so we can't use the one provided by updater
	std::shared_lock<std::shared_timed_mutex> bodiesLock(m_bodiesLock);
	for (size_t i = 0; i < m_bodies.size(); ++i)
	{
		m_currentInfo[i]->UpdateDistance(m_bodies[i].body->GetActor()->getPosition(), 
			m_bodies[i].parent ? m_bodies[i].parent->GetActor()->getPosition() : m_bodies[i].body->GetActor()->getPosition());
		m_currentInfo[i]->UpdateVelocity(stepProgress);
	}
}

void AnimationBodyInfoUpdater::SwapPrimitives()
{
	assert(m_bodies.size() == m_newInfo.size() && m_bodies.size() == m_currentInfo.size());
	std::shared_lock<std::shared_timed_mutex> bodiesLock(m_bodiesLock);

	for (size_t i = 0; i < m_bodies.size(); ++i)
	{
		if (m_currentInfo[i]->getParent())
			m_currentInfo[i]->getParent()->removeChild(m_currentInfo[i]);
		m_bodies[i].body->GetActor()->addChild(m_newInfo[i]);
	}

	m_newInfo.swap(m_currentInfo);
}

void AnimationBodyInfoUpdater::InfoActor::UpdateDistance(const oxygine::Vector2 & position, const oxygine::Vector2 & parentPosition)
{
	m_distance->Set({ 0.0f, 0.0f }, parentPosition - position);
}

void AnimationBodyInfoUpdater::InfoActor::UpdateVelocity(float stepProgress)
{
	float angle = m_velocityInitialAngle + stepProgress*m_velocityAngleStepUpdate;

	m_velocity->setRotation(angle);
	m_velocity->GetRightText()->setRotation(-angle);
}

void AnimationBodyInfoUpdater::UpdateParents(AnimationBodyNode & tree)
{
	for (BodyRegistration & registration : m_bodies)
		registration.parent = tree.GetParent(registration.body);
}
