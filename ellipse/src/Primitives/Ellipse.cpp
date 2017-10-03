#include "Ellipse.h"
#include <cmath>
#include "Common.h"
#include "nanovg.h"

extern NVGcontext * g_nanovgContext;
#define ELLIPSE_POINTS 80

namespace primitive
{
	float ComputeDpt(float a, float b, float theta)
	{
		float dpt_sin = pow(a*sin(theta), 2.0f);
		float dpt_cos = pow(b*cos(theta), 2.0f);

		return sqrt(dpt_sin + dpt_cos);
	}

	std::vector<oxygine::Vector2> GeneratePoints(float a, float b)
	{
		std::vector<oxygine::Vector2> points;
		float theta = 0.0f;
		float twoPi = (float)PI*2.0f;
		float deltaTheta = 0.0001f;
		float numIntegrals = round(twoPi / deltaTheta);
		float circ = 0.0f;
		float dpt = 0.0f;

		/* integrate over the elipse to get the circumference */
		for (int i = 0; i < numIntegrals; i++)
		{
			theta += i*deltaTheta;
			dpt = ComputeDpt(a, b, theta);
			circ += dpt;
		}

		int nextPoint = 0;
		float run = 0.0f;

		theta = 0.0f;

		for (int i = 0; i < numIntegrals; i++)
		{
			theta += deltaTheta;
			float subIntegral = ELLIPSE_POINTS*run / circ;
			if ((int)subIntegral >= nextPoint)
			{
				float x = a * cos(theta);
				float y = b * sin(theta);
				points.push_back({ x, y });
				nextPoint++;
			}
			run += ComputeDpt(a, b, theta);
		}

		return points;
	}

	Ellipse::Ellipse(float a, float b, DrawType type)
		: Primitive(type), m_a(a), m_b(b)
	{
		if (type == DrawType::oxygine)
		{
			m_points = GeneratePoints(a, b);
			m_lineType = PrimitiveDraw::LineType::closed;
		}
	}

	void Ellipse::Draw()
	{
		nvgEllipse(g_nanovgContext, 0, 0, m_a, m_b);
	}


}
