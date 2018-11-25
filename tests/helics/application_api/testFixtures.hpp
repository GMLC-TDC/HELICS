/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "helics/application_api/Federate.hpp"
#include "helics/core/BrokerFactory.hpp"
#include "helics/core/Core.hpp"
#include "helics/core/CoreFactory.hpp"
#include <iostream>
#include <memory>
#include <stdexcept>

extern const std::vector<std::string> ztypes;
extern const std::vector<std::string> core_types;

extern const std::vector<std::string> core_types_2;

extern const std::vector<std::string> core_types_simple;
extern const std::vector<std::string> core_types_single;
extern const std::vector<std::string> core_types_all;
extern const std::vector<std::string> core_types_extended;

extern const std::string defaultNamePrefix;

struct FederateTestFixture
{
    FederateTestFixture () = default;
    ~FederateTestFixture ();

    std::shared_ptr<helics::Broker> AddBroker (const std::string &core_type_name, int count);
    std::shared_ptr<helics::Broker>
    AddBroker (const std::string &core_type_name, const std::string &initialization_string);

    template <class FedType>
    void SetupTest (const std::string &core_type_name,
                    int count,
                    helics::Time time_delta = helics::timeZero,
                    const std::string &name_prefix = defaultNamePrefix)
    {
        ctype = core_type_name;
        auto broker = AddBroker (core_type_name, count);

        if (!broker->isConnected ())
        {
            broker->disconnect ();
            broker = nullptr;
            helics::cleanupHelicsLibrary ();
            broker = AddBroker (core_type_name, count);
            if (!broker->isConnected ())
            {
                throw (std::runtime_error ("Unable to connect rootbroker"));
            }
        }
        AddFederates<FedType> (core_type_name, count, broker, time_delta, name_prefix);
    }

