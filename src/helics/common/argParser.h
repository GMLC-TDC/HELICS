/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

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
    enum class arg_type_t : int
    {
        string_type,  //!< descriptor of a string argument
        flag_type,  //!< descriptor of a flag argument implying no values, just a flag
        double_type,  //!< descriptor for a floating point argument
        int_type,  //!< descriptor for an integer argument
        vector_string,  //!< descriptor for multiple string arguments
        vector_double,  //!< descriptor for multiple double arguments
    };
    std::string arg_;  //!< a key for the argument
    arg_type_t type_ = arg_type_t::string_type;  //!< the type of argument that is expected
    std::string desc_;  //!< a help file description
    ArgDescriptor () = default;
    /** constructor that takes a key, a type and a description string
    @param arg the key for the flag of the for "key", or "key,k" if a short argument is desired
    @param type the type of the argument
    @param desc a descriptor displayed in the help window for a string*/
    ArgDescriptor (std::string arg, arg_type_t type, std::string desc)
        : arg_ (std::move (arg)), type_ (type), desc_ (std::move (desc))
    {
    }
    /** two argument constructor assuming the results will be strings
    @param arg the key for the parameter
    @param desc a description of the argument displayed for help*/
    ArgDescriptor (std::string arg, std::string desc) : arg_ (std::move (arg)), desc_ (std::move (desc)) {}
};
/** convenience alias*/
using ArgDescriptors = std::vector<ArgDescriptor>;

using variable_map = boost::program_options::variables_map;
/** return value if the help argument is called*/
constexpr int helpReturn (-1);
/** return argument if a version argument was detected*/
constexpr int versionReturn (-2);

/** using boost argument_parser to process a set of command line arguments
@param argc the number of arguments
@param argv the actual strings in the argument
@param[out] vm_map the variable_map of all the arguments
@param argDefinitions the definition targets used for the processing
@param posName  (optional) the name of a positional argument
@return helpReturn(-1) if the help argument was called and displayed, versionReturn(-2) if version argument was
used 0 otherwise
*/
int argumentParser (int argc,
                    const char *const *argv,
                    variable_map &vm_map,
                    const ArgDescriptors &argDefinitions,
                    const std::string &posName = std::string ());

}  // namespace helics
