/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _ARG_PARSER_
#define _ARG_PARSER_

/**
@file common function for parsing command line arguments for core and brokers
*/

#include <boost/program_options.hpp>
#include <tuple>
#include <vector>

namespace helics
{
using argDescriptors = std::vector<std::tuple<std::string, std::string, std::string>>;

void argumentParser(int argc, char *argv[], boost::program_options::variables_map &vm_map, const argDescriptors &additionalArgs);
}

#endif //_ARG_PARSER_
