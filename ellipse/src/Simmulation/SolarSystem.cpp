#include "SolarSystem.h"
#include "SimmulationStepper.h"
#include "Common.h"
#include <json/json.h>
#include "BodyData_generated.h"
#include <fstream>

namespace simmulation
{
	std::unique_ptr<SolarSystem> g_system;

	SolarSystem::~SolarSystem()
	{
		SaveData();
	}

	void SolarSystem::SetName(const char * name)
	{
		m_name = name;
	}

	const std::string & SolarSystem::GetName()
	{
		return m_name;
	}

	std::shared_ptr<CelestialBody> SolarSystem::SetStar(const CelestialBodyDef & def)
	{
		m_star = std::make_shared<CelestialBody>(def);

		for (auto & b : m_bodies)
		{
			std::lock_guard<const simmulation::CelestialBody> lock(*b.get());
			b->SetParent(m_star);
		}

		return m_star;
	}

	std::shared_ptr<CelestialBody> SolarSystem::AddBody(const CelestialBodyDef & def, const std::shared_ptr<const CelestialBody> * parent)
	{
		m_bodies.push_back(std::make_shared<CelestialBody>(def));

		std::shared_ptr<CelestialBody> tmp; // TODO simplify
		std::shared_ptr<CelestialBody> * nonConstParent = &m_star;
		if (parent)
		{
			tmp = std::const_pointer_cast<CelestialBody>(*parent);
			nonConstParent = &tmp;
		}

		std::lock_guard<const simmulation::CelestialBody> lock(*m_bodies.back().get());
		m_bodies.back()->SetParent(*nonConstParent);
		m_bodies.back()->RebuildParentRelativeData();

		WriteBody(def);

		return m_bodies.back();
	}

	void SolarSystem::ClearData(size_t index)
	{
		if (m_star)
			m_star->ClearData(index);
		for (auto & b : m_bodies)
			b->ClearData(index);
	}

	void SolarSystem::Simmulate(size_t steps)
	{
		std::vector<SimmulationStepper::SimmulationObject> simmulationObjects;

		std::vector<std::vector<oxygine::VectorD2>> positions(m_bodies.size() + 1, std::vector<oxygine::VectorD2>());
		std::vector<std::vector<oxygine::VectorD2>> momenta(m_bodies.size() + 1, std::vector<oxygine::VectorD2>());
		
		// --- prepare data

		// star
		if (m_star)
		{
			std::lock_guard<const simmulation::CelestialBody> lock(*m_star.get());

			positions[0].push_back(m_star->GetPositions().back());
			momenta[0].push_back(m_star->GetMomenta().back());

			simmulationObjects.push_back({ positions[0], momenta[0], m_star->GetMass() });
		}
		
		// bodies
		for (size_t i = 0; i < m_bodies.size(); ++i)
		{
			std::lock_guard<const simmulation::CelestialBody> lock(*m_bodies[i].get());

			positions[i + 1].push_back(m_bodies[i]->GetPositions().back());
			momenta[i + 1].push_back(m_bodies[i]->GetMomenta().back());

			simmulationObjects.push_back({ positions[i + 1], momenta[i + 1], m_bodies[i]->GetMass() });
		}

		// --- simmulate

		SimmulationStepper stepper(true);
		stepper.Simmulate(simmulationObjects, steps, SIMMULATION_DT);

		// --- copy simmulated data

		// star
		if (m_star)
		{
			m_star->lock();

			positions[0].erase(std::begin(positions[0]));
			momenta[0].erase(std::begin(momenta[0]));

			m_star->AppendData(positions[0], momenta[0]);
		}

		// bodies
		for (size_t i = 0; i < m_bodies.size(); ++i)
		{
			m_bodies[i]->lock();

			positions[i + 1].erase(std::begin(positions[i + 1]));
			momenta[i + 1].erase(std::begin(momenta[i + 1]));

			m_bodies[i]->AppendData(positions[i + 1], momenta[i + 1]);
		}

		// rebuild parent relative data
		if (m_star)
		{
			m_star->RebuildParentRelativeData();
			m_star->unlock();
		}

		for (auto & body : m_bodies)
		{
			body->RebuildParentRelativeData();
			body->unlock();
		}
	}

	std::vector<std::shared_ptr<CelestialBody>> & SolarSystem::GetBodies()
	{
		return m_bodies;
	}

	std::shared_ptr<CelestialBody> & SolarSystem::GetSun()
	{
		return m_star;
	}

