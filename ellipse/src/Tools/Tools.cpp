#include "Tools.h"
#include "MainActor.h"
#include "Common.h"
#include "Simmulation\SolarSystem.h"

#define FOLLOW_TOUCH_RADIUS 40.0f
#define FOLLOW_DEFAULT_ZOOM 400.0f;

Tools::Tools(spAnimationSystem system)
	: m_system(system), m_planetCreatorTool(system)
{
	m_simmulationRunning = false;

	m_follow = new gui::Button("follow", true, [this](bool clicked)
	{
		oxygine::log::messageln("follow clicked");
		if (!clicked)
		{
			g_main->GetCamera().SetFollowChild(nullptr);
			g_main->GetCamera().SetMaximumScale(CAMERA_GLOBAL_MAXIMUM_SCALE);
			if (m_system->GetFollowBody())
				g_main->GetCamera().SetPositionFromLocal(m_system->GetFollowBody()->GetActor()->getPosition());
			m_system->SetFollowBody(nullptr);
		}
	});
	addChild(m_follow);
	m_follow->setPosition({ 0.0f, 0.0f });

	m_simmulate = new gui::Button("simmulate", false, [this](bool)
	{
		oxygine::log::messageln("simmulate clicked");
		if (m_simmulationRunning)
			return;
		m_simmulationRunning = true;

		if (m_simmulationThread.joinable())
			m_simmulationThread.join();

		m_simmulationThread = std::thread([this]()
		{
			simmulation::g_system->Simmulate(SIMMULATION_POINT_COUNT);
			simmulation::g_system->RebuildParents();
			m_system->Rebuild();
			m_simmulationRunning = false;
		});
		
	});
	addChild(m_simmulate);
	m_simmulate->setAnchor(0.5f, 0.5f);
	m_simmulate->setPosition(oxygine::Vector2{ 0.0f, m_follow->getSize().y } + m_simmulate->getSize()/2.0f);

	m_info = new gui::Button("info", true, [this](bool clicked)
	{
		oxygine::log::messageln("info clicked");
		if (!clicked)
			m_system->SetShowInfo(false);
	});
	addChild(m_info);
	m_info->setPosition(oxygine::Vector2{ 0.0f, m_simmulate->getSize().y + m_follow->getSize().y });

	auto planetCreatorButton = m_planetCreatorTool.GetButton();
	addChild(planetCreatorButton);
	planetCreatorButton->setPosition(oxygine::Vector2{ 0.0f, m_simmulate->getSize().y + m_follow->getSize().y + m_info->getSize().y});

	oxygine::getStage()->addEventListener(oxygine::TouchEvent::CLICK, CLOSURE(this, &Tools::OnClick));
}

Tools::~Tools()
{
	if (m_simmulationThread.joinable())
		m_simmulationThread.join();
}

AnimationBody * Tools::GetClickBody(const oxygine::Vector2 & pos)
{
	auto bodies = m_system->Query(pos, FOLLOW_TOUCH_RADIUS);
	if (bodies.empty())
		return nullptr;

	std::sort(std::begin(bodies), std::end(bodies),
			[](const AnimationBody * a, const AnimationBody * b)
	{
		std::lock(*a->GetCelestialBody().get(), *b->GetCelestialBody().get());
		std::lock_guard<const simmulation::CelestialBody> locka(*a->GetCelestialBody().get(), std::adopt_lock);
		std::lock_guard<const simmulation::CelestialBody> lockb(*b->GetCelestialBody().get(), std::adopt_lock);

		return a->GetCelestialBody()->GetMass() > b->GetCelestialBody()->GetMass();
	});

	return bodies[0];
}

void Tools::OnClick(oxygine::Event * event)
{
	oxygine::TouchEvent* touch = oxygine::safeCast<oxygine::TouchEvent*>(event);
	AnimationBody * destination = nullptr;

	if (m_follow->IsClicked() || m_info->IsClicked())
		destination = GetClickBody(touch->localPosition);

	if (m_follow->IsClicked())
	{
		if (destination)
		{
			g_main->GetCamera().SetFollowChild(destination->GetActor());
			g_main->GetCamera().SetMaximumScale(destination->GetMaximumCameraScale());
			m_system->SetFollowBody(destination);

			// zoom-in, show all direct childs
			auto childs = m_system->GetDirectChilds(destination);
			float scale = FOLLOW_DEFAULT_ZOOM;
			auto destinationPosition = destination->GetActor()->getPosition();

			if (!childs.empty())
			{
				float maxDistance = 0.0f;
				for (auto & child : childs)
				{
					auto childPosition = child->GetActor()->getPosition();
					maxDistance = std::max(maxDistance, (childPosition - destinationPosition).length());
				}

				float size = std::min(oxygine::getStage()->getSize().x, oxygine::getStage()->getSize().y) / 2.0f;
				scale = size / maxDistance;
			}

			g_main->GetCamera().SetScaleCamera(scale);
		}
	}

	if (m_info->IsClicked())
	{
		if (destination)
			m_system->ToggleInfo(destination);
	}
}

void Tools::doUpdate(const oxygine::UpdateState & us)
{
	if (m_simmulationRunning)
		m_simmulate->setRotation(m_simmulate->getRotation() + 0.08f);

	m_planetCreatorTool.Update();
}


