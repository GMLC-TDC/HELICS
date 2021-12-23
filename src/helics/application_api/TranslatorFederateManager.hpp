/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "../common/GuardedTypes.hpp"
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
class  TranslatorFederateManager {
  public:
    /** construct from a pointer to a core and a specified federate id
     */
    TranslatorFederateManager(Core* coreObj, Federate* fFed, LocalFederateId id);
    /** destructor */
    ~TranslatorFederateManager();

    /** register a Filter
    @details call is only valid in startup mode
    @param name the name of the endpoint
    @param type_in the type the filter is expecting as an input
    @param type_out the type the filter generates
    */
    Translator& registerTranslator(const std::string& name,
                           const std::string& type_in,
                           const std::string& type_out);


    /** register a Filter
    @details call is only valid in startup mode
    @param name the name of the endpoint
    @param type the defined type of the interface for endpoint checking if requested
    */
    Translator& registerTranslator(TranslatorTypes type, const std::string& name);

   

    /** get a registered Filter
    @param name the publication id
    @return ivalid_publication_id if name is not recognized otherwise returns the publication_id*/
    Translator& getTranslator(const std::string& name);
    const Translator& getTranslator(const std::string& name) const;
    Translator& getTranslator(int index);
    const Translator& getTranslator(int index) const;

    /** get the number of registered filters in the federate*/
    int getTranslatorCount() const;
    /** close all filters*/
    void closeAllTranslators();
    /** close all filters*/
    void disconnectAllTranslators();
    /** disconnect from the coreObject*/
    void disconnect();

  private:
    Core* coreObject{nullptr};
    shared_guarded<gmlc::containers::MappedVector<std::unique_ptr<Translator>, std::string>> translators;
    Federate* fed = nullptr;  //!< pointer back to the message Federate
    const LocalFederateId fedID;  //!< storage for the federate ID
};
}  // namespace helics
