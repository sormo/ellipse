#pragma once
#include "oxygine-framework.h"
#include <map>
#include <bitset>
#include <mutex>

#define CAMERA_GLOBAL_MAXIMUM_SCALE 300000.0f
#define CAMERA_ZOOM_FACTOR 1.1f

class Camera
{
public:
	Camera(oxygine::Actor & parent);

	void SetFollowChild(oxygine::spActor child);
	void EnableMove(bool enable);
	void EnableZoom(bool enable);

	void SetPositionFromLocal(const oxygine::Vector2 & localPosition);

	oxygine::Vector2 LocalFromChild(oxygine::spActor & child);
	oxygine::Vector2 LocalFromStage(const oxygine::Vector2 & position);

	void SetScaleCamera(float scale);
	float GetScaleCamera();
	void SetMaximumScale(float scale);
	void RegisterOnScaleChange(void * key, std::function<void(float)> cbk);
	void UnregisterOnScaleChange(void * key);

	void Update();

private:
	float const m_zoomFactor = 1.1f;

	void OnWheelUp(oxygine::Event* event);
	void OnWheelDown(oxygine::Event* event);
	void OnTouchDown(oxygine::Event* event);
	void OnTouchUp(oxygine::Event* event);
	void OnMove(oxygine::Event* event);

	void Zoom(oxygine::Event* event, float factor);

	oxygine::Vector2 m_position;
	float m_maximumScale = CAMERA_GLOBAL_MAXIMUM_SCALE;

	enum class EnableBit { move = 0, zoom };
	std::bitset<2> m_enable;

	struct Touch
	{
		oxygine::Vector2 current;
		oxygine::Vector2 previous;
	};
	std::map<oxygine::pointer_index, Touch> m_touches;

	void ZoomMulti(const Touch & p1, const Touch & p2);

	oxygine::Actor & m_parent;

	void UpdateFollowPosition();
	oxygine::spActor m_follow;
	std::mutex m_followLock;

	void NotifyOnScaleChange();
	std::map<void*, std::function<void(float)>> m_onScaleChangeRegistrations;
};
