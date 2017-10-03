#include "Line.h"
#include "Common.h"
#include "nanovg.h"

extern NVGcontext * g_nanovgContext;

namespace primitive
{
	Line::Line(float length, DrawType type)
		: Primitive(type)
	{
		m_points.push_back({-length/2.0f, 0.0f});
		m_points.push_back({ length/2.0f, 0.0f});
	}

	Line::Line(const oxygine::Vector2 & p1, const oxygine::Vector2 & p2, DrawType type)
		: Primitive(type)
	{
		m_points.push_back(p1);
		m_points.push_back(p2);
	}

	void Line::Set(const oxygine::Vector2 & p1, const oxygine::Vector2 & p2)
	{
		m_points[0] = p1;
		m_points[1] = p2;
	}

	const oxygine::Vector2 & Line::GetLeftPoint()
	{
		return m_points[0];
	}

	const oxygine::Vector2 & Line::GetRightPoint()
	{
		return m_points[1];
	}

	void Line::Draw()
	{
		nvgMoveTo(g_nanovgContext, m_points[0].x, m_points[0].y);
		nvgLineTo(g_nanovgContext, m_points[1].x, m_points[1].y);
	}

	// directed line

	void DirectedLine::InitArrows()
	{
		m_leftArrow = new Arrow(m_drawType);
		m_rightArrow = new Arrow(m_drawType);
		AddAggregatedPrimitive(m_leftArrow.get());
		AddAggregatedPrimitive(m_rightArrow.get());
		addChild(m_leftArrow);
		addChild(m_rightArrow);

		m_leftArrow->setVisible(false);
		m_rightArrow->setVisible(false);

		m_leftArrow->SetNotZoomable(true);
		m_rightArrow->SetNotZoomable(true);
	}

	DirectedLine::DirectedLine(float length, DrawType type)
		: Line(length, type)
	{
		InitArrows();
		UpdateArrows();
	}

	DirectedLine::DirectedLine(const oxygine::Vector2 & p1, const oxygine::Vector2 & p2, DrawType type)
		: Line(p1, p2, type)
	{
		InitArrows();
		UpdateArrows();
	}

	void DirectedLine::Set(const oxygine::Vector2 & p1, const oxygine::Vector2 & p2)
	{
		Line::Set(p1, p2);

		UpdateArrows();
	}

	DirectedLine::~DirectedLine()
	{
		RemoveAggregatedPrimitive(m_leftArrow.get());
		RemoveAggregatedPrimitive(m_rightArrow.get());
	}

	DirectedLine::Arrow::Arrow(DrawType type)
		: Primitive(type)
	{
		static const float ARROW_LENGHT = 5.0f;

		m_left = new primitive::Line({ 0.0f, 0.0f }, rotate({ ARROW_LENGHT, 0.0f }, fromDegToRad(150.0f)), m_drawType);
		m_right = new primitive::Line({ 0.0f, 0.0f }, rotate({ ARROW_LENGHT, 0.0f }, fromDegToRad(210.0f)), m_drawType);
		addChild(m_left);
		addChild(m_right);
		AddAggregatedPrimitive(m_left.get());
		AddAggregatedPrimitive(m_right.get());
	}
	DirectedLine::Arrow::~Arrow()
	{
		RemoveAggregatedPrimitive(m_left.get());
		RemoveAggregatedPrimitive(m_right.get());
	}

	void DirectedLine::SetLeftArrow(bool set)
	{
		m_leftArrow->setVisible(set);
	}

	void DirectedLine::SetRightArrow(bool set)
	{
		m_rightArrow->setVisible(set);
	}

	void DirectedLine::UpdateArrows()
	{
		float angle = getAngle(m_points[1] - m_points[0]);
		
		m_leftArrow->setRotation(angle + (float)PI);
		m_leftArrow->setPosition(m_points[0]);

		m_rightArrow->setRotation(angle);
		m_rightArrow->setPosition(m_points[1]);
	}

	// --- directed text line ---

	DirectedTextLine::DirectedTextLine(float length, DrawType type)
		: DirectedLine(length, type)
	{

	}

	DirectedTextLine::DirectedTextLine(const oxygine::Vector2 & p1, const oxygine::Vector2 & p2, DrawType type)
		: DirectedLine(p1, p2, type)
	{

	}

	spText & DirectedTextLine::GetLeftText()
	{
		if (!m_left)
		{
			m_left = new Text("");
			m_left->setPosition(m_points[0]);
			addChild(m_left);
		}
		return m_left;
	}

	spText & DirectedTextLine::GetCenterText()
	{
		if (!m_center)
		{
			m_center = new Text("");
			m_center->setPosition(m_points[0] + (m_points[1] - m_points[0]) / 2.0f);
			addChild(m_center);
		}
		return m_center;
	}

	spText & DirectedTextLine::GetRightText()
	{
		if (!m_right)
		{
			m_right = new Text("");
			m_right->setPosition(m_points[1]);
			addChild(m_right);
		}
		return m_right;
	}

	void DirectedTextLine::Set(const oxygine::Vector2 & p1, const oxygine::Vector2 & p2)
	{
		DirectedLine::Set(p1, p2);
		if (m_left)
			m_left->setPosition(m_points[0]);
		if (m_center)
			m_center->setPosition(m_points[0] + (m_points[1] - m_points[0]) / 2.0f);
		if (m_right)
			m_right->setPosition(m_points[1]);
	}
}
