/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef TEST_FIXTURES_HEADER_
#define TEST_FIXTURES_HEADER_

#include <memory>

#include "helics/core/CoreBroker.h"
#include "helics/application_api/MessageFederate.h"
#include "helics/application_api/ValueFederate.h"

struct ValueFederateTestFixture
{
	ValueFederateTestFixture() = default;
	~ValueFederateTestFixture();

    void Setup1FederateTest(std::string core_type_name, helics::Time time_delta = helics::timeZero);
    void Setup2FederateTest(std::string core_type_name, helics::Time time_delta = helics::timeZero);

    void StartBroker(const std::string &core_type_name, const std::string &initialization_string);

    std::shared_ptr<helics::CoreBroker> broker;
    std::shared_ptr<helics::ValueFederate> vFed1;
    std::shared_ptr<helics::ValueFederate> vFed2;
};

struct MessageFederateTestFixture
{
    MessageFederateTestFixture() = default;
    ~MessageFederateTestFixture();

    void Setup1FederateTest(const std::string &core_type_name);
    void Setup2FederateTest(const std::string &core_type_name);

    void StartBroker(const std::string &core_type_name, const std::string &initialization_string);

    std::shared_ptr<helics::CoreBroker> broker;
    std::shared_ptr<helics::MessageFederate> mFed1;
    std::shared_ptr<helics::MessageFederate> mFed2;
};


struct MultipleValueFederateTestFixture
{
	MultipleValueFederateTestFixture() = default;
	~MultipleValueFederateTestFixture();

	void SetupFederateTests(std::string core_type_name, int cnt, helics::Time time_delta = helics::timeZero);

	void StartBroker(const std::string &core_type_name, const std::string &initialization_string);

	std::shared_ptr<helics::CoreBroker> broker;
	std::vector<std::shared_ptr<helics::ValueFederate>> federates;
};
#endif