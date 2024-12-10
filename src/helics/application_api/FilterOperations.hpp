/*
Copyright (c) 2017-2024,
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
#include "gmlc/libguarded/cow_guarded.hpp"

#include <atomic>
#include <memory>
#include <set>
#include <string>
#include <string_view>
#include <vector>

namespace helics {
class Core;
class FilterOperator;
class MessageTimeOperator;
class Message;
class MessageConditionalOperator;
class MessageDestOperator;
class CloneOperator;
class FirewallOperator;
/** class for managing filter operations*/
class FilterOperations {
  public:
    FilterOperations() = default;
    virtual ~FilterOperations() = default;
    // still figuring out if these functions have a use or not
    FilterOperations(const FilterOperations& fo) = delete;
    FilterOperations(FilterOperations&& fo) = delete;
    FilterOperations& operator=(const FilterOperations& fo) = delete;
    FilterOperations& operator=(FilterOperations&& fo) = delete;

    /** set a property on a filter
    @param property the name of the property of the filter to change
    @param val the numerical value of the property
    */
    virtual void set(std::string_view property, double val);
    /** set a string property on a filter
    @param property the name of the property of the filter to change
    @param val the numerical value of the property
    */
    virtual void setString(std::string_view property, std::string_view val);

    /** get a property from a filter
    @param property the name of the property of the filter to get
    */
    virtual double getProperty(std::string_view property);
    /** get a string property on a filter
    @param property the name of the property of the filter to get
    */
    virtual std::string getString(std::string_view property);

    /** retrieve the filter operator*/
    virtual std::shared_ptr<FilterOperator> getOperator() = 0;
};

/**filter for delaying a message in time*/
class DelayFilterOperation: public FilterOperations {
  private:
    std::atomic<Time> delay{timeZero};
    std::shared_ptr<MessageTimeOperator> td;

  public:
    explicit DelayFilterOperation(Time delayTime = timeZero);
    virtual void set(std::string_view property, double val) override;
    virtual void setString(std::string_view property, std::string_view val) override;
    virtual double getProperty(std::string_view property) override;
    virtual std::string getString(std::string_view property) override;
    virtual std::shared_ptr<FilterOperator> getOperator() override;
};

class RandomDelayGenerator;

/** filter for generating a random delay time for a message*/
class RandomDelayFilterOperation: public FilterOperations {
  private:
    std::shared_ptr<MessageTimeOperator> td;  //!< pointer to the time operator
    std::unique_ptr<RandomDelayGenerator> rdelayGen;  //!< pointer to the random number generator

  public:
    /** default constructor*/
    RandomDelayFilterOperation();
    // the destructor is defined mainly to prevent the need to define the RandomDelayGenerator
    // object here
    /** destructor*/
    ~RandomDelayFilterOperation();
    virtual void set(std::string_view property, double val) override;
    virtual void setString(std::string_view property, std::string_view val) override;
    virtual double getProperty(std::string_view property) override;
    virtual std::string getString(std::string_view property) override;
    virtual std::shared_ptr<FilterOperator> getOperator() override;
};

/** filter for randomly dropping a packet*/
class RandomDropFilterOperation: public FilterOperations {
  private:
    std::atomic<double> dropProb{0.0};
    std::shared_ptr<MessageConditionalOperator> tcond;

  public:
    RandomDropFilterOperation();
    ~RandomDropFilterOperation();
    virtual void set(std::string_view property, double val) override;
    virtual void setString(std::string_view property, std::string_view val) override;
    virtual double getProperty(std::string_view property) override;
    virtual std::string getString(std::string_view property) override;
    virtual std::shared_ptr<FilterOperator> getOperator() override;
};

/** filter for rerouting a packet to a particular endpoint*/
class RerouteFilterOperation: public FilterOperations {
  private:
    std::shared_ptr<MessageDestOperator> op;  //!< the actual operator
    atomic_guarded<std::string> newDest;  //!< the target destination
    shared_guarded<std::set<std::string>>
        conditions;  //!< the conditions on which the rerouting will occur

  public:
    RerouteFilterOperation();
    ~RerouteFilterOperation();
    virtual void set(std::string_view property, double val) override;
    virtual void setString(std::string_view property, std::string_view val) override;
    virtual double getProperty(std::string_view property) override;
    virtual std::string getString(std::string_view property) override;
    virtual std::shared_ptr<FilterOperator> getOperator() override;

  private:
    /** function to execute the rerouting operation*/
    std::string rerouteOperation(const std::string& src, const std::string& dest) const;
};

/** filter for rerouting a packet to a particular endpoint*/
class FirewallFilterOperation: public FilterOperations {
  private:
    std::shared_ptr<FirewallOperator> op;  //!< the actual operator
    /// the conditions on which the rerouting will occur
    gmlc::libguarded::cow_guarded<std::vector<std::string>> allowed;
    /// the conditions that block a message
    gmlc::libguarded::cow_guarded<std::vector<std::string>> blocked;

  public:
    FirewallFilterOperation();
    ~FirewallFilterOperation();
    virtual void set(std::string_view property, double val) override;
    virtual void setString(std::string_view property, std::string_view val) override;
    virtual double getProperty(std::string_view property) override;
    virtual std::string getString(std::string_view property) override;
    virtual std::shared_ptr<FilterOperator> getOperator() override;

  private:
    /** function to execute the rerouting operation*/
    bool allowPassed(const Message* mess) const;
};

/** filter for rerouting a packet to a particular endpoint*/
class CloneFilterOperation: public FilterOperations {
  private:
    std::shared_ptr<CloneOperator> op;  //!< the actual operator
    /// the endpoints to deliver the cloned data to
    shared_guarded<std::vector<std::string>> deliveryAddresses;

  public:
    explicit CloneFilterOperation();

    ~CloneFilterOperation();
    virtual void set(std::string_view property, double val) override;
    virtual void setString(std::string_view property, std::string_view val) override;
    virtual double getProperty(std::string_view property) override;
    virtual std::string getString(std::string_view property) override;
    virtual std::shared_ptr<FilterOperator> getOperator() override;

  private:
    /** run the send message function which copies the message and forwards to all destinations
    @param mess a message to clone*/
    std::vector<std::unique_ptr<Message>> sendMessage(const Message* mess) const;
};

}  // namespace helics