	std::shared_ptr<CelestialBody> & SolarSystem::GetBody(const char * name)
	{
		if (m_star)
		{
			std::lock_guard<const simmulation::CelestialBody> lock(*m_star.get());
			if (m_star->GetName() == name)
				return m_star;
		}
		auto it = std::find_if(std::begin(m_bodies), std::end(m_bodies),
			[name](const std::shared_ptr<CelestialBody> & b) { std::lock_guard<const simmulation::CelestialBody> lock(*b.get()); return b->GetName() == name; });
		if (it != std::end(m_bodies))
			return *it;
		return CelestialBody::invalidPtr;
	}

	// ---
	double CheckForPeriodicOrbit(const CelestialBody & a, const CelestialBody & b)
	{
		assert(a.GetPositions().size() == b.GetPositions().size());
		const size_t n = a.GetPositions().size();

		const CelestialBody & lighter = a.GetMass() < b.GetMass() ? a : b;
		const CelestialBody & heavier = a.GetMass() > b.GetMass() ? a : b;

		std::vector<oxygine::VectorD2> lp = lighter.GetPositions();

		//double meanDistance = 0.0;
		for (size_t i = 0; i < n; ++i)
		{
			lp[i] -= heavier.GetPositions()[i];
			//meanDistance += lp[i].length();
		}
		//meanDistance /= n;

		auto lc = simmulation::ApproximateConic(lp);
		if (lc.GetType() != simmulation::Conic::ellipse)
			return std::numeric_limits<double>::max();

		auto le = lc.GetEllipse();
		auto fle = le.GetFoci();

		double focusRefValue = std::min(fle.first.length(), fle.second.length()) / heavier.GetMass();

		return focusRefValue;
	}

	std::vector<double> GetEccentricities(const CelestialBody & a, const CelestialBody & b)
	{
		assert(a.GetPositions().size() == b.GetPositions().size());
		const size_t n = a.GetPositions().size();
		double ma = a.GetMass(), mb = b.GetMass();
		double rm = (ma*mb) / (ma + mb);

		double G = scaleGravityConstant(GRAVITY_CONSTANT);

		std::vector<double> E(n);
		for (size_t i = 0; i < n; ++i)
		{
			oxygine::VectorD2 velocity = a.GetMomenta()[i] / ma - b.GetMomenta()[i] / mb;

			double v = velocity.length();
			double vt = rotate(velocity, getAngle(velocity)).y;
			double r = (a.GetPositions()[i] - b.GetPositions()[i]).length();
			double energy = 0.5*rm*v*v - (G*ma*mb) / r;

			double angularMomentum = rm*r*vt; // use tangential part of velocity
			double semilatusRectum = pow(angularMomentum, 2) / (rm*G*ma*mb);
			double eccentricity = sqrt(1 + (2.0*energy*pow(angularMomentum, 2)) / (rm*pow(G*ma*mb, 2)));

			E[i] = eccentricity;
		}

		return E;
	}

	std::vector<double> GetEnergies(const CelestialBody & a, const CelestialBody & b)
	{
		assert(a.GetPositions().size() == b.GetPositions().size());
		const size_t n = a.GetPositions().size();
		double ma = a.GetMass(), mb = b.GetMass();
		double rm = (ma*mb) / (ma + mb);

		double G = scaleGravityConstant(GRAVITY_CONSTANT);

		std::vector<double> E(n);
		for (size_t i = 0; i < n; ++i)
		{
			oxygine::VectorD2 velocity = a.GetMomenta()[i] / ma - b.GetMomenta()[i] / mb;

			double v = velocity.length();
			double vt = rotate(velocity, getAngle(velocity)).y;
			double r = (a.GetPositions()[i] - b.GetPositions()[i]).length();
			double energy = 0.5*rm*v*v - (G*ma*mb) / r;

			E[i] = energy;
		}

		return E;
	}

	std::vector<double> GetGravities(const CelestialBody & a, const CelestialBody & b)
	{
		assert(a.GetPositions().size() == b.GetPositions().size());
		const size_t n = a.GetPositions().size();
		double ma = a.GetMass(), mb = b.GetMass();
		double rm = (ma*mb) / (ma + mb);

		double G = scaleGravityConstant(GRAVITY_CONSTANT);

		std::vector<double> E(n);
		for (size_t i = 0; i < n; ++i)
		{
			double r = (a.GetPositions()[i] - b.GetPositions()[i]).length();

			E[i] = (G*ma*mb)/r;
		}

		return E;
	}

