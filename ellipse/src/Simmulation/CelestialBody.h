#pragma once
#include "oxygine-framework.h"
#include <string>
#include <vector>
#include <mutex>
#include <thread>
#include "ConicApproximation.h"

namespace simmulation
{
	struct CelestialBodyDef
	{
		std::string name;
		oxygine::VectorD2 position;
		oxygine::VectorD2 velocity;
		double mass;
		double radius;
		bool isStar;
	};

	struct ParentRelativeData
	{
		std::vector<oxygine::VectorD2> positions;
		Conic conic;
		double period; // in seconds

	private:
		static const size_t INVALID_INDEX = (size_t)(-1);
		size_t rebuildFromIndex = INVALID_INDEX;
		friend class CelestialBody;
	};

	class CelestialBody
	{
	public:
		CelestialBody(const CelestialBodyDef & def);

		const std::string & GetName() const;
		double GetMass() const;
		bool IsStar() const;
		double GetRadius() const;

		const std::vector<oxygine::VectorD2> & GetPositions() const;
		const std::vector<oxygine::VectorD2> & GetMomenta() const;
		// data must be appended to all bodies atomically and called RebuildParentRelativeData
		void AppendData(const std::vector<oxygine::VectorD2> & positions, const std::vector<oxygine::VectorD2> & momenta);
		void ClearData(size_t index = 0);
		
		// parent must be updated in all bodies atomically and called RebuildParentRelativeData
		void SetParent(const std::shared_ptr<CelestialBody> & parent);
		std::shared_ptr<CelestialBody> & GetParent();
		std::shared_ptr<const CelestialBody> GetParent() const;

		const ParentRelativeData & GetParentRelativeData() const;
		void RebuildParentRelativeData();

		void Dump() const;

		static std::shared_ptr<CelestialBody> invalidPtr;
		static std::shared_ptr<const CelestialBody> invalidCPtr;

		void lock() const;
		void unlock() const;
		bool try_lock() const;

		bool is_locked() const;

	private:
		mutable std::recursive_mutex m_lock;
		struct LockInfo
		{
			std::thread::id holder;
			size_t count = 0;
		};
		mutable LockInfo m_lockInfo; // debug only ???

		std::vector<oxygine::VectorD2> m_positions;
		std::vector<oxygine::VectorD2> m_momenta;
		
		// constant data
		const std::string m_name;
		const double m_mass;
		const bool m_isStar;
		const double m_radius;

		// parent relative data
		std::shared_ptr<CelestialBody> m_parent;
		ParentRelativeData m_parentRelativeData;
	};
}
