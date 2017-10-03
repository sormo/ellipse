#include "AnimationInterpolation.h"
#include "AnimationBody.h"
#include "Common.h"
#include <cmath>

#define FPS_UPPER_BOUND 60 // frames per second
#define MPF_UPPER_BOUND ((float)1000/(float)FPS_UPPER_BOUND) // milliseconds per frame

AnimationInterpolation::AnimationInterpolation()
{
	m_nextStepsWorker = std::thread([this]()
	{
		auto & ready = m_nextStepsDataReady;

		while (m_nextStepsWorkerRunning)
		{
			std::unique_lock<std::mutex> lk(m_nextDataLock);
			m_nextStepsWorkerCV.wait(lk, [&ready] { return ready.load(); });
			ready = false;

			if (!m_nextStepsWorkerRunning)
				return;

			size_t p = m_nextData.nextPoint;
			m_nextData.nextDataProcessed = true;

			std::lock_guard<std::mutex> registrationLock(m_registrationLock);
			for (size_t i = 0; i < m_nextData.nextSteps.size(); ++i)
			{
				if (p + 1 < m_registrationData.points[i].size())
				{
					Interpolate(m_registrationData.points[i].at(p), m_registrationData.points[i].at(p + 1), m_nextData.nextSteps[i]);
				}
				else
					m_nextData.nextDataProcessed = false;
			}
		}
	});
}

AnimationInterpolation::~AnimationInterpolation()
{
	if (m_nextStepsWorker.joinable())
	{
		m_nextStepsWorkerRunning = false;
		m_nextStepsDataReady = true;
		m_nextStepsWorkerCV.notify_one();
		m_nextStepsWorker.join();
	}
}

void AnimationInterpolation::Register(const AnimationBody * body)
{
	std::lock_guard<std::mutex> nextDataLock(m_nextDataLock);
	std::lock_guard<std::mutex> registrationLock(m_registrationLock);
	
	std::vector<oxygine::VectorD2> positions;
	{
		std::lock_guard<const simmulation::CelestialBody> celestialLock(*body->GetCelestialBody().get());
		positions = body->GetCelestialBody()->GetPositions();
	}

	std::vector<oxygine::VectorD2> current(GetNumberOfStepsBetweenTwoPoints(), oxygine::VectorD2(0.0, 0.0));
	std::vector<oxygine::VectorD2> next(GetNumberOfStepsBetweenTwoPoints(), oxygine::VectorD2(0.0, 0.0));

	size_t registrationIndex = m_registrationData.points.size();
	m_registrationData.points.push_back(std::move(positions));
	m_registrationData.indexMap[body] = registrationIndex;

	m_currentData.currentSteps.push_back(std::move(current));
	m_nextData.nextSteps.push_back(std::move(next));

	InterpolateForBodyWithIndex(registrationIndex);

	// TODO
	if (positions.size() > m_currentData.currentPoint + 2)
	{
		m_nextData.nextPoint = m_currentData.currentPoint + 1;
		m_nextData.nextDataProcessed = true; // TODO
	}
}

void AnimationInterpolation::InterpolateForBodyWithIndex(size_t index)
{
	std::vector<oxygine::VectorD2> & positions = m_registrationData.points[index];
	std::vector<oxygine::VectorD2> & current = m_currentData.currentSteps[index];
	std::vector<oxygine::VectorD2> & next = m_nextData.nextSteps[index];

	// interpolate current
	if (positions.size() == m_currentData.currentPoint + 1)
	{
		// special case when only one position is available
		Interpolate(positions[m_currentData.currentPoint], positions[m_currentData.currentPoint], current);
		Interpolate(positions[m_currentData.currentPoint], positions[m_currentData.currentPoint], next);
	}
	else
	{
		if (positions.size() > m_currentData.currentPoint + 1)
			Interpolate(positions[m_currentData.currentPoint], positions[m_currentData.currentPoint + 1], current);

		if (positions.size() > m_currentData.currentPoint + 2)
			Interpolate(positions[m_currentData.currentPoint + 1], positions[m_currentData.currentPoint + 2], next);
	}
}

void AnimationInterpolation::RebuildPositions()
{
	for (auto & bodyIndexData : m_registrationData.indexMap)
	{
		const AnimationBody * body = bodyIndexData.first;
		size_t index = bodyIndexData.second;

		std::lock_guard<const simmulation::CelestialBody> celestialLock(*body->GetCelestialBody().get());

		const auto & newPositions = body->GetCelestialBody()->GetPositions();
		size_t currentPointsSize = m_registrationData.points[index].size();

		if (newPositions.size() >= currentPointsSize)
		{
			// append new points locally
			std::copy(std::begin(newPositions) + currentPointsSize, std::end(newPositions),
				std::back_inserter(m_registrationData.points[index]));
		}
		else
		{
			m_registrationData.points[index].erase(std::begin(m_registrationData.points[index]) + newPositions.size(),
				std::end(m_registrationData.points[index]));
		}
	}
}

