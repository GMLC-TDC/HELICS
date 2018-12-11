/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "../common/GuardedTypes.hpp"
#include "../common/MappedVector.hpp"
#include "../core/Core.hpp"
#include "Filters.hpp"
#include <cstdint>
#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <utility>
namespace helics
{
class Core;
class Federate;
/** class handling the implementation details of a value Federate
@details the functions will parallel those in message Federate and contain the actual implementation details
*/
class FilterFederateManager
{
  public:
    /** construct from a pointer to a core and a specified federate id
     */
    FilterFederateManager (Core *coreObj, Federate *fFed, federate_id_t id);
    ~FilterFederateManager ();
    /** register a Filter
    @details call is only valid in startup mode
    @param[in] name the name of the endpoint
    @param[in] type the defined type of the interface for endpoint checking if requested
    */
    Filter &registerFilter (const std::string &name, const std::string &type_int, const std::string &type_out);

    /** register a cloningFilter
    @details call is only valid in startup mode
    @param[in] name the name of the endpoint
    @param[in] type the defined type of the interface for endpoint checking if requested
    */
    CloningFilter &
    registerCloningFilter (const std::string &name, const std::string &type_int, const std::string &type_out);

    /** register a Filter
    @details call is only valid in startup mode
    @param[in] name the name of the endpoint
    @param[in] type the defined type of the interface for endpoint checking if requested
    */
    Filter &registerFilter (filter_types type, const std::string &name);

    /** register a cloningFilter
    @details call is only valid in startup mode
    @param[in] name the name of the endpoint
    @param[in] type the defined type of the interface for endpoint checking if requested
    */
    CloningFilter &registerCloningFilter (filter_types type, const std::string &name);

    /** get a registered Filter
    @param[in] name the publication id
    @return ivalid_publication_id if name is not recognized otherwise returns the publication_id*/
    Filter &getFilter (const std::string &name);
    const Filter &getFilter (const std::string &name) const;
    Filter &getFilter (int index);
    const Filter &getFilter (int index) const;

    /** get the number of registered filters in the federate*/
    int getFilterCount () const;

  private:
    Core *coreObject = nullptr;
    shared_guarded<MappedVector<std::unique_ptr<Filter>, std::string>> filters;
    Federate *fed = nullptr;  //!< pointer back to the message Federate
    const federate_id_t fedID;  //!< storage for the federate ID
};
}  // namespace helics