    template <class FedType>
    std::vector<std::shared_ptr<FedType>> AddFederates (std::string core_type_name,
                                                        int count,
                                                        std::shared_ptr<helics::Broker> broker,
                                                        helics::Time time_delta = helics::timeZero,
                                                        const std::string &name_prefix = defaultNamePrefix)
    {
        bool hasIndex = hasIndexCode (core_type_name);
        int setup = (hasIndex) ? getIndexCode (core_type_name) : 1;
        if (hasIndex)
        {
            core_type_name.pop_back ();
            core_type_name.pop_back ();
        }

        std::string initString =
          std::string ("--broker=") + broker->getIdentifier () + " --broker_address=" + broker->getAddress ();

        if (!extraCoreArgs.empty ())
        {
            initString.push_back (' ');
            initString.append (extraCoreArgs);
        }

        helics::FederateInfo fi (helics::coreTypeFromString (core_type_name));
        if (time_delta != helics::timeZero)
        {
            fi.setTimeProperty (helics_property_time_delta, time_delta);
        }

        std::vector<std::shared_ptr<FedType>> federates_added;

        switch (setup)
        {
        case 1:
        default:
        {
            auto core_type = helics::coreTypeFromString (core_type_name);
            auto core =
              helics::CoreFactory::create (core_type, initString + " --federates " + std::to_string (count));
            fi.coreName = core->getIdentifier ();
            size_t offset = federates.size ();
            federates.resize (count + offset);
            for (int ii = 0; ii < count; ++ii)
            {
                auto fedname = name_prefix + std::to_string (ii + offset);
                auto fed = std::make_shared<FedType> (fedname, fi);
                federates[ii + offset] = fed;
                federates_added.push_back (fed);
            }
        }
        break;
        case 2:
        {  // each federate has its own core
            auto core_type = helics::coreTypeFromString (core_type_name);
            size_t offset = federates.size ();
            federates.resize (count + offset);
            for (int ii = 0; ii < count; ++ii)
            {
                auto core = helics::CoreFactory::create (core_type, initString + " --federates 1");
                fi.coreName = core->getIdentifier ();

                auto fedname = name_prefix + std::to_string (ii + offset);
                auto fed = std::make_shared<FedType> (fedname, fi);
                federates[ii + offset] = fed;
                federates_added.push_back (fed);
            }
        }
        break;
        case 3:
        {
            auto subbroker = AddBroker (core_type_name, initString + " --federates " + std::to_string (count));
            if (!subbroker->isConnected ())
            {
                throw (std::runtime_error ("Unable to connect subbroker"));
            }
            auto newTypeString = core_type_name;
            newTypeString.push_back ('_');
            newTypeString.push_back ('2');
            for (int ii = 0; ii < count; ++ii)
            {
                AddFederates<FedType> (newTypeString, 1, subbroker, time_delta, name_prefix);
            }
        }
        break;
        case 4:
        {
            auto newTypeString = core_type_name;
            newTypeString.push_back ('_');
            newTypeString.push_back ('2');
            for (int ii = 0; ii < count; ++ii)
            {
                auto subbroker = AddBroker (core_type_name, initString + " --federates 1");
                if (!subbroker->isConnected ())
                {
                    throw (std::runtime_error ("Unable to connect subbroker(mode 4)"));
                }
                AddFederates<FedType> (newTypeString, 1, subbroker, time_delta, name_prefix);
            }
        }
        break;
        case 5:  // pairs of federates per core
        {
            auto core_type = helics::coreTypeFromString (core_type_name);
            size_t offset = federates.size ();
            federates.resize (count + offset);
            for (int ii = 0; ii < count; ii += 2)
            {
                auto core = helics::CoreFactory::create (core_type, initString + " --federates " +
                                                                      std::to_string ((ii < count - 1) ? 2 : 1));
                fi.coreName = core->getIdentifier ();

                auto fedname = name_prefix + std::to_string (ii + offset);
                auto fed = std::make_shared<FedType> (fedname, fi);
                federates[ii + offset] = fed;
                federates_added.push_back (fed);
                if (ii + 1 < count)
                {
                    auto fedname2 = name_prefix + std::to_string (ii + offset + 1);
                    auto fed2 = std::make_shared<FedType> (fedname2, fi);
                    federates[ii + offset + 1] = fed2;
                    federates_added.push_back (fed2);
                }
            }
        }
        break;
        case 6:  // pairs of cores per subbroker
        {
            auto newTypeString = core_type_name;
            newTypeString.push_back ('_');
            newTypeString.push_back ('5');
            for (int ii = 0; ii < count; ii += 4)
            {
                int fedcnt = (ii > count - 3) ? 4 : (count - ii);
                auto subbroker =
                  AddBroker (core_type_name, initString + " --federates " + std::to_string (fedcnt));
                if (!subbroker->isConnected ())
                {
                    throw (std::runtime_error ("Unable to connect subbroker(mode 4)"));
                }
                AddFederates<FedType> (newTypeString, fedcnt, subbroker, time_delta, name_prefix);
            }
        }
        break;
        case 7:  // two layers of subbrokers
        {
            auto newTypeString = core_type_name;
            newTypeString.push_back ('_');
            newTypeString.push_back ('4');
            for (int ii = 0; ii < count; ++ii)
            {
                auto subbroker = AddBroker (core_type_name, initString + " --federates 1");
                if (!subbroker->isConnected ())
                {
                    throw (std::runtime_error ("Unable to connect subbroker(mode 4)"));
                }
                AddFederates<FedType> (newTypeString, 1, subbroker, time_delta, name_prefix);
            }
        }
        break;
        }

        return federates_added;
    }

    template <class FedType>
    std::shared_ptr<FedType> GetFederateAs (int index)
    {
        return std::dynamic_pointer_cast<FedType> (federates[index]);
    }

    void FullDisconnect ();

    std::vector<std::shared_ptr<helics::Broker>> brokers;
    std::vector<std::shared_ptr<helics::Federate>> federates;
    std::string extraCoreArgs;
    std::string extraBrokerArgs;
    std::string ctype;

  private:
    bool hasIndexCode (const std::string &type_name);
    int getIndexCode (const std::string &type_name);
};
