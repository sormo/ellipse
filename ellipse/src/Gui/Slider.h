#pragma once
#include <oxygine-framework.h>
#include "Common.h"
#include "Primitives\Line.h"
#include "Primitives\Circle.h"

namespace gui
{
	class Slider : public oxygine::Actor
	{
	public:
		static const float SLIDER_RADIUS;

		Slider(float size, double left, double current, double right, std::function<void(double)> onChange);

		void SetDisplayMask(const char * mask);
		void SetValue(double current);

	private:
		double m_left;
		double m_right;
		double m_current;

		void UpdateCircleAccordingToCurrent();
		void UpdateCurrentAccordingToCircle();

		std::function<void(double)> m_onChange;

		void OnTouchDown(oxygine::Event* event);
		void OnTouchMove(oxygine::Event* event);
		void OnTouchUp(oxygine::Event* event);

		oxygine::pointer_index m_currentTouch = INVALID_TOUCH;

		primitive::spDirectedTextLine m_line;
		primitive::spCircle m_circle;

		std::string m_displayMask;
	};

	typedef oxygine::intrusive_ptr<Slider> spSlider;
}
