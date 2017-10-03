#include "Button.h"
#include "Common.h"

#define DEFAULT_BUTTON_SIZE 75.0f

namespace gui
{
	Button::Button(const char * sprite, bool clickable, std::function<void(bool)> callback, bool checked)
		: m_clickable(clickable), m_clicked(checked), m_callback(callback)
	{
		setName(sprite);
		m_resAnim = g_gameResources.getResAnim(sprite);
		Update();
		setSize({ DEFAULT_BUTTON_SIZE, DEFAULT_BUTTON_SIZE });
		setExtendedClickArea(5);

		addEventListener(oxygine::TouchEvent::CLICK, CLOSURE(this, &Button::OnClick));
		addEventListener(oxygine::TouchEvent::TOUCH_DOWN, [](oxygine::Event * event) { event->stopsImmediatePropagation = true; });
		addEventListener(oxygine::TouchEvent::MOVE, [](oxygine::Event * event) { event->stopsImmediatePropagation = true; });
		addEventListener(oxygine::TouchEvent::TOUCH_UP, [](oxygine::Event * event) { event->stopsImmediatePropagation = true; });
	}

	void Button::OnClick(oxygine::Event * event)
	{
		oxygine::log::messageln("button click");
		event->stopsImmediatePropagation = true;
		if (m_clickable)
		{
			m_clicked = !m_clicked;
			Update();
		}
		m_callback(m_clicked);
	}

	void Button::Update()
	{
		setResAnim(m_resAnim, m_clicked ? 1 : 0, 0);
	}

	bool Button::IsClicked()
	{
		return m_clicked;
	}
}
