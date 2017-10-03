#include "oxygine-framework.h"
#include <functional>
#include "Common.h"
#include "Primitives\Line.h"
#include "Simmulation\SolarSystem.h"
#include "Tools\Tools.h"
#include "MainActor.h"
#include "Animation\AnimationSystem.h"
#include "NanovgInclude.h"

//it is our resources
//in real project you would have more than one Resources declarations.
//It is important on mobile devices with limited memory and you would load/unload them
oxygine::Resources g_gameResources;

NVGcontext * g_nanovgContext = nullptr;

void EllipsePreinit() {}

void InitSolarSystemLegacy(simmulation::SolarSystem & system)
{
	system.SetName("solar_system");

	system.SetStar({ "sun",       scalePosition({ 0.0, 0.0 }),                     scaleVelocity({ 0.0, 0.0 }),                scaleKilograms(1.989e30) });
	system.AddBody({ "mercury",   scalePosition({ 5.790e10, 0.0 }),                scaleVelocity({ 0.0, 4.790e04 }),           scaleKilograms(3.302e23) });
	system.AddBody({ "venus",     scalePosition({ 1.082e11, 0.0 }),                scaleVelocity({ 0.0, 3.500e04 }),           scaleKilograms(4.869e24) });
	system.AddBody({ "mars",      scalePosition({ 2.279e11, 0.0 }),                scaleVelocity({ 0.0, 2.410e04 }),           scaleKilograms(6.419e23) });
	system.AddBody({ "earth",     scalePosition({ 1.496e11, 0.0 }),                scaleVelocity({ 0.0, 2.980e04 }),           scaleKilograms(5.974e24) });
	system.AddBody({ "moon",      scalePosition({ 1.496e11 + 3.844e8, 0.0 }),      scaleVelocity({ 0.0, 2.980e04 + 0.1e04 }),  scaleKilograms(0.07342e24) });
	system.AddBody({ "comet",     scalePosition({ 1.00e11, 0.0 }),                 scaleVelocity({ 0.0, 2.0e4 }),              scaleKilograms(5.0e22) });
	system.AddBody({ "jupiter",   scalePosition({ 7.785e11, 0.0 }),                scaleVelocity({ 0.0, 1.307e4 }),            scaleKilograms(1.898e27) });
	system.AddBody({ "ganymede",  scalePosition({ 7.785e11 + 1.0704e9, 0.0 }),     scaleVelocity({ 0.0, 1.307e4 + 1.088e4 }),  scaleKilograms(1.4819e23) });
	system.AddBody({ "europa",    scalePosition({ 7.785e11 + 6.709e8, 0.0 }),      scaleVelocity({ 0.0, 1.307e4 + 1.3740e4 }), scaleKilograms(4.7998e22) });
	system.AddBody({ "io",        scalePosition({ 7.785e11 + 4.217e8, 0.0 }),      scaleVelocity({ 0.0, 1.307e4 + 1.7334e4 }), scaleKilograms(8.9319e22) });
	system.AddBody({ "callisto",  scalePosition({ 7.785e11 + 1.8827e9, 0.0 }),     scaleVelocity({ 0.0, 1.307e4 + 8.204e3 }),  scaleKilograms(1.075938e23) });
	system.AddBody({ "saturn",    scalePosition({ 1.429e12, 0.0 }),                scaleVelocity({ 0.0, 9.69e3 }),             scaleKilograms(5.6836e26) });
	system.AddBody({ "enceladus", scalePosition({ 1.429e12 + 2.38037e8, 0.0 }),    scaleVelocity({ 0.0, 9.69e3 + 1.2635e4 }),  scaleKilograms(1.08e20) });
	system.AddBody({ "dione",     scalePosition({ 1.429e12 + 3.774e8, 0.0 }),      scaleVelocity({ 0.0, 9.69e3 + 1.003e4 }),   scaleKilograms(1.1e21) });
	system.AddBody({ "rhea",      scalePosition({ 1.429e12 + 5.27108e8, 0.0 }),    scaleVelocity({ 0.0, 9.69e3 + 8.48e3 }),    scaleKilograms(2.3e21) });
	system.AddBody({ "titan",     scalePosition({ 1.429e12 + 1.22187e9, 0.0 }),    scaleVelocity({ 0.0, 9.69e3 + 5.58e3 }),    scaleKilograms(1.35e23) });
	system.AddBody({ "lapetus",   scalePosition({ 1.429e12 + 3.56082e9, 0.0 }),    scaleVelocity({ 0.0, 9.69e3 + 3.26e3 }),    scaleKilograms(1.8e21) });
	system.AddBody({ "uranus",    scalePosition({ 2.871e12, 0.0 }),                scaleVelocity({ 0.0, 6.8352e3 }),           scaleKilograms(8.681e25) });
	system.AddBody({ "miranda",   scalePosition({ 2.871e12 + 1.299e8, 0.0 }),      scaleVelocity({ 0.0, 6.8352e3 + 6.66e3 }),  scaleKilograms(6.59e19) });
	system.AddBody({ "ariel",     scalePosition({ 2.871e12 + 1.9e8, 0.0 }),        scaleVelocity({ 0.0, 6.8352e3 + 5.51e3 }),  scaleKilograms(1.353e21) });
	system.AddBody({ "umbriel",   scalePosition({ 2.871e12 + 2.66e8, 0.0 }),       scaleVelocity({ 0.0, 6.8352e3 + 4.67e3 }),  scaleKilograms(1.172e21) });
	system.AddBody({ "titania",   scalePosition({ 2.871e12 + 4.36e8, 0.0 }),       scaleVelocity({ 0.0, 6.8352e3 + 3.64e3 }),  scaleKilograms(3.527e21) });
	system.AddBody({ "oberon",    scalePosition({ 2.871e12 + 5.84e8, 0.0 }),       scaleVelocity({ 0.0, 6.8352e3 + 3.15e3 }),  scaleKilograms(3.014e21) });
	system.AddBody({ "neptune",   scalePosition({ 4.498e12, 0.0 }),                scaleVelocity({ 0.0, 5.43e3 }),             scaleKilograms(1.0243e26) });
	system.AddBody({ "proteus",   scalePosition({ 4.498e12 + 9.28e7, 0.0 }),       scaleVelocity({ 0.0, 5.43e3 + 7.623e3 }),   scaleKilograms(4.4e19) });
	system.AddBody({ "triton",    scalePosition({ 4.498e12 + 3.3e8, 0.0 }),        scaleVelocity({ 0.0, 5.43e3 - 4.39e3 }),    scaleKilograms(2.14e22) }); // retrograde
	system.AddBody({ "nereid",    scalePosition({ 4.498e12 + 5.48865e9, 0.0 }),    scaleVelocity({ 0.0, 5.43e3 + 9.34e2 }),    scaleKilograms(3.1e19) }); // eccentricity 0.7507
	system.AddBody({ "pluto",     scalePosition({ 5.90638e12, 0.0 }),              scaleVelocity({ 0.0, 4.67e3 }),             scaleKilograms(1.303e22) });
	system.AddBody({ "charon",    scalePosition({ 5.90638e12 + 1.964e7, 0.0 }),    scaleVelocity({ 0.0, 4.67e3 + 2.1e2 }),     scaleKilograms(1.587e21) });
	system.AddBody({ "eris",      scalePosition({ 1.4062e13, 0.0 }),               scaleVelocity({ 0.0, 3.4338e3 }),           scaleKilograms(1.66e22) });
}

