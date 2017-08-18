/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "Federate.h"
#include "core/CoreFactory.h"
#include "helics/core/core.h"
#include "asyncFedCallInfo.h"
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4702)
#include "json/json.h"
#pragma warning(pop)
#else
#include "json/json.h"
#endif

#include <cassert>
#include <fstream>

namespace helics
{
CoreFederateInfo generateCoreInfo (const FederateInfo &fi)
{
    CoreFederateInfo cfi;
    cfi.lookAhead = fi.lookAhead;
	cfi.impactWindow = fi.impactWindow;
    cfi.observer = fi.observer;
    cfi.timeDelta = fi.timeDelta;
    cfi.uninteruptible = fi.uninterruptible;
    cfi.time_agnostic = fi.timeAgnostic;
	cfi.source_only = fi.sourceOnly;
    return cfi;
}

Federate::Federate (const FederateInfo &fi) : FedInfo (fi)
{
	if (fi.coreName.empty())
	{
		coreObject = CoreFactory::create(coreTypeFromString(fi.coreType), fi.coreInitString);
	}
	else
	{
		coreObject = CoreFactory::FindOrCreate(coreTypeFromString(fi.coreType), fi.coreName, fi.coreInitString);
	}
    if (!coreObject)
    {
         state = op_states::error;
         return;
    }
	/** make sure the core is connected */
	if (!coreObject->isConnected())
	{
		coreObject->connect();
	}
    fedID = coreObject->registerFederate (fi.name.c_str (), generateCoreInfo (fi));
}

Federate::Federate (const std::string &file) : Federate (LoadFederateInfo (file))
{
    if (state == op_states::startup)
    {
        registerInterfaces (file);
    }
}


Federate::Federate () noexcept
{
	
    // this function needs to be defined for the virtual inheritance to compile but shouldn't actually be executed
}

Federate::Federate(Federate &&fed) noexcept
{
	auto tstate = fed.state.load();
	state.store(tstate);
	fedID = fed.fedID;
	coreObject = std::move(fed.coreObject);
	currentTime = fed.currentTime;
	FedInfo = std::move(fed.FedInfo);
	if (asyncCallInfo)
	{
		asyncCallInfo = std::move(fed.asyncCallInfo);
	}
}

Federate &Federate::operator=(Federate &&fed) noexcept
{
	auto tstate = fed.state.load();
	state.store(tstate);
	fedID = fed.fedID;
	coreObject = std::move(fed.coreObject);
	currentTime = fed.currentTime;
	FedInfo = std::move(fed.FedInfo);
	if (asyncCallInfo)
	{
		asyncCallInfo = std::move(fed.asyncCallInfo);
	}
	return *this;
}

Federate::~Federate ()
{
    if (coreObject)
    {
        finalize ();
    }
}

void Federate::enterInitializationState ()
{
    if (state == op_states::startup)
    {
        coreObject->enterInitializingState (fedID);
        state = op_states::initialization;
        StartupToInitializeStateTransition ();
    }
	else if (state == op_states::pendingInit)
	{
		enterInitializationStateFinalize();
	}
    else if (state != op_states::initialization)  // if we are already in initialization do nothing
    {
        throw (InvalidStateTransition ("cannot transition from current state to initialization state"));
    }
}


void Federate::enterInitializationStateAsync()
{
	if (state == op_states::startup)
	{
		if (!asyncCallInfo)
		{
			asyncCallInfo = std::make_unique<asyncFedCallInfo>();
		}
		state = op_states::pendingInit;
		asyncCallInfo->initFuture = std::async(std::launch::async, [this]() {coreObject->enterInitializingState(fedID);});
		
	}
	else if (state == op_states::pendingInit)
	{
		return;
	}
	else if (state != op_states::initialization)  // if we are already in initialization do nothing
	{
		throw (InvalidStateTransition("cannot transition from current state to initialization state"));
	}
}

bool Federate::asyncOperationCompleted() const
{
	switch (state)
	{
	case op_states::pendingInit:
		return (asyncCallInfo->initFuture.wait_for(std::chrono::seconds(0))== std::future_status::ready);
	case op_states::pendingExec:
		return (asyncCallInfo->execFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready);
	case op_states::pendingTime:
		return (asyncCallInfo->timeRequestFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready);
	case op_states::pendingIterativeTime:
		return (asyncCallInfo->timeRequestIterativeFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready);
	default:
		return false;
	}
}

void Federate::enterInitializationStateFinalize()
{
	if (state == op_states::pendingInit)
	{
		asyncCallInfo->initFuture.get();
		state = op_states::initialization;
		StartupToInitializeStateTransition();
	}
	else
	{
		throw(InvalidFunctionCall("cannot call finalize function without first calling async function"));
	}
	
}

convergence_state Federate::enterExecutionState (convergence_state ProcessComplete)
{
	convergence_state res = convergence_state::complete;
    switch (state)
    {
    case op_states::startup:
	case op_states::pendingInit:
        enterInitializationState ();
    //FALLTHROUGH
    case op_states::initialization:
    {
        res = coreObject->enterExecutingState (fedID, ProcessComplete);
        if (res==convergence_state::complete)
        {
            state = op_states::execution;
            InitializeToExecuteStateTransition ();
        }
        else
        {
            //?? not sure how this all works yet
        }
        break;
    }
	case op_states::pendingExec:
		return enterExecutionStateFinalize();
    case op_states::execution:
        // already in this state --> do nothing
        break;
	case op_states::pendingTime:
		requestTimeFinalize();
		break;
	case op_states::pendingIterativeTime:  //since this isn't gauranteed to progress it shouldn't be called in this fashion
    default:
        throw (InvalidStateTransition ("cannot transition from current state to execution state"));
        break;
    }
    return res;
}

void Federate::enterExecutionStateAsync(convergence_state ProcessComplete)
{
	switch (state)
	{
	case op_states::startup:
	{
		if (!asyncCallInfo)
		{
			asyncCallInfo = std::make_unique<asyncFedCallInfo>();
		}
		
		auto eExecFunc = [this, ProcessComplete]()
		{
			coreObject->enterInitializingState(fedID);
			StartupToInitializeStateTransition();
			return coreObject->enterExecutingState(fedID, ProcessComplete);
		};
		state = op_states::pendingExec;
		asyncCallInfo->execFuture = std::async(std::launch::async, eExecFunc);
	}
		break;
	case op_states::pendingInit:
		enterInitializationStateFinalize();
		//FALLTHROUGH
	case op_states::initialization:
	{
			if (!asyncCallInfo)
			{
				asyncCallInfo = std::make_unique<asyncFedCallInfo>();
			}

			auto eExecFunc = [this, ProcessComplete]()
			{
				return coreObject->enterExecutingState(fedID, ProcessComplete);
			};
			state = op_states::pendingExec;
			asyncCallInfo->execFuture = std::async(std::launch::async, eExecFunc);
	}
	break;
	case op_states::pendingExec:
		break;
	case op_states::execution:
		// already in this state --> do nothing
		break;
	default:
		throw (InvalidStateTransition("cannot transition from current state to execution state"));
		break;
	}
}

convergence_state Federate::enterExecutionStateFinalize()
{
	if (state == op_states::pendingExec)
	{
		auto res = asyncCallInfo->execFuture.get();
		if (convergence_state::complete==res)
		{
			state = op_states::execution;
			InitializeToExecuteStateTransition();
			
		}
		else
		{
			state = op_states::initialization;
			//?? not sure how this all works yet
		}
		return res;
	}
	else
	{
		throw(InvalidFunctionCall("cannot call finalize function without first calling async function"));
	}
}

void Federate::setTimeDelta (Time tdelta) 
{ 
	if (tdelta < timeZero)
	{
		throw(InvalidParameterValue("timeDelta must be >=0"));
	}
	coreObject->setTimeDelta (fedID, tdelta); 
}


void Federate::setLookAhead (Time lookAhead) 
{ 
	if (lookAhead < timeZero)
	{
		throw(InvalidParameterValue("lookahead must be >=0"));
	}
	coreObject->setLookAhead (fedID, lookAhead); 
}

void Federate::setImpactWindow(Time window)
{
	if (window < timeZero)
	{
		throw(InvalidParameterValue("Impact Window must be >=0"));
	}
	coreObject->setImpactWindow(fedID, window);
}

void Federate::finalize ()
{
    switch (state)
    {
    case op_states::startup:
		break;
	case op_states::pendingInit:
        enterInitializationStateFinalize ();
		break;
    case op_states::initialization:
		break;
	case op_states::pendingExec:
		enterInitializationStateFinalize();
	case op_states::pendingTime:
		requestTimeFinalize();
		break;
    case op_states::execution:
        break;
	case op_states::pendingIterativeTime:
		requestTimeIterativeFinalize();//I don't care about the return any more
		break;
    case op_states::finalize:
		return;
        // do nothing
    default://basically only error state
		throw(InvalidFunctionCall("cannot call finalize in present state"));
    }
	coreObject->finalize(fedID);
	state = op_states::finalize;
}

void Federate::error (int errorcode)
{
    state = op_states::error;
    std::string errorString = "error " + std::to_string (errorcode) + " in federate " + FedInfo.name;
    coreObject->logMessage (fedID, errorcode, errorString.c_str ());
}

void Federate::error (int errorcode, const std::string &message)
{
    state = op_states::error;
    coreObject->logMessage (fedID, errorcode, message.c_str ());
}


Time Federate::requestTime (Time nextInternalTimeStep)
{
	if (state == op_states::execution)
	{
		auto newTime = coreObject->timeRequest(fedID, nextInternalTimeStep);
		Time oldTime = currentTime;
		currentTime = newTime;
		updateTime(newTime, oldTime);
		return newTime;
	}
	else
	{
		throw(InvalidFunctionCall("cannot call request time in present state"));
	}
}

iterationTime Federate::requestTimeIterative(Time nextInternalTimeStep, convergence_state iterationComplete)
{
	if (state == op_states::execution)
	{
		auto iterationTime = coreObject->requestTimeIterative(fedID, nextInternalTimeStep, iterationComplete);
		Time oldTime = currentTime;
		if (iterationTime.state==convergence_state::complete)
		{
			currentTime = iterationTime.stepTime;
		}
		updateTime(currentTime, oldTime);
		return iterationTime;
	}
	else
	{
		throw(InvalidFunctionCall("cannot call request time in present state"));
	}
}


void Federate::requestTimeAsync(Time nextInternalTimeStep)
{
	if (state == op_states::execution)
	{
		if (!asyncCallInfo)
		{
			asyncCallInfo = std::make_unique<asyncFedCallInfo>();
		}
		state = op_states::pendingTime;
		asyncCallInfo->timeRequestFuture = std::async(std::launch::async, [this, nextInternalTimeStep]() {return coreObject->timeRequest(fedID, nextInternalTimeStep); });
	}
	else
	{
		throw(InvalidFunctionCall("cannot call request time in present state"));
	}
}

/** request a time advancement
@param[in] the next requested time step
@return the granted time step*/
void Federate::requestTimeIterativeAsync(Time nextInternalTimeStep, convergence_state iterationComplete)
{
	if (state == op_states::execution)
	{
	if (!asyncCallInfo)
	{
		asyncCallInfo = std::make_unique<asyncFedCallInfo>();
	}
	state = op_states::pendingIterativeTime;
	asyncCallInfo->timeRequestIterativeFuture = std::async(std::launch::async, [this, nextInternalTimeStep,iterationComplete]() {return coreObject->requestTimeIterative(fedID, nextInternalTimeStep, iterationComplete); });
	}
	else
	{
		throw(InvalidFunctionCall("cannot call request time in present state"));
	}
}

/** request a time advancement
@param[in] the next requested time step
@return the granted time step*/
Time Federate::requestTimeFinalize()
{
	if (state == op_states::pendingTime)
	{
		auto newTime = asyncCallInfo->timeRequestFuture.get();
		state = op_states::execution;
		Time oldTime = currentTime;
		currentTime = newTime;
		updateTime(newTime, oldTime);
		return newTime;
	}
	else
	{
		throw(InvalidFunctionCall("cannot call finalize function without first calling async function"));
	}
	
}

/** finalize the time advancement request
@return the granted time step*/
iterationTime Federate::requestTimeIterativeFinalize()
{
	if (state == op_states::pendingIterativeTime)
	{
		auto iterativeTime = asyncCallInfo->timeRequestIterativeFuture.get();
		state = op_states::execution;
		Time oldTime = currentTime;
		if (iterativeTime.state==convergence_state::complete)
		{
			currentTime = iterativeTime.stepTime;
		}
		updateTime(currentTime, oldTime);
		return iterativeTime;
	}
	else
	{
		throw(InvalidFunctionCall("cannot call finalize function without first calling async function"));
	}
}

void Federate::updateTime (Time /*newTime*/, Time /*oldTime*/)
{
    // child classes would likely implement this
}

void Federate::StartupToInitializeStateTransition ()
{
    // child classes may do something with this
}
void Federate::InitializeToExecuteStateTransition ()
{
    // child classes may do something with this
}

void Federate::registerInterfaces (const std::string & /*jsonString*/)
{
    // child classes would implement this
}

FederateInfo LoadFederateInfo (const std::string &jsonString)
{
    FederateInfo fi;
    std::ifstream file (jsonString);
	Json_helics::Value doc;
	
    if (file.is_open ())
    {
		Json_helics::CharReaderBuilder rbuilder;
		std::string errs;
		bool ok = Json_helics::parseFromStream(rbuilder, file, &doc, &errs);
		if (!ok)
		{
			// should I throw an error here?
			return fi;
		}
    }
	else
	{
		Json_helics::Reader stringReader;
		bool ok = stringReader.parse(jsonString, doc, false);
		if (!ok)
		{
			// should I throw an error here?
			return fi;
		}
	}

    
   
    if (doc.isMember ("name"))
    {
        fi.name = doc["name"].asString ();
    }

    if (doc.isMember ("observer"))
    {
        fi.observer = doc["observer"].asBool ();
    }
    if (doc.isMember ("rollback"))
    {
        fi.rollback = doc["rollback"].asBool ();
    }
    if (doc.isMember ("timeAgnostic"))
    {
        fi.timeAgnostic = doc["timeAgnostic"].asBool ();
    }
	if (doc.isMember("sourceOnly"))
	{
		fi.sourceOnly = doc["sourceOnly"].asBool();
	}
    if (doc.isMember ("uninterruptible"))
    {
        fi.uninterruptible = doc["uninterruptible"].asBool ();
    }
	if (doc.isMember("interruptible")) //can use either flag
	{
		fi.uninterruptible = !doc["uninterruptible"].asBool();
	}
    if (doc.isMember ("forwardCompute"))
    {
        fi.forwardCompute = doc["forwardCompute"].asBool ();
    }
    if (doc.isMember ("coreType"))
    {
        fi.coreType = doc["coreType"].asString ();
    }
    if (doc.isMember ("coreName"))
    {
        fi.coreName = doc["coreName"].asString ();
    }
    if (doc.isMember ("coreInit"))
    {
        fi.coreInitString = doc["coreInit"].asString ();
    }
	if (doc.isMember("maxiterations"))
	{
		fi.max_iterations = static_cast<int16_t>(doc["maxiterations"].asInt());
	}
    if (doc.isMember ("period"))
    {
        if (doc["period"].isObject ())
        {
        }
        else
        {
            fi.timeDelta = doc["period"].asDouble ();
        }
    }
   
	if (doc.isMember("timeDelta"))
	{
		if (doc["timeDelta"].isObject())
		{
		}
		else
		{
			fi.timeDelta = doc["timeDelta"].asDouble();
		}
	}

    if (doc.isMember ("lookAhead"))
    {
        if (doc["lookAhead"].isObject ())
        {
			//TODO:: something about units yet
        }
        else
        {
            fi.lookAhead = doc["lookAhead"].asDouble ();
        }
    }
	if (doc.isMember("impactWindow"))
	{
		if (doc["impactWindow"].isObject())
		{
			//TOOD:: something about units yet
		}
		else
		{
			fi.impactWindow = doc["impactWindow"].asDouble();
		}
	}
    return fi;
}
}
