/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "CommonCore.h"
#include "ActionMessage.h"
#include "BasicHandleInfo.h"
#include "EndpointInfo.h"
#include "FederateState.h"
#include "FilterInfo.h"
#include "PublicationInfo.h"
#include "SubscriptionInfo.h"
#include <boost/filesystem.hpp>

#include "helics/common/stringToCmdLine.h"

#include "CoreFactory.h"
#include "FilterFunctions.h"
#include "helics/core/core-exceptions.h"

#include <algorithm>
#include <boost/program_options.hpp>
#include <cassert>
#include <cstring>
#include <fstream>
#include <functional>
#include <sstream>

#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid.hpp>  // uuid class
#include <boost/uuid/uuid_generators.hpp>  // generators
#include <boost/uuid/uuid_io.hpp>  // streaming operators etc.

#define USE_LOGGING 1
#if USE_LOGGING
#if HELICS_HAVE_GLOG
#include <glog/logging.h>
#define ENDL ""
#else
#define LOG(LEVEL) std::cout
#define ENDL std::endl
#endif
#else
#define LOG(LEVEL) std::ostringstream ()
#define ENDL std::endl
#endif

static inline std::string gen_id ()
{
    boost::uuids::uuid uuid = boost::uuids::random_generator () ();
    std::string uuid_str = boost::lexical_cast<std::string> (uuid);
#ifdef _WIN32
    std::string pid_str = boost::lexical_cast<std::string> (GetCurrentProcessId ());
#else
    std::string pid_str = boost::lexical_cast<std::string> (getpid ());
#endif
    return pid_str + "-" + uuid_str;
}