	std::vector<double> GetDistances(const CelestialBody & a, const CelestialBody & b)
	{
		assert(a.GetPositions().size() == b.GetPositions().size());
		const size_t n = a.GetPositions().size();
		double ma = a.GetMass(), mb = b.GetMass();
		double rm = (ma*mb) / (ma + mb);

		double G = scaleGravityConstant(GRAVITY_CONSTANT);

		std::vector<double> E(n);
		for (size_t i = 0; i < n; ++i)
		{
			double r = (a.GetPositions()[i] - b.GetPositions()[i]).length();

			E[i] = r;
		}

		return E;
	}

	double GetMeanDeviation(const std::vector<double> & data)
	{
		double mean = 0.0;
		std::for_each(std::begin(data), std::end(data), [&mean](double e) { mean += e; });
		mean /= (double)data.size();

		//scale
		//std::for_each(std::begin(E), std::end(E), [mean](double & e) { e /= mean; });
		//mean = 1.0;

		//dumpData("eccentricity_" + a.GetName() + "-" + b.GetName(), E);

		double accumulatedMeanDistances = 0.0;
		std::for_each(std::begin(data), std::end(data), [&accumulatedMeanDistances, mean](double e) { accumulatedMeanDistances += fabs(e - mean); });

		return accumulatedMeanDistances / (double)data.size();
	}

#ifdef DUMP_DATA_FOR_ANALYSIS
	struct BodyPairData
	{
		std::string name1;
		double mass1;
		std::vector<oxygine::VectorD2> positions1;
		std::vector<oxygine::VectorD2> momenta1;

		std::string name2;
		double mass2;
		std::vector<oxygine::VectorD2> positions2;
		std::vector<oxygine::VectorD2> momenta2;
		
		double periodicOrbit;

		std::vector<double> eccentricities;
		std::vector<double> gravities;
		std::vector<double> distances;
		std::vector<double> energies;
	};

	void DumpData(const std::vector<BodyPairData> & data)
	{
		for (const auto & pair : data)
		{
			std::string namePrefix(pair.name1 + "-" + pair.name2);
			std::ofstream f(std::string("../analyse/data/") + namePrefix + ".txt");
			f << pair.mass1 << " " << pair.mass2 << " " << pair.periodicOrbit << " "
				<< GetMeanDeviation(pair.eccentricities) << " "
				<< GetMeanDeviation(pair.gravities) << " "
				<< GetMeanDeviation(pair.distances) << " "
				<< GetMeanDeviation(pair.energies);

			dumpData(namePrefix + "-eccentricities", pair.eccentricities);
			dumpData(namePrefix + "-gravities", pair.gravities);
			dumpData(namePrefix + "-distances", pair.distances);
			dumpData(namePrefix + "-energies", pair.energies);
			dumpData(pair.name1 + "-positions", pair.positions1);
			dumpData(pair.name1 + "-momenta", pair.momenta1);
			dumpData(pair.name2 + "-positions", pair.positions2);
			dumpData(pair.name2 + "-momenta", pair.momenta2);
		}
	}
#endif
	void SolarSystem::RebuildParents()
	{
		// constant of allowed deviation of eccentricity for elliptical orbit
		static const double PERIODIC_ORBIT_FOCUS_SIGMA = 1.0;

		std::map<std::shared_ptr<CelestialBody>, std::shared_ptr<CelestialBody>> childParentMapping;

#ifdef DUMP_DATA_FOR_ANALYSIS
		std::vector<BodyPairData> data;
#endif

		// test
		m_bodies.push_back(m_star); // TODO star should be part of m_bodies
		for (size_t i = 0; i < m_bodies.size(); ++i)
		{
			using ValuedBody = std::pair<double, std::shared_ptr<CelestialBody>>;
			std::vector<ValuedBody> values;

			for (size_t j = 0; j < m_bodies.size(); ++j)
			{
				if (j == i || m_bodies[j]->GetMass() < m_bodies[i]->GetMass())
					continue;

				std::lock(*m_bodies[i].get(), *m_bodies[j].get());
				std::lock_guard<const simmulation::CelestialBody> locki(*m_bodies[i].get(), std::adopt_lock);
				std::lock_guard<const simmulation::CelestialBody> lockj(*m_bodies[j].get(), std::adopt_lock);

#ifdef DUMP_DATA_FOR_ANALYSIS
				BodyPairData pairData;

				pairData.name1 = m_bodies[i]->GetName();
				pairData.mass1 = m_bodies[i]->GetMass();
				pairData.positions1 = m_bodies[i]->GetPositions();
				pairData.momenta1 = m_bodies[i]->GetMomenta();

				pairData.name2 = m_bodies[j]->GetName();
				pairData.mass2 = m_bodies[j]->GetMass();
				pairData.positions2 = m_bodies[j]->GetPositions();
				pairData.momenta2 = m_bodies[j]->GetMomenta();

				pairData.periodicOrbit = CheckForPeriodicOrbit(*m_bodies[i], *m_bodies[j]);
				pairData.eccentricities = GetEccentricities(*m_bodies[i], *m_bodies[j]);
				pairData.distances = GetDistances(*m_bodies[i], *m_bodies[j]);
				pairData.energies = GetEnergies(*m_bodies[i], *m_bodies[j]);
				pairData.gravities = GetGravities(*m_bodies[i], *m_bodies[j]);

				data.push_back(std::move(pairData));
#endif

				double value = GetMeanDeviation(GetEccentricities(*m_bodies[i], *m_bodies[j]));
				values.push_back({ value, m_bodies[j] });
			}

			std::shared_ptr<CelestialBody> parent;
			std::shared_ptr<CelestialBody> child = m_bodies[i];

			if (!values.empty())
			{
				std::sort(std::begin(values), std::end(values), [](const ValuedBody & a, const ValuedBody & b) { return a.first < b.first; });

				if (values[0].first < PERIODIC_ORBIT_FOCUS_SIGMA)
					parent = values[0].second;
				if (values[0].second->IsStar() && values.size() > 1 && values[1].first < PERIODIC_ORBIT_FOCUS_SIGMA)
					parent = values[1].second;
			}

			childParentMapping[child] = parent;
		}
		m_bodies.pop_back();

		{
			// keep bodies with changed parents locked
			for (auto & mapping : childParentMapping)
			{
				mapping.first->lock();
				mapping.first->SetParent(mapping.second);
			}
			// rebuild all parent data
			for (auto & b : m_bodies)
			{
				std::lock_guard<const simmulation::CelestialBody> lock(*b.get());
				b->RebuildParentRelativeData();
			}

			for (auto & mapping : childParentMapping)
				mapping.first->unlock();
		}
#ifdef DUMP_DATA_FOR_ANALYSIS
		DumpData(data);
#endif
	}

