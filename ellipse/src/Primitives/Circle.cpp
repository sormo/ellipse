#include "Circle.h"
#include "Common.h"
#include "nanovg.h"

extern NVGcontext * g_nanovgContext;

#define CIRCLE_POINTS 80

namespace primitive
{
	Circle::Circle(float radius, DrawType type)
		: Primitive(type), m_radius(radius)
	{
		if (type == DrawType::oxygine)
		{
			m_lineType = PrimitiveDraw::LineType::closed;
			Rebuild();
		}
	}

	float Circle::GetRadius()
	{
		return m_radius;
	}

	void Circle::SetRadius(float radius)
	{
		m_radius = radius;
		if (m_drawType == DrawType::oxygine)
			Rebuild();
	}

	void Circle::Draw()
	{
		nvgCircle(g_nanovgContext, 0.0f, 0.0f, m_radius);
	}

	void Circle::Rebuild()
	{
		m_points.clear();

		// TODO number of points according to radius
		for (float a = 0.0f; a < 2.0f*(float)PI; a = a + (2.0f*(float)PI) / (float)CIRCLE_POINTS)
			m_points.push_back({ m_radius * (float)cos(a) , m_radius * (float)sin(a) });

	}
}
