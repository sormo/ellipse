#include "Hyperbola.h"
#include <cmath>
#include "Common.h"
#include "nanovg.h"

extern NVGcontext * g_nanovgContext;

namespace primitive
{
	Hyperbola::Hyperbola(float a, float b, DrawType type)
		: Primitive(type)
	{
		m_positive = new HyperbolaHalf(a,b, type);
		m_negative = new HyperbolaHalf(-a, -b, type);

		addChild(m_positive);
		addChild(m_negative);

		AddAggregatedPrimitive(m_positive.get());
		AddAggregatedPrimitive(m_negative.get());
	}

	Hyperbola::~Hyperbola()
	{
		RemoveAggregatedPrimitive(m_positive.get());
		RemoveAggregatedPrimitive(m_negative.get());
	}

	void Hyperbola::HidePositive(bool hide)
	{
		m_positive->setVisible(!hide);
	}

	void Hyperbola::HideNegative(bool hide)
	{
		m_negative->setVisible(!hide);
	}

	Hyperbola::HyperbolaHalf::HyperbolaHalf(float a, float b, DrawType type)
		: Primitive(type)
	{
		for (float theta = -40.0f; theta < 40.0f; theta += 0.1f)
		{
			float x = a * cosh(theta);
			float y = b * sinh(theta);
			m_points.push_back({ x, y });
		}
	}

	void Hyperbola::HyperbolaHalf::Draw()
	{
		nvgBeginPath(g_nanovgContext);

		nvgMoveTo(g_nanovgContext, m_points[0].x, m_points[0].y);
		for (size_t i = 1; i < m_points.size(); ++i)
			nvgLineTo(g_nanovgContext, m_points[i].x, m_points[i].y);
	}
}
