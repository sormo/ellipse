#include "PlanetCreatorTool.h"
#include "Common.h"
#include "Gui\Button.h"
#include "Simmulation\SolarSystem.h"
#include "MainActor.h"

PlanetCreatorTool::PlanetCreatorTool(spAnimationSystem & system)
	: m_system(system)
{
	m_button = new gui::Button("debug", true, [this](bool clicked)
	{
		oxygine::log::messageln("debug clicked");
		g_main->GetCamera().EnableMove(!clicked);
		
		if (clicked)
		{
			m_system->Stop();

			static const double minWeight = 1.0e10;
			auto parent = GetParent();
			double maxWeight = scaleKilogramsInv(parent->GetMass()) / 10.0f;
			
			double weightExponent = log10(minWeight) + (log10(maxWeight) - log10(minWeight)) / 2.0;
			
			CreateSlider(log10(minWeight), weightExponent, log10(maxWeight));

			m_weight = pow(10.0, weightExponent);
		}
		else
		{
			m_system->Start();
			RemoveSlider();
			
			if (!m_conicActor)
				return;

			simmulation::g_system->ClearData(1);

			oxygine::Vector2 localPosition = g_main->GetCamera().LocalFromStage(m_stagePosition);
			oxygine::VectorD2 systemPosition(localPosition.x, localPosition.y);
			oxygine::VectorD2 systemVelocity(m_velocity);

			const std::shared_ptr<const simmulation::CelestialBody> * parent = nullptr;
			if (auto tmp = m_system->GetFollowBody())
			{
				parent = &tmp->GetCelestialBody();
				std::lock_guard<const simmulation::CelestialBody> lock(*parent->get());
				systemPosition += parent->get()->GetPositions()[0];
				systemVelocity += parent->get()->GetMomenta()[0] / parent->get()->GetMass();
			}

			double radius = radiusFromMassAndDensity(scaleKilograms(m_weight), EARTH_DENSITY);
			simmulation::CelestialBodyDef def{ m_name->GetText(), systemPosition, systemVelocity, scaleKilograms(m_weight), radius, false };
			auto body = simmulation::g_system->AddBody(def, parent);
			m_system->AddBody(body);

			m_system->Rebuild();

			RemoveFromActor();
		}
	});

	oxygine::getStage()->addEventListener(oxygine::TouchEvent::TOUCH_DOWN, CLOSURE(this, &PlanetCreatorTool::OnTouchDown));
	oxygine::getStage()->addEventListener(oxygine::TouchEvent::MOVE, CLOSURE(this, &PlanetCreatorTool::OnTouchMove));
	oxygine::getStage()->addEventListener(oxygine::TouchEvent::TOUCH_UP, CLOSURE(this, &PlanetCreatorTool::OnTouchUp));
}

void PlanetCreatorTool::RemoveFromActor()
{
	if (m_conicActor)
		g_main->GetWorld()->removeChild(m_conicActor);
	m_conicActor = nullptr;

	if (m_parentLine)
		g_main->GetWorld()->removeChild(m_parentLine);
	m_parentLine = nullptr;

	if (m_velocityLine)
		g_main->GetWorld()->removeChild(m_velocityLine);
	m_velocityLine = nullptr;

	if (m_planet)
		g_main->GetWorld()->removeChild(m_planet);
	m_planet = nullptr;

	if (m_name)
		g_main->GetGui()->removeChild(m_name);
	m_name = nullptr;
}

gui::spButton PlanetCreatorTool::GetButton()
{
	return m_button;
}

