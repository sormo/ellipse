#pragma once
#include "oxygine-framework.h"
#include "Primitive.h"

namespace primitive
{
	DECLARE_SMART(Circle, spCircle);
	class Circle : public Primitive
	{
	public:
		Circle(float radius, DrawType type = DrawType::nanovg);
		virtual ~Circle() {}
		float GetRadius();
		void SetRadius(float radius);
	private:
		void Rebuild();
		virtual void Draw() override;

		float m_radius;
	};
}
