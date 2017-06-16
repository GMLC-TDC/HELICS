/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include <boost/test/floating_point_comparison.hpp>
#include <boost/test/unit_test.hpp>


#include "helics/application_api/ValueFederate.h"
#include "helics/application_api/coreInstantiation.h"
#include "helics/application_api/Subscriptions.hpp"

#include "test_configuration.h"
#include <thread>
#include <fstream>

using helics::operator "" _t;
helics::Time tend = 3600.0_t; //simulation end time

class HeatUnitBlock
{
public:
	double ThermalCap = 4.0; //thermal capacity
	double tRate = 0.05; //rate of energy transfer per degree
	int x = 0;
	int y = 0;
	
	double T = 25.0; //temperature
	helics::Time deltaTime=5.0; //sampling rate
private:
	std::unique_ptr<helics::ValueFederate> vFed;
	helics::publication_id_t pub;
	helics::subscription_id_t sub[4];
	bool initialized=false;
public:
	HeatUnitBlock() = default;
	
	void run(const std::string &coreName="")
	{
		if (!initialized)
		{
			initialize(coreName);
		}
		
		vFed->enterInitializationState();
		vFed->publish(pub, T);
		vFed->enterExecutionState();
		mainLoop();
	};
	void initialize(const std::string &coreName)
	{
		std::string name = "heatUnit_(" + std::to_string(x) + "," + std::to_string(y) + ")";
		helics::FederateInfo fi(name);
		fi.coreName = coreName;
		fi.timeDelta = deltaTime;
		vFed=std::make_unique<helics::ValueFederate>(fi);
		pub = vFed->registerPublication<double>("temp", x, y);
		if (x - 1 < 0)
		{
			sub[0] = vFed->registerRequiredSubscription<double>("temp_wall");
		}
		else
		{
			sub[0] = vFed->registerRequiredSubscription<double>("temp",x-1,y);
		}
		vFed->setDefaultValue(sub[0], T);
		sub[1] = vFed->registerOptionalSubscription<double>("temp", x + 1, y);
		vFed->setDefaultValue(sub[1], -512.0);
		sub[2] = vFed->registerOptionalSubscription<double>("temp", x, y-1);
		vFed->setDefaultValue(sub[2], -512.0);
		sub[3] = vFed->registerOptionalSubscription<double>("temp", x, y+1);
		vFed->setDefaultValue(sub[3], -512.0);
		initialized = true;
	}

	void mainLoop()
	{
		auto cTime = 0.0_t;
		while (cTime < tend)
		{
			double T0 = vFed->getValue<double>(sub[0]);

			double Etransfer = (T0 - T)*tRate*deltaTime;

			for (int ii = 1; ii < 3; ++ii)
			{
				double TT = vFed->getValue<double>(sub[ii]);
				if (TT > -500)
				{
					Etransfer += (TT - T)*tRate*deltaTime;
				}
			}
			double deltaT = Etransfer / ThermalCap;
			T = T + deltaT;
			vFed->publish(pub, T);
			cTime = vFed->requestTime(cTime + deltaTime);
		}
		vFed->finalize();
	}
};


class Wall
{
	double Temp = 80; //thermal capacity
	std::vector<std::pair<helics::Time, double>>schedTemp;
private:
	std::unique_ptr<helics::ValueFederate> vFed;
	helics::publication_id_t pub;
	int index = 0;
	bool initialized = false;
public:
	Wall() = default;

	void run(const std::string &coreName="")
	{
		if (!initialized)
		{
			initialize(coreName);
		}
		vFed->enterInitializationState();
		vFed->publish(pub, Temp);
		vFed->enterExecutionState();
		vFed->publish(pub, Temp);
		mainLoop();
	};
	void initialize(const std::string &coreName)
	{
		std::string name = "Wall";
		helics::FederateInfo fi(name);
		fi.coreName = coreName;
		vFed = std::make_unique<helics::ValueFederate>(fi);
		pub = vFed->registerGlobalPublication<double>("temp_wall");
		initialized = true;
	}

	void addTemp(helics::Time timechange, double newTemp)
	{
		schedTemp.emplace_back(timechange, newTemp);
	}
	void mainLoop()
	{
		auto nextTime = tend;
		helics::Time retTime = 0;
		if (!schedTemp.empty())
		{
			nextTime = schedTemp[0].first;
		}
		do
		{
			retTime=vFed->requestTime(nextTime);
			if (index < static_cast<int>(schedTemp.size()))
			{
				vFed->publish(pub, schedTemp[index].second);
				nextTime = schedTemp[index].first;
				++index;
				
			}
			else
			{
				nextTime = tend;
			}
			
		}
		while (retTime < tend);
		vFed->finalize();
	}
};

class observer
{
public:
	std::vector<std::vector<double>> vals;
	std::vector<helics::Time> times;
private:
	std::unique_ptr<helics::ValueFederate> vFed;
	helics::VectorSubscription2d<double> vSub;
	std::string subName;
	int index = 0;
	int m_count = 20;
	bool initialized;
public:
	observer(std::string name, int count):subName(std::move(name)),m_count(count)
	{

	}
	void run(const std::string &coreName="")
	{
		if (!initialized)
		{
			initialize(coreName);
		}
		vFed->enterExecutionState();
		mainLoop();
	};
	void initialize(const std::string &coreName)
	{
		std::string name = "observer";
		helics::FederateInfo fi(name);
		fi.coreName = coreName;
		fi.observer = true;
		fi.timeDelta = 10.0;
		vFed = std::make_unique<helics::ValueFederate>(fi);
		vSub = helics::VectorSubscription2d<double>(vFed.get(), subName, 0, m_count,0,1,0.0);
		initialized = true;
	}

	void mainLoop()
	{
		helics::Time nextTime = 0.0;
		do
		{
			vals.push_back(vSub.getVals());
			times.push_back(nextTime);
			++index;
			nextTime=vFed->requestTime(nextTime+10.0);
			

		} while (nextTime < tend);
		vFed->finalize();
	}
	void saveFile(const std::string &fileName) const
	{
		std::ofstream out(fileName);
		for (int ii = 0; ii < static_cast<int>(times.size()); ++ii)
		{
			out << static_cast<double>(times[ii]) ;
			for (auto &val : vals[ii])
			{
				out << ", " << val;
			}
			out << '\n';
		}
	}
};

BOOST_AUTO_TEST_SUITE(heat_tranfer_tests)

BOOST_AUTO_TEST_CASE(linear_tests)
{
	auto wcore = initializeCore("wallcore", core_types::test_core, "22");
	Wall w;
	w.initialize("wallcore");
	int blockCount = 20;
	std::vector<HeatUnitBlock> block(blockCount);
	for (int ii = 0; ii < blockCount; ++ii)
	{
		block[ii].x = ii;
		block[ii].initialize("wallcore");
	}
	observer obs("temp", blockCount);
	obs.initialize("wallcore");

	std::vector<std::thread> threads(blockCount+2);
	for (int ii = 0; ii < blockCount; ++ii)
	{
		threads[ii] = std::thread([](HeatUnitBlock &blk) {blk.run(); },std::ref(block[ii]));
	}
	threads[blockCount] = std::thread([&]() {obs.run(); });
	threads[blockCount+1] = std::thread([&]() {w.run(); });
	for (auto &thrd:threads)
	{
		thrd.join();
	}
	obs.saveFile("tempData.csv");
}

BOOST_AUTO_TEST_SUITE_END()