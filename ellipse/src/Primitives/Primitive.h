#pragma once
#include "oxygine-framework.h"
#include "PrimitiveDraw.h"
#include <bitset>

namespace primitive
{
	enum class DrawType { oxygine, nanovg };

	// TODO remove inheritance from actor ???
	class Primitive : public oxygine::Actor
	{
	public:
		Primitive(DrawType type);
		virtual ~Primitive();

		void SetLineColor(const oxygine::Color & color);
		const oxygine::Color & GetLineColor();

		void SetFillColor(const oxygine::Color & color);
		const oxygine::Color & GetFillColor();

		void SetLineWidth(float width);
		float GetLineWidth();

		void SetNotZoomable(bool set);

	protected:
		void ApplyAndPropagateNotZoomable(bool set);

		virtual void Draw() {};

		virtual void doRender(const oxygine::RenderState& rs) override;
		virtual void onAdded2Stage() override;

		PrimitiveDraw::LineType m_lineType;
		oxygine::Color m_lineColor;
		oxygine::Color m_fillColor;
		float m_lineWidth = 1.0f;

		std::vector<oxygine::Vector2> m_points;

		void AddAggregatedPrimitive(Primitive * p);
		void RemoveAggregatedPrimitive(Primitive * p);

		DrawType m_drawType;

	private:
		std::vector<Primitive*> m_aggregatedPrimitives;

		enum class Flags : size_t
		{
			onMainActor,
			notZoomable,
			forceNotZoomable,

			count
		};
		std::bitset<(size_t)Flags::count> m_flags;
	};
}