	std::string JsonPathFromName(const char * name)
	{
		return std::string("systems/") + name + ".json";
	}

	std::string DataPathFromName(const char * name)
	{
		return std::string("systems/") + name + ".data";
	}

	oxygine::VectorD2 GetPoint(Json::Value & value)
	{
		oxygine::VectorD2 ret;
		ret.x = value["x"].asDouble();
		ret.y = value["y"].asDouble();
		return ret;
	}

	void SetPoint(Json::Value & value, const oxygine::VectorD2 & point)
	{
		value["x"] = point.x;
		value["y"] = point.y;
	}

	void SolarSystem::LoadBodies(const char * name)
	{
		m_star.reset();
		m_bodies.clear();
		m_name = name;

		// read from file
		oxygine::file::buffer bf;
		if (!oxygine::file::exists(JsonPathFromName(m_name.c_str())))
			return;
		if (!oxygine::file::read(JsonPathFromName(m_name.c_str()), bf))
			return;

		// parse
		Json::Reader reader;
		Json::Value value;
		reader.parse((char*)&bf.front(), (char*)&bf.front() + bf.size(), value, false);

		// read bodies
		for (const std::string & name : value.getMemberNames())
		{
			CelestialBodyDef def;
			def.name = name;
			def.position = scalePosition(GetPoint(value[name]["position"]));
			def.velocity = scaleVelocity(GetPoint(value[name]["velocity"]));
			def.mass = scaleKilograms(value[name]["mass"].asDouble());
			def.radius = scaleSize(value[name]["radius"].asDouble());
			def.isStar = value[name]["star"].asBool();

			if (def.isStar)
				SetStar(def);
			else
				AddBody(def);
		}
	}

	void CopyPoints(const std::vector<BodyData::Point> & src, std::vector<oxygine::VectorD2> & dst)
	{
		dst.clear();
		dst.resize(src.size());
		for (size_t i = 0; i < src.size(); ++i)
			dst[i].set(src[i].x(), src[i].y());
	}

