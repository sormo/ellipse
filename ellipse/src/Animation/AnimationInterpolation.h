#pragma once
#include "oxygine-framework.h"
#include "Common.h"
#include <vector>
#include <unordered_map>
#include <mutex>
#include <thread>
#include <atomic>
#include <condition_variable>

class AnimationBody;

class AnimationInterpolation
{
public:
	AnimationInterpolation();
	~AnimationInterpolation();

	void Register(const AnimationBody * body);
	void Unregister(const AnimationBody * body);

	void UpdatePointsOfAllBodies();

	oxygine::VectorD2 GetCurrentPoint(const AnimationBody * body);

	// increase current time in milliseconds
	// return true if time has been updated
	bool Update(oxygine::timeMS ms);

	// time for one simmulation dt in milliseconds (time between two indexes)
	void SetSpeed(oxygine::timeMS ms);

	// in seconds TODO synchronize ???
	double GetCurrentTime();
	size_t GetCurrentIndex();
	void SetCurrentIndex(size_t index);

	// get number between 0 and 1 representing progress of current step
	// step is advancing from one index to another
	float GetCurrentStepProgress();

private:
	void Interpolate(const oxygine::VectorD2 & p1, const oxygine::VectorD2 & p2, std::vector<oxygine::VectorD2> & steps);

	// length of one step in milliseconds
	oxygine::timeMS m_speed = ANIMATION_SPEED;
	oxygine::timeMS m_time = 0;

	size_t GetNumberOfStepsBetweenTwoPoints();
	float GetLengthOfStepInMilliseconds();

	size_t GetCurrentStep(oxygine::timeMS ms);
	size_t GetCurrentPoint(oxygine::timeMS ms);
	
	// registration data
	struct RegistrationData
	{
		// map of body to index to points vector
		std::unordered_map<const AnimationBody*, size_t> indexMap;
		std::vector<std::vector<oxygine::VectorD2>> points;

	} m_registrationData;
	std::mutex m_registrationLock;

	void RebuildPositions();
	void RebuildInterpolationsForIndex(size_t index); // TODO this is position index

	void InterpolateForBodyWithIndex(size_t index); // TODO this is body index !!! ):

	// current steps data
	struct CurrentStepsData
	{
		std::vector<std::vector<oxygine::VectorD2>> currentSteps;
		size_t currentPoint = 0;

	} m_currentData;

	// next steps worker
	struct NextStepsData
	{
		std::vector<std::vector<oxygine::VectorD2>> nextSteps;
		size_t nextPoint = 0;
		bool nextDataProcessed = false;

	} m_nextData;
	std::mutex m_nextDataLock;

	std::thread m_nextStepsWorker;
	std::atomic<bool> m_nextStepsWorkerRunning{ true };

	std::condition_variable m_nextStepsWorkerCV;
	std::atomic<bool> m_nextStepsDataReady{ false };
};
