#pragma once
#include "oxygine-framework.h"
#include <vector>
#include "AnimationBody.h"
#include "AnimationBodyInfoUpdater.h"
#include "AnimationInterpolation.h"
#include "AnimationBodyTree.h"

class AnimationSystem : public oxygine::Actor
{
public:
	AnimationSystem();
	~AnimationSystem();

	void AddBody(std::shared_ptr<const simmulation::CelestialBody> body);

	void Rebuild();

	AnimationBody * GetAnimationBody(std::shared_ptr<const simmulation::CelestialBody> & celestial);
	const std::shared_ptr<const simmulation::CelestialBody> & GetCelestialBody(AnimationBody * body);

	void Start();
	void Pause();
	void Stop();

	std::vector<AnimationBody*> Query(const oxygine::Vector2 & position, float radius);
	std::vector<AnimationBody*> GetDirectChilds(AnimationBody * parent);

	// running time of animation in seconds
	double GetRunningTime();
	// current index in celestial data
	size_t GetCurrentIndexInCelestialData();

	void SetShowInfo(bool show);
	void ToggleInfo(AnimationBody * body);

	void SetFollowBody(AnimationBody * body);
	AnimationBody * GetFollowBody();

private:
	std::vector<std::unique_ptr<AnimationBody>> m_bodies;

	bool m_isStarted;

	AnimationInterpolation m_interpolation;
	AnimationBodyInfoUpdater m_infoUpdater;

	virtual void doUpdate(const oxygine::UpdateState & us) override;
	void UpdateBodies();
	// All childs of animation system are drawn relative to follow body position
	// i.e. follow body position is point [0,0] for all childs.
	// This is because of float overflow for planets farther from sun with bigger zoom.
	void UpdateFollowPosition();
	AnimationBody * m_followBody = nullptr;

	// planet tree hierarchy from simmulation stored locally in animation
	std::unique_ptr<AnimationBodyNode> m_bodyTree;
	std::shared_timed_mutex m_bodyTreeLock;
};

typedef oxygine::intrusive_ptr<AnimationSystem> spAnimationSystem;
