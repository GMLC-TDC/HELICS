/*
Copyright (c) 2017-2021,
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
enum class TranslatorTypes {
    CUSTOM = HELICS_TRANSLATOR_TYPE_CUSTOM,
    JSON = HELICS_TRANSLATOR_TYPE_JSON,
    BINARY=HELICS_TRANSLATOR_TYPE_BINARY,
    UNRECOGNIZED = 7
};

#define EMPTY_STRING std::string()

/** get the translator type from a string*/
HELICS_CXX_EXPORT TranslatorTypes translatorTypeFromString(const std::string& translatorType) noexcept;

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
    /** construct through a federate*/
    explicit Translator(Federate* ffed, const std::string& translatorName = EMPTY_STRING);
    /** construct from handle and federate*/
    Translator(Federate* ffed, const std::string& translatorName, InterfaceHandle ihandle);
    /** construct from handle and core*/
    Translator(Core* core, const std::string& translatorName, InterfaceHandle ihandle);
    /** construct through a federate*/
    Translator(InterfaceVisibility locality,
           Federate* ffed,
           const std::string& translatorName = EMPTY_STRING);
    /** construct through a core object*/
    explicit Translator(Core* cr, const std::string& translatorName = EMPTY_STRING);
    /** virtual destructor*/
    virtual ~Translator() = default;

    Translator(Translator&& filt) = default;
    /** copy the translator, a copied translator will point to the same object*/
    Translator(const Translator& filt) = default;
    Translator& operator=(Translator&& filt) = default;
    /** copy the translator, a copied translator will point to the same object as the original*/
    Translator& operator=(const Translator& filt) = default;

    /** set a message operator to process the message*/
    void setOperator(std::shared_ptr<TranslatorOperator> mo);

    virtual const std::string& getDisplayName() const override { return getName(); }

    /** set a property on a translator
    @param property the name of the property of the translator to change
    @param val the numerical value of the property
    */
    virtual void set(const std::string& property, double val);

    /** set a string property on a translator
    @param property the name of the property of the translator to change
    @param val the numerical value of the property
    */
    virtual void setString(const std::string& property, const std::string& val);

    void addTarget(const std::string& target) { addSourceTarget(target); }

  protected:
    /** set a translator operations object */
    void setTranslatorOperations(std::shared_ptr<TranslatorOperations> translatorOps);
    /** add a defined operation to a translator*/
    friend void addOperations(Translator* translator, TranslatorTypes type, Core* cptr);
};

/** create a  translator
@param type the type of translator to create
@param fed the federate to create the translator through
@param name the name of the translator (optional)
@return a unique pointer to a destination Translator object,  note destroying the object does not
deactivate the translator
*/
HELICS_CXX_EXPORT Translator&
    make_translator(TranslatorTypes type, Federate* fed, const std::string& name = EMPTY_STRING);

/** create a  translator
@param locality the visibility of the translator global or local
@param type the type of translator to create
@param fed the federate to create the translator through
@param name the name of the translator (optional)
@return a unique pointer to a destination Translator object,  note destroying the object does not
deactivate the translator
*/
HELICS_CXX_EXPORT Translator& make_translator(InterfaceVisibility locality,
                                      TranslatorTypes type,
                                      Federate* fed,
                                      const std::string& name = EMPTY_STRING);

/** create a translator
@param type the type of translator to create
@param cr the core to create the translator through
@param name the name of the translator (optional)
@return a unique pointer to a source Translator object,  note destroying the object does not deactivate
the translator
*/
HELICS_CXX_EXPORT std::unique_ptr<Translator>
    make_translator(TranslatorTypes type, Core* cr, const std::string& name = EMPTY_STRING);

/** create a translator
@param type the type of translator to create
@param cr the core to create the translator through
@param name the name of the translator (optional)
@return a unique pointer to a source Translator object,  note destroying the object does not deactivate
the translator
*/
HELICS_CXX_EXPORT std::unique_ptr<Translator>
    make_translator(TranslatorTypes type, CoreApp& cr, const std::string& name = EMPTY_STRING);


}  // namespace helics
