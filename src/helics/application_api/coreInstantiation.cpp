/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

#include "coreInstantiation.h"
#include "helics/core/core-factory.h"
#include "helics/core/core.h"
#include <map>
#include <mutex>

/** a storage system for the available core objects allowing references by name to the core
 */
static std::map<std::string, std::weak_ptr<helics::Core>> availableCores;
/** map of the types of the named cores*/
static std::map<std::string, helics_core_type> namedCoreType;
/** we expect operations on core object that modify the map to be rare but we absolutely need them to be thread
 safe so we are going to use a lock that is entirely controlled by this file*/
static std::mutex mapLock;

/** convert the application_api core type definitions to the ones used by the core factory
 */
helics_core_type gethcType (core_types type)
{
    switch (type)
    {
    case core_types::mpi_core:
        return HELICS_MPI;
    case core_types::test_core:
        return HELICS_TEST;
    case core_types::zmq_core:
    default:
        return HELICS_ZMQ;
    }
}

std::string helicsTypeString (helics_core_type type)
{
    switch (type)
    {
    case HELICS_MPI:
        return "_mpi";
    case HELICS_TEST:
        return "_test";
    case HELICS_ZMQ:
        return "_zmq";
    default:
        return "";
    }
}

core_types coreTypeFromString (const std::string &type)
{
    if ((type.empty ()) || (type == "default"))
    {
        return core_types::default_core;
    }
    else if ((type == "mpi") || (type == "MPI"))
    {
        return core_types::mpi_core;
    }
    else if ((type == "0mq") || (type == "zmq") || (type == "zeromq")||(type=="ZMQ"))
    {
        return core_types::zmq_core;
    }
    else if ((type == "test") || (type == "test1")||(type=="local"))
    {
        return core_types::test_core;
    }
    throw (std::invalid_argument ("unrecognized core type"));
}

std::shared_ptr<helics::Core>
initializeCore (std::string name, core_types type, const std::string &initialization_string)
{
    if (type == core_types::default_core)
    {
        if (helics::CoreFactory::available (HELICS_ZMQ))
        {
            type = core_types::zmq_core;
        }
        else if (helics::CoreFactory::available (HELICS_MPI))
        {
            type = core_types::mpi_core;
        }
        else
        {
            type = core_types::test_core;
        }
    }
    auto hctype = gethcType (type);
	bool emptyNameFlag = false;
	if (name.empty())
	{
		emptyNameFlag = true;
		name = helicsTypeString(hctype);
	}
    std::lock_guard<std::mutex> corelock (mapLock);
    auto fnd = availableCores.find (name);
    if (fnd == availableCores.end ())
    {
        if (helics::CoreFactory::available (hctype))
        {
            auto newCore =
              std::shared_ptr<helics::Core> (helics::CoreFactory::create (hctype, initialization_string.c_str ()));
            availableCores.emplace (name, newCore);
            namedCoreType.emplace (name, hctype);
			if (emptyNameFlag)
			{
				auto fndE = namedCoreType.find("");
				if (fndE != namedCoreType.end())
				{
					availableCores.emplace("", newCore);
					namedCoreType.emplace("", hctype);
				}
			}
            return newCore;
        }
        else
        {
            throw (std::invalid_argument ("core type not available"));
        }
    }
    else  // now we know it is in the map already
    {
        if (fnd->second.expired ())
        {
            availableCores.erase (fnd);  // erase the expired version
                                         // make a new one that is not expired
            auto cType = namedCoreType[name];
            auto newCore =
              std::shared_ptr<helics::Core> (helics::CoreFactory::create (cType, initialization_string.c_str ()));
            availableCores.emplace (name, newCore);
            return newCore;
        }
        else
        {
            try
            {
                return std::shared_ptr<helics::Core> (fnd->second);  // make a shared_ptr from the weak_ptr
            }
            catch (const std::bad_weak_ptr &)
            {
                availableCores.erase (fnd);  // erase the expired version
                                             // make a new one that is not expired
                auto newCore = std::shared_ptr<helics::Core> (
                  helics::CoreFactory::create (hctype, initialization_string.c_str ()));
                availableCores.emplace (name, newCore);
                return newCore;
            }
        }
    }
}


/** check if a core is available*/
bool isAvailable (const std::string &name)
{
    // don't need lock on this one since nothing can be modified and doesn't matter if it is in the process
    // of being constructed and if it fails inapropriately it will just return appropriately when the subsequant call to make it is made
    auto fnd = availableCores.find (name);
    return (fnd != availableCores.end ());
}

/** get a shared_ptr to a core object
@param[in] name  the name of the core to retrieve
@return a shared_ptr to the core object, the resulting shared ptr is empty if the requested name is not available
*/
std::shared_ptr<helics::Core> getCore (const std::string &name)
{
    std::lock_guard<std::mutex> corelock (
      mapLock);  // just to ensure that nothing funny happens if you try to get a core
    // while it is being constructed
    auto fnd = availableCores.find (name);
    if (fnd != availableCores.end ())
    {
        try
        {
            return std::shared_ptr<helics::Core> (fnd->second);
        }
        catch (const std::bad_weak_ptr &)
        {
            return std::shared_ptr<helics::Core> ();
        }
    }
    else
    {  // return an empty shared_ptr
        return std::shared_ptr<helics::Core> ();
    }
}

/** close the named core interface for new Federates
@details this does not destroy the core for Federates that are already using it, only removes its ability to accept
new federates
*/
void closeCore(const std::string &name)
{
	std::lock_guard<std::mutex> corelock(mapLock);
	auto fnd = availableCores.find(name);
	if (fnd != availableCores.end())
	{
		availableCores.erase(fnd);
		if (name.empty())
		{
			auto cType = namedCoreType[name];
			auto tString = helicsTypeString(cType);
			auto fnd2 = availableCores.find(tString);
			if (fnd2 != availableCores.end())
			{
				availableCores.erase(fnd2);
				namedCoreType.erase(tString);
			}
			namedCoreType.erase(name);
		}

	}

}