	bool SolarSystem::LoadData()
	{
		oxygine::file::buffer bf;
		if (!oxygine::file::exists(DataPathFromName(m_name.c_str())))
			return false;
		if (!oxygine::file::read(DataPathFromName(m_name.c_str()), bf))
			return false;

		auto data = BodyData::UnPackBodies(bf.getData());

		// validate data
		for (auto & b : data->bodies)
		{
			auto celestial = GetBody(b->name.c_str());
			if (!celestial)
				return false;

			std::lock_guard<const simmulation::CelestialBody> lock(*celestial.get());

			if (!sigmaCompare({ b->positions[0].x(), b->positions[0].y() }, celestial->GetPositions()[0], 0.00001) ||
				!sigmaCompare({ b->momenta[0].x(), b->momenta[0].y() }, celestial->GetMomenta()[0], 0.00001))
				return false;
		}

		for (auto & b : data->bodies)
		{
			auto celestial = GetBody(b->name.c_str());

			std::lock_guard<const simmulation::CelestialBody> lock(*celestial.get());

			std::vector<oxygine::VectorD2> positions, momenta;

			CopyPoints(b->positions, positions);
			CopyPoints(b->momenta, momenta);

			celestial->ClearData();
			celestial->AppendData(positions, momenta);

			auto celestialParent = GetBody(b->parent.c_str());
			if (celestialParent)
				celestial->SetParent(celestialParent);
		}

		if (m_star)
			m_star->RebuildParentRelativeData();
		for (auto & b : m_bodies)
		{
			std::lock_guard<const simmulation::CelestialBody> lock(*b.get());
			b->RebuildParentRelativeData();
		}

		return true;
	}

	std::vector<BodyData::Point> CopyPoints(const std::vector<oxygine::VectorD2> & src, flatbuffers::FlatBufferBuilder & fbb)
	{
		std::vector<BodyData::Point> dst;
		for (const auto & p : src)
			dst.push_back(BodyData::Point(p.x, p.y));
		return dst;
	}

	void SaveBody(flatbuffers::FlatBufferBuilder & fbb, std::vector<flatbuffers::Offset<BodyData::Body>> & data,
		std::shared_ptr<CelestialBody> & b)
	{
		std::lock_guard<const simmulation::CelestialBody> lock(*b.get());

		auto name = fbb.CreateString(b->GetName());
		auto positions = fbb.CreateVectorOfStructs(CopyPoints(b->GetPositions(), fbb));
		auto momenta = fbb.CreateVectorOfStructs(CopyPoints(b->GetMomenta(), fbb));

		flatbuffers::Offset<flatbuffers::String> parent;
		if (b->GetParent())
		{
			std::lock_guard<const simmulation::CelestialBody> lock(*b->GetParent().get());
			parent = fbb.CreateString(b->GetParent()->GetName());
		}

		BodyData::BodyBuilder body(fbb);

		body.add_name(name);
		body.add_positions(positions);
		body.add_momenta(momenta);

		if (b->GetParent())
			body.add_parent(parent);

		data.push_back(body.Finish());
	}

	void SolarSystem::SaveData()
	{
		flatbuffers::FlatBufferBuilder fbb;

		std::vector<flatbuffers::Offset<BodyData::Body>> data;
		for (auto & b : m_bodies)
			SaveBody(fbb, data, b);
		if (m_star)
			SaveBody(fbb, data, m_star);

		auto dataOffset = fbb.CreateVector(data);
		BodyData::BodiesBuilder bodies(fbb);
		bodies.add_bodies(dataOffset);

		fbb.Finish(bodies.Finish());

		oxygine::file::makeDirectory("systems");
		oxygine::file::write(DataPathFromName(m_name.c_str()), fbb.GetBufferPointer(), fbb.GetSize(), oxygine::ep_show_warning);
	}

	void SolarSystem::WriteBody(const CelestialBodyDef & def)
	{
		Json::Value value;
		oxygine::file::buffer bf;
		if (oxygine::file::read(JsonPathFromName(m_name.c_str()), bf))
		{
			Json::Reader reader;
			reader.parse((char*)&bf.front(), (char*)&bf.front() + bf.size(), value, false);
		}

		SetPoint(value[def.name]["position"], scalePositionInv(def.position));
		SetPoint(value[def.name]["velocity"], scaleVelocityInv(def.velocity));
		value[def.name]["mass"] = scaleKilogramsInv(def.mass);
		value[def.name]["radius"] = scaleSizeInv(def.radius);
		value[def.name]["star"] = def.isStar;

		auto data = value.toStyledString();

		oxygine::file::makeDirectory("systems");
		oxygine::file::write(JsonPathFromName(m_name.c_str()), data.data(), data.size(), oxygine::ep_show_warning);
	}

	double ComputeHillSphereRadius(double R, double m, double M)
	{
		return R*pow(m / (3.0*M), 1.0 / 3.0);
	}
}
