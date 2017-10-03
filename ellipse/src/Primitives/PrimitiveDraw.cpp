#include "PrimitiveDraw.h"
#include <atomic>
#include "core/gl/VideoDriverGLES20.h"
#include "core/gl/ShaderProgramGL.h"
#include "Material.h"
#include "STDMaterial.h"

const char * VERTEX_SHADER_DATA = "\
									uniform mediump mat4 projection;\
									attribute vec2 a_position;\
									void main() {\
									vec4 position = vec4(a_position, 0.0, 1.0);\
									gl_Position = projection * position;\
									}\
									";

const char * FRAGMENT_SHADER_DATA = "\
									  uniform mediump vec4 color;\
									  void main() { \
									  gl_FragColor = color; \
									  } \
									  ";

PrimitiveDraw::PrimitiveDraw()
{
	int vs = oxygine::ShaderProgramGL::createShader(GL_VERTEX_SHADER, VERTEX_SHADER_DATA, 0, 0);
	int fs = oxygine::ShaderProgramGL::createShader(GL_FRAGMENT_SHADER, FRAGMENT_SHADER_DATA, 0, 0);

	int pr = oxygine::ShaderProgramGL::createProgram(vs, fs, 
		(oxygine::VertexDeclarationGL*)oxygine::IVideoDriver::instance->getVertexDeclaration(oxygine::VERTEX_POSITION));
	m_shaderProgram = new oxygine::ShaderProgramGL(pr);
}

PrimitiveDraw::~PrimitiveDraw()
{
	if (m_shaderProgram)
		delete m_shaderProgram;
}

void PrimitiveDraw::DrawPrimitives(const oxygine::Transform& tr, unsigned char alpha, const std::vector<oxygine::Vector2> & points,
	const oxygine::Color & lineColor, const oxygine::Color & fillColor,	float lineWidth, LineType lineType)
{
	oxygine::Material::setCurrent(0);

	oxygine::IVideoDriver* driver = oxygine::IVideoDriver::instance;
	driver->setShaderProgram(m_shaderProgram);

	oxygine::Matrix m = oxygine::Matrix(tr) * oxygine::STDMaterial::instance->getRenderer()->getViewProjection();
	driver->setUniform("projection", &m);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glLineWidth(lineWidth);

	// ---

	oxglEnableVertexAttribArray(0);
	oxglVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (GLfloat*)points.data());

	if (fillColor != oxygine::Color::Zero)
	{
		oxygine::Vector4 c = fillColor.toVector();
		c.w = alpha/255.0f - 1.0f + c.w;

		oxygine::IVideoDriver::instance->setUniform("color", &c, 1);
		glDrawArrays(GL_TRIANGLE_FAN, 0, points.size());
	}

	if (lineColor != oxygine::Color::Zero)
	{
		oxygine::Vector4 c = lineColor.toVector();
		c.w = alpha / 255.0f - 1.0f + c.w;

		oxygine::IVideoDriver::instance->setUniform("color", &c, 1);

		GLenum mode = GL_LINE_LOOP;
		if (lineType == LineType::opened)
			mode = GL_LINE_STRIP;

		glDrawArrays(mode, 0, points.size());
	}

	oxglDisableVertexAttribArray(0);
}

PrimitiveDraw & PrimitiveDraw::Instance()
{
	static PrimitiveDraw instance;
	return instance;
}
