/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _HELICS_FILTEROPERATIONS_H_
#define _HELICS_FILTEROPERATIONS_H_
#pragma once
/** @file
file defines some common filter operations
*/

#include "../core/Core.hpp"

#include "../common/GuardedTypes.hpp"
#include <mutex>
#include <set>
#include <atomic>

namespace helics
{
    class MessageTimeOperator;
    class MessageConditionalOperator;
    class MessageDestOperator;
    class CloneOperator;
    /** class for managing filter operations*/
    class FilterOperations
    {
    public:
        FilterOperations() = default;
        virtual ~FilterOperations() = default;
        // still figuring out if these functions have a use or not
        FilterOperations(const FilterOperations &fo) = delete;
        FilterOperations(FilterOperations &&fo) = delete;
        FilterOperations &operator= (const FilterOperations &fo) = delete;
        FilterOperations &operator= (FilterOperations &&fo) = delete;

        /** set a property on a filter
        @param property the name of the property of the filter to change
        @param val the numerical value of the property
        */
        virtual void set(const std::string &property, double val);
        /** set a string property on a filter
        @param property the name of the property of the filter to change
        @param val the numerical value of the property
        */
        virtual void setString(const std::string &property, const std::string &val);
        virtual std::shared_ptr<FilterOperator> getOperator() = 0;
    };

    /**filter for delaying a message in time*/
    class DelayFilterOperation : public FilterOperations
    {
    private:
        std::atomic<Time> delay{ timeZero };
        std::shared_ptr<MessageTimeOperator> td;

    public:
        explicit DelayFilterOperation(Time delayTime = timeZero);
        virtual void set(const std::string &property, double val) override;
        virtual std::shared_ptr<FilterOperator> getOperator() override;
    };

    class randomDelayGenerator;

    /** filter for generating a random delay time for a message*/
    class RandomDelayFilterOperation : public FilterOperations
    {
    private:
        std::shared_ptr<MessageTimeOperator> td;
        std::unique_ptr<randomDelayGenerator> rdelayGen;

    public:
        RandomDelayFilterOperation();
        ~RandomDelayFilterOperation();
        virtual void set(const std::string &property, double val) override;
        virtual void setString(const std::string &property, const std::string &val) override;
        virtual std::shared_ptr<FilterOperator> getOperator() override;
    };

    /** filter for randomly dropping a packet*/
    class RandomDropFilterOperation : public FilterOperations
    {
    private:
        std::atomic<double> dropProb{ 0.0 };
        std::shared_ptr<MessageConditionalOperator> tcond;

    public:
        RandomDropFilterOperation();
        ~RandomDropFilterOperation();
        virtual void set(const std::string &property, double val) override;
        virtual void setString(const std::string &property, const std::string &val) override;
        virtual std::shared_ptr<FilterOperator> getOperator() override;
    };

    /** filter for rerouting a packet to a particular endpoint*/
    class RerouteFilterOperation : public FilterOperations
    {
    private:
        std::shared_ptr<MessageDestOperator> op;  //!< the actual operator
        atomic_guarded<std::string> newTarget;  //!< the target destination
		shared_guarded<std::set<std::string>> conditions;
    public:
        RerouteFilterOperation();
        ~RerouteFilterOperation();
        virtual void set(const std::string &property, double val) override;
        virtual void setString(const std::string &property, const std::string &val) override;
        virtual std::shared_ptr<FilterOperator> getOperator() override;

    private:
        /** function to execute the rerouting operation*/
        std::string rerouteOperation(const std::string &dest) const;
    };

    /** filter for rerouting a packet to a particular endpoint*/
    class CloneFilterOperation : public FilterOperations
    {
    private:
        Core *coreptr; //!< pointer to a core object
        std::shared_ptr<CloneOperator> op;  //!< the actual operator
		shared_guarded<std::vector<std::string>> deliveryAddresses;  //!< the endpoints to deliver the cloned data to
    public:
        /** this operation needs a pointer to a core to operate*/
        explicit CloneFilterOperation(Core *core);

        ~CloneFilterOperation();
        virtual void set(const std::string &property, double val) override;
        virtual void setString(const std::string &property, const std::string &val) override;
        virtual std::shared_ptr<FilterOperator> getOperator() override;

    private:
        /** run the send message function which copies the message and forwards to all destinations
        @param mess a message to clone*/
        void sendMessage(const Message *mess) const;
    };

} //namespace helics

#endif /*_HELICS_FILTEROPERATIONS_H_*/