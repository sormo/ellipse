#include "Camera.h"

Camera::Camera(oxygine::Actor & parent)
	: m_parent(parent)
{
	EnableMove(true);
	EnableZoom(true);

	oxygine::getStage()->addEventListener(oxygine::TouchEvent::TOUCH_DOWN, CLOSURE(this, &Camera::OnTouchDown));
	oxygine::getStage()->addEventListener(oxygine::TouchEvent::MOVE, CLOSURE(this, &Camera::OnMove));
	oxygine::getStage()->addEventListener(oxygine::TouchEvent::TOUCH_UP, CLOSURE(this, &Camera::OnTouchUp));
	oxygine::getStage()->addEventListener(oxygine::TouchEvent::WHEEL_UP, CLOSURE(this, &Camera::OnWheelUp));
	oxygine::getStage()->addEventListener(oxygine::TouchEvent::WHEEL_DOWN, CLOSURE(this, &Camera::OnWheelDown));
}

void Camera::OnMove(oxygine::Event* event)
{
	if (m_touches.size() == 1 && !m_enable[(size_t)EnableBit::move])
		return;
	if (m_touches.size() > 1 && !m_enable[(size_t)EnableBit::zoom])
		return;

	oxygine::TouchEvent* touch = oxygine::safeCast<oxygine::TouchEvent*>(event);
	m_position = touch->localPosition;

	if (m_touches.find(touch->index) == std::end(m_touches))
		return;

	m_touches[touch->index].previous = m_touches[touch->index].current;
	m_touches[touch->index].current = touch->localPosition;

	//oxygine::log::messageln("move local %d %f %f", touch->index, touch->localPosition.x, touch->localPosition.y);
	//oxygine::log::messageln("move global %d %f %f", touch->index, touch->position.x, touch->position.y);

	if (m_touches.size() == 1)
	{
		oxygine::Vector2 offset = m_touches[touch->index].current - m_touches[touch->index].previous;

		m_parent.setPosition(m_parent.getPosition() + offset);
	}
	else if (m_touches.size() > 1)
	{
		Touch p1, p2;
		for (const auto & t : m_touches)
		{
			if (t.first == touch->index)
				p1 = t.second;
			else
				p2 = t.second;
		}
		ZoomMulti(p1, p2);
	}
}

void Camera::ZoomMulti(const Touch & p1, const Touch & p2)
{
	// Calculate new scale
	oxygine::Vector2 prevScale = m_parent.getScale();
	oxygine::Vector2 curScale = m_parent.getScale() * p1.current.distance(p2.current) / p1.previous.distance(p2.previous);
	if (curScale.x > m_maximumScale)
		curScale = { m_maximumScale, m_maximumScale };

	//oxygine::log::messageln("zoom multi scale %f %f", curScale.x, curScale.y);

	NotifyOnScaleChange();

	oxygine::Vector2 curPosLayer = (p1.current + p2.current) / 2.0f;
	oxygine::Vector2 prevPosLayer = (p1.previous + p2.previous) / 2.0f;
	oxygine::Vector2 t = m_parent.stage2local(curPosLayer);

	m_parent.setScale(curScale);

	if (curScale != prevScale)
	{
		float deltaX = (t.x) * (m_parent.getScale() - prevScale).x;
		float deltaY = (t.y) * (m_parent.getScale() - prevScale).x;

		m_parent.setPosition(m_parent.getPosition() - oxygine::Vector2{ deltaX, deltaY });
	}

	if (prevPosLayer != curPosLayer)
	{
		m_parent.setPosition(m_parent.getPosition() + oxygine::Vector2{ curPosLayer.x - prevPosLayer.x, curPosLayer.y - prevPosLayer.y });
	}
}

void Camera::OnTouchDown(oxygine::Event* event)
{
	oxygine::TouchEvent * touch = dynamic_cast<oxygine::TouchEvent*>(event);
	//oxygine::log::messageln("touch down %d %f %f", touch->index, touch->localPosition.x, touch->localPosition.y);

	m_touches[touch->index].current = m_touches[touch->index].previous = touch->localPosition;
}

void Camera::OnTouchUp(oxygine::Event* event)
{
	oxygine::TouchEvent * touch = dynamic_cast<oxygine::TouchEvent*>(event);
	//oxygine::log::messageln("touch up %f %f", touch->localPosition.x, touch->localPosition.y);

	auto it = m_touches.find(touch->index);
	if (it != std::end(m_touches))
		m_touches.erase(it);
}

