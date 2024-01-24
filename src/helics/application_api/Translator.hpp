/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "../core/Core.hpp"
#include "Federate.hpp"
#include "helics/helics_enums.h"

#include <memory>
#include <string>

namespace helics {
class TranslatorOperations;
class TranslatorOperator;
class CoreApp;
/** a set of common defined translators*/
enum TranslatorTypes : std::int32_t {
    CUSTOM = HELICS_TRANSLATOR_TYPE_CUSTOM,
    JSON = HELICS_TRANSLATOR_TYPE_JSON,
    BINARY = HELICS_TRANSLATOR_TYPE_BINARY,
    UNRECOGNIZED = 7
};

constexpr std::string_view emptyString{""};

/** get the translator type from a string*/
HELICS_CXX_EXPORT TranslatorTypes
    translatorTypeFromString(std::string_view translatorType) noexcept;

/** class for managing a particular translator*/
class HELICS_CXX_EXPORT Translator: public Interface {
  protected:
    bool disableAssign = false;  //!< disable assignment for the object
  private:
    std::shared_ptr<TranslatorOperations>
        transOp;  //!< a class running any specific operation of the Translator
  public:
    /** default constructor*/
    Translator() = default;
    /** construct from handle and federate*/
    Translator(Federate* ffed, std::string_view translatorName, InterfaceHandle ihandle);
    /** construct from handle and core*/
    Translator(Core* core, std::string_view translatorName, InterfaceHandle ihandle);
    /** construct through a core object*/
    explicit Translator(Core* core, std::string_view translatorName = emptyString);
    /** virtual destructor*/
    virtual ~Translator() = default;

    Translator(Translator&& trans) = default;
    /** copy the translator, a copied translator will point to the same object*/
    Translator(const Translator& trans) = default;
    Translator& operator=(Translator&& trans) = default;
    /** copy the translator, a copied translator will point to the same object as the original*/
    Translator& operator=(const Translator& trans) = default;

    /** set a message operator to process the message*/
    void setOperator(std::shared_ptr<TranslatorOperator> operation);

    virtual const std::string& getDisplayName() const override { return getName(); }

    /** set a property on a translator
    @param property the name of the property of the translator to change
    @param val the numerical value of the property
    */
    virtual void set(std::string_view property, double val);

    /** set a string property on a translator
    @param property the name of the property of the translator to change
    @param val the numerical value of the property
    */
    virtual void setString(std::string_view property, std::string_view val);

    void addPublication(std::string_view target)
    {
        addSourceTarget(target, InterfaceType::PUBLICATION);
    }
    void addInputTarget(std::string_view target)
    {
        addDestinationTarget(target, InterfaceType::INPUT);
    }
    void addSourceEndpoint(std::string_view target)
    {
        addSourceTarget(target, InterfaceType::ENDPOINT);
    }
    void addDestinationEndpoint(std::string_view target)
    {
        addDestinationTarget(target, InterfaceType::ENDPOINT);
    }
    void addSourceFilter(std::string_view filterName)
    {
        addSourceTarget(filterName, InterfaceType::FILTER);
    }
    /** add a named filter to an endpoint for all message going to the endpoint*/
    void addDestinationFilter(std::string_view filterName)
    {
        addDestinationTarget(filterName, InterfaceType::FILTER);
    }
    /** set the type of operations specifying how the translator should operate*/
    void setTranslatorType(std::int32_t type);

  protected:
    /** set a translator operations object */
    void setTranslatorOperations(std::shared_ptr<TranslatorOperations> translatorOps);
    /** add a defined operation to a translator*/
    friend void addOperations(Translator* translator, TranslatorTypes type);
};

}  // namespace helics
