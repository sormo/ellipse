#include "Primitive.h"
#include "PrimitiveDraw.h"
#include "nanovg.h"
#include "Material.h"
#include "MainActor.h"

extern NVGcontext * g_nanovgContext;

namespace primitive
{
	Primitive::Primitive(DrawType type)
		: m_lineColor(0), m_fillColor(0), m_drawType(type)
	{
	}

	Primitive::~Primitive()
	{
		if (m_flags[(size_t)Flags::notZoomable] && g_main)
			g_main->GetCamera().UnregisterOnScaleChange(this);
	}

	void Primitive::SetLineColor(const oxygine::Color & color)
	{
		m_lineColor = color;
		for (auto p : m_aggregatedPrimitives)
			p->SetLineColor(color);
	}

	const oxygine::Color & Primitive::GetLineColor()
	{
		return m_lineColor;
	}

	void Primitive::SetFillColor(const oxygine::Color & color)
	{
		m_fillColor = color;
		for (auto p : m_aggregatedPrimitives)
			p->SetFillColor(color);
	}

	const oxygine::Color & Primitive::GetFillColor()
	{
		return m_fillColor;
	}

	void Primitive::SetLineWidth(float width)
	{
		m_lineWidth = width;
		for (auto p : m_aggregatedPrimitives)
			p->SetLineWidth(width);
	}

	float Primitive::GetLineWidth()
	{
		return m_lineWidth;
	}

	void Primitive::doRender(const oxygine::RenderState& rs)
	{
		if (m_drawType == DrawType::oxygine)
		{
			PrimitiveDraw::Instance().DrawPrimitives(rs.transform, rs.alpha, m_points, m_lineColor, m_fillColor, m_lineWidth, m_lineType);
			return;
		}

		oxygine::Material::setCurrent(0);

		nvgSave(g_nanovgContext);
		//nvgBeginFrame(g_nanovgContext, (int)oxygine::getStage()->getWidth(), (int)oxygine::getStage()->getHeight(), 1.0);
		nvgResetTransform(g_nanovgContext);
		nvgTransform(g_nanovgContext, rs.transform.a, rs.transform.b, rs.transform.c, rs.transform.d, rs.transform.x, rs.transform.y);

		nvgBeginPath(g_nanovgContext);

		Draw();

		if (m_fillColor.argb != 0)
		{
			nvgFillColor(g_nanovgContext, nvgRGBA(m_fillColor.r, m_fillColor.g, m_fillColor.b, m_fillColor.a));
			nvgFill(g_nanovgContext);
		}

		if (m_lineColor.argb != 0)
		{
			float width = m_lineWidth;
			if (m_flags[(size_t)Flags::onMainActor] && !m_flags[(size_t)Flags::notZoomable])
				width = m_lineWidth / g_main->GetCamera().GetScaleCamera();
			nvgStrokeWidth(g_nanovgContext, width);

			nvgStrokeColor(g_nanovgContext, nvgRGBA(m_lineColor.r, m_lineColor.g, m_lineColor.b, m_lineColor.a));
			nvgStroke(g_nanovgContext);
		}

		//nvgEndFrame(g_nanovgContext);
		nvgRestore(g_nanovgContext);
	}

	void Primitive::AddAggregatedPrimitive(Primitive * p)
	{
		auto it = std::find(std::begin(m_aggregatedPrimitives), std::end(m_aggregatedPrimitives), p);
		if (it != std::end(m_aggregatedPrimitives))
			return;
		m_aggregatedPrimitives.push_back(p);

		p->SetLineColor(GetLineColor());
		p->SetFillColor(GetFillColor());
		p->SetLineWidth(GetLineWidth());

		p->ApplyAndPropagateNotZoomable(m_flags[(size_t)Flags::notZoomable]);
	}

	void Primitive::RemoveAggregatedPrimitive(Primitive * p)
	{
		auto it = std::find(std::begin(m_aggregatedPrimitives), std::end(m_aggregatedPrimitives), p);
		if (it == std::end(m_aggregatedPrimitives))
			return;
		m_aggregatedPrimitives.erase(it);
	}

	void Primitive::SetNotZoomable(bool set)
	{
		// user requested not zoomable state
		m_flags[(size_t)Flags::forceNotZoomable] = set;

		if (m_flags[(size_t)Flags::notZoomable] == set)
			return;

		m_flags[(size_t)Flags::notZoomable] = set;

		if (set)
			g_main->GetCamera().RegisterOnScaleChange(this, [this](float s) { setScale(1.0f/s); });
		else
			g_main->GetCamera().UnregisterOnScaleChange(this);

		for (auto c : m_aggregatedPrimitives)
			c->ApplyAndPropagateNotZoomable(set);
	}

	void Primitive::ApplyAndPropagateNotZoomable(bool set)
	{
		// only parent can be registered on scale change
		if (m_flags[(size_t)Flags::notZoomable])
			g_main->GetCamera().UnregisterOnScaleChange(this);

		m_flags[(size_t)Flags::notZoomable] = set;

		// if not zoomable is forced, prevent following invalid state
		if (m_flags[(size_t)Flags::forceNotZoomable] && !m_flags[(size_t)Flags::notZoomable])
		{
			SetNotZoomable(true);
			return;
		}

		for (auto c : m_aggregatedPrimitives)
			c->ApplyAndPropagateNotZoomable(set);
	}

	void Primitive::onAdded2Stage()
	{
		oxygine::Actor * parent = getParent();
		oxygine::Actor * mainActorParent = dynamic_cast<oxygine::Actor*>(g_main.get());
		
		while (parent && parent != mainActorParent)
			parent = parent->getParent();

		m_flags[(size_t)Flags::onMainActor] = parent == mainActorParent;
	}
}
