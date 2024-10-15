/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "CallbackFederate.hpp"

#include "helics/core/Core.hpp"

#include <memory>
#include <string>
#include <utility>

namespace helics {

class CallbackFederateOperator: public FederateOperator {
  private:
    CallbackFederate* mfed;
    friend CallbackFederate;

  public:
    explicit CallbackFederateOperator(CallbackFederate* fed): mfed(fed) {}
    virtual IterationRequest initializeOperations() override final
    {
        return mfed->initializeOperationsCallback();
    }

    virtual std::pair<Time, IterationRequest> operate(iteration_time newTime) override final
    {
        return mfed->operateCallback(newTime);
    }

    virtual void finalize() override final { mfed->finalizeCallback(); }
    /** run any operations for handling an error*/
    virtual void error_handler(int error_code, std::string_view errorString) override final
    {
        mfed->errorHandlerCallback(error_code, errorString);
    }
};
CallbackFederate::CallbackFederate() {}

// NOTE: the CallbackFederate must call the federate constructor do to the virtual inheritance in
// CombinationFederate

CallbackFederate::CallbackFederate(std::string_view fedName, const FederateInfo& fedInfo):
    Federate(fedName, fedInfo), CombinationFederate(fedName, fedInfo)
{
    loadOperator();
}
CallbackFederate::CallbackFederate(std::string_view fedName,
                                   const std::shared_ptr<Core>& core,
                                   const FederateInfo& fedInfo):
    Federate(fedName, core, fedInfo), CombinationFederate(fedName, core, fedInfo)
{
    loadOperator();
}

CallbackFederate::CallbackFederate(std::string_view fedName,
                                   CoreApp& core,
                                   const FederateInfo& fedInfo):
    Federate(fedName, core, fedInfo), CombinationFederate(fedName, core, fedInfo)
{
    loadOperator();
}

CallbackFederate::CallbackFederate(const std::string& configString):
    Federate(std::string_view{}, loadFederateInfo(configString)),
    CombinationFederate(std::string_view{}, loadFederateInfo(configString))
{
    loadOperator();
}

CallbackFederate::CallbackFederate(std::string_view fedName, const std::string& configString):
    Federate(fedName, loadFederateInfo(configString)), CombinationFederate(fedName, configString)
{
    loadOperator();
}

CallbackFederate::CallbackFederate(CallbackFederate&&) noexcept = default;
CallbackFederate::~CallbackFederate() = default;

CallbackFederate& CallbackFederate::operator=(CallbackFederate&&) noexcept = default;

void CallbackFederate::loadOperator()
{
    op = std::make_shared<CallbackFederateOperator>(this);
    coreObject->setFederateOperator(getID(), op);
    coreObject->setFlagOption(getID(), HELICS_FLAG_CALLBACK_FEDERATE, true);
    mEventTriggered = coreObject->getFlagOption(getID(), HELICS_FLAG_EVENT_TRIGGERED);
    setAsyncCheck([this]() { return (getCurrentMode() >= Modes::FINALIZE); });
}

/** callback operations*/
IterationRequest CallbackFederate::initializeOperationsCallback()
{
    if (currentMode.load() == Modes::STARTUP) {
        enteringInitializingMode(IterationResult::NEXT_STEP);
    } else {
        enteringExecutingMode({timeZero, IterationResult::ITERATING});
    }

    return (initializationOperation) ? initializationOperation() : IterationRequest::NO_ITERATIONS;
}

std::pair<Time, IterationRequest> CallbackFederate::operateCallback(iteration_time newTime)
{
    if (newTime.grantedTime == timeZero && newTime.state == IterationResult::NEXT_STEP) {
        enteringExecutingMode(newTime);
    } else {
        postTimeRequestOperations(newTime.grantedTime, newTime.state == IterationResult::ITERATING);
    }
    // time zero here is equivalent to the next allowable time
    auto rval = std::make_pair((mEventTriggered) ? Time::maxVal() : timeZero,
                               IterationRequest::NO_ITERATIONS);

    if (newTime.grantedTime >= mStopTime) {
        rval = std::make_pair(Time::maxVal(), IterationRequest::HALT_OPERATIONS);
    } else if (nextTimeOperation1) {
        rval = nextTimeOperation1(newTime);
    } else if (nextTimeOperation2) {
        rval = std::make_pair(nextTimeOperation2(newTime.grantedTime),
                              IterationRequest::NO_ITERATIONS);
    }
    if (rval.second <= IterationRequest::ITERATE_IF_NEEDED) {
        preTimeRequestOperations(rval.first, rval.second != IterationRequest::NO_ITERATIONS);
    }

    return rval;
}

void CallbackFederate::setFlagOption(int32_t property, bool val)
{
    if (property == HELICS_FLAG_EVENT_TRIGGERED) {
        mEventTriggered = val;
        // this does need to fallthrough
    }
    CombinationFederate::setFlagOption(property, val);
}

void CallbackFederate::finalizeCallback()
{
    finalizeOperations();
}

void CallbackFederate::errorHandlerCallback(int errorCode, std::string_view errorString)
{
    handleError(errorCode, errorString, true);
}

}  // namespace helics
