#pragma once
#include "oxygine-framework.h"
#include "Primitives\Ellipse.h"
#include "Primitives\Line.h"
#include "Primitives\Circle.h"
#include "Primitives\Hyperbola.h"
#include "Simmulation\SolarSystem.h"

// TODO possibly remove line and points from here and put it directly to primitives

DECLARE_SMART(AnimationConic, spAnimationConic);
class AnimationConic : public oxygine::Actor
{
public:
	const simmulation::Conic & GetConic();
	static spAnimationConic Create(const simmulation::Conic & conic, const oxygine::Color & color, 
		primitive::DrawType type);
private:
	simmulation::Conic m_conic;
};

class AnimationEllipse : public AnimationConic
{
public:
	AnimationEllipse(const simmulation::Ellipse & ellipse, const oxygine::Color & color,
		primitive::DrawType type);
private:
	primitive::spEllipse m_ellipse;
	primitive::spLine m_majorAxis;
	primitive::spLine m_minorAxis;
	primitive::spCircle m_focus1;
	primitive::spCircle m_focus2;
};

class AnimationHyperbola : public AnimationConic
{
public:
	AnimationHyperbola(const simmulation::Hyperbola & hyperbola, const oxygine::Color & color,
		primitive::DrawType type);
private:
	primitive::spHyperbola m_hyperbola;
	primitive::spLine m_axis1;
	primitive::spLine m_axis2;
	primitive::spCircle m_focus1;
	primitive::spCircle m_focus2;
};

class AnimationParabola : public AnimationConic
{
public:
	AnimationParabola(const simmulation::Parabola & parabola, const oxygine::Color & color,
		primitive::DrawType type);
};
