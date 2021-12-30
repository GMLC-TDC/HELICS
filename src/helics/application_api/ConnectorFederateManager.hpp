/*
Copyright (c) 2017-2022,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "../common/GuardedTypes.hpp"
#include "Filters.hpp"
#include "Translator.hpp"
#include "gmlc/containers/MappedVector.hpp"

#include <memory>
#include <string>

namespace helics {
class Core;
class Federate;
/** class handling the implementation details of a value Federate
@details the functions will parallel those in message Federate and contain the actual implementation
details
*/
class ConnectorFederateManager {
  public:
    /** construct from a pointer to a core and a specified federate id
     */
    ConnectorFederateManager(Core* coreObj, Federate* fFed, LocalFederateId id);
    /** destructor */
    ~ConnectorFederateManager();

    /** register a Filter
    @details call is only valid in startup mode
    @param name the name of the endpoint
    @param type_in the type the filter is expecting as an input
    @param type_out the type the filter generates
    */
    Filter& registerFilter(const std::string& name,
                           const std::string& type_in,
                           const std::string& type_out);

    /** register a cloningFilter
    @details call is only valid in startup mode
    @param name the name of the endpoint
    @param type_in the type the filter is expecting as an input
    @param type_out the type the filter generates
    */
    CloningFilter& registerCloningFilter(const std::string& name,
                                         const std::string& type_in,
                                         const std::string& type_out);

    /** register a Filter
    @details call is only valid in startup mode
    @param type the defined type of the filter
    @param name the name of the filter
    */
    Filter& registerFilter(FilterTypes type, const std::string& name);

    /** register a cloningFilter
    @details call is only valid in startup mode
    @param name the name of the endpoint
    @param type the defined type of the interface for endpoint checking if requested
    */
    CloningFilter& registerCloningFilter(FilterTypes type, const std::string& name);

    /** get a registered Filter
    @param name the publication id
    @return ivalid_publication_id if name is not recognized otherwise returns the publication_id*/
    Filter& getFilter(const std::string& name);
    const Filter& getFilter(const std::string& name) const;
    Filter& getFilter(int index);
    const Filter& getFilter(int index) const;


    /** register a Translator
    @details call is only valid in startup mode
    @param name the name of the translator
    @param type_in the type the translator is expecting on the value interface
    @param type_out the type the translator for the endpoint
    */
    Translator& registerTranslator(std::string_view name,
                           std::string_view type_in,
                           std::string_view type_out);


    /** register a Translator
    @details call is only valid in startup mode
    @param type the defined type of the translator
    @param name the name of the translator
    */
    Translator& registerTranslator(TranslatorTypes type, std::string_view name);


    /** get a registered Translator
    @param name the translator name
    @return invalid translator object if name is not recognized otherwise returns the translator*/
    Translator& getTranslator(const std::string& name);
    const Translator& getTranslator(const std::string& name) const;
    Translator& getTranslator(int index);
    const Translator & getTranslator(int index) const;

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
    shared_guarded<gmlc::containers::MappedVector<std::unique_ptr<Filter>, std::string>> filters;
    shared_guarded<gmlc::containers::MappedVector<std::unique_ptr<Translator>, std::string>> translators;
    Federate* fed = nullptr;  //!< pointer back to the message Federate
    const LocalFederateId fedID;  //!< storage for the federate ID
};
}  // namespace helics
