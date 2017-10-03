#pragma once
#include "oxygine-framework.h"
#include <vector>

namespace simmulation
{
	struct Ellipse
	{
		oxygine::VectorD2 radius;
		oxygine::VectorD2 position;
		double angle = 0.0;

		std::pair<oxygine::VectorD2, oxygine::VectorD2> GetMajorAxis() const;
		std::pair<oxygine::VectorD2, oxygine::VectorD2> GetMinorAxis() const;
		std::pair<oxygine::VectorD2, oxygine::VectorD2> GetFoci() const;

		double GetEccentricity();
	};

	struct Hyperbola
	{
		oxygine::VectorD2 radius;
		oxygine::VectorD2 position;
		double angle = 0.0;

		std::pair<oxygine::VectorD2, oxygine::VectorD2> GetFoci() const;

		double GetEccentricity();
	};

	struct Parabola
	{
		double radius;
		oxygine::VectorD2 position;
		double angle;
	};

	struct Conic
	{
		// AX^2 + BXY + CY^2 + DX + EY + F = 0
		double A = 0.0, B = 0.0, C = 0.0, D = 0.0, E = 0.0, F = 0.0;

		Ellipse GetEllipse() const;
		Hyperbola GetHyperbola() const;
		Parabola GetParabola() const;

		void Rotate(double rad);
		void Translate(const oxygine::VectorD2 & vec);

		enum Type
		{
			invalid,
			ellipse,
			parabola,
			hyperbola
		};
		Type GetType() const;
	};

	// all conics are parent relative

	Conic ApproximateConic(const std::vector<oxygine::VectorD2> & positions);
	Conic ApproximateConic(std::vector<oxygine::VectorD2>::const_iterator begin, std::vector<oxygine::VectorD2>::const_iterator end);

	Conic ComputeConic(double mass1, double mass2, const oxygine::VectorD2 & position2, const oxygine::VectorD2 & velocity2);
}