void InitializeNanovg()
{
	g_nanovgContext = nvgCreateGLES2(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);

	//read font to nanovg
	static oxygine::file::buffer fontBuffer;
	if (oxygine::file::read("fonts\\calibri.ttf", fontBuffer))
	{
		nvgCreateFontMem(g_nanovgContext, "main-font", fontBuffer.data.data(), fontBuffer.data.size(), 0);
	}
}

void EllipseInit()
{
	InitializeNanovg();

	oxygine::DebugActor::setCorner(1);

	g_gameResources.loadXML("res.xml");

	oxygine::getStage()->setName("stage");

	simmulation::g_system.reset(new simmulation::SolarSystem());
	simmulation::g_system->LoadBodies("solar_system");
	if (!simmulation::g_system->LoadData())
	{
		simmulation::g_system->Simmulate(SIMMULATION_POINT_COUNT);
		simmulation::g_system->RebuildParents();
	}

	g_main = new MainActor;
	g_main->setSize(oxygine::getStage()->getSize() * 2);
	g_main->attachTo(oxygine::getStage());

	g_main->Init();
}


void EllipseUpdate()
{

}

void EllipseDestroy()
{
	nvgDeleteGLES2(g_nanovgContext);

	simmulation::g_system.reset(nullptr);
	g_gameResources.free();
	g_main = nullptr;
}
