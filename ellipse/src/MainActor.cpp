#include "MainActor.h"
#include "Tools\Tools.h"
#include "Animation\AnimationSystem.h"

spMainActor g_main;

MainActor::MainActor()
{
	setName("main");

	m_world = new oxygine::Actor;
	m_world->setName("world");
	m_world->attachTo(this);
	m_world->setPosition(oxygine::getStage()->getSize() / 2.0f);

	m_gui = new oxygine::Actor;
	m_gui->setName("gui");
	m_gui->attachTo(this);

	m_camera.reset(new Camera(*m_world));

	m_animation = new AnimationSystem;
	m_animation->setName("animation");
	m_animation->attachTo(m_world);

	m_tools = new Tools(m_animation);
	m_tools->setName("tools");
	m_tools->attachTo(m_gui);
}

Camera & MainActor::GetCamera()
{
	return *m_camera.get();
}

oxygine::spActor MainActor::GetGui()
{
	return m_gui;
}

oxygine::spActor MainActor::GetWorld()
{
	return m_world;
}

void MainActor::update(const oxygine::UpdateState & us)
{
	oxygine::Actor::update(us);

	m_camera->Update();
}

void MainActor::Init()
{
	if (simmulation::g_system->GetSun())
		m_animation->AddBody(std::const_pointer_cast<const simmulation::CelestialBody>(simmulation::g_system->GetSun()));
	for (auto & b : simmulation::g_system->GetBodies())
		m_animation->AddBody(std::const_pointer_cast<const simmulation::CelestialBody>(b));
	m_animation->Rebuild();
	m_animation->Start();
}
