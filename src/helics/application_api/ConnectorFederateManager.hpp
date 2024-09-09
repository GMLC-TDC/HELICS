/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "../common/GuardedTypes.hpp"
#include "Filters.hpp"
#include "Translator.hpp"
#include "gmlc/containers/StringMappedVector.hpp"

#include <memory>
#include <string>

namespace helics {
class Core;
class Federate;
/** class handling the implementation details for managing connectors(Filters and Translators)
@details the functions match those in Federate.hpp dealing with filters and Translators and contain
the actual implementation details
*/
class ConnectorFederateManager {
  public:
    /** construct from a pointer to a core and a specified federate id
     */
    ConnectorFederateManager(Core* coreObj,
                             Federate* fFed,
                             LocalFederateId fid,
                             bool singleThreaded);
    /** destructor */
    ~ConnectorFederateManager();

    /** register a Filter
    @details call is only valid in startup mode
    @param name the name of the endpoint
    @param type_in the type the filter is expecting as an input
    @param type_out the type the filter generates
    */
    Filter&
        registerFilter(std::string_view name, std::string_view type_in, std::string_view type_out);

    /** register a cloningFilter
    @details call is only valid in startup mode
    @param name the name of the endpoint
    @param type_in the type the filter is expecting as an input
    @param type_out the type the filter generates
    */
    CloningFilter& registerCloningFilter(std::string_view name,
                                         std::string_view type_in,
                                         std::string_view type_out);

    /** register a Filter
    @details call is only valid in startup mode
    @param type the defined type of the filter
    @param name the name of the filter
    */
    Filter& registerFilter(FilterTypes type, std::string_view name);

    /** register a cloningFilter
    @details call is only valid in startup mode
    @param name the name of the endpoint
    @param type the defined type of the interface for endpoint checking if requested
    */
    CloningFilter& registerCloningFilter(FilterTypes type, std::string_view name);

    /** get a registered Filter
    @param name the publication id
    @return ivalid_publication_id if name is not recognized otherwise returns the publication_id*/
    Filter& getFilter(std::string_view name);
    const Filter& getFilter(std::string_view name) const;
    Filter& getFilter(int index);
    const Filter& getFilter(int index) const;

    /** register a Translator
    @details call is only valid in startup mode
    @param name the name of the translator
    @param endpointType is the type of the endpoints
    @param units is the units associated with the value interface
    */
    Translator& registerTranslator(std::string_view name,
                                   std::string_view endpointType,
                                   std::string_view units);

    /** get a registered Translator
    @param name the translator name
    @return invalid translator object if name is not recognized otherwise returns the translator*/
    Translator& getTranslator(std::string_view name);
    const Translator& getTranslator(std::string_view name) const;
    Translator& getTranslator(int index);
    const Translator& getTranslator(int index) const;

    /** get the number of registered filters in the federate*/
    int getFilterCount() const;
    /** get the number of registered filters in the federate*/
    int getTranslatorCount() const;
    /** close all filters*/
    void closeAllConnectors();
    /** close all filters*/
    void disconnectAllConnectors();
    /** disconnect from the coreObject*/
    void disconnect();

  private:
    Core* coreObject{nullptr};
    shared_guarded_opt<gmlc::containers::StringMappedVector<std::unique_ptr<Filter>>> filters;
    shared_guarded_opt<gmlc::containers::StringMappedVector<Translator>> translators;
    Federate* fed = nullptr;  //!< pointer back to the message Federate
    const LocalFederateId fedID;  //!< storage for the federate ID
};
}  // namespace helics
