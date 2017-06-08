/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _CORE_INSTANTIATION_H_
#define _CORE_INSTANTIATION_H_
#pragma once

#include <string>
#include <memory>

/** available core types*/
enum class core_types
{
	default_core,  //!< automatic selection
	zmq_core,	//!< use the zmq_core type
	mpi_core,	//!< use the mpi_core type
	test_core,  //!< use the test_core type
};

core_types coreTypeFromString(const std::string &type);
/** forward declare the Core object*/
namespace helics
{
	class Core;
}

/** initialize a named core interface
@param[in] name  the name of the core interface -leave empty for the default
@param[in] type the type of core to create  either core_types::zmq_core for a zmq based core
or core_types::mpi_core for the mpi based core using core_types::default lets the system pick
@param[in] initialization_string  a string of arguments for the core initialization
@return a shared_ptr to a helics core
*/
std::shared_ptr<helics::Core> initializeCore(std::string name, core_types type=core_types::default_core, const std::string &initializion_string="");

/** check if a core is available*/
bool isAvailable(const std::string &name);

/** get a shared_ptr to a core object
@param[in] name  the name of the core to retrieve
@return a shared_ptr to the core object, the resulting shared ptr is empty if the requested name is not available
*/
std::shared_ptr<helics::Core> getCore(const std::string &name);

/** close the named core interface for new Federates
@details this does not destroy the core for Federates that are already using it, only removes its ability to accept new federates
*/
void closeCore(const std::string &name);


#endif