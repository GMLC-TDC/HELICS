/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#ifndef HELICS_CPP98_TRANSLATOR_HPP_
#define HELICS_CPP98_TRANSLATOR_HPP_

#include "DataBuffer.hpp"
#include "helics/helics.h"
#include "helicsExceptions.hpp"

#include <string>

namespace helicscpp {
/** object managing a translator in the C++98 interface*/
class Translator {
  public:
    /** construct from C level HelicsTranslator object*/
    explicit Translator(HelicsTranslator htrans) HELICS_NOTHROW: translator(htrans) {}
    /** default constructor*/
    Translator() HELICS_NOTHROW: translator(HELICS_NULL_POINTER) {}
    /** copy constructor*/
    Translator(const Translator& trans): translator(trans.translator) {}
    /** copy assignment*/
    Translator& operator=(const Translator& translatorer)
    {
        translator = translatorer.translator;
        return *this;
    }

    /** cast operator to get the underlying object*/
    operator HelicsTranslator() const { return translator; }
    /** get the underlying HelicsTranslator object*/
    HelicsTranslator baseObject() const { return translator; }
    /** check if the translator is valid */
    bool isValid() const { return (helicsTranslatorIsValid(translator) == HELICS_TRUE); }
    /** get the name for the translator*/
    const char* getName() const { return helicsTranslatorGetName(translator); }
    /** set a property on a translator
    @param property the name of the property of the translator to change
    @param val the numerical value of the property
    */
    void set(const std::string& property, double val)
    {
        helicsTranslatorSet(translator, property.c_str(), val, hThrowOnError());
    }
    /** set a string property on a translator
   @param property the name of the property of the translator to change
   @param val the numerical value of the property
   */
    void setString(const std::string& property, const std::string& val)
    {
        helicsTranslatorSetString(translator, property.c_str(), val.c_str(), hThrowOnError());
    }

    /** add a destination target to a cloning translator
    @details all messages going to a destination are copied to the delivery address(es)*/
    void addDestinationEndpoint(const std::string& dest)
    {
        helicsTranslatorAddDestinationEndpoint(translator, dest.c_str(), hThrowOnError());
    }

    /** add a source target to a cloning translator
    @details all messages coming from a source are copied to the delivery address(es)*/
    void addSourceEndpoint(const std::string& source)
    {
        helicsTranslatorAddSourceEndpoint(translator, source.c_str(), hThrowOnError());
    }

    /** add a destination target to a cloning translator
    @details all messages going to a destination are copied to the delivery address(es)*/
    void addDestinationInput(const std::string& dest)
    {
        helicsTranslatorAddInputTarget(translator, dest.c_str(), hThrowOnError());
    }

    /** add a source target to a cloning translator
    @details all messages coming from a source are copied to the delivery address(es)*/
    void addSourcePublication(const std::string& source)
    {
        helicsTranslatorAddPublicationTarget(translator, source.c_str(), hThrowOnError());
    }
    /** remove a destination target from a cloning translator*/
    void removeTarget(const std::string& dest)
    {
        helicsTranslatorRemoveTarget(translator, dest.c_str(), hThrowOnError());
    }
    /** get the interface information field of the translator*/
    const char* getInfo() const { return helicsTranslatorGetInfo(translator); }
    /** set the interface information field of the translator*/
    void setInfo(const std::string& info)
    {
        helicsTranslatorSetInfo(translator, info.c_str(), HELICS_IGNORE_ERROR);
    }
    /** get the value of a tag for the translator*/
    const char* getTag(const std::string& tagname) const
    {
        return helicsTranslatorGetTag(translator, tagname.c_str());
    }
    /** set the value of a tag for the translator*/
    void setTag(const std::string& tagname, const std::string& tagvalue)
    {
        helicsTranslatorSetTag(translator, tagname.c_str(), tagvalue.c_str(), HELICS_IGNORE_ERROR);
    }
    void setOption(int32_t option, int32_t value = 1)
    {
        helicsTranslatorSetOption(translator, option, value, HELICS_IGNORE_ERROR);
    }
    int32_t getOption(int32_t option) { return helicsTranslatorGetOption(translator, option); }

    /*
    void setCallback(HelicsMessage (*translatorCall)(HelicsMessage message, void* userData),
                     void* userData)
    {
        helicsTranslatorSetCustomCallback(translator, translatorCall, userData,
    HELICS_IGNORE_ERROR);
    }
    */
  protected:
    HelicsTranslator translator;  //!< the reference to the underlying HelicsTranslator object
};

}  // namespace helicscpp
#endif