namespace helics
{
using federate_id_t = Core::federate_id_t;
using Handle = Core::Handle;


static void argumentParser (int argc, char *argv[], boost::program_options::variables_map &vm_map);

CommonCore::CommonCore () noexcept {}

CommonCore::CommonCore (const std::string &core_name) : identifier (core_name) {}

void CommonCore::initialize (const std::string &initializationString)
{
    if (!_initialized)  // don't do the compare exchange here since we do that in the initialize fromArgs
    {  // and we can tolerate a spurious call
        stringToCmdLine cmdline (initializationString);
        initializeFromArgs (cmdline.getArgCount (), cmdline.getArgV ());
    }
}

void CommonCore::initializeFromArgs (int argC, char *argv[])
{
    namespace po = boost::program_options;
    bool exp = false;
    if (_initialized.compare_exchange_strong (exp, true))
    {
        po::variables_map vm;
        argumentParser (argC, argv, vm);
        if (vm.count ("min") > 0)
        {
            _min_federates = vm["min"].as<int> ();
        }
        if (vm.count ("minfed") > 0)
        {
            _min_federates = vm["minfed"].as<int> ();
        }

        if (vm.count ("maxiter") > 0)
        {
            _max_iterations = vm["maxiter"].as<int> ();
        }

        if (vm.count ("name") > 0)
        {
            identifier = vm["name"].as<std::string> ();
        }

        if (vm.count ("identifier") > 0)
        {
            identifier = vm["identifier"].as<std::string> ();
        }

        if (identifier.empty ())
        {
            identifier = gen_id ();
        }
        _queue_processing_thread = std::thread (&CommonCore::queueProcessingLoop, this);
    }
}

bool CommonCore::connect ()
{
    if (_initialized)
    {
        bool exp = false;
        if (_connected.compare_exchange_strong (exp, true))
        {
            auto res = brokerConnect ();
            if (res)
            {
                // now register this core object as a broker
                ActionMessage m (CMD_REG_BROKER);
                m.name = getIdentifier ();
                m.info ().target = getAddress ();
                transmit (0, m);
                _connected = true;
            }
            return res;
        }
        return true;
    }
    return false;
}


bool CommonCore::isConnected () const { return _connected; }

void CommonCore::disconnect ()
{
    brokerDisconnect ();
    _connected = false;
    /*We need to enrure that the destructor is not called immediately upon calling unregister
    otherwise this would be a mess and probably cause seg faults so we capture it in a local variable
    that will be destroyed on function exit
    */
    auto keepCoreAlive = CoreFactory::findCore (identifier);
    if (keepCoreAlive)
    {
        CoreFactory::unregisterCore (identifier);
    }

    if (!prevIdentifier.empty ())
    {
        auto keepCoreAlive2 = CoreFactory::findCore (prevIdentifier);
        if (keepCoreAlive2)
        {
            CoreFactory::unregisterCore (prevIdentifier);
        }
    }
}

void argumentParser (int argc, char *argv[], boost::program_options::variables_map &vm_map)
{
    namespace po = boost::program_options;
    po::options_description cmd_only ("command line only");
    po::options_description config ("configuration");
    po::options_description hidden ("hidden");

    // clang-format off
	// input boost controls
	cmd_only.add_options()
		("help,h", "produce help message")
		("version,v", "helics version number")
		("config-file", po::value<std::string>(), "specify a configuration file to use");


	config.add_options()
		("name,n", po::value<std::string>(), "name of the core")
		("minfed", po::value<int>(), "type of the publication to use")
		("maxiter",po::value<int>(),"maximum number of iterations")
		("identifier", po::value<std::string>(), "name of the core");
		

	hidden.add_options() ("min", po::value<int>(), "minimum number of federates");

    // clang-format on

    po::options_description cmd_line ("command line options");
    po::options_description config_file ("configuration file options");
    po::options_description visible ("allowed options");

    cmd_line.add (cmd_only).add (config).add (hidden);
    config_file.add (config);
    visible.add (cmd_only).add (config);

    po::positional_options_description p;
    p.add ("min", -1);

    po::variables_map cmd_vm;
    try
    {
        po::store (
          po::command_line_parser (argc, argv).options (cmd_line).positional (p).allow_unregistered ().run (),
          cmd_vm);
    }
    catch (std::exception &e)
    {
        std::cerr << e.what () << std::endl;
        throw (e);
    }

    po::notify (cmd_vm);

    // objects/pointers/variables/constants


    // program options control
    if (cmd_vm.count ("help") > 0)
    {
        std::cout << visible << '\n';
        return;
    }

    if (cmd_vm.count ("version") > 0)
    {
        std::cout << "Helics Version " << HELICS_VERSION_MAJOR << "." << HELICS_VERSION_MINOR << "."
                  << HELICS_VERSION_PATCH << "  " << HELICS_DATE << '\n';
        return;
    }


    po::store (po::command_line_parser (argc, argv).options (cmd_line).positional (p).allow_unregistered ().run (),
               vm_map);

    if (cmd_vm.count ("config-file") > 0)
    {
        std::string config_file_name = cmd_vm["config-file"].as<std::string> ();
        if (!boost::filesystem::exists (config_file_name))
        {
            std::cerr << "config file " << config_file_name << " does not exist\n";
            throw (std::invalid_argument ("unknown config file"));
        }
        else
        {
            std::ifstream fstr (config_file_name.c_str ());
            po::store (po::parse_config_file (fstr, config_file), vm_map);
            fstr.close ();
        }
    }

    po::notify (vm_map);
}

CommonCore::~CommonCore ()
{
    if (_initialized)
    {
        _queue.push (CMD_STOP);
        _queue_processing_thread.join ();
    }
}

FederateState *CommonCore::getFederate (federate_id_t federateID) const
{
    // only activate the lock if we not in an operating state
    auto lock = (_operating) ? std::unique_lock<std::mutex> (_mutex, std::defer_lock) :
                               std::unique_lock<std::mutex> (_mutex);

    if (isValidIndex (federateID, _federates))
    {
        return _federates[federateID].get ();
    }
    else
    {
        auto fnd = global_id_translation.find (federateID);
        if (fnd != global_id_translation.end ())
        {
            return _federates[fnd->second].get ();
        }
    }

    return nullptr;
}

FederateState *CommonCore::getHandleFederate (Handle id_)
{
    // only activate the lock if we not in an operating state
    auto lock = (_operating) ? std::unique_lock<std::mutex> (_handlemutex, std::defer_lock) :
                               std::unique_lock<std::mutex> (_handlemutex);
    // this list is now constant no need to lock
    if (isValidIndex (id_, handles))
    {  // now need to be careful about deadlock here
        auto lock2 = (_operating) ? std::unique_lock<std::mutex> (_mutex, std::defer_lock) :
                                    std::unique_lock<std::mutex> (_mutex);
        return _federates[handles[id_]->local_fed_id].get ();
    }

    return nullptr;
}

BasicHandleInfo *CommonCore::getHandleInfo (Handle id_) const
{
    // only activate the lock if we not in an operating state
    auto lock = (_operating) ? std::unique_lock<std::mutex> (_handlemutex, std::defer_lock) :
                               std::unique_lock<std::mutex> (_handlemutex);
    if (isValidIndex (id_, handles))
    {
        return handles[id_].get ();
    }

    return nullptr;
}


BasicHandleInfo *CommonCore::getLocalEndpoint (const std::string &name)
{
    // only activate the lock if we not in an operating state
    auto lock = (_operating) ? std::unique_lock<std::mutex> (_handlemutex, std::defer_lock) :
                               std::unique_lock<std::mutex> (_handlemutex);
    auto fnd = endpoints.find (name);
    if (fnd != endpoints.end ())
    {
        return getHandleInfo (fnd->second);
    }
    return nullptr;
}


bool CommonCore::isLocal (Core::federate_id_t global_id) const
{
    // only activate the lock if we not in an operating state
    auto lock = (_operating) ? std::unique_lock<std::mutex> (_mutex, std::defer_lock) :
                               std::unique_lock<std::mutex> (_mutex);
    auto fnd = global_id_translation.find (global_id);
    return (fnd != global_id_translation.end ());
}

int32_t CommonCore::getRoute (Core::federate_id_t global_id) const
{
    // only activate the lock if we not in an operating state
    auto lock = (_operating) ? std::unique_lock<std::mutex> (_mutex, std::defer_lock) :
                               std::unique_lock<std::mutex> (_mutex);
    auto fnd = routing_table.find (global_id);
    return (fnd != routing_table.end ()) ? fnd->second : 0;
}

bool CommonCore::isInitialized () const { return _initialized; }


void CommonCore::error (federate_id_t federateID, int errorCode)
{
    auto fed = getFederate (federateID);
    if (fed == nullptr)
    {
        throw (invalidIdentifier ("federateID not valid"));
    }
    ActionMessage m (CMD_ERROR);
    m.source_id = fed->global_id;
    m.source_handle = errorCode;
    _queue.push (m);
    fed->addAction (m);
    convergence_state ret = convergence_state::complete;
    while (ret != convergence_state::error)
    {
        ret = fed->genericUnspecifiedQueueProcess ();
        if (ret == convergence_state::halted)
        {
            break;
        }
    }
}


void CommonCore::finalize (federate_id_t federateID)
{
    auto fed = getFederate (federateID);
    if (fed == nullptr)
    {
        throw (invalidIdentifier ("federateID not valid"));
    }
    ActionMessage bye (CMD_DISCONNECT);
    bye.source_id = fed->global_id;
    _queue.push (bye);
    fed->addAction (bye);
    convergence_state ret = convergence_state::complete;
    while (ret != convergence_state::halted)
    {
        ret = fed->genericUnspecifiedQueueProcess ();
        if (ret == convergence_state::error)
        {
            break;
        }
    }
}

bool CommonCore::allInitReady () const
{
    std::lock_guard<std::mutex> lock (_mutex);
    // the federate count must be greater than the min size
    if (static_cast<decltype (_min_federates)> (_federates.size ()) < _min_federates)
    {
        return false;
    }
    // all federates must be requesting init
    for (auto &fed : _federates)
    {
        if (fed->init_requested == false)
        {
            return false;
        }
    }
    return true;
}


bool CommonCore::allDisconnected () const
{
    // all federates must have hit finished state
    for (auto &fed : _federates)
    {
        if (fed->getState () != HELICS_FINISHED)
        {
            return false;
        }
    }
    return true;
}

void CommonCore::enterInitializingState (federate_id_t federateID)
{
    auto fed = getFederate (federateID);
    if (fed == nullptr)
    {
        throw (invalidIdentifier ("federateID not valid"));
    }
    switch (fed->getState ())
    {
    case HELICS_CREATED:
        break;
    case HELICS_INITIALIZING:
        return;
    default:
        throw (invalidFunctionCall ());
    }

    bool exp = false;
    if (fed->init_requested.compare_exchange_strong (exp, true))
    {  // only enter this loop once per federate
        ActionMessage m (CMD_INIT);
        m.source_id = fed->global_id;
        _queue.push (m);

        auto check = fed->enterInitState ();
        if (check != convergence_state::complete)
        {
            fed->init_requested = false;
            throw (functionExecutionFailure ());
        }
        return;
    }
    throw (invalidFunctionCall ());
}


convergence_state CommonCore::enterExecutingState (federate_id_t federateID, convergence_state converged)
{
    auto fed = getFederate (federateID);
    if (fed == nullptr)
    {
        throw (invalidIdentifier("federateID not valid"));
    }
    if (HELICS_EXECUTING == fed->getState ())
    {
        return convergence_state::complete;
    }
    if (HELICS_INITIALIZING != fed->getState ())
    {
        throw (invalidFunctionCall ());
    }
    // do an exec check on the fed to process previously received messages so it can't get in a deadlocked state
    ActionMessage exec (CMD_EXEC_CHECK);
    fed->addAction (exec);
    // TODO:: check for error conditions?
    return fed->enterExecutingState (converged);
}


federate_id_t CommonCore::registerFederate (const std::string &name, const CoreFederateInfo &info)
{
    if (!_initialized)
    {
        throw (invalidFunctionCall ());
    }
    if (_operating)
    {
        throw (invalidFunctionCall ());
    }
    auto fed = std::make_unique<FederateState> (name, info);

    std::unique_lock<std::mutex> lock (_mutex);
    auto id = fed->local_id = static_cast<decltype (fed->local_id)> (_federates.size ());

	fed->setParent(this);
    _federates.push_back (std::move (fed));
    federateNames.emplace (name, id);
    lock.unlock ();

    ActionMessage m (CMD_REG_FED);
    m.name = name;
    if (global_broker_id != 0)
    {
        m.source_id = global_broker_id;

        transmit (0, m);  // just directly transmit, no need to process in the queue since it is a priority message
    }
    else
    {
        // this will get processed when this core is assigned a global id
        delayTransmitQueue.push (m);
    }


    // now wait for the federateQueue to get the response
    auto valid = getFederate (id)->waitSetup ();
    if (valid == convergence_state::complete)
    {
        return id;
    }
    throw (registrationFailure ());
}


const std::string &CommonCore::getFederateName (federate_id_t federateID) const
{
    auto fed = getFederate (federateID);
    if (fed == nullptr)
    {
        throw (invalidIdentifier ("federateID not valid"));
    }
    return fed->name;
}


federate_id_t CommonCore::getFederateId (const std::string &name)
{
    std::lock_guard<std::mutex> lock (_mutex);

    auto res = federateNames.find (name);
    if (res != federateNames.end ())
    {
        return res->second;
    }

    return invalid_fed_id;
}


int32_t CommonCore::getFederationSize ()
{
    if (_operating)
    {
        return _global_federation_size;
    }
    // if we are in initialization return the local federation size
    std::lock_guard<std::mutex> lock (_mutex);
    return static_cast<int32_t> (_federates.size ());
}


Time CommonCore::timeRequest (federate_id_t federateID, Time next)
{
    auto fed = getFederate (federateID);
    if (fed == nullptr)
    {
        throw (invalidIdentifier ("federateID not valid"));
    }
    if (HELICS_EXECUTING == fed->getState ())
    {
        auto ret = fed->requestTime (next, convergence_state::complete);
        return ret.stepTime;
    }
    throw (invalidFunctionCall ());
}


iterationTime CommonCore::requestTimeIterative (federate_id_t federateID, Time next, convergence_state converged)
{
    auto fed = getFederate (federateID);
    if (fed == nullptr)
    {
        throw (invalidIdentifier ("federateID not valid"));
    }
    switch (converged)
    {
    case convergence_state::halted:
        finalize (federateID);
        return {fed->grantedTime (), converged};
    case convergence_state::error:
        error (federateID);
        return {fed->grantedTime (), converged};
    default:
        if (HELICS_EXECUTING != fed->getState ())
        {
            throw (invalidFunctionCall ());
        }
    }

    // limit the iterations
    if (converged != convergence_state::complete)
    {
        if (fed->getCurrentIteration () >= _max_iterations)
        {
            converged = convergence_state::complete;
        }
    }

    return fed->requestTime (next, converged);
}


uint64_t CommonCore::getCurrentReiteration (federate_id_t federateID)
{
    auto fed = getFederate (federateID);
    if (fed == nullptr)
    {
        throw invalidIdentifier ("federateID not valid");
    }
    return fed->getCurrentIteration ();
}


void CommonCore::setMaximumIterations (federate_id_t federateID, uint64_t iterations)
{
    auto fed = getFederate (federateID);
    if (fed == nullptr)
    {
        throw (invalidIdentifier ("federateID not valid"));
    }

    auto info = fed->getInfo ();
    info.max_iterations = iterations;
    fed->UpdateFederateInfo (info);
}


void CommonCore::setTimeDelta (federate_id_t federateID, Time time)
{
    auto fed = getFederate (federateID);
    if (fed == nullptr)
    {
        throw (invalidIdentifier ("federateID not valid"));
    }
    if (time < timeZero)
    {
        throw (invalidParameter ("timeDelta must be greater than or equal to zero"));
    }
    // time delta should not be zero but we allow it here for convenience
    if (time == timeZero)
    {
        time = timeEpsilon;
    }
    auto info = fed->getInfo ();
    info.timeDelta = time;
    fed->UpdateFederateInfo (info);
}


void CommonCore::setLookAhead (federate_id_t federateID, Time lookAheadTime)
{
    auto fed = getFederate (federateID);
    if (fed == nullptr)
    {
        throw (invalidIdentifier ("federateID not valid"));
    }
    if (lookAheadTime < timeZero)
    {
        throw (invalidParameter ());
    }
    auto info = fed->getInfo ();
    info.lookAhead = lookAheadTime;
    fed->UpdateFederateInfo (info);
}

void CommonCore::setImpactWindow (federate_id_t federateID, Time impactTime)
{
    auto fed = getFederate (federateID);
    if (fed == nullptr)
    {
        throw (invalidIdentifier ("federateID not valid"));
    }

    if (impactTime < timeZero)
    {
        throw (invalidParameter ());
    }
    auto info = fed->getInfo ();
    info.impactWindow = impactTime;
    fed->UpdateFederateInfo (info);
}


Core::Handle CommonCore::getNewHandle () { return handleCounter++; }

// comparison auto lambda  Functions like a template
static auto compareFunc = [](const auto &A, const auto &B) { return (A->id < B->id); };

BasicHandleInfo* CommonCore::createBasicHandle (Handle id_,
                                    federate_id_t global_federateId,
									federate_id_t local_federateId,
                                    BasicHandleType HandleType,
                                    const std::string &key,
                                    const std::string &type,
                                    const std::string &units,
                                    bool required)
{
    auto hndl = std::make_unique<BasicHandleInfo> (id_, global_federateId, HandleType, key, type, units, required);
	hndl->local_fed_id = local_federateId;
    std::lock_guard<std::mutex> lock (_handlemutex);

    // may need to resize the handles
    if (static_cast<Handle> (handles.size ()) <= id_)
    {
        handles.resize (id_ + 5);
    }
	auto infoPtr = hndl.get();
    handles[id_] = std::move (hndl);
	return infoPtr;
}

Handle CommonCore::registerSubscription (federate_id_t federateID,
                                         const std::string &key,
                                         const std::string &type,
                                         const std::string &units,
                                         handle_check_mode check_mode)
{
    LOG (INFO) << "registering SUB " << key << ENDL;

    auto fed = getFederate (federateID);
    if (fed == nullptr)
    {
        throw (invalidIdentifier ("federateID not valid"));
    }
    if (fed->getState () != HELICS_CREATED)
    {
        throw (invalidFunctionCall ());
    }

    auto id = getNewHandle ();
    fed->createSubscription (id, key, type, units, check_mode);

    createBasicHandle (id, fed->global_id, fed->local_id,HANDLE_SUB, key, type, units,
                       (check_mode == handle_check_mode::required));

	
    ActionMessage m (CMD_REG_SUB);
    m.source_id = fed->global_id;
    m.source_handle = id;
    m.name = key;
    m.info ().type = type;
    m.info ().units = units;
    m.required = (check_mode == handle_check_mode::required);

    std::unique_lock<std::mutex> lock (_handlemutex);
    auto fndpub = publications.find (key);
    if (fndpub != publications.end ())
    {
        auto pubhandle = fndpub->second;
        auto pubid = handles[pubhandle]->fed_id;
        lock.unlock ();
        m.processingComplete = true;
        // send to broker and core
        addCommand (m);
        // now send the same command to the publication
        m.dest_handle = pubhandle;
        m.dest_id = pubid;
        // send to
        addCommand (m);
        // now send the notification to the subscription
        ActionMessage notice (CMD_NOTIFY_PUB);
        notice.dest_id = fed->global_id;
        notice.dest_handle = id;
        notice.source_id = pubid;
        notice.source_handle = pubhandle;
        fed->addAction (notice);
    }
    else
    {
        lock.unlock ();
        // we didn't find it so just pass it on to the broker
        addCommand (m);
    }

    return id;
}


Handle CommonCore::getSubscription (federate_id_t federateID, const std::string &key)
{
    auto fed = getFederate (federateID);
    if (fed != nullptr)
    {
        return fed->getSubscription (key)->id;
    }
    return invalid_Handle;
}


Handle CommonCore::registerPublication (federate_id_t federateID,
                                        const std::string &key,
                                        const std::string &type,
                                        const std::string &units)
{
    LOG (INFO) << "registering PUB " << key << ENDL;


    auto fed = getFederate (federateID);
    if (fed == nullptr)
    {
        throw (invalidIdentifier ("federateID not valid"));
    }
    if (fed->getState () != HELICS_CREATED)
    {
        throw (invalidFunctionCall ());
    }
    std::unique_lock<std::mutex> lock (_handlemutex);
    auto fnd = publications.find (key);
    if (fnd != publications.end ())  // this key is already found
    {
        throw (invalidParameter ());
    }
    auto id = getNewHandle ();
    publications.emplace (key, id);
    lock.unlock ();
    fed->createPublication (id, key, type, units);

    createBasicHandle (id, fed->global_id,fed->local_id, HANDLE_PUB, key, type, units, false);

    ActionMessage m (CMD_REG_PUB);
    m.source_id = fed->global_id;
    m.source_handle = id;
    m.name = key;
    m.info ().type = type;
    m.info ().units = units;

    _queue.push (m);
    return id;
}


Handle CommonCore::getPublication (federate_id_t federateID, const std::string &key)
{
    auto fed = getFederate (federateID);
    if (fed != nullptr)
    {
        auto pub = fed->getPublication (key);
        if (pub != nullptr)
        {
            return pub->id;
        }
    }
    return invalid_Handle;
}

const std::string nullStr;

const std::string &CommonCore::getUnits (Handle handle) const
{
    auto handleInfo = getHandleInfo (handle);
    if (handleInfo != nullptr)
    {
        return handleInfo->units;
    }
    return nullStr;
}


const std::string &CommonCore::getType (Handle handle) const
{
    auto handleInfo = getHandleInfo (handle);
    if (handleInfo != nullptr)
    {
        return handleInfo->units;
    }
    return nullStr;
}


void CommonCore::setValue (Handle handle_, const char *data, uint64_t len)
{
    auto handleInfo = getHandleInfo (handle_);
	if (handleInfo == nullptr)
	{
		throw(invalidIdentifier("Handle not valid"));
	}
    if (handleInfo->what != HANDLE_PUB)
    {
        throw (invalidIdentifier ("handle does not point to a publication"));
    }


    if (!handleInfo->used)
    {
        return;  // if the value is not required do nothing
    }
    LOG (INFO) << "setValue: '" << std::string (data, len) << "'" << ENDL;
    ActionMessage mv (CMD_PUB);
    mv.source_id = handleInfo->fed_id;
    mv.source_handle = handle_;
    mv.payload = std::string (data, len);

    _queue.push (mv);
}


std::shared_ptr<const data_block> CommonCore::getValue (Handle handle_)
{
    auto handleInfo = getHandleInfo (handle_);
    if (handleInfo == nullptr)
    {
        throw (invalidIdentifier ("Handle is invalid"));
    }
    if (handleInfo->what != HANDLE_SUB)
    {
        throw (invalidIdentifier ("Handle does not identify a subscription"));
    }

    return getFederate (handleInfo->local_fed_id)->getSubscription (handle_)->getData ();
}


const std::vector<Handle> &CommonCore::getValueUpdates (federate_id_t federateID)
{
    auto fed = getFederate (federateID);
    if (fed == nullptr)
    {
        throw (invalidIdentifier ("federateID not valid"));
    }
    return fed->getEvents ();
}


Handle CommonCore::registerEndpoint (federate_id_t federateID, const std::string &name, const std::string &type)
{
    auto fed = getFederate (federateID);
    if (fed == nullptr)
    {
        throw (invalidIdentifier ("federateID not valid"));
    }
    if (fed->getState () != HELICS_CREATED)
    {
        throw (invalidFunctionCall ());
    }

    std::unique_lock<std::mutex> lock (_mutex);
    auto fnd = endpoints.find (name);
    if (fnd != endpoints.end ())
    {
        throw (-1);  // TODO: make a set of exceptions;
    }
    auto id = getNewHandle ();
    endpoints.emplace (name, id);
    lock.unlock ();

    fed->createEndpoint (id, name, type);

    createBasicHandle (id, fed->global_id,fed->local_id, HANDLE_END, name, type, "", false);

    ActionMessage m (CMD_REG_END);
    m.source_id = fed->global_id;
    m.source_handle = id;
    m.name = name;
    m.info ().type = type;

    _queue.push (m);
    return id;
}


Handle CommonCore::registerSourceFilter (federate_id_t federateID,
                                         const std::string &filterName,
                                         const std::string &source,
                                         const std::string &type_in)
{
    auto fed = getFederate (federateID);
    if (fed == nullptr)
    {
        throw (invalidIdentifier ("federateID not valid"));
    }
    if (fed->getState () != HELICS_CREATED)
    {
        throw (invalidFunctionCall ());
    }

    auto id = getNewHandle ();
    fed->createSourceFilter (id, filterName, source, type_in);

    createBasicHandle (id, fed->global_id,fed->local_id, HANDLE_SOURCE_FILTER, filterName, type_in, source, false);

    ActionMessage m (CMD_REG_SRC_FILTER);
    m.source_id = fed->global_id;
    m.source_handle = id;
    m.name = filterName;
    m.info ().target = source;
    m.info ().type = type_in;

    std::unique_lock<std::mutex> lock (_handlemutex);
    auto fndend = endpoints.find (source);
    if (fndend != endpoints.end ())
    {
        auto endhandle = fndend->second;
        auto endid = handles[endhandle]->fed_id;
        lock.unlock ();
        m.processingComplete = true;
        // send to broker and core
        addCommand (m);
        // now send the same command to the endpoint
        m.dest_handle = endhandle;
        m.dest_id = endid;
        // send to
        addCommand (m);
    }
    else
    {
        lock.unlock ();
        //
        addCommand (m);
    }
    return id;
}


Handle CommonCore::registerDestinationFilter (federate_id_t federateID,
                                              const std::string &filterName,
                                              const std::string &dest,
                                              const std::string &type_in)
{
    auto fed = getFederate (federateID);
    if (fed == nullptr)
    {
        throw (invalidIdentifier ("federateID not valid"));
    }
    if (fed->getState () != HELICS_CREATED)
    {
        throw (invalidFunctionCall ());
    }

    auto id = getNewHandle ();
    fed->createDestFilter (id, filterName, dest, type_in);

    createBasicHandle (id, fed->global_id,fed->local_id, HANDLE_DEST_FILTER, filterName, type_in, dest, true);

    ActionMessage m (CMD_REG_DST_FILTER);
    m.source_id = fed->global_id;
    m.source_handle = id;
    m.name = filterName;
    m.info ().target = dest;
    m.info ().type = type_in;

    std::unique_lock<std::mutex> lock (_handlemutex);

    auto fndend = endpoints.find (dest);
    if (fndend != endpoints.end ())
    {
        auto endhandle = fndend->second;
        auto endid = handles[endhandle]->fed_id;
        lock.unlock ();
        m.processingComplete = true;
        // send to broker and core
        addCommand (m);
        // now send the same command to the endpoint
        m.dest_handle = endhandle;
        m.dest_id = endid;
        // send to
        addCommand (m);
    }
    else
    {
        lock.unlock ();
        //
        addCommand (m);
    }
    return id;
}


void CommonCore::registerFrequentCommunicationsPair (const std::string &source, const std::string &dest)
{
    std::lock_guard<std::mutex> lock (_mutex);

    assert (isInitialized ());
    assert (false);
}

void CommonCore::addDependency (federate_id_t federateId, const std::string &federateName) {}

void CommonCore::send (Handle sourceHandle, const std::string &destination, const char *data, uint64_t length)
{
    auto hndl = getHandleInfo (sourceHandle);
    if (hndl == nullptr)
    {
        throw (invalidIdentifier ("handle is not valid"));
    }

	if (hndl->what != HANDLE_END)
	{
		throw(invalidIdentifier("handle does not point to an endpoint"));
	}
    auto fed = getFederate (hndl->local_fed_id);
    ActionMessage m (CMD_SEND_MESSAGE);

    m.info ().orig_source = hndl->key;
    m.info ().source = hndl->key;
    m.source_handle = sourceHandle;
    m.source_id = hndl->fed_id;

    m.payload = std::string (data, length);
    m.info ().target = destination;
    m.actionTime = fed->grantedTime ();

    queueMessage (processMessage (hndl, m));
}


void CommonCore::sendEvent (Time time,
                            Handle sourceHandle,
                            const std::string &destination,
                            const char *data,
                            uint64_t length)
{
    auto hndl = getHandleInfo (sourceHandle);
    if (hndl == nullptr)
    {
        throw (invalidIdentifier ("handle is not valid"));
    }
	if (hndl->what != HANDLE_END)
	{
		throw(invalidIdentifier("handle does not point to an endpoint"));
	}
    ActionMessage m (CMD_SEND_MESSAGE);
    m.source_handle = sourceHandle;
    m.source_id = hndl->fed_id;
    m.actionTime = time;
    m.payload = std::string (data, length);
    m.info ().orig_source = hndl->key;
    m.info ().source = hndl->key;
    m.info ().target = destination;


    queueMessage (processMessage (hndl, m));
}


void CommonCore::sendMessage (Handle sourceHandle, std::unique_ptr<Message> message)
{
    auto hndl = getHandleInfo (sourceHandle);
    if (hndl == nullptr)
    {
        throw (invalidIdentifier ("handle is not valid"));
    }
	if (hndl->what != HANDLE_END)
	{
		throw(invalidIdentifier("handle does not point to an endpoint"));
	}
    ActionMessage m (CMD_SEND_MESSAGE);

    m.info ().orig_source = std::move (message->origsrc);

    if (hndl == nullptr)
    {
        m.info ().source = std::move (message->src);
    }
    else
    {
        m.info ().source = hndl->key;
        m.source_handle = hndl->id;
        m.source_id = hndl->fed_id;
    }
    m.payload = std::move (message->data.to_string ());
    m.info ().target = std::move (message->dest);
    m.actionTime = message->time;
    m.source_handle = sourceHandle;

    queueMessage (processMessage (hndl, m));
}

ActionMessage &CommonCore::processMessage (BasicHandleInfo *hndl, ActionMessage &m)
{
    if (hndl == nullptr)
    {
        return m;
    }
    auto filtFunc = getFilterFunctions (hndl->id);
    if (filtFunc->hasSourceOperators)
    {
        auto tempMessage = createMessage (std::move (m));
        for (auto &so : filtFunc->sourceOperators)
        {
            auto FiltI = getFederate (so.first)->getFilter (so.second);
            assert (FiltI->filterOp != nullptr);
            tempMessage = FiltI->filterOp->process (std::move (tempMessage));
        }
        m = ActionMessage (std::move (tempMessage));
    }
    if (filtFunc->hasSourceFilter)
    {
        m.setAction (CMD_SEND_FOR_FILTER);
        m.dest_handle = filtFunc->finalSourceFilter.second;
        m.dest_id = filtFunc->finalSourceFilter.first;
    }
    return m;
}

void CommonCore::queueMessage (ActionMessage &message)
{
    if (message.action () == CMD_SEND_MESSAGE)
    {
        // Find the destination endpoint
        auto localP = getLocalEndpoint (message.info ().target);
        if (localP == nullptr)
        {  // must be a remote endpoint push it to the main queue to deal with
            _queue.push (message);
            return;
        }
        if (localP->destFilter)  // the endpoint has a destination filter
        {
            auto ffunc = getFilterFunctions (localP->id);
            assert (ffunc->hasDestOperator);
            auto FiltI = getFederate (ffunc->destOperator.first)->getFilter (ffunc->destOperator.second);
            assert (FiltI->filterOp != nullptr);

            auto tempMessage = createMessage (message);
            auto nmessage = FiltI->filterOp->process (std::move (tempMessage));

            message.payload = std::move (nmessage->data.to_string ());


            message.actionTime = nmessage->time;
        }
        auto fed = getFederate (localP->local_fed_id);
        fed->addAction (message);
    }
}


uint64_t CommonCore::receiveCount (Handle destination)
{
    auto fed = getHandleFederate (destination);
    if (fed == nullptr)
    {
        return 0;
    }
    return fed->getQueueSize (destination);
}


std::unique_ptr<Message> CommonCore::receive (Handle destination)
{
    auto fed = getHandleFederate (destination);
    if (fed == nullptr)
    {
        throw (invalidIdentifier ("invalid handle"));
    }
    if (fed->getState () != HELICS_EXECUTING)
    {
        return nullptr;
    }

    return fed->receive (destination);
}


std::unique_ptr<Message> CommonCore::receiveAny (federate_id_t federateID, Handle &endpoint_id)
{
    auto fed = getFederate (federateID);
    if (fed == nullptr)
    {
        throw (invalidIdentifier ("FederateID is not valid"));
    }
    if (fed->getState () != HELICS_EXECUTING)
    {
        endpoint_id = invalid_Handle;
        return nullptr;
    }
    return fed->receiveAny (endpoint_id);
}


uint64_t CommonCore::receiveCountAny (federate_id_t federateID)
{
    auto fed = getFederate (federateID);
    if (fed == nullptr)
    {
        throw (invalidIdentifier ("FederateID is not valid"));
    }
    if (fed->getState () != HELICS_EXECUTING)
    {
        return 0;
    }

    return fed->getQueueSize ();
}

void CommonCore::logMessage (federate_id_t federateID, int logLevel, const std::string &logMessage)
{
   
    auto fed = getFederate (federateID);
    if (fed == nullptr)
    {
        throw (invalidIdentifier ("FederateID is not valid"));
    }
	ActionMessage m(CMD_LOG);

    m.source_id = fed->global_id;
    m.index = logLevel;
    m.payload = logMessage;
    _queue.push (m);
    sendToLogger (federateID, logLevel, fed->name, logMessage);
}

void CommonCore::sendToLogger (federate_id_t federateID,
                               int logLevel,
                               const std::string &name,
                               const std::string &message) const
{
    auto fed = getFederate (federateID);
    if (fed == nullptr)
    {
        throw (invalidIdentifier ());
    }
    // TODO:: make federateState logging function
}

void CommonCore::setLoggingFunction (
  federate_id_t federateID,
  std::function<void(int, const std::string &, const std::string &)> logFunction)
{
    auto fed = getFederate (federateID);
    if (fed == nullptr)
    {
        throw (invalidIdentifier ("Invalid FederateID"));
    }
    fed->setLogger (std::move (logFunction));
}

void CommonCore::setFilterOperator (Handle filter, std::shared_ptr<FilterOperator> callback)
{
    auto hndl = getHandleInfo (filter);
	if (hndl == nullptr)
	{
		throw(invalidIdentifier("filter is not a valid handle"));
	}
    if ((hndl->what != HANDLE_DEST_FILTER) && (hndl->what != HANDLE_SOURCE_FILTER))
    {
        throw (invalidIdentifier ("filter identifier does not point a filter"));
    }

    auto FiltI = getFederate (hndl->fed_id)->getFilter (filter);

    FiltI->filterOp = callback;
}

FilterFunctions *CommonCore::getFilterFunctions (Handle id_)
{
    auto hndl = getHandleInfo (id_);
	if (hndl == nullptr)
	{
		throw(invalidIdentifier("filter is not a valid handle"));
	}
	if ((hndl->what != HANDLE_DEST_FILTER) && (hndl->what != HANDLE_SOURCE_FILTER))
	{
		throw (invalidIdentifier("filter identifier does not point a filter"));
	}
    auto fnd = filters.find (id_);
    if (fnd == filters.end ())
    {
		//just make a dummy filterFunction so we have something to return
        auto ff = std::make_unique<FilterFunctions> ();
        auto ffp = ff.get ();
        filters.emplace (id_, std::move (ff));
        return ffp;
    }
    else
    {
        return fnd->second.get ();
    }
}


uint64_t CommonCore::receiveFilterCount (federate_id_t federateID)
{
    auto fed = getFederate (federateID);
    if (fed == nullptr)
    {
        throw (invalidIdentifier ("federateID is not valid"));
    }
    if (fed->getState () != HELICS_EXECUTING)
    {
        return 0;
    }

    return fed->getQueueSize ();
}

std::unique_ptr<Message> CommonCore::receiveAnyFilter (federate_id_t federateID, Handle &filter_id)
{
    auto fed = getFederate (federateID);
    if (fed == nullptr)
    {
        throw (invalidIdentifier ("federateID is not valid"));
    }
    if (fed->getState () != HELICS_EXECUTING)
    {
        filter_id = invalid_Handle;
        return nullptr;
    }
    return fed->receiveAny (filter_id);
}


void CommonCore::setIdentifier (const std::string &name)
{
    if (!_initialized)
    {
        std::lock_guard<std::mutex> lock (_mutex);
        identifier = name;
    }
	else
	{
		throw(invalidFunctionCall("setIdentifier can only be called before the core is initialized"));
	}
}


void CommonCore::addCommand (const ActionMessage &m)
{
    if (isPriorityCommand (m))
    {
        processPriorityCommand (m);
    }
    else
    {
        // just route to the general queue;
        _queue.push (m);
    }
}

void CommonCore::queueProcessingLoop ()
{
    while (true)
    {
        auto command = _queue.pop ();
        // LOG (INFO) << "\"\"\"" << command << std::endl << "\"\"\"" << ENDL;

        switch (command.action ())
        {
        case CMD_IGNORE:
            break;
        case CMD_STOP:
            processCommand (command);
            return disconnect ();  // this can potential cause object destruction so do nothing after this call
        default:
            processCommand (command);
        }
    }
}


void CommonCore::processPriorityCommand (const ActionMessage &command)
{
    // deal with a few types of message immediately
    switch (command.action ())
    {
    case CMD_REG_FED:
    case CMD_REG_BROKER:
        // These really shouldn't happen here probably means something went wrong in setup but we can handle it
        // forward the connection request to the higher level
        transmit (0, command);
        break;
    case CMD_BROKER_ACK:
        if (command.payload == identifier)
        {
            if (!command.error)
            {
                global_broker_id = command.dest_id;
                transmitDelayedMessages ();
                return;
            }
            else
            {
                // generate error messages in response to all the delayed messages
            }
        }
        break;
    case CMD_FED_ACK:
    {
        auto id = getFederateId (command.name);
        if (id != invalid_fed_id)
        {
            auto fed = getFederate (id);
            if (fed == nullptr)
            {
                break;
            }
            // now add the new global id to the translation table
            {  // scope for the lock
                std::lock_guard<std::mutex> lock (_mutex);
                global_id_translation.emplace (command.dest_id, fed->local_id);
            }
            // push the command to the local queue
            fed->addAction (command);
        }
    }
    break;
    case CMD_REG_ROUTE:
        // TODO:: double check this
        addRoute (command.dest_handle, command.payload);
        break;
    }
}


void CommonCore::transmitDelayedMessages ()
{
    auto msg = delayTransmitQueue.pop ();
    while (msg)
    {
        msg->source_id = global_broker_id;
        transmit (0, *msg);
        msg = delayTransmitQueue.pop ();
    }
}

void CommonCore::processCommand (ActionMessage &command)
{
    // LOG (INFO) << "\"\"\"" << command << std::endl << "\"\"\"" << ENDL
    switch (command.action ())
    {
    case CMD_IGNORE:
        break;
    case CMD_STOP:
        if (!allDisconnected ())
        {  // only send a disconnect message if we haven't done so already
            ActionMessage m (CMD_DISCONNECT);
            m.source_id = global_broker_id;
            transmit (0, m);
        }
        break;
    case CMD_TIME_REQUEST:
    case CMD_TIME_GRANT:
    case CMD_EXEC_GRANT:
    case CMD_EXEC_REQUEST:
    {
        if (command.dest_id == 0)
        {
            // route the message to all dependent feds
            auto fed = getFederate (command.source_id);
            if (fed == nullptr)
            {
                return;
            }
            auto &dep = fed->getDependents ();
            for (auto &fed_id : dep)
            {
                routeMessage (command, fed_id);
            }
        }
        else
        {
            routeMessage (command);
        }
    }
    break;
    case CMD_SEND_FOR_FILTER:
        routeMessage (command);
        break;
    case CMD_PUB:
    {
        // route the message to all the subscribers
        auto pubInfo = getFederate (command.source_id)->getPublication (command.source_handle);
        for (auto &subscriber : pubInfo->subscribers)
        {
            command.dest_id = subscriber.first;
            command.dest_handle = subscriber.second;
            routeMessage (command);
        }
    }
    break;
    case CMD_DISCONNECT:
        if (allDisconnected ())
        {
            command.source_id = global_broker_id;
            transmit (0, command);
            addCommand (CMD_STOP);
        }
        break;
    case CMD_LOG:
    case CMD_ERROR:
        routeMessage (command);
        break;
    case CMD_REG_SUB:
        // for these registration filters any processing is already done in the
        // registration functions so this is just a router
        if (command.dest_id != 0)
        {
            auto fed = getFederate (command.dest_id);
            if (fed != nullptr)
            {
                fed->addAction (command);
                auto pubhandle = getHandleInfo (command.dest_handle);
                if (pubhandle != nullptr)
                {
                    pubhandle->used = true;
                }
            }
        }
        else
        {
            transmit (0, command);
        }

        break;
    case CMD_REG_END:
        // for these registration filters any processing is already done in the
        // registration functions so this is just a router
        if (command.dest_id != 0)
        {
            auto fed = getFederate (command.dest_id);
            if (fed != nullptr)
            {
                fed->addAction (command);
                auto filthandle = getHandleInfo (command.dest_handle);
                if (filthandle != nullptr)
                {
                    if ((filthandle->what == HANDLE_DEST_FILTER) || (filthandle->what == HANDLE_SOURCE_FILTER))
                    {
                        filthandle->used = true;
                    }
                }
            }
        }
        else
        {
            transmit (0, command);
        }

        break;
    case CMD_REG_PUB:
    case CMD_REG_DST_FILTER:
    case CMD_REG_SRC_FILTER:
        // for these registration filters any processing is already done in the
        // registration functions so this is just a router
        routeMessage (command);
        break;

    case CMD_NOTIFY_SUB:
    {
        // just forward these to the appropriate federate
        auto fed = getFederate (command.dest_id);
        if (fed != nullptr)
        {
            fed->addAction (command);
            auto pubhandle = getHandleInfo (command.dest_handle);
            if (pubhandle != nullptr)
            {
                pubhandle->used = true;
            }
        }
    }
    break;
    case CMD_NOTIFY_END:
    {
        // just forward these to the appropriate federate
        auto fed = getFederate (command.dest_id);
        if (fed != nullptr)
        {
            fed->addAction (command);
            auto filthandle = getHandleInfo (command.dest_handle);
            if (filthandle != nullptr)
            {
                if ((filthandle->what == HANDLE_DEST_FILTER) || (filthandle->what == HANDLE_SOURCE_FILTER))
                {
                    filthandle->used = true;
                }
            }
        }
    }
    break;
    case CMD_NOTIFY_PUB:
    case CMD_NOTIFY_DST_FILTER:
    case CMD_NOTIFY_SRC_FILTER:
    {
        // just forward these to the appropriate federate
        auto fed = getFederate (command.dest_id);
        if (fed != nullptr)
        {
            fed->addAction (command);
        }
    }
    break;
    case CMD_INIT:
        if (allInitReady ())
        {
            command.source_id = global_broker_id;
            transmit (0, command);
        }
        break;
    case CMD_INIT_GRANT:
        // forward the grant to all federates
        for (auto &fed : _federates)
        {
            fed->addAction (command);
        }
        break;

    case CMD_SEND_MESSAGE:
    {
        auto fnd = endpoints.find (command.info ().target);
        if (fnd != endpoints.end ())
        {  // destination is local
            auto fed = getHandleFederate (fnd->second);
            fed->addAction (command);
            command.dest_id = fed->global_id;
            command.dest_handle = fnd->second;
        }
        else
        {
            auto kfnd = knownExternalEndpoints.find (command.info ().target);
            if (kfnd != knownExternalEndpoints.end ())
            {  // destination is known
                auto route = getRoute (kfnd->second);
                transmit (route, command);
            }
            else
            {
                transmit (0, command);
            }
        }
    }
    break;
    default:
        if (isPriorityCommand (command))
        {  // this is a backup if somehow one of these message got here
            processPriorityCommand (command);
        }
        break;
    }
}

void CommonCore::routeMessage (ActionMessage &cmd, federate_id_t dest)
{
    if (isLocal (dest))
    {
        auto fed = getFederate (dest);
        if (fed != nullptr)
        {
            fed->addAction (cmd);
        }
    }
    else
    {
        auto route = getRoute (dest);
        cmd.dest_id = dest;
        transmit (route, cmd);
    }
}

void CommonCore::routeMessage (const ActionMessage &cmd)
{
    if (isLocal (cmd.dest_id))
    {
        auto fed = getFederate (cmd.dest_id);
        if (fed != nullptr)
        {
            fed->addAction (cmd);
        }
    }
    else
    {
        auto route = getRoute (cmd.dest_id);
        transmit (route, cmd);
    }
}

}  // namespace helics
