/*

Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _ARG_PARSER_
#define _ARG_PARSER_

/**
@file common function for parsing command line arguments for core and brokers
*/

#include <string>
#include <vector>
#include <boost/program_options/variables_map.hpp>

namespace helics
{
/** class to contain a descriptor for a command line argument*/
class ArgDescriptor
{
  public:
      /** enumeration of possible types of the arguments*/
      enum class arg_type_t :int
      {
          string_type,
          flag_type, 
          double_type,
          int_type,
          vector_string,
          vector_double,
      };
    std::string arg_;  //!< a key for the argument
    arg_type_t type_=arg_type_t::string_type;  //!< the type of argument that is expected
    std::string desc_;  //!< a help file description
    ArgDescriptor () = default;
    ArgDescriptor (std::string arg, arg_type_t type, std::string desc)
        : arg_ (std::move (arg)), type_ (type), desc_ (std::move (desc))
    {
        
    }
    /** two argument constructor assuming the results will be strings*/
    ArgDescriptor(std::string arg, std::string desc)
        : arg_(std::move(arg)), desc_(std::move(desc))
    {
    }
};
using ArgDescriptors = std::vector<ArgDescriptor>;

using variable_map = boost::program_options::variables_map;
constexpr int helpReturn(-1);
constexpr int versionReturn(-2);

/** using boost argument_parser to process a set of command line arguments
@param[in] argc the number of arguments
@param[in] argv the actual strings in the argument
@param[out] vm_map the variable_map of all the arguments
@param[in] argDefinitions the definition targets used for the processing
@param[in] posName, (optional) the name of a positional argument
@return helpReturn(-1) if the help argument was called and displayed, versionReturn(-2) if version argument was used 0 otherwise
*/
int argumentParser (int argc,
                     const char *const *argv,
                        variable_map &vm_map,
                     const ArgDescriptors &argDefinitions, const std::string &posName=std::string());

}

#endif  //_ARG_PARSER_
