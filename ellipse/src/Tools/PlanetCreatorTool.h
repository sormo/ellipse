#pragma once
#include <oxygine-framework.h>
#include "Primitives\Line.h"
#include "Primitives\Circle.h"
#include "Gui\Slider.h"
#include "Gui\Button.h"
#include "Animation\AnimationConic.h"
#include "Animation\AnimationSystem.h"

class PlanetCreatorTool
{
public:
	PlanetCreatorTool(spAnimationSystem & system);

	gui::spButton GetButton();
	void Update();

private:
	void OnTouchDown(oxygine::Event* event);
	void OnTouchMove(oxygine::Event* event);
	void OnTouchUp(oxygine::Event* event);

	void RemoveFromActor();

	gui::spButton m_button;
	gui::spSlider m_slider;
	void CreateSlider(double minWeight, double currentWeight, double maxWeight);
	void RemoveSlider();

	double m_weight;
	void UpdateWeight();

	oxygine::pointer_index m_currentTouch = INVALID_TOUCH;

	oxygine::Vector2 m_stagePosition;
	oxygine::VectorD2 m_velocity;

	spAnimationConic m_conicActor;
	simmulation::Conic m_conic;

	primitive::spDirectedTextLine m_velocityLine;
	primitive::spDirectedTextLine m_parentLine;
	primitive::spCircle m_planet;
	primitive::spText m_name;

	spAnimationSystem m_system;

	const simmulation::CelestialBody * GetParent();

	static bool ValidatePlanetName(const std::string & name);
	static std::string GeneratePlanetName();
};
