#pragma once
#include "AnimationBody.h"
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <atomic>
#include <condition_variable>
#include "AnimationBodyTree.h"

class AnimationBodyInfoUpdater
{
public:
	AnimationBodyInfoUpdater();
	~AnimationBodyInfoUpdater();

	void Register(AnimationBody * body, AnimationBody * parent);
	void Unregister(AnimationBody * body);

	void UpdateInfo(size_t nextIndex, float stepProgress);
	void UpdateParents(AnimationBodyNode & tree);

private:
	struct BodyRegistration
	{
		AnimationBody * body;
		AnimationBody * parent;
	};
	std::vector<BodyRegistration> m_bodies;
	std::shared_timed_mutex m_bodiesLock;

	void SwapPrimitives();

	DECLARE_SMART(InfoActor, spInfoActor);
	class InfoActor : public oxygine::Actor
	{
	public:
		InfoActor();
		// update distance each step
		void UpdateDistance(const oxygine::Vector2 & position, const oxygine::Vector2 & parentPosition);
		// update velocity each step
		void UpdateVelocity(float stepProgress); // progress of current step

		void UpdateIndex(size_t index, AnimationBody * body, AnimationBody * parent);

	private:
		primitive::spDirectedTextLine m_distance;
		primitive::spDirectedTextLine m_velocity;
		primitive::spText m_dataValue;

		// initial angle of velocity vector
		float m_velocityInitialAngle = 0.0;
		// angle by which will rotate velocity vector in current step
		float m_velocityAngleStepUpdate = 0.0;
	};

	std::vector<spInfoActor> m_newInfo;
	std::vector<spInfoActor> m_currentInfo;
	std::mutex m_infoLock;

	// index that will be computed by worker
	size_t m_newIndex = 0;

	void InfoWorkerCallback();
	std::thread m_infoWorker;
	std::atomic<bool> m_infoWorkerRunning{ true };
	std::condition_variable m_infoWorkerCV;
	std::atomic<bool> m_infoDataReady{ false };
};
