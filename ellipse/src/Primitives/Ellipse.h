#pragma once
#include "oxygine-framework.h"
#include "Primitive.h"
#include "nanovg.h"

namespace primitive
{
	DECLARE_SMART(Ellipse, spEllipse);
	class Ellipse : public Primitive
	{
	public:
		Ellipse(float a, float b, DrawType type = DrawType::nanovg);

	private:
		void Draw() override;

		float m_a;
		float m_b;
	};
}
