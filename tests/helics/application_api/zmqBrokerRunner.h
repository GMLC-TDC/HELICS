/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef BROKER_RUNNER_HELPER_H_
#define BROKER_RUNNER_HELPER_H_
#pragma once

#include <string>
/** class designed to run the ZMQ BROKER*/
class BrokerRunner
{
private:
	std::string exeString;
	bool active;
	static int counter;
	std::string outFile;
public:
	BrokerRunner();
	BrokerRunner(const std::string &baseLocation, const std::string &target);
	BrokerRunner(const std::string &baseLocation, const std::string &baseLocation2, const std::string &target);
	bool findFileLocation(const std::string &baseLocation, const std::string &target);
	bool isActive() const { return active; };

	int run(const std::string &args) const;

	std::pair<int, std::string> runCaptureOutput(const std::string &args) const;
	const std::string &getExeString() const
	{
		return exeString;
	}
private:
	void buildOutFile();
};

#endif //BROKER_RUNNER_HELPER_H_