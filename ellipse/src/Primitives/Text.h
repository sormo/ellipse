#pragma once
#include "oxygine-framework.h"
#include "Primitive.h"

namespace primitive
{
	DECLARE_SMART(Text, spText);
	class Text : public Primitive
	{
	public:
		Text(const char * text, DrawType type = DrawType::nanovg);

		void SetText(const char * text);
		const std::string & GetText();

		void SetColor(const oxygine::Color & color);
		const oxygine::Color & GetColor();

		void SetFontSize(int size);
		int GetFontSize();

		void EnableEditing(std::function<bool(const std::string&)> validate);

		oxygine::Vector2 GetTextSize();

	private:
		void OnClick(oxygine::Event * event);
		void OnEditComplete(oxygine::Event* ev);
		virtual void Draw() override;

		oxygine::spColorRectSprite m_editButton;
		oxygine::spTextField m_text;
		oxygine::spInputText m_input;

		std::function<bool(const std::string&)> m_validate;
	};
}