void Camera::OnWheelUp(oxygine::Event* event)
{
	if (!m_enable[(size_t)EnableBit::zoom])
		return;

	Zoom(event, 1.0f / m_zoomFactor);
}

void Camera::OnWheelDown(oxygine::Event* event)
{
	if (!m_enable[(size_t)EnableBit::zoom])
		return;

	Zoom(event, m_zoomFactor);
}

void Camera::Zoom(oxygine::Event* event, float factor)
{
	//oxygine::log::message("Zoom: %s\t", event->target->getName().c_str());
	//oxygine::log::messageln("%s", event->currentTarget->getName().c_str());
	oxygine::TouchEvent* touch = oxygine::safeCast<oxygine::TouchEvent*>(event);

	auto shift = m_position - m_parent.getPosition();
	shift *= (1.0f - factor);

	//oxygine::log::messageln("factor %f scale %f", factor, getScale().x * factor);

	float newScale = m_parent.getScale().x * factor;
	if (newScale > m_maximumScale)
		newScale = m_maximumScale;

	m_parent.setScale(newScale);
	NotifyOnScaleChange();

	m_parent.setPosition(m_parent.getPosition() + shift);
	auto const newPos = m_parent.getPosition();
	//oxygine::log::messageln("%f %f", newPos.x, newPos.y);
}

void Camera::Update()
{
	UpdateFollowPosition();

#ifdef _DEBUG
	// show data on debug actor
	auto convertedPosition = LocalFromStage(m_position);
	oxygine::DebugActor::addDebugString("screen: %.0f %.0f", m_position.x, m_position.y);
	oxygine::DebugActor::addDebugString("camera: %.3f %.3f", convertedPosition.x, convertedPosition.y);
	oxygine::DebugActor::addDebugString("zoom: %.6f", m_parent.getScale().x);
#endif
}

oxygine::Vector2 Camera::LocalFromChild(oxygine::spActor & child)
{
	auto pos = child->getParent()->local2stage(child->getPosition());
	return m_parent.getTransformInvert().transform(pos);
}

oxygine::Vector2 Camera::LocalFromStage(const oxygine::Vector2 & position)
{
	return m_parent.getTransformInvert().transform(position);
}

void Camera::SetScaleCamera(float scale)
{
	m_parent.setScale(scale > m_maximumScale ? m_maximumScale : scale);
}

float Camera::GetScaleCamera()
{
	return m_parent.getScale().x;
}

void Camera::SetMaximumScale(float scale)
{
	m_maximumScale = scale > CAMERA_GLOBAL_MAXIMUM_SCALE ? CAMERA_GLOBAL_MAXIMUM_SCALE : scale;
}

// --- on scale change --- 

void Camera::RegisterOnScaleChange(void * key, std::function<void(float)> cbk)
{
	m_onScaleChangeRegistrations[key] = cbk;
}

void Camera::UnregisterOnScaleChange(void * key)
{
	auto it = m_onScaleChangeRegistrations.find(key);
	if (it != std::end(m_onScaleChangeRegistrations))
		m_onScaleChangeRegistrations.erase(it);
}

void Camera::NotifyOnScaleChange()
{
	for (auto & c : m_onScaleChangeRegistrations)
		c.second(GetScaleCamera());
}

// --- follow child ---

void Camera::SetFollowChild(oxygine::spActor child)
{
	std::lock_guard<std::mutex> lock(m_followLock);

	m_follow = child;
}

void Camera::UpdateFollowPosition()
{
	std::lock_guard<std::mutex> lock(m_followLock);

	if (m_follow)
	{
		if (m_follow->_ref_counter == 1)
			m_follow = nullptr;
		else
			SetPositionFromLocal(LocalFromChild(m_follow));
	}
}

void Camera::SetPositionFromLocal(const oxygine::Vector2 & localPosition)
{
	oxygine::Vector2 center = oxygine::getStage()->getSize() / 2.0f;
	oxygine::Vector2 p = center - localPosition * m_parent.getScale().x;

	m_parent.setPosition(p);
}

// ---

void Camera::EnableMove(bool enable)
{
	m_enable[(size_t)EnableBit::move] = enable;
}

void Camera::EnableZoom(bool enable)
{
	m_enable[(size_t)EnableBit::zoom] = enable;
}
