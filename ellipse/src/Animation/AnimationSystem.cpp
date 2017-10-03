#include "AnimationSystem.h"
#include <chrono>
#include "MainActor.h"
#include "Common.h"

AnimationSystem::AnimationSystem()
{
	setSize(oxygine::getStage()->getSize());
}

AnimationSystem::~AnimationSystem()
{
	for (auto & b : m_bodies)
		m_interpolation.Unregister(b.get());
}

void AnimationSystem::AddBody(std::shared_ptr<const simmulation::CelestialBody> body)
{
	m_bodies.push_back(std::unique_ptr<AnimationBody>(new AnimationBody(body, this)));
	m_interpolation.Register(m_bodies.back().get());
}

void AnimationSystem::Rebuild()
{
	for (auto & b : m_bodies)
		b->Rebuild();
	
	m_interpolation.UpdatePointsOfAllBodies();
	
	std::unique_lock<std::shared_timed_mutex> treeLock(m_bodyTreeLock);
	m_bodyTree.reset(new AnimationBodyNode(m_bodies));
}

AnimationBody * AnimationSystem::GetAnimationBody(std::shared_ptr<const simmulation::CelestialBody> & celestial)
{
	for (auto & b : m_bodies)
	{
		if (b->GetCelestialBody() == celestial)
			return b.get();
	}
	return nullptr;
}

const std::shared_ptr<const simmulation::CelestialBody> & AnimationSystem::GetCelestialBody(AnimationBody * body)
{
	for (auto & b : m_bodies)
	{
		if (b.get() == body)
			return b->GetCelestialBody();
	}
	return simmulation::CelestialBody::invalidCPtr;
}

void AnimationSystem::Start()
{
	m_isStarted = true;
}

void AnimationSystem::Pause()
{
	m_isStarted = false;
}

void AnimationSystem::Stop()
{
	m_isStarted = false;
	m_interpolation.SetCurrentIndex(0);

	UpdateFollowPosition();
	UpdateBodies();
	m_infoUpdater.UpdateInfo(m_interpolation.GetCurrentIndex() + 1, m_interpolation.GetCurrentStepProgress());
}

void AnimationSystem::doUpdate(const oxygine::UpdateState & us)
{
	if (m_isStarted)
		m_interpolation.Update(us.dt);
	
	UpdateFollowPosition();
	UpdateBodies();
	m_infoUpdater.UpdateInfo(m_interpolation.GetCurrentIndex(), m_interpolation.GetCurrentStepProgress());
}

void AnimationSystem::UpdateFollowPosition()
{
	oxygine::VectorD2 followPositionOffset(0.0, 0.0);
	if (m_followBody)
		followPositionOffset = m_interpolation.GetCurrentPoint(m_followBody);
	setPosition(-followPositionOffset);
}

void AnimationSystem::UpdateBodies()
{
	std::shared_lock<std::shared_timed_mutex> treeLock(m_bodyTreeLock);

	for (auto & b : m_bodies)
	{
		oxygine::VectorD2 position = m_interpolation.GetCurrentPoint(b.get());

		AnimationBody * parent = m_bodyTree->GetParent(b.get());
		oxygine::VectorD2 parentPosition;
		if (parent)
			parentPosition = m_interpolation.GetCurrentPoint(m_bodyTree->GetParent(b.get()));

		b->Update(position, parent ? &parentPosition : nullptr);
	}

}

std::vector<AnimationBody*> AnimationSystem::Query(const oxygine::Vector2 & position, float radius)
{
	std::vector<AnimationBody*> ret;

	auto p2 = g_main->GetCamera().LocalFromStage(position);
	float lr = radius / g_main->GetCamera().GetScaleCamera();
	for (auto & b : m_bodies)
	{
		auto p1 = g_main->GetCamera().LocalFromChild(b->GetActor());

		if ((p2 - p1).length() < lr)
			ret.push_back(b.get());
	}

	return ret;
}

std::vector<AnimationBody*> AnimationSystem::GetDirectChilds(AnimationBody * parent)
{
	std::shared_lock<std::shared_timed_mutex> treeLock(m_bodyTreeLock);
	return m_bodyTree->GetChilds(parent);
}

double AnimationSystem::GetRunningTime()
{
	return m_interpolation.GetCurrentTime();
}

size_t AnimationSystem::GetCurrentIndexInCelestialData()
{
	return m_interpolation.GetCurrentIndex();
}

void AnimationSystem::SetShowInfo(bool show)
{
	for (auto & b : m_bodies)
		b->ShowInfo(show);
}

void AnimationSystem::ToggleInfo(AnimationBody * body)
{
	if (body->IsInfoShown())
	{
		body->ShowInfo(false);
		m_infoUpdater.Unregister(body);
	}
	else
	{
		body->ShowInfo(true);

		std::shared_lock<std::shared_timed_mutex> treeLock(m_bodyTreeLock);
		m_infoUpdater.Register(body, m_bodyTree->GetParent(body));
	}
}

void AnimationSystem::SetFollowBody(AnimationBody * body)
{
	m_followBody = body;
}

AnimationBody * AnimationSystem::GetFollowBody()
{
	return m_followBody;
}

spAnimationSystem g_animationActor = nullptr;
