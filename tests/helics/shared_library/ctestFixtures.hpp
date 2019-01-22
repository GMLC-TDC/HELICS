/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include <vector>
#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include "../coreTypeLists.hpp"
#include "helics/chelics.h"
#include "helics/helics-config.h"

#define CE(command)                                                                                               \
    helicsErrorClear (&err);                                                                                      \
    command;                                                                                                      \
    BOOST_CHECK_MESSAGE (err.error_code == helics_ok, err.message)

#define HELICS_SIZE_MAX 512

typedef helics_federate (*FedCreator) (char const *, helics_federate_info, helics_error *err);

struct FederateTestFixture
{
    FederateTestFixture ();
    ~FederateTestFixture ();

    helics_broker AddBroker (const std::string &core_type_name, int count);
    helics_broker AddBroker (const std::string &core_type_name, const std::string &initialization_string);

    void SetupTest (FedCreator ctor,
                    const std::string &core_type_name,
                    int count,
                    helics_time time_delta = helics_time_zero,
                    const std::string &name_prefix = "fed");

    std::vector<helics_federate> AddFederates (FedCreator ctor,
                                               std::string core_type_name,
                                               int count,
                                               helics_broker broker,
                                               helics_time time_delta = helics_time_zero,
                                               const std::string &name_prefix = "fed");

    helics_federate GetFederateAt (int index);

    std::vector<helics_broker> brokers;
    std::vector<helics_federate> federates;
    std::string extraCoreArgs;
    std::string extraBrokerArgs;
    helics_error err;

    std::string ctype;

  private:
    bool hasIndexCode (const std::string &type_name);
    int getIndexCode (const std::string &type_name);
};
