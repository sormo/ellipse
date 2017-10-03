#pragma once
#include "oxygine-framework.h"
#include "Camera.h"
#include <memory>

class AnimationSystem;
class Tools;

DECLARE_SMART(MainActor, spMainActor);
class MainActor : public oxygine::Actor
{
public:
	MainActor();

	Camera & GetCamera();
	oxygine::spActor GetGui();
	oxygine::spActor GetWorld();

	void Init();

private:
	virtual void update(const oxygine::UpdateState & us) override;

	oxygine::spActor m_world;
	oxygine::spActor m_gui;

	oxygine::intrusive_ptr<AnimationSystem> m_animation;
	oxygine::intrusive_ptr<Tools> m_tools;

	std::unique_ptr<Camera> m_camera;
};

// global instance of main actor
extern spMainActor g_main;
