#include "Polyline.h"
#include "nanovg.h"

extern NVGcontext * g_nanovgContext;

namespace primitive
{
	Polyline::Polyline(const std::vector<oxygine::Vector2> & points, DrawType type)
		: Primitive(type)
	{
		m_points = points;
		m_lineType = PrimitiveDraw::LineType::opened;
	}

	Polyline::Polyline(std::vector<oxygine::Vector2> && points, DrawType type)
		: Primitive(type)
	{
		m_points = std::move(points);
		m_lineType = PrimitiveDraw::LineType::opened;
	}

	void Polyline::Draw()
	{
		nvgMoveTo(g_nanovgContext, m_points[0].x, m_points[0].y);
		for (size_t i = 1; i < m_points.size(); ++i)
			nvgLineTo(g_nanovgContext, m_points[i].x, m_points[i].y);
	}
}
