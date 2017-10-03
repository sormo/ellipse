#pragma once
#include <vector>
#include "oxygine-framework.h"

namespace oxygine
{
	class ShaderProgramGL;
}

class PrimitiveDraw
{
public:

	enum class LineType
	{
		closed, // first point connected to last
		opened  // first point is not connected to last
	};

	void DrawPrimitives(const oxygine::Transform& tr, unsigned char alpha, const std::vector<oxygine::Vector2> & points,
		const oxygine::Color & lineColor, const oxygine::Color & fillColor, float lineWidth, LineType lineType);

	static PrimitiveDraw & Instance();

private:
	PrimitiveDraw();
	~PrimitiveDraw();

	oxygine::ShaderProgramGL * m_shaderProgram = nullptr;
};