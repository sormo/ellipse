#include "CelestialBody.h"
#include "Common.h"
#include <fstream>

namespace simmulation
{
	std::shared_ptr<CelestialBody> CelestialBody::invalidPtr;
	std::shared_ptr<const CelestialBody> CelestialBody::invalidCPtr;

	CelestialBody::CelestialBody(const CelestialBodyDef & def)
		: m_name(def.name), m_mass(def.mass), m_isStar(def.isStar), m_radius(def.radius)
	{
		m_positions.push_back(def.position);
		m_parentRelativeData.positions.push_back(def.position);
		m_momenta.push_back(def.velocity * def.mass);
	}

	const std::string & CelestialBody::GetName() const
	{
		return m_name;
	}

	bool CelestialBody::IsStar() const
	{
		return m_isStar;
	}

	double CelestialBody::GetRadius() const
	{
		return m_radius;
	}

	double CelestialBody::GetMass() const
	{
		return m_mass;
	}

	const std::vector<oxygine::VectorD2> & CelestialBody::GetPositions() const
	{
		assert(is_locked());
		return m_positions;
	}

	const std::vector<oxygine::VectorD2> & CelestialBody::GetMomenta() const
	{
		assert(is_locked());
		return m_momenta;
	}

	void CelestialBody::AppendData(const std::vector<oxygine::VectorD2> & positions, const std::vector<oxygine::VectorD2> & momenta)
	{
		assert(is_locked());
		assert(positions.size() == momenta.size());

		m_positions.insert(std::end(m_positions), std::begin(positions), std::end(positions));
		m_momenta.insert(std::end(m_momenta), std::begin(momenta), std::end(momenta));

		m_parentRelativeData.rebuildFromIndex = m_positions.size() - positions.size();
	}

	void CelestialBody::ClearData(size_t index)
	{
		if (index < m_positions.size())
		{
			m_positions.erase(std::begin(m_positions) + index, std::end(m_positions));
			m_momenta.erase(std::begin(m_momenta) + index, std::end(m_momenta));
			m_parentRelativeData.positions.erase(std::begin(m_parentRelativeData.positions) + index, std::end(m_parentRelativeData.positions));
		}
	}

	void CelestialBody::Dump() const
	{
		assert(is_locked());
		dumpData(m_name + "_positions", m_positions);
		dumpData(m_name + "_momenta", m_momenta);

		std::ofstream f_mass("../analyse/data/" + m_name + "_mass.txt");
		f_mass << m_mass << std::endl;

		std::ofstream f_conic("../analyse/data/" + m_name + "_conic.txt");
		auto conic = m_parentRelativeData.conic;
		f_conic << conic.A << " " << conic.B << " " << conic.C << " " << conic.D << " " << conic.E << " " << conic.F;

		if (conic.GetType() == simmulation::Conic::ellipse)
		{
			auto e = conic.GetEllipse();
			std::ofstream f_ellipse("../analyse/data/" + m_name + "_ellipse.txt");
			f_ellipse << e.radius.x << " " << e.radius.y << " " << e.position.x << " " << e.position.y << " " << e.angle;
		}
	}

	void CelestialBody::SetParent(const std::shared_ptr<CelestialBody> & parent)
	{
		assert(is_locked());
		if (m_parent != parent)
			m_parentRelativeData.rebuildFromIndex = 0;
		m_parent = parent;
	}

	std::shared_ptr<CelestialBody> & CelestialBody::GetParent()
	{
		assert(is_locked());
		return m_parent;
	}

	std::shared_ptr<const CelestialBody> CelestialBody::GetParent() const
	{
		assert(is_locked());
		return std::const_pointer_cast<const CelestialBody>(m_parent);
	}

	const ParentRelativeData & CelestialBody::GetParentRelativeData() const
	{
		assert(is_locked());
		assert(m_parentRelativeData.rebuildFromIndex == ParentRelativeData::INVALID_INDEX);
		return m_parentRelativeData;
	}

	void CelestialBody::RebuildParentRelativeData()
	{
		if (m_parentRelativeData.rebuildFromIndex == ParentRelativeData::INVALID_INDEX)
			return;

		if (!m_parent)
		{
			m_parentRelativeData.positions.insert(std::end(m_parentRelativeData.positions), 
				std::begin(m_positions) + m_parentRelativeData.rebuildFromIndex, std::end(m_positions));
			m_parentRelativeData.rebuildFromIndex = ParentRelativeData::INVALID_INDEX;
			return;
		}

		m_parent->RebuildParentRelativeData();

		double parentMass = 0.0;
		// positions
		m_parentRelativeData.positions.resize(m_positions.size(), oxygine::VectorD2{ 0.0, 0.0 });
		{
			std::lock_guard<simmulation::CelestialBody> lock(*m_parent.get());

			for (size_t i = m_parentRelativeData.rebuildFromIndex; i < m_parentRelativeData.positions.size(); ++i)
				m_parentRelativeData.positions[i] = m_positions[i] - m_parent->GetParentRelativeData().positions[i];

			parentMass = m_parent->GetMass();
		}

		// conic
		m_parentRelativeData.conic = simmulation::ApproximateConic(m_parentRelativeData.positions);

		// period
		m_parentRelativeData.period = 0.0;
		if (m_parentRelativeData.conic.GetType() == simmulation::Conic::ellipse)
		{
			auto ellipse = m_parentRelativeData.conic.GetEllipse();
			double major = std::max(ellipse.radius.x, ellipse.radius.y);
			m_parentRelativeData.period = 2.0*PI*sqrt(std::pow(major, 3.0) / (scaleGravityConstant(GRAVITY_CONSTANT)*parentMass));
		}

		m_parentRelativeData.rebuildFromIndex = ParentRelativeData::INVALID_INDEX;
	}

	void CelestialBody::lock() const
	{
		m_lock.lock();
		m_lockInfo.holder = std::this_thread::get_id();
		m_lockInfo.count++;
	}

	void CelestialBody::unlock() const
	{
		m_lock.unlock();
		m_lockInfo.count--;
		if (m_lockInfo.count == 0)
			m_lockInfo.holder = std::thread::id();
	}

	bool CelestialBody::try_lock() const
	{
		if (m_lock.try_lock())
		{
			m_lockInfo.holder = std::this_thread::get_id();
			m_lockInfo.count++;
			return true;
		}
		return false;
	}

	bool CelestialBody::is_locked() const
	{
		return m_lockInfo.holder == std::this_thread::get_id();
	}
}
