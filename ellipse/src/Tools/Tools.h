#pragma once
#include <oxygine-framework.h>
#include <thread>
#include "Gui\Button.h"
#include "Animation\AnimationSystem.h"
#include "PlanetCreatorTool.h"

class Tools : public oxygine::Actor
{
public:
	Tools(spAnimationSystem system);
	~Tools();
private:
	void OnClick(oxygine::Event* event);
	AnimationBody * GetClickBody(const oxygine::Vector2 & pos);

	virtual void doUpdate(const oxygine::UpdateState & us) override;

	spAnimationSystem m_system;

	gui::spButton m_follow;

	gui::spButton m_simmulate;
	std::thread m_simmulationThread;
	std::atomic<bool> m_simmulationRunning;

	gui::spButton m_info;

	PlanetCreatorTool m_planetCreatorTool;
};


typedef oxygine::intrusive_ptr<Tools> spTools;