#include "Slider.h"
#include "Primitives\Line.h"
#include "Primitives\Circle.h"

namespace gui
{
	const float Slider::SLIDER_RADIUS = 20.0f;

	Slider::Slider(float size, double left, double current, double right, std::function<void(double)> onChange)
		: m_left(left), m_right(right), m_current(current), m_onChange(onChange)
	{
		assert(left <= current && right >= current);

		m_line = new primitive::DirectedTextLine({ 0.0f, SLIDER_RADIUS }, { size, SLIDER_RADIUS });
		m_line->SetLineColor(oxygine::Color(oxygine::Color::White, 40));
		m_line->SetLineWidth(3.0f);
		addChild(m_line);
		
		m_circle = new primitive::Circle(SLIDER_RADIUS);
		m_circle->SetLineColor(oxygine::Color(oxygine::Color::White, 60));
		m_circle->SetLineWidth(6.0f);
		m_circle->setAnchor(0.5f, 0.5f);
		addChild(m_circle);

		UpdateCircleAccordingToCurrent();

		addEventListener(oxygine::TouchEvent::TOUCH_DOWN, CLOSURE(this, &Slider::OnTouchDown));
		addEventListener(oxygine::TouchEvent::MOVE, CLOSURE(this, &Slider::OnTouchMove));
		addEventListener(oxygine::TouchEvent::TOUCH_UP, CLOSURE(this, &Slider::OnTouchUp));

		setSize({ size, 2.0f*SLIDER_RADIUS });
	}

	void Slider::OnTouchDown(oxygine::Event* event)
	{
		if (m_currentTouch != INVALID_TOUCH)
			return;

		oxygine::TouchEvent * touch = oxygine::safeCast<oxygine::TouchEvent*>(event);
		m_currentTouch = touch->index;
		m_circle->setPosition({ touch->localPosition.x, SLIDER_RADIUS });
		UpdateCurrentAccordingToCircle();

		event->stopsImmediatePropagation = true;
	}

	void Slider::OnTouchMove(oxygine::Event* event)
	{
		oxygine::TouchEvent * touch = oxygine::safeCast<oxygine::TouchEvent*>(event);
		if (touch->index != m_currentTouch)
			return;

		touch->stopsImmediatePropagation = true;

		m_circle->setPosition({ touch->localPosition.x, SLIDER_RADIUS });
		UpdateCurrentAccordingToCircle();
	}

	void Slider::OnTouchUp(oxygine::Event* event)
	{
		oxygine::TouchEvent * touch = oxygine::safeCast<oxygine::TouchEvent*>(event);
		if (touch->index != m_currentTouch)
			return;
		
		m_currentTouch = INVALID_TOUCH;

		touch->stopsImmediatePropagation = true;
	}

	void Slider::SetValue(double current)
	{
		assert(m_left <= m_current && m_right >= m_current);

		m_current = current;
		m_onChange(m_current);
	}

	void Slider::UpdateCircleAccordingToCurrent()
	{
		double newX = ((m_current - m_left)*m_line->GetRightPoint().x) / (m_right - m_left) + 0;
		m_circle->setPosition({ (float)newX, SLIDER_RADIUS });
	}

	void Slider::UpdateCurrentAccordingToCircle()
	{
		double x = m_circle->getPosition().x - m_line->getPosition().x;
		double max = m_line->GetRightPoint().x;

		m_current = x / max;
		m_current = m_current*(m_right - m_left) + m_left;

		m_onChange(m_current);
	}

	void Slider::SetDisplayMask(const char * mask)
	{
		if (mask == nullptr || (m_displayMask != mask && !m_displayMask.empty()))
		{
			m_line->GetLeftText()->SetText("");
			m_line->GetRightText()->SetText("");
		}

		if (mask == nullptr)
			return;

		m_displayMask = mask;

		char buffer[128];

		sprintf(buffer, m_displayMask.c_str(), m_left);
		m_line->GetLeftText()->SetText(buffer);

		sprintf(buffer, m_displayMask.c_str(), m_right);
		m_line->GetRightText()->SetText(buffer);
	}
}