void PlanetCreatorTool::Update()
{
	if (!m_button->IsClicked())
		return;

	oxygine::VectorD2 position, radius;
	double angle = 0.0;

	if (m_conic.GetType() == simmulation::Conic::ellipse)
	{
		auto ellipse = m_conic.GetEllipse();
		oxygine::DebugActor::addDebugString("ellipse");
		position = ellipse.position;
		radius = ellipse.radius;
		angle = ellipse.angle;
	}
	else if (m_conic.GetType() == simmulation::Conic::hyperbola)
	{
		auto hyperbola = m_conic.GetHyperbola();
		oxygine::DebugActor::addDebugString("hyperbola");
		position = hyperbola.position;
		radius = hyperbola.radius;
		angle = hyperbola.angle;
	}
	else if (m_conic.GetType() == simmulation::Conic::parabola)
		oxygine::DebugActor::addDebugString("parabola");

	oxygine::DebugActor::addDebugString("%.3f %.3f %.3f %.3f %.3f %.3f",
		m_conic.A, m_conic.B, m_conic.C, m_conic.D, m_conic.E, m_conic.F);
	oxygine::DebugActor::addDebugString("position: %.3f %.3f", position.x, position.y);
	oxygine::DebugActor::addDebugString("radius: %.3f %.3f", radius.x, radius.y);
	oxygine::DebugActor::addDebugString("angle: %.3f", angle);
}

void PlanetCreatorTool::OnTouchDown(oxygine::Event* event)
{
	if (!m_button->IsClicked() || m_currentTouch != INVALID_TOUCH)
		return;
	
	oxygine::TouchEvent* touch = oxygine::safeCast<oxygine::TouchEvent*>(event);
	m_currentTouch = touch->index;
	m_stagePosition = touch->localPosition;
}

void PlanetCreatorTool::OnTouchMove(oxygine::Event* event)
{
	if (!m_button->IsClicked() || m_currentTouch == INVALID_TOUCH)
		return;

	oxygine::TouchEvent* touch = oxygine::safeCast<oxygine::TouchEvent*>(event);
	m_velocity.set(touch->localPosition.x - m_stagePosition.x, touch->localPosition.y - m_stagePosition.y);

	auto localStartPosition = g_main->GetCamera().LocalFromStage(m_stagePosition);
	auto localEndPosition = g_main->GetCamera().LocalFromStage(touch->localPosition);

	const simmulation::CelestialBody * parent = GetParent();

	oxygine::VectorD2 position(localStartPosition.x, localStartPosition.y);
	// follow body is at 0,0
	double distanceToParent = localStartPosition.length();

	// velocity vector scale
	static const double VELOCITY_VECTOR_LENGTH_FOR_CIRCULAR_ORBIT = 100.0;
	double velocityValueForCircularOrbit = sqrt(scaleGravityConstant(GRAVITY_CONSTANT)*(parent->GetMass() / distanceToParent));

	m_velocity *= velocityValueForCircularOrbit / VELOCITY_VECTOR_LENGTH_FOR_CIRCULAR_ORBIT;

	m_conic = simmulation::ComputeConic(parent->GetMass(), scaleKilograms(m_weight),
		position, m_velocity);

	if (m_conic.GetType() == simmulation::Conic::invalid)
		return;

	if (m_conicActor)
		g_main->GetWorld()->removeChild(m_conicActor);
	m_conicActor = AnimationConic::Create(m_conic, oxygine::Color::WhiteSmoke, primitive::DrawType::nanovg);
	g_main->GetWorld()->addChild(m_conicActor);

	if (!m_parentLine)
	{
		m_parentLine = new primitive::DirectedTextLine({ 0.0f, 0.0f }, localStartPosition);
		m_parentLine->SetLineColor({ 190, 10, 10, 190 });
		g_main->GetWorld()->addChild(m_parentLine);
	}
	else
		m_parentLine->Set({ 0.0f, 0.0f }, localStartPosition);

	if (!m_velocityLine)
	{
		m_velocityLine = new primitive::DirectedTextLine(localStartPosition, localEndPosition);
		m_velocityLine->SetLineColor({ 190, 10, 10, 190 });
		g_main->GetWorld()->addChild(m_velocityLine);

		m_velocityLine->SetRightArrow(true);
		UpdateWeight();
	}
	else
		m_velocityLine->Set(localStartPosition, localEndPosition);

	char buffer[128];
	sprintf(buffer, "%.2e m/s", scaleVelocityInv(m_velocity).length());
	m_velocityLine->GetCenterText()->SetText(buffer);
	m_velocityLine->GetCenterText()->SetNotZoomable(true);
	sprintf(buffer, "%.2e kg", m_weight);
	m_velocityLine->GetLeftText()->SetText(buffer);
	m_velocityLine->GetLeftText()->SetNotZoomable(true);

	sprintf(buffer, "%.2e m", scaleSizeInv(distanceToParent));
	m_parentLine->GetCenterText()->SetText(buffer);
	m_parentLine->GetCenterText()->SetNotZoomable(true);

	if (!m_planet)
	{
		m_planet = new primitive::Circle(3.0f);
		m_planet->SetFillColor({ 190, 10, 10, 190 });
		g_main->GetWorld()->addChild(m_planet);
		m_planet->SetNotZoomable(true);
	}
	m_planet->setPosition(localStartPosition);

	if (!m_name)
	{
		m_name = new primitive::Text(GeneratePlanetName().c_str());
		m_name->SetFontSize(40);
		m_name->EnableEditing(&PlanetCreatorTool::ValidatePlanetName);
		g_main->GetGui()->addChild(m_name);
		m_name->setAnchor(1.0f, 0.0f);

		oxygine::Vector2 pos = oxygine::getStage()->getSize();
		pos.y = 0.0f;
		pos.x /= 2.0f;

		m_name->setPosition(pos);
	}
}

