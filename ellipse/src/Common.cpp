#include "Common.h"
#include <fstream>
#include <cmath>

oxygine::VectorD2 scaleVelocity(const oxygine::VectorD2 & v)
{
	return v / 1.0e9;
}
oxygine::VectorD2 scalePosition(const oxygine::VectorD2 & v)
{
	return v / 1.0e9;
}
double scaleSize(double v)
{
	return v / 1.0e9;
}
double scaleSizeInv(double v)
{
	return v*1.0e9;
}

oxygine::VectorD2 scaleVelocityInv(const oxygine::VectorD2 & v)
{
	return v * 1.0e9;
}

oxygine::VectorD2 scalePositionInv(const oxygine::VectorD2 & v)
{
	return v * 1.0e9;
}

double scaleKilogramsInv(double v)
{
	return v * 1.0e27;
}

void dumpData(const std::string & name, const std::vector<oxygine::VectorD2> & data)
{
	std::ofstream f(std::string("../analyse/data/") + name + ".txt");
	for (const auto & p : data)
		f << p.x << " " << p.y << std::endl;
}

void dumpData(const std::string & name, const std::vector<double> & data)
{
	std::ofstream f(std::string("../analyse/data/") + name + ".txt");
	for (const auto & p : data)
		f << p << std::endl;
}

void dumpData(const std::string & name, const std::vector<std::pair<std::string, double>> & data)
{
	std::ofstream f(std::string("../analyse/data/") + name + ".txt");
	for (const auto & p : data)
		f << p.first << " " << p.second << std::endl;
}

oxygine::VectorD2 rotate(oxygine::VectorD2 && v, double angle)
{
	double s = std::sin(angle);
	double c = std::cos(angle);

	oxygine::VectorD2 t;

	t.x = v.x * c - v.y * s;
	t.y = v.x * s + v.y * c;

	return t;
}

oxygine::VectorD2 & rotate(oxygine::VectorD2 & v, double angle)
{
	double s = std::sin(angle);
	double c = std::cos(angle);

	oxygine::VectorD2 t;

	t.x = v.x * c - v.y * s;
	t.y = v.x * s + v.y * c;

	v = t;

	return v;
}

double fromDegToRad(double deg)
{
	return deg * 0.0174533;
}

double fromRadToDeg(double rad)
{
	return rad * 57.2958;
}

oxygine::VectorD2 convertVec(const oxygine::VectorD2 & v)
{
	return{ (float)v.x, (float)v.y };
}

double crossProduct(const oxygine::VectorD2 & v1, const oxygine::VectorD2 & v2)
{
	return v1.x*v2.y - v1.y*v2.x;
}

double normalizeAngle(double radians)
{
	if (radians > 0.0)
	{
		while (radians > PI)
			radians -= 2.0*PI;
	}
	else
	{
		while (radians < PI)
			radians += 2.0*PI;
	}
	return radians;
}

bool sigmaCompare(const oxygine::VectorD2 & a, const oxygine::VectorD2 & b, double sigma)
{
	return fabs(a.x - b.x) < sigma && fabs(a.y - b.y) < sigma;
}

double radiusFromMassAndDensity(double mass, double density)
{
	double volume = mass / density;
	return cbrt((3.0*volume)/(4.0*PI));
}
