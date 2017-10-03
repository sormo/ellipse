#pragma once
#include "oxygine-framework.h"
#include "Primitive.h"

namespace primitive
{
	DECLARE_SMART(Polyline, spPolyline);
	class Polyline : public Primitive
	{
	public:
		Polyline(const std::vector<oxygine::Vector2> & points, DrawType type = DrawType::oxygine);
		Polyline(std::vector<oxygine::Vector2> && points, DrawType type = DrawType::oxygine);
	private:
		virtual void Draw() override;
	};
}
