#include "AnimationConic.h"
#include "Common.h"
#include "MainActor.h"
#include "Simmulation\SolarSystem.h"

const simmulation::Conic & AnimationConic::GetConic()
{
	return m_conic;
}

spAnimationConic AnimationConic::Create(const simmulation::Conic & conic, const oxygine::Color & color, 
	primitive::DrawType type)
{
	switch (conic.GetType())
	{
	case simmulation::Conic::ellipse:
		return new AnimationEllipse(conic.GetEllipse(), color, type);
	case simmulation::Conic::parabola:
		return new AnimationParabola(conic.GetParabola(), color, type);
	case simmulation::Conic::hyperbola:
		return new AnimationHyperbola(conic.GetHyperbola(), color, type);
	default:
		return nullptr;
	}
}

primitive::spCircle CreatePoint(const oxygine::VectorD2 & position, const oxygine::Color & color, oxygine::Actor * parent)
{
	primitive::spCircle point = new primitive::Circle(2.0f);

	point->SetFillColor(color);
	point->setPosition(position);

	parent->addChild(point);
	point->SetNotZoomable(true);

	return point;
};

primitive::spLine CreateLine(double length, double angle, const oxygine::VectorD2 & position, const oxygine::Color & color, oxygine::Actor * parent)
{
	primitive::spLine line = new primitive::Line((float)length);

	line->setRotation((float)angle);
	line->SetLineColor(color);

	parent->addChild(line);

	return line;
}

AnimationEllipse::AnimationEllipse(const simmulation::Ellipse & ellipse, const oxygine::Color & color, primitive::DrawType type)
{
	simmulation::Ellipse ellipseCopy = ellipse;

	setRotation((float)ellipseCopy.angle);
	setPosition(oxygine::Vector2{ (float)ellipseCopy.position.x, (float)ellipseCopy.position.y });

	m_ellipse = new primitive::Ellipse((float)ellipseCopy.radius.x, (float)ellipseCopy.radius.y, type);
	m_ellipse->SetLineColor(color);
	addChild(m_ellipse);

	//ellipseCopy.position = { 0.0, 0.0 };
	//ellipseCopy.angle = 0.0;

	//m_majorAxis = CreateLine(ellipseCopy.radius.x * 2.0f, 0.0, ellipseCopy.position, color, this);
	//m_majorAxis->SetLineColor({ color.r, color.g, color.b, 30 });
	//m_minorAxis = CreateLine(ellipseCopy.radius.y * 2.0f, fromDegToRad(90.0), ellipseCopy.position, color, this);
	//m_minorAxis->SetLineColor({ color.r, color.g, color.b, 30 });

	//auto majorAxisData = ellipseCopy.GetMajorAxis();
	//createPoint(majorAxisData.first);
	//createPoint(majorAxisData.second);

	//auto minorAxisData = ellipseCopy.GetMinorAxis();
	//createPoint(minorAxisData.first);
	//createPoint(minorAxisData.second);

	//auto fociData = ellipseCopy.GetFoci();
	//m_focus1 = CreatePoint(fociData.first, color, this);
	//m_focus1->SetFillColor({ color.r, color.g, color.b, 30 });
	//m_focus2 = CreatePoint(fociData.second, color, this);
	//m_focus2->SetFillColor({ color.r, color.g, color.b, 30 });
}

AnimationHyperbola::AnimationHyperbola(const simmulation::Hyperbola & hyperbola, const oxygine::Color & color, primitive::DrawType type)
{
	simmulation::Hyperbola hyperbolaCopy = hyperbola;

	setRotation((float)hyperbolaCopy.angle);
	setPosition(oxygine::Vector2{ (float)hyperbolaCopy.position.x, (float)hyperbolaCopy.position.y });

	m_hyperbola = new primitive::Hyperbola((float)hyperbolaCopy.radius.x, (float)hyperbolaCopy.radius.y, type);
	m_hyperbola->SetLineColor(color);
	
	addChild(m_hyperbola);

	//hyperbolaCopy.position = { 0.0, 0.0 };
	//hyperbolaCopy.angle = 0.0;

	//auto fociData = hyperbolaCopy.GetFoci();
	//m_focus1 = CreatePoint(fociData.first, color, this);
	//m_focus2 = CreatePoint(fociData.second, color, this);

	//m_axis1 = new primitive::Line(2000.0f);
	//m_axis1->SetLineColor(color);
	//m_axis1->setRotation(atan(hyperbola.radius.y/hyperbola.radius.x));
	//addChild(m_axis1);

	//m_axis2 = new primitive::Line(2000.0f);
	//m_axis2->SetLineColor(color);
	//m_axis2->setRotation(atan(-hyperbola.radius.y/hyperbola.radius.x));
	//addChild(m_axis2);
}

AnimationParabola::AnimationParabola(const simmulation::Parabola & parabola, const oxygine::Color & color, primitive::DrawType type)
{

}
