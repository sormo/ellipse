LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := main

#SDK_ROOT points to folder with SDL and oxygine-framework
LOCAL_SRC_FILES := ../../../..//SDL/src/main/android/SDL_android_main.c

#LOCAL_SRC_FILES += ../../../src/example.cpp ../../../src/main.cpp 
#FILE_LIST := $(wildcard $(LOCAL_PATH)/../../../src/*.cpp)
#LOCAL_SRC_FILES := $(FILE_LIST:$(LOCAL_PATH)/%=%)
LOCAL_SRC_FILES += ../../../src/main.cpp \
				../../../src/Common.cpp \
				../../../src/EllipseMain.cpp \
				../../../src/MainActor.cpp \
				../../../src/Camera.cpp \
				../../../src/Animation/AnimationBody.cpp \
				../../../src/Animation/AnimationBodyInfoUpdater.cpp \
				../../../src/Animation/AnimationSystem.cpp \
				../../../src/Animation/AnimationInterpolation.cpp \
				../../../src/Animation/AnimationBodyTree.cpp \
				../../../src/Animation/AnimationConic.cpp \
				../../../src/Primitives/Circle.cpp \
				../../../src/Primitives/Ellipse.cpp \
				../../../src/Primitives/Hyperbola.cpp \
				../../../src/Primitives/Line.cpp \
				../../../src/Primitives/Text.cpp \
				../../../src/Primitives/Polyline.cpp \
				../../../src/Primitives/Primitive.cpp \
				../../../src/Primitives/PrimitiveDraw.cpp \
				../../../src/Simmulation/SimmulationStepper.cpp \
				../../../src/Simmulation/CelestialBody.cpp \
				../../../src/Simmulation/SolarSystem.cpp \
				../../../src/Simmulation/ConicApproximation.cpp \
				../../../src/Gui/Button.cpp \
				../../../src/Gui/Slider.cpp \
				../../../src/Tools/Tools.cpp \
				../../../src/Tools/PlanetCreatorTool.cpp \
				../../../../nanovg/src/nanovg.c
				
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../nanovg/src/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../src/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../boost/

LOCAL_STATIC_LIBRARIES := oxygine-framework_static
LOCAL_SHARED_LIBRARIES := SDL2

include $(BUILD_SHARED_LIBRARY)


#import from NDK_MODULE_PATH defined in build.cmd
$(call import-module, oxygine-framework)