void AnimationInterpolation::UpdatePointsOfAllBodies()
{
	std::lock_guard<std::mutex> nextDataLock(m_nextDataLock);
	std::lock_guard<std::mutex> registrationLock(m_registrationLock);

	RebuildPositions();
	RebuildInterpolationsForIndex(m_currentData.currentPoint);
}

void AnimationInterpolation::Unregister(const AnimationBody * body)
{
	std::lock_guard<std::mutex> nextDataLock(m_nextDataLock);
	std::lock_guard<std::mutex> registrationLock(m_registrationLock);

	auto it = m_registrationData.indexMap.find(body);
	if (it != std::end(m_registrationData.indexMap))
	{
		size_t index = it->second;
		m_registrationData.indexMap.erase(it);

		m_currentData.currentSteps.erase(std::begin(m_currentData.currentSteps) + index);
		m_nextData.nextSteps.erase(std::begin(m_nextData.nextSteps) + index);

		// update indices after removed one
		assert(m_currentData.currentSteps.size() == m_nextData.nextSteps.size());
		for (auto & v : m_registrationData.indexMap)
		{
			if (v.second > index)
				v.second--;
		}
	}
}

oxygine::VectorD2 AnimationInterpolation::GetCurrentPoint(const AnimationBody * body)
{
	std::lock_guard<std::mutex> registrationLock(m_registrationLock);
	size_t index = m_registrationData.indexMap[body];
	
	return m_currentData.currentSteps[index][GetCurrentStep(m_time)];
}

// update current time in milliseconds
bool AnimationInterpolation::Update(oxygine::timeMS ms)
{
	bool readyToUpdate = true;
	size_t p = GetCurrentPoint(m_time + ms);

	if (p > m_currentData.currentPoint)
	{
		std::lock_guard<std::mutex> nextDataLock(m_nextDataLock);

		if (m_nextData.nextPoint == p && m_nextData.nextDataProcessed)
		{
			for (size_t i = 0; i < m_nextData.nextSteps.size(); ++i)
				m_nextData.nextSteps[i].swap(m_currentData.currentSteps[i]);
			m_nextData.nextPoint = p + 1;
		}
		else
		{
			m_nextData.nextPoint = p;
			readyToUpdate = false;
		}

		m_nextData.nextDataProcessed = false;

		m_nextStepsDataReady = true;
		m_nextStepsWorkerCV.notify_one();
	}

	if (readyToUpdate)
	{
		m_time += ms;
		m_currentData.currentPoint = p;
	}

	return readyToUpdate;
}

// time for one dt in milliseconds
void AnimationInterpolation::SetSpeed(oxygine::timeMS ms)
{
	m_speed = ms;
}

void AnimationInterpolation::Interpolate(const oxygine::VectorD2 & p1, const oxygine::VectorD2 & p2, std::vector<oxygine::VectorD2> & steps)
{
	double length = (p2 - p1).length();
	if (length < LENGTH_SIGMA)
	{
		for (auto & p : steps)
			p = p1;
		return;
	}

	oxygine::VectorD2 v = (p2 - p1).normalized();
	double size = length / (double)steps.size();

	v *= size;

	steps[0] = p1;
	for (size_t i = 1; i < steps.size(); ++i)
	{
		steps[i] = steps[i - 1] + v;
	}
}

size_t AnimationInterpolation::GetNumberOfStepsBetweenTwoPoints()
{
	return (size_t)std::ceil((float)m_speed / (float)MPF_UPPER_BOUND);
}

float AnimationInterpolation::GetLengthOfStepInMilliseconds()
{
	return (float)m_speed / GetNumberOfStepsBetweenTwoPoints();
}

size_t AnimationInterpolation::GetCurrentStep(oxygine::timeMS ms)
{
	return (size_t)((ms % m_speed) / GetLengthOfStepInMilliseconds());
}

size_t AnimationInterpolation::GetCurrentPoint(oxygine::timeMS ms)
{
	return ms / m_speed;
}

double AnimationInterpolation::GetCurrentTime()
{
	return m_currentData.currentPoint * SIMMULATION_DT;
}

void AnimationInterpolation::RebuildInterpolationsForIndex(size_t index)
{
	m_currentData.currentPoint = index;
	for (size_t i = 0; i < m_registrationData.points.size(); ++i)
		InterpolateForBodyWithIndex(i);
	
	// TODO
	m_nextData.nextPoint = m_currentData.currentPoint + 1;
	m_nextData.nextDataProcessed = true; // TODO
}

void AnimationInterpolation::SetCurrentIndex(size_t index)
{
	std::lock_guard<std::mutex> nextDataLock(m_nextDataLock);
	std::lock_guard<std::mutex> registrationLock(m_registrationLock);

	m_time = static_cast<oxygine::timeMS>(index*GetLengthOfStepInMilliseconds());
	RebuildInterpolationsForIndex(index);
}

size_t AnimationInterpolation::GetCurrentIndex()
{
	return m_currentData.currentPoint;
}

float AnimationInterpolation::GetCurrentStepProgress()
{
	return (float)(m_time % m_speed) / (float)m_speed;
}
