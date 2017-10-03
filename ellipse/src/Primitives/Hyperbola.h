#pragma once
#include "oxygine-framework.h"
#include "Primitive.h"

namespace primitive
{
	DECLARE_SMART(Hyperbola, spHyperbola);
	class Hyperbola : public Primitive
	{
	public:
		Hyperbola(float a, float b, DrawType type = DrawType::oxygine);
		~Hyperbola();

		void HidePositive(bool hide);
		void HideNegative(bool hide);

	private:
		DECLARE_SMART(HyperbolaHalf, spHyperbolaHalf);
		class HyperbolaHalf : public Primitive
		{
		public:
			HyperbolaHalf(float a, float b, DrawType type);
		private:
			void Draw() override;
		};

		spHyperbolaHalf m_positive;
		spHyperbolaHalf m_negative;
	};
}