bool PlanetCreatorTool::ValidatePlanetName(const std::string & name)
{
	if (name.empty())
		return false;
	for (auto & b : simmulation::g_system->GetBodies())
	{
		if (b->GetName() == name)
			return false;
	}

	return true;
}

std::string PlanetCreatorTool::GeneratePlanetName()
{
	static const char * names[] = { "lv426", "alpha", "beta", "gamma", "delta" };
	std::string name = names[rand() % COUNTOF(names)];

	int counter = 1;
	std::string tmp = name;
	while (!ValidatePlanetName(name))
	{
		name = tmp;
		char buffer[64];
		sprintf(buffer, "-%d", counter++);
		name.append(buffer);
	}

	return name;
}

void PlanetCreatorTool::UpdateWeight()
{
	if (!m_velocityLine)
		return;

	char buffer[128];
	sprintf(buffer, "%.2e kg", m_weight);
	m_velocityLine->GetLeftText()->SetText(buffer);
}

const simmulation::CelestialBody * PlanetCreatorTool::GetParent()
{
	const simmulation::CelestialBody * parent = simmulation::g_system->GetSun().get();
	if (auto followBody = m_system->GetFollowBody())
		parent = followBody->GetCelestialBody().get();

	return parent;
}

void PlanetCreatorTool::OnTouchUp(oxygine::Event* event)
{
	oxygine::TouchEvent* touch = oxygine::safeCast<oxygine::TouchEvent*>(event);
	if (touch->index == m_currentTouch)
		m_currentTouch = INVALID_TOUCH;

	static const float CLICK_DISTANCE = 5.0f;
	if ((touch->localPosition - m_stagePosition).length() < CLICK_DISTANCE/ g_main->GetCamera().GetScaleCamera())
		oxygine::log::messageln("planet clicked");
}

void PlanetCreatorTool::CreateSlider(double minWeight, double currentWeight, double maxWeight)
{
	static const float SLIDER_XMARGIN = 20.0f;
	static const float SLIDER_YMARGIN = 5.0f;

	oxygine::Vector2 size = oxygine::getStage()->getSize();

	m_slider = new gui::Slider(size.x - 2.0f * SLIDER_XMARGIN, minWeight, currentWeight, maxWeight, [this](double value)
	{
		m_weight = pow(10.0, value);
		UpdateWeight();
	});
	g_main->GetGui()->addChild(m_slider);
	m_slider->setPosition({ SLIDER_XMARGIN, size.y - 2.0f*gui::Slider::SLIDER_RADIUS - SLIDER_YMARGIN });
}

void PlanetCreatorTool::RemoveSlider()
{
	g_main->GetGui()->removeChild(m_slider);
	m_slider = nullptr;
}
