#pragma once
#include "oxygine-framework.h"
#include "Primitive.h"
#include "Text.h"

namespace primitive
{
	DECLARE_SMART(Line, spLine);
	class Line : public Primitive
	{
	public:
		// horizontal line from -length/2 to +length/2
		Line(float length, DrawType type = DrawType::nanovg);
		// line from p1 to p2
		Line(const oxygine::Vector2 & p1, const oxygine::Vector2 & p2, DrawType type = DrawType::nanovg);

		virtual void Set(const oxygine::Vector2 & p1, const oxygine::Vector2 & p2);

		const oxygine::Vector2 & GetLeftPoint();
		const oxygine::Vector2 & GetRightPoint();

		virtual void Draw() override;
	};

	DECLARE_SMART(DirectedLine, spDirectedLine);
	class DirectedLine : public Line
	{
	public:
		DirectedLine(float length, DrawType type = DrawType::nanovg);
		DirectedLine(const oxygine::Vector2 & p1, const oxygine::Vector2 & p2, DrawType type = DrawType::nanovg);
		~DirectedLine();
		virtual void Set(const oxygine::Vector2 & p1, const oxygine::Vector2 & p2) override;

		void SetLeftArrow(bool set);
		void SetRightArrow(bool set);

	private:
		DirectedLine(DrawType type);

		void InitArrows();

		DECLARE_SMART(Arrow, spArrow);
		class Arrow : public Primitive
		{
		public:
			Arrow(DrawType type);
			~Arrow();
		private:
			spLine m_left;
			spLine m_right;
		};
		
		void UpdateArrows();
		spArrow m_leftArrow;
		spArrow m_rightArrow;
	};

	DECLARE_SMART(DirectedTextLine, spDirectedTextLine);
	class DirectedTextLine : public DirectedLine
	{
	public:
		DirectedTextLine(float length, DrawType type = DrawType::nanovg);
		DirectedTextLine(const oxygine::Vector2 & p1, const oxygine::Vector2 & p2, DrawType type = DrawType::nanovg);

		virtual void Set(const oxygine::Vector2 & p1, const oxygine::Vector2 & p2) override;

		spText & GetLeftText();
		spText & GetCenterText();
		spText & GetRightText();

	private:

		spText m_left;
		spText m_center;
		spText m_right;
	};
}
