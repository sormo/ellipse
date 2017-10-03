#pragma once
#include <oxygine-framework.h>

namespace gui
{
	class Button : public oxygine::Sprite
	{
	public:
		Button(const char * sprite, bool clickable, std::function<void(bool)> callback, bool checked = false);
		bool IsClicked();

	private:
		void OnClick(oxygine::Event * event);
		bool m_clickable;
		bool m_clicked;
		std::function<void(bool)> m_callback;
		const oxygine::ResAnim * m_resAnim;

		void Update();
	};

	typedef oxygine::intrusive_ptr<Button> spButton;
}