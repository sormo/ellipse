#include "Text.h"
#include "Common.h"
#include "nanovg.h"

extern NVGcontext * g_nanovgContext;

namespace primitive
{
	Text::Text(const char * text, DrawType type)
		: Primitive(type)
	{
		m_text = new oxygine::TextField();
		m_text->setFont(g_gameResources.getResFont("main"));
		m_text->setText(text);

		if (type == DrawType::oxygine)
		{
			addChild(m_text);
		}

		m_input = new oxygine::InputText;
		m_input->setAllowedSymbols("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-_ ");
		m_input->addEventListener(oxygine::Event::COMPLETE, CLOSURE(this, &Text::OnEditComplete));

		m_editButton = new oxygine::ColorRectSprite;
		m_editButton->setSize(GetTextSize());
		m_editButton->setColor(oxygine::Color(oxygine::Color::Red, 40));
		m_editButton->setSize(GetTextSize());
	}

	oxygine::Vector2 Text::GetTextSize()
	{
		if (m_drawType == DrawType::oxygine)
		{
			auto rect = m_text->getTextRect();
			return rect.getSize();
		}

		float bound[4];

		float fontSize = (float)GetFontSize();

		nvgSave(g_nanovgContext);
		nvgFontSize(g_nanovgContext, fontSize);
		nvgFontFace(g_nanovgContext, "main-font");
		nvgTextBoxBounds(g_nanovgContext, m_text->getPosition().x, m_text->getPosition().y + fontSize, std::numeric_limits<float>::max(),
			m_text->getText().c_str(), nullptr, bound);
		nvgRestore(g_nanovgContext);

		return {bound[2], bound[3]};
	}

	void Text::Draw()
	{
		if (m_drawType == DrawType::oxygine)
			return;
		
		float fontSize = (float)GetFontSize();

		nvgFontSize(g_nanovgContext, fontSize);
		nvgFontFace(g_nanovgContext, "main-font");
		//nvgTextAlign(g_nanovgContext, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);

		//nvgFontBlur(g_nanovgContext, 2);
		nvgFillColor(g_nanovgContext, nvgRGBA(m_text->getColor().r, m_text->getColor().g, m_text->getColor().b, m_text->getColor().a));

		//nvgText(g_nanovgContext, m_text->getPosition().x, m_text->getPosition().y + fontSize, m_text->getText().c_str(), nullptr);
		nvgTextBox(g_nanovgContext, m_text->getPosition().x, m_text->getPosition().y + fontSize, std::numeric_limits<float>::max(), 
			m_text->getText().c_str(), nullptr);

	}

	void Text::SetText(const char * text)
	{
		m_text->setText(text);
	}

	const std::string & Text::GetText()
	{
		return m_text->getText();
	}

	void Text::SetColor(const oxygine::Color & color)
	{
		m_text->setColor(color);
	}

	const oxygine::Color & Text::GetColor()
	{
		return m_text->getColor();
	}

	void Text::EnableEditing(std::function<bool(const std::string&)> validate)
	{
		removeAllEventListeners();

		m_validate = validate;

		if (m_validate)
		{
			addChild(m_editButton);

			m_editButton->addEventListener(oxygine::TouchEvent::CLICK, CLOSURE(this, &Text::OnClick));
			m_editButton->addEventListener(oxygine::TouchEvent::TOUCH_DOWN, [](oxygine::Event * event) { event->stopsImmediatePropagation = true; });
			m_editButton->addEventListener(oxygine::TouchEvent::MOVE, [](oxygine::Event * event) { event->stopsImmediatePropagation = true; });
			m_editButton->addEventListener(oxygine::TouchEvent::TOUCH_UP, [](oxygine::Event * event) { event->stopsImmediatePropagation = true; });
		}
	}

	void Text::OnEditComplete(oxygine::Event* ev)
	{
		if (m_validate(m_text->getText()))
		{
			oxygine::InputText::stopAnyInput();
			m_text->setColor(oxygine::Color::White);

			m_editButton->setSize(GetTextSize());
		}
		else
		{
			m_input->start(m_text);
		}
	}

	void Text::OnClick(oxygine::Event * event)
	{
		m_text->setColor(oxygine::Color::Red);
		m_input->start(m_text);
	}

	void Text::SetFontSize(int size)
	{
		m_text->setFontSize(size);
		m_editButton->setSize(GetTextSize());
	}

	int Text::GetFontSize()
	{
		int size = m_text->getFontSize();
		return size == 0 ? m_text->getFont()->getSize() : size;
	}
}
