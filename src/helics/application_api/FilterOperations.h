/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _HELICS_FILTEROPERATIONS_H_
#define _HELICS_FILTEROPERATIONS_H_
#pragma once

#include "../core/core.h"

#include "libguarded/guarded.hpp"
#include "libguarded/shared_guarded.hpp"
#include "libguarded/atomic_guarded.hpp"
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
    class delayFilterOperation : public FilterOperations
    {
    private:
        std::atomic<Time> delay{ timeZero };
        std::shared_ptr<MessageTimeOperator> td;

    public:
        explicit delayFilterOperation(Time delayTime = timeZero);
        virtual void set(const std::string &property, double val) override;
        virtual std::shared_ptr<FilterOperator> getOperator() override;
    };

    class randomDelayGenerator;

    /** filter for generating a random delay time for a message*/
    class randomDelayFilterOperation : public FilterOperations
    {
    private:
        std::shared_ptr<MessageTimeOperator> td;
        std::unique_ptr<randomDelayGenerator> rdelayGen;

    public:
        randomDelayFilterOperation();
        ~randomDelayFilterOperation();
        virtual void set(const std::string &property, double val) override;
        virtual void setString(const std::string &property, const std::string &val) override;
        virtual std::shared_ptr<FilterOperator> getOperator() override;
    };

    /** filter for randomly dropping a packet*/
    class randomDropFilterOperation : public FilterOperations
    {
    private:
        std::atomic<double> dropProb{ 0.0 };
        std::shared_ptr<MessageConditionalOperator> tcond;

    public:
        randomDropFilterOperation();
        ~randomDropFilterOperation();
        virtual void set(const std::string &property, double val) override;
        virtual void setString(const std::string &property, const std::string &val) override;
        virtual std::shared_ptr<FilterOperator> getOperator() override;
    };

    /** filter for rerouting a packet to a particular endpoint*/
    class rerouteFilterOperation : public FilterOperations
    {
    private:
        std::shared_ptr<MessageDestOperator> op;  //!< the actual operator
        libguarded::atomic_guarded<std::string> newTarget;  //!< the target destination
#ifdef HAVE_SHARED_TIMED_MUTEX
        
        libguarded::shared_guarded<std::set<std::string>>
            conditions;  //!< the original destination must match one of these conditions
#else
        libguarded::shared_guarded<std::set<std::string>, std::mutex>
            conditions;  //!< the original destination must match one of these conditions
#endif
    public:
        rerouteFilterOperation();
        ~rerouteFilterOperation();
        virtual void set(const std::string &property, double val) override;
        virtual void setString(const std::string &property, const std::string &val) override;
        virtual std::shared_ptr<FilterOperator> getOperator() override;

    private:
        /** function to execute the rerouting operation*/
        std::string rerouteOperation(const std::string &dest) const;
    };

    /** filter for rerouting a packet to a particular endpoint*/
    class cloneFilterOperation : public FilterOperations
    {
    private:
        Core *coreptr; //!< pointer to a core object
        std::shared_ptr<CloneOperator> op;  //!< the actual operator
#ifdef HAVE_SHARED_TIMED_MUTEX
        libguarded::shared_guarded<std::vector<std::string>>
            deliveryAddresses;  //!< the original destination must match one of these conditions
#else
        libguarded::shared_guarded<std::vector<std::string>, std::mutex>
            deliveryAddresses;  //!< the original destination must match one of these conditions
#endif
    public:
        /** this operation needs a pointer to a core to operate*/
        explicit cloneFilterOperation(Core *core);

        ~cloneFilterOperation();
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