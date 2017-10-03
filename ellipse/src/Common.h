#pragma once
#include "oxygine-framework.h"
#include "MainActor.h"

#define PI 3.141592654
#define GRAVITY_CONSTANT 6.674e-11
#define SIMMULATION_DT 10000.0 // seconds
#define SIMMULATION_POINT_COUNT 1000 // number of points simmulated on one time
#define LENGTH_SIGMA 0.000001
#define ANIMATION_SPEED 200 // length of one step in milliseconds
#define EARTH_DENSITY 5.5

#define COUNTOF(arr) sizeof(arr)/sizeof(arr[0])

static const oxygine::pointer_index INVALID_TOUCH = 255;

double radiusFromMassAndDensity(double mass, double density);

double fromDegToRad(double deg);
double fromRadToDeg(double rad);

// dump data to file <name>.txt
void dumpData(const std::string & name, const std::vector<oxygine::VectorD2> & data);
void dumpData(const std::string & name, const std::vector<double> & data);
void dumpData(const std::string & name, const std::vector<std::pair<std::string, double>> & data);

oxygine::VectorD2 & rotate(oxygine::VectorD2 & v, double angle);
oxygine::VectorD2 rotate(oxygine::VectorD2 && v, double angle);

template <class T>
T getAngle(const oxygine::VectorT2<T> & v);
template <class T>
T getAngle(const oxygine::VectorT2<T> & v1, const oxygine::VectorT2<T> & v2);

double crossProduct(const oxygine::VectorD2 & v1, const oxygine::VectorD2 & v2);
// -PI , PI
double normalizeAngle(double radians);

// pos  [m]
// vel  [m * s^-2]
// mass [kg]
// G    [m ^ 3 * kg^-1 * s^-2]

oxygine::VectorD2 scaleVelocity(const oxygine::VectorD2 & v);
oxygine::VectorD2 scalePosition(const oxygine::VectorD2 & v);
double scaleSize(double v);
double scaleSizeInv(double v);
constexpr double scaleKilograms(double v)
{
	return v / 1.0e27;
}
constexpr double scaleGravityConstant(double v)
{
	return v * 1.0e27 / 1.0e27;
}

oxygine::VectorD2 scaleVelocityInv(const oxygine::VectorD2 & v);
oxygine::VectorD2 scalePositionInv(const oxygine::VectorD2 & v);
double scaleKilogramsInv(double v);

template<class T>
bool sigmaCompare(T a, T b, double sigma = 0.00000001);

oxygine::VectorD2 convertVec(const oxygine::VectorD2 & v);

extern oxygine::Resources g_gameResources;

// --- template implementations ---

template <class T>
T getAngle(const oxygine::VectorT2<T> & v)
{
	return atan2(v.y, v.x);
}

template <class T>
T getAngle(const oxygine::VectorT2<T> & v1, const oxygine::VectorT2<T> & v2)
{
	return atan2(v2.y, v2.x) - atan2(v1.y, v1.x);
}

template<class T>
bool sigmaCompare(T a, T b, double sigma)
{
	return fabs(a - b) < sigma;
}
bool sigmaCompare(const oxygine::VectorD2 & a, const oxygine::VectorD2 & b, double sigma);
