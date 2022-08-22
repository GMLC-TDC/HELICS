/*
Copyright (c) 2017-2022,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "CallbackFederate.hpp"
#include "helics/core/Core.hpp"
#include <memory>
#include <string>

namespace helics {

    class CallbackFederateOperator : public FederateOperator
    {
    private:
        CallbackFederate *mfed;
        friend CallbackFederate;
    public:
        explicit CallbackFederateOperator(CallbackFederate* fed) :mfed(fed) {}
        virtual IterationRequest initializeOperations()override final
        {
            return mfed->initializeOperationsCallback();
        }

        virtual std::pair<Time, IterationRequest> operate(iteration_time newTime) override final
        {
            return mfed->operateCallback(newTime);
        }
        
        virtual void finalize() override final
        {
            mfed->finalizeCallback();
        }
        /** run any operations for handling an error*/
        virtual void error_handler(int error_code, std::string_view errorString) override final {
            mfed->errorHandlerCallback(error_code,errorString);
        }
    };
    CallbackFederate::CallbackFederate(){

    }

    //NOTE: the CallbackFederate must call the federate constructor do to the virtual inheritance in CombinationFederate

CallbackFederate::CallbackFederate(std::string_view fedName, const FederateInfo& fi):
    Federate(fedName,fi),CombinationFederate(fedName, fi)
{
    loadOperator();
}
CallbackFederate::CallbackFederate(std::string_view fedName,
                                         const std::shared_ptr<Core>& core,
                                         const FederateInfo& fi):
    Federate(fedName,core,fi),CombinationFederate(fedName, core, fi)
{
    loadOperator();
}

CallbackFederate::CallbackFederate(std::string_view fedName,
                                         CoreApp& core,
                                         const FederateInfo& fi):
    Federate(fedName,core,fi),CombinationFederate(fedName, core, fi)
{
    loadOperator();
}

CallbackFederate::CallbackFederate(const std::string& configString):
    Federate(std::string_view{}, loadFederateInfo(configString)),CombinationFederate(std::string_view{}, loadFederateInfo(configString))
{
    loadOperator();
}

CallbackFederate::CallbackFederate(std::string_view fedName, const std::string& configString):
    Federate(fedName, loadFederateInfo(configString)),CombinationFederate(fedName, configString)
{
    loadOperator();
}

CallbackFederate::CallbackFederate(CallbackFederate&&) noexcept = default;
CallbackFederate::~CallbackFederate() = default;

CallbackFederate& CallbackFederate::operator=(CallbackFederate&&) noexcept = default;


void CallbackFederate::loadOperator()
{
    op=std::make_shared<CallbackFederateOperator>(this);
    coreObject->setFederateOperator(getID(),op);
    coreObject->setFlagOption(getID(),HELICS_FLAG_CALLBACK_FEDERATE,true);
}

/** callback operations*/
IterationRequest CallbackFederate::initializeOperationsCallback()
{
    enteringInitializingMode();
    return (initializationOperation)?initializationOperation():IterationRequest::NO_ITERATIONS;
}

std::pair<Time, IterationRequest> CallbackFederate::operateCallback(iteration_time newTime)
{
    
    if (nextTimeOperation1)
    {
        return nextTimeOperation1(newTime);
    }
    if (nextTimeOperation2)
    {
        return std::make_pair(nextTimeOperation2(newTime.grantedTime),IterationRequest::NO_ITERATIONS);
    }
    // time zero here is equivalent to the next allowable time
    return std::make_pair(timeZero,IterationRequest::NO_ITERATIONS);
}

void CallbackFederate::finalizeCallback()
{
    
}
void CallbackFederate::errorHandlerCallback(int errorCode, std::string_view errorString)
{
    handleError(errorCode,errorString,true);
}


}  // namespace helics
