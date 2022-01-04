/*
Copyright (c) 2017-2022,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

/** @file
file defines some common filter operations
*/

#include "../common/GuardedTypes.hpp"
#include "../core/helicsTime.hpp"
#include "../core/core-data.hpp"

#include <atomic>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace helics {
class Core;

/** class defining a message operator that can operate on any part of a message*/
class JsonTranslatorOperator: public TranslatorOperator {
  public:
    /** default constructor*/
    JsonTranslatorOperator() = default;
    /** set the function to modify the data of the message*/
  private:

      virtual SmallBuffer convertToValue(std::unique_ptr<Message> message) override;

    /** convert a value to a message*/
    virtual std::unique_ptr<Message> convertToMessage(const SmallBuffer& value) override;

};

/** class defining a message operator that can operate on any part of a message*/
class BinaryTranslatorOperator: public TranslatorOperator {
  public:
    /** default constructor*/
    BinaryTranslatorOperator() = default;
    /** set the function to modify the data of the message*/
  private:
    virtual SmallBuffer convertToValue(std::unique_ptr<Message> message) override;

    /** convert a value to a message*/
    virtual std::unique_ptr<Message> convertToMessage(const SmallBuffer& value) override;
};

/** class for managing translator operations*/
class TranslatorOperations {
  public:
    TranslatorOperations() = default;
    virtual ~TranslatorOperations() = default;
    // still figuring out if these functions have a use or not
    TranslatorOperations(const TranslatorOperations& fo) = delete;
    TranslatorOperations(TranslatorOperations&& fo) = delete;
    TranslatorOperations& operator=(const TranslatorOperations& fo) = delete;
    TranslatorOperations& operator=(TranslatorOperations&& fo) = delete;

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
    virtual std::shared_ptr<TranslatorOperator> getOperator() = 0;
};

/**translator for converting a message to JSON and vice versa*/
class JsonTranslatorOperation: public TranslatorOperations {
  private:
    std::shared_ptr<JsonTranslatorOperator> to;
  public:
    JsonTranslatorOperation();
    virtual std::shared_ptr<TranslatorOperator> getOperator() override
    {
        return std::static_pointer_cast<TranslatorOperator>(to);
    }
};


/** binary translator*/
class BinaryTranslatorOperation: public TranslatorOperations {
  private:
    std::shared_ptr<BinaryTranslatorOperator> to;
  public:
    /** default constructor*/
    BinaryTranslatorOperation();
    
    virtual std::shared_ptr<TranslatorOperator> getOperator() override
    {
        return std::static_pointer_cast<TranslatorOperator>(to);
    }

};

}  // namespace helics
