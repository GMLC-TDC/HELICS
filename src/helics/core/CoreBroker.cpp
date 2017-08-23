/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "CoreBroker.h"
#include "common/stringToCmdLine.h"
#include "BrokerFactory.h"

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid.hpp>  // uuid class
#include <boost/uuid/uuid_generators.hpp>  // generators
#include <boost/uuid/uuid_io.hpp>  // streaming operators etc.
#include "TimeCoordinator.h"
#include <fstream>


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
bool matchingTypes (const std::string &type1, const std::string &type2);

void CoreBroker::queueProcessingLoop ()
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
			processCommand(command);
			return disconnect();
        default:
            processCommand (command);
            break;
        }
    }
}

CoreBroker::~CoreBroker ()
{
    if (_initialized)
    {
		_queue.push(CMD_STOP);
		_queue_processing_thread.join();
    }
}


void CoreBroker::setIdentifier (const std::string &name)
{
    if (!_connected)  // can't be changed after initialization
    {
        std::lock_guard<std::mutex> lock (mutex_);
        local_broker_identifier = name;
    }
}
int32_t CoreBroker::getRoute (Core::federate_id_t fedid) const
{
    // only activate the lock if we not in an operating state
    auto lock = (_operating) ? std::unique_lock<std::mutex> (mutex_, std::defer_lock) :
                               std::unique_lock<std::mutex> (mutex_);

    auto fnd = routing_table.find (fedid);
    return (fnd != routing_table.end ()) ? fnd->second : 0;  // zero is the default route
}

int32_t CoreBroker::getRouteNoLock (Core::federate_id_t fedid) const
{
    auto fnd = routing_table.find (fedid);
    return (fnd != routing_table.end ()) ? fnd->second : 0;  // zero is the default route
}


int32_t CoreBroker::getFedByName (const std::string &fedName) const
{
    // only activate the lock if we not in an operating state
    auto lock = (_operating) ? std::unique_lock<std::mutex> (mutex_, std::defer_lock) :
                               std::unique_lock<std::mutex> (mutex_);

    auto fnd = fedNames.find (fedName);
    return (fnd != fedNames.end ()) ? fnd->second : -1;
}


int32_t CoreBroker::getBrokerByName (const std::string &brokerName) const
{
    // only activate the lock if we not in an operating state
    auto lock = (_operating) ? std::unique_lock<std::mutex> (mutex_, std::defer_lock) :
                               std::unique_lock<std::mutex> (mutex_);

    auto fnd = brokerNames.find (brokerName);
    return (fnd != brokerNames.end ()) ? fnd->second : -1;
}

int32_t CoreBroker::getBrokerById (Core::federate_id_t fedid) const
{
    if (_isRoot)
    {
        return static_cast<int32_t> (fedid - global_broker_id_shift);
    }
    else
    {
        auto lock = (_operating) ? std::unique_lock<std::mutex> (mutex_, std::defer_lock) :
                                   std::unique_lock<std::mutex> (mutex_);

        auto fnd = broker_table.find (fedid);
        return (fnd != broker_table.end ()) ? fnd->second : -1;
    }
}

int32_t CoreBroker::getFedById (Core::federate_id_t fedid) const
{
    if (_isRoot)
    {
        return static_cast<int32_t> (fedid-global_federate_id_shift);
    }
    else
    {
        auto lock = (_operating) ? std::unique_lock<std::mutex> (mutex_, std::defer_lock) :
                                   std::unique_lock<std::mutex> (mutex_);

        auto fnd = federate_table.find (fedid);
        return (fnd != federate_table.end ()) ? fnd->second : -1;
    }
}

void CoreBroker::addMessage (const ActionMessage &m)
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


void CoreBroker::processPriorityCommand (const ActionMessage &command)
{
    // deal with a few types of message immediately
    switch (command.action ())
    {
    case CMD_REG_FED:
    {
        if (!_operating)
        {
            if (allInitReady ())
            {
                // send an init not ready
            }
        }
        else  // we are initialized already
        {
            ActionMessage badInit (CMD_FED_ACK);
            badInit.error = true;
            badInit.source_id = global_broker_id;
            badInit.name = command.name;
            transmit (command.source_id, badInit);  // this isn't correct
            return;
        }
        std::unique_lock<std::mutex> lock (mutex_);
        _federates.emplace_back (command.name);
        _federates.back ().route_id = getRouteNoLock (command.source_id);

        fedNames.emplace (command.name, static_cast<int32_t> (_federates.size () - 1));
        if (!_isRoot)
        {
            lock.unlock ();  // finished critical section
            if (_gateway)
            {
                ActionMessage mcopy (CMD_REG_FED);
                mcopy.name = command.name;
                mcopy.info ().target = getAddress ();
                if (global_broker_id != 0)
                {
                    mcopy.source_id = global_broker_id;
                    transmit (0, mcopy);
                }
                else
                {
                    // delay the response if we are not fully registered yet
                    delayTransmitQueue.push (mcopy);
                }
            }
            else
            {
                transmit (0, command);
            }
        }
        else
        {
            _federates.back ().global_id = static_cast<Core::federate_id_t> (_federates.size ()) - 1+global_federate_id_shift;
            auto route_id = _federates.back ().route_id;
            auto global_id = _federates.back ().global_id;
            routing_table.emplace (global_id, route_id);
            // don't bother with the federate_table
            lock.unlock ();
            // transmit the response
            ActionMessage fedReply (CMD_FED_ACK);
            fedReply.source_id = 0;
            fedReply.dest_id = global_id;
            fedReply.name = command.name;
            transmit (route_id, fedReply);
        }
    }
    break;
    case CMD_REG_BROKER:
    {
        if (!_operating)
        {
            if (allInitReady ())
            {
                // send an init not ready as we were ready now we are not
            }
        }
        else  // we are initialized already
        {
            ActionMessage badInit (CMD_BROKER_ACK);
            badInit.error = true;
            badInit.source_id = global_broker_id;
            badInit.name = command.name;
            transmit (command.source_id, badInit);
            return;
        }
        std::unique_lock<std::mutex> lock (mutex_);
        _brokers.emplace_back (command.name);
        _brokers.back ().route_id = static_cast<decltype (_brokers.back ().route_id)> (_brokers.size ());
        brokerNames.emplace (command.name, static_cast<int32_t> (_brokers.size () - 1));
        addRoute (_brokers.back ().route_id, command.info ().target);
        if (!_isRoot)
        {
            lock.unlock ();
            if (_gateway)
            {
                auto mcopy = command;
                mcopy.source_id = global_broker_id;
                transmit (0, mcopy);
            }
            else
            {
                transmit (0, command);
            }
        }
        else
        {
            _brokers.back ().global_id =
              static_cast<Core::federate_id_t> (_brokers.size ()) - 1 + global_broker_id_shift;
            auto global_id = _brokers.back ().global_id;
            auto route_id = _brokers.back ().route_id;
            routing_table.emplace (global_id, route_id);
            // don't bother with the broker_table for root broker
            // the protected tables are finished we can now unlock
            lock.unlock ();

            // sending the response message
            ActionMessage brokerReply (CMD_BROKER_ACK);
            brokerReply.source_id = 0;  // source is global root
            brokerReply.dest_id = global_id;  // the new id
            brokerReply.name = command.name;  // the identifier of the broker
            transmit (route_id, brokerReply);
        }
    }
    break;
    case CMD_FED_ACK:
    {  // we can't be root if we got one of these
        auto fed_num = getFedByName (command.name);
        int32_t route;
        if (fed_num >= 0)
        {
            std::unique_lock<std::mutex> lock (mutex_);
            _federates[fed_num].global_id = command.dest_id;
            route = _federates[fed_num].route_id;
            federate_table.emplace (command.dest_id, fed_num);
            lock.unlock ();
            transmit (_federates[fed_num].route_id, command);
        }
        else
        {
            // this means we haven't seen this federate before for some reason
            std::unique_lock<std::mutex> lock (mutex_);
            _federates.emplace_back (command.name);
            _federates.back ().route_id = getRoute (command.source_id);
            _federates.back ().global_id = command.dest_id;
            fedNames.emplace (command.name, static_cast<int32_t> (_federates.size () - 1));
            federate_table.emplace (command.dest_id, static_cast<int32_t> (_federates.size () - 1));
            // it also means we don't forward it
        }
    }
    break;
    case CMD_BROKER_ACK:
    {  // we can't be root if we got one of these
        if (command.name == local_broker_identifier)
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
                return;
            }
        }
        auto broker_num = getBrokerByName (command.name);
        int32_t route;
        if (broker_num >= 0)
        {
            std::unique_lock<std::mutex> lock (mutex_);
            _brokers[broker_num].global_id = command.dest_id;
            route = _brokers[broker_num].route_id;
            broker_table.emplace (command.dest_id, broker_num);
            lock.unlock ();
            transmit (_brokers[broker_num].route_id, command);
        }
        else
        {
            std::unique_lock<std::mutex> lock (mutex_);
            _brokers.emplace_back (command.name);
            _brokers.back ().route_id = getRoute (command.source_id);
            _brokers.back ().global_id = command.dest_id;
            brokerNames.emplace (command.name, static_cast<int32_t> (_brokers.size () - 1));
            broker_table.emplace (command.dest_id, static_cast<int32_t> (_brokers.size () - 1));
        }
    }
    break;
	case CMD_DISCONNECT:
	{
		auto brkNum = getBrokerById(command.source_id);
		if (brkNum >= 0)
		{
			_brokers[brkNum]._disconnected = true;
		}
		if (allDisconnected())
		{
			if (!_isRoot)
			{
				ActionMessage dis(CMD_DISCONNECT);
				dis.source_id = global_broker_id;
				transmit(0, dis);
			}
			addMessage(CMD_STOP);
		}
	}
	break;
	case CMD_REG_ROUTE:
		break;
    }
}


void CoreBroker::transmitDelayedMessages ()
{
    auto msg = delayTransmitQueue.pop ();
    while (msg)
    {
        msg->source_id = global_broker_id;
        transmit (0, *msg);
        msg = delayTransmitQueue.pop ();
    }
}

void CoreBroker::processCommand (ActionMessage &command)
{
    switch (command.action ())
    {
    case CMD_IGNORE:
	case CMD_PROTOCOL:
        break;
    case CMD_INIT:
    {
        auto brkNum = getBrokerById (command.source_id);
        if (brkNum >= 0)
        {
            std::lock_guard<std::mutex> lock (mutex_);
            _brokers[brkNum]._initRequested = true;
        }
        if (allInitReady ())
        {
            if (_isRoot)
            {
                checkSubscriptions ();
                checkEndpoints ();
                checkFilters ();
                // computeDependencies();
                ActionMessage m (CMD_INIT_GRANT);
                for (auto &brk : _brokers)
                {
                    transmit (brk.route_id, m);
                }
            }
            else
            {
                command.source_id = global_broker_id;
                transmit (0, command);
            }
        }
    }
    break;
    case CMD_INIT_GRANT:
        for (auto &brk : _brokers)
        {
            transmit (brk.route_id, command);
        }
        break;
	case CMD_STOP:
		if ((!allDisconnected())&&(!_isRoot))
		{ //only send a disconnect message if we haven't done so already
			ActionMessage m(CMD_DISCONNECT);
			m.source_id = global_broker_id;
			transmit(0, m);
		}
		break;
    case CMD_EXEC_REQUEST:
	case CMD_EXEC_GRANT:
		if (command.dest_id == global_broker_id)
		{
			timeCoord->processExecRequest(command);
		}
		else if (command.source_id == global_broker_id)
		{
			for (auto dep : timeCoord->getDependents())
			{
				command.dest_id = dep;
				transmit(getRoute(dep), command);
			}
		}
		else
		{
			transmit(getRoute(command.dest_id), command);
		}
        
        break;
    case CMD_TIME_REQUEST:
	case CMD_TIME_GRANT:
		if (command.source_id == global_broker_id)
		{
			for (auto dep : timeCoord->getDependents())
			{
				command.dest_id = dep;
				transmit(getRoute(dep), command);
			}
		}
		else if (command.dest_id == global_broker_id)
		{
			timeCoord->processExternalTimeMessage(command);
		}
		else
		{
			transmit(getRoute(command.dest_id), command);
		}
        
        break;
    case CMD_SEND_MESSAGE:
        transmit (getRoute (command.dest_id), command);
        break;
    case CMD_SEND_FOR_FILTER:
        transmit (getRoute (command.dest_id), command);
        break;
    case CMD_PUB:
        transmit (getRoute (command.dest_id), command);
        break;
    
    case CMD_LOG:
        if (_isRoot)
        {
            // do some logging
        }
        else
        {
            transmit (0, command);
        }
        break;
    case CMD_ERROR:
        if (_isRoot)
        {
            // do some logging
        }
        else
        {
            transmit (0, command);
        }
        break;
    case CMD_REG_PUB:
		if (!_isRoot)
		{
			if (command.dest_id != 0)
			{
				auto rt = getRoute(command.dest_id);
				transmit(rt, command);
				break;
			}
		}
		addPublication(command);
        break;
    case CMD_REG_SUB:
		if (!_isRoot)
		{
			if (command.dest_id != 0)
			{
				auto rt = getRoute(command.dest_id);
				transmit(rt, command);
				break;
			}
		}
		addSubscription(command);
        break;
    case CMD_REG_END:
		if (!_isRoot)
		{
			if (command.dest_id != 0)
			{
				auto rt = getRoute(command.dest_id);
				transmit(rt, command);
				break;
			}
		}
		addEndpoint(command);
        break;
    case CMD_REG_DST_FILTER:
		if (!_isRoot)
		{
			if (command.dest_id != 0)
			{
				auto rt = getRoute(command.dest_id);
				transmit(rt, command);
				break;
			}
		}
		addDestFilter(command);
        break;
    case CMD_REG_SRC_FILTER:
		if (!_isRoot)
		{
			if (command.dest_id != 0)
			{
				auto rt = getRoute(command.dest_id);
				transmit(rt, command);
				break;
			}
		}
		addSourceFilter(command);
        break;
    default:
        // check again if it is a priority command and if so process it in that function
        if (isPriorityCommand (command))
        {
            processPriorityCommand (command);
            break;
        }
    }
}


void CoreBroker::addLocalInfo(BasicHandleInfo &handleInfo, const ActionMessage &m)
{
	std::unique_lock<std::mutex> lock(mutex_);
	auto res = global_id_translation.find(m.source_id);
	if (res != global_id_translation.end())
	{
		handleInfo.local_fed_id = res->second;
	}
}

void CoreBroker::addPublication(ActionMessage &m)
{
	_handles.emplace_back(m.source_handle, m.source_id, HANDLE_PUB, m.name, m.info().type, m.info().units);
	publications.emplace(m.name, static_cast<int32_t>(_handles.size() - 1));
	
	addLocalInfo(_handles.back(), m);
	if (!_isRoot)
	{
		transmit(0, m);
	}
	else
	{
		FindandNotifyPublicationSubscribers(_handles.back());
	}
	
	
}
void CoreBroker::addSubscription(ActionMessage &m)
{
	_handles.emplace_back(m.source_handle, m.source_id, HANDLE_SUB, m.name, m.info().type, m.info().units);
	addLocalInfo(_handles.back(), m);
	subscriptions.emplace(m.name, static_cast<int32_t>(_handles.size() - 1));
	_handles.back().processed = m.processingComplete;
	if (!m.processingComplete)
	{
		bool proc = FindandNotifySubscriptionPublisher(_handles.back());
		if (!_isRoot)
		{
			//just let any higher level brokers know we have found the publisher and let them know
			m.processingComplete = proc;
			transmit(0, m);
		}
	}
	
}

void CoreBroker::addEndpoint(ActionMessage &m)
{
	_handles.emplace_back(m.source_handle, m.source_id, HANDLE_END, m.name, m.info().type, m.info().units);
	endpoints.emplace(m.name, static_cast<int32_t>(_handles.size() - 1));

	addLocalInfo(_handles.back(), m);
	
	if (!_isRoot)
	{
		bool addDep = (!m.processingComplete);
		
		m.processingComplete = true;
		
		
		transmit(0, m);
		if (addDep)
		{
			bool added = timeCoord->addDependency(m.source_id);
			if (added)
			{
				ActionMessage add(CMD_ADD_DEPENDENCY);
				add.source_id = global_broker_id;
				add.dest_id = m.source_id;
				transmit(getRoute(add.source_id), add);
				timeCoord->addDependent(m.source_id);
			}
		}
		
	}
	else
	{
		FindandNotifyEndpointFilters(_handles.back());
	}
}
void CoreBroker::addSourceFilter(ActionMessage &m)
{
	_handles.emplace_back(m.source_handle, m.source_id, HANDLE_SOURCE_FILTER, m.name, m.info().type, m.info().target);
	addLocalInfo(_handles.back(), m);
	bool proc = FindandNotifyFilterEndpoint(_handles.back());
	if (!_isRoot)
	{
		m.processingComplete = proc;
		transmit(0, m);
	}
}
void CoreBroker::addDestFilter(ActionMessage &m)
{
	_handles.emplace_back(m.source_handle, m.source_id, HANDLE_DEST_FILTER, m.name, m.info().type, m.info().target,true);
	addLocalInfo(_handles.back(), m);
	bool proc = FindandNotifyFilterEndpoint(_handles.back());
	if (!_isRoot)
	{
		m.processingComplete = proc;
		transmit(0, m);
	}
}

static void argumentParser (int argc, char *argv[], boost::program_options::variables_map &vm_map);


CoreBroker::CoreBroker (bool isRoot) noexcept : _isRoot (isRoot) {}


CoreBroker::CoreBroker (const std::string &broker_name) : local_broker_identifier (broker_name) {}

void CoreBroker::Initialize (const std::string &initializationString)
{
    if (!_initialized)
    {
        stringToCmdLine cmdline (initializationString);
        InitializeFromArgs (cmdline.getArgCount (), cmdline.getArgV ());
    }
}

void CoreBroker::InitializeFromArgs (int argc, char *argv[])
{
    namespace po = boost::program_options;
    bool exp = false;
    if (_initialized.compare_exchange_strong (exp, true))
    {
        po::variables_map vm;
        argumentParser (argc, argv, vm);
        if (vm.count ("min") > 0)
        {
            _min_federates = vm["min"].as<int> ();
        }
        if (vm.count ("minfed") > 0)
        {
            _min_federates = vm["minfed"].as<int> ();
        }
        if (vm.count ("minbrokers") > 0)
        {
            _min_brokers = vm["minbrokers"].as<int> ();
        }
        if (vm.count ("name") > 0)
        {
            local_broker_identifier = vm["name"].as<std::string> ();
        }

        if (vm.count ("identifier") > 0)
        {
            local_broker_identifier = vm["identifier"].as<std::string> ();
        }

        if (local_broker_identifier.empty ())
        {  // don't allow an empty identifier, that causes all sorts of issues
            local_broker_identifier = gen_id ();
        }

        _queue_processing_thread = std::thread (&CoreBroker::queueProcessingLoop, this);
    }
}


bool CoreBroker::connect ()
{
    if (_initialized)
    {
        bool exp = false;
        if (_connected.compare_exchange_strong (exp, true))
        {
            auto res = brokerConnect ();
            if (res)
            {
                if (!_isRoot)
                {
                    ActionMessage m (CMD_REG_BROKER);
                    m.name = getIdentifier ();
                    m.info ().target = getAddress ();
                    transmit (0, m);
                }
                _connected = true;
            }
            return res;
        }
        return true;
    }
    return false;
}

void CoreBroker::disconnect()
{
	brokerDisconnect();
	_connected = false;
	/*We need to enrure that the destructor is not called immediately upon calling unregister
	otherwise this would be a mess and probably cause seg faults so we capture it in a local variable
	that will be destroyed on function exit
	*/
	auto keepBrokerAlive = findBroker(local_broker_identifier);
	if (keepBrokerAlive)
	{
		unregisterBroker(local_broker_identifier);
	}
	if (!previous_local_broker_identifier.empty())
	{
		auto keepBrokerAlive2 = findBroker(previous_local_broker_identifier);
		if (keepBrokerAlive2)
		{
			unregisterBroker(previous_local_broker_identifier);
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
		("broker,b", po::value<std::string>(), "address to connect the broker to")
		("name,n", po::value<std::string>(), "name of the core")
		("minfed", po::value<int>(), "type of the publication to use")
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
        std::cout << 0.1 << '\n';
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

bool CoreBroker::FindandNotifySubscriptionPublisher(BasicHandleInfo &handleInfo)
{
	if (!handleInfo.processed)
	{
		auto pubHandle = publications.find(handleInfo.key);
		if (pubHandle != publications.end())
		{
			auto &pubInfo = _handles[pubHandle->second];
			if (!matchingTypes(pubInfo.type, handleInfo.type))
			{
				// LOG(WARN) << "sub " << hndl->key << " does not match types" << hndl->type << " " <<
				// pubInfo->type << ENDL;
			}
			//notify the subscription about its publisher
			ActionMessage m(CMD_NOTIFY_PUB);
			m.source_id = pubInfo.fed_id;
			m.source_handle = pubInfo.id;
			m.dest_id = handleInfo.fed_id;
			m.dest_handle = handleInfo.id;

			transmit(getRoute(m.dest_id), m);

			//notify the publisher about its subscription
			m.setAction(CMD_NOTIFY_SUB);
			m.source_id = handleInfo.fed_id;
			m.source_handle = handleInfo.id;
			m.dest_id = pubInfo.fed_id;
			m.dest_handle = pubInfo.id;

			transmit(getRoute(m.dest_id), m);

			handleInfo.processed = true;
		}
	}
	return handleInfo.processed;
}

void CoreBroker::FindandNotifyPublicationSubscribers(BasicHandleInfo &handleInfo)
{
		auto subHandles = subscriptions.equal_range(handleInfo.key);
		for (auto sub = subHandles.first; sub != subHandles.second; ++sub)
		{
			auto &subInfo = _handles[sub->second];
			if (subInfo.processed)
			{
				continue;
			}
			if (!matchingTypes(subInfo.type, handleInfo.type))
			{
				// LOG(WARN) << "sub " << hndl->key << " does not match types" << hndl->type << " " <<
				// pubInfo->type << ENDL;
			}
			//notify the subscription about its publisher
			ActionMessage m(CMD_NOTIFY_SUB);
			m.source_id = subInfo.fed_id;
			m.source_handle = subInfo.id;
			m.dest_id = handleInfo.fed_id;
			m.dest_handle = handleInfo.id;

			transmit(getRoute(m.dest_id), m);

			//notify the publisher about its subscription
			m.setAction(CMD_NOTIFY_PUB);
			m.source_id = handleInfo.fed_id;
			m.source_handle = handleInfo.id;
			m.dest_id = subInfo.fed_id;
			m.dest_handle = subInfo.id;

			transmit(getRoute(m.dest_id), m);
			subInfo.processed = true;
		}

}

bool CoreBroker::FindandNotifyFilterEndpoint(BasicHandleInfo &handleInfo)
{
	if (!handleInfo.processed)
	{
		auto endHandle = endpoints.find(handleInfo.target);
		if (endHandle != endpoints.end())
		{
			auto &endInfo = _handles[endHandle->second];
			if (!matchingTypes(endInfo.type, handleInfo.type))
			{
				// LOG(WARN) << "sub " << hndl->key << " does not match types" << hndl->type << " " <<
				// pubInfo->type << ENDL;
			}
			//notify the filter about its endpoint
			ActionMessage m(CMD_NOTIFY_END);
			m.source_id = endInfo.fed_id;
			m.source_handle = endInfo.id;
			m.dest_id = handleInfo.fed_id;
			m.dest_handle = handleInfo.id;

			transmit(getRoute(m.dest_id), m);

			//notify the endpoint about its filter
			m.setAction((handleInfo.what == HANDLE_SOURCE_FILTER) ? CMD_NOTIFY_SRC_FILTER : CMD_NOTIFY_DST_FILTER);
			m.source_id = handleInfo.fed_id;
			m.source_handle = handleInfo.id;
			m.dest_id = endInfo.fed_id;
			m.dest_handle = endInfo.id;

			transmit(getRoute(m.dest_id), m);

			handleInfo.processed = true;
		}
	}
	return handleInfo.processed;
}


void CoreBroker::FindandNotifyEndpointFilters(BasicHandleInfo &handleInfo)
{
	auto filtHandles = filters.equal_range(handleInfo.target);
	for (auto filt = filtHandles.first; filt != filtHandles.second; ++filt)
	{
		auto &filtInfo = _handles[filt->second];
		if (filtInfo.processed)
		{
			continue;
		}
		if (!matchingTypes(filtInfo.type, handleInfo.type))
		{
			// LOG(WARN) << "sub " << hndl->key << " does not match types" << hndl->type << " " <<
			// pubInfo->type << ENDL;
		}
		//notify the endpoint about a filter
		ActionMessage m((handleInfo.what == HANDLE_SOURCE_FILTER) ? CMD_NOTIFY_SRC_FILTER : CMD_NOTIFY_DST_FILTER);
		m.source_id = filtInfo.fed_id;
		m.source_handle = filtInfo.id;
		m.dest_id = handleInfo.fed_id;
		m.dest_handle = handleInfo.id;

		transmit(getRoute(m.dest_id), m);

		//notify the publisher about its subscription
		m.setAction(CMD_NOTIFY_END);
		m.source_id = handleInfo.fed_id;
		m.source_handle = handleInfo.id;
		m.dest_id = filtInfo.fed_id;
		m.dest_handle = filtInfo.id;

		transmit(getRoute(m.dest_id), m);
		filtInfo.processed = true;
	}
}

void CoreBroker::checkSubscriptions ()
{
    // pub/sub checks
    // LOG(INFO) << "performing pub/sub check" << ENDL;
    for (auto &hndl : _handles)
    {
        if (hndl.what == HANDLE_SUB)
        {
			if (!hndl.processed)
			{
				auto fnd=FindandNotifySubscriptionPublisher(hndl);
				if ((!fnd)&&(hndl.flag))
				{
					// LOG(WARN) << "sub " << hndl->key << " has no corresponding pub" << ENDL;
				}
			}
            
         
        }
    }
}

void CoreBroker::checkEndpoints () {}

void CoreBroker::checkFilters ()
{
    // LOG(INFO) << "performing filter check" << ENDL;
    for (auto &hndl : _handles)
    {
        if ((hndl.what == HANDLE_DEST_FILTER)||(hndl.what==HANDLE_SOURCE_FILTER))
        {
			auto fnd = FindandNotifyFilterEndpoint(hndl);
			if (!fnd)
			{
				// LOG(WARN) << "sub " << hndl->key << " has no corresponding pub" << ENDL;
			}
        }
    }
}

bool CoreBroker::allInitReady () const
{
    // the federate count must be greater than the min size
    if (static_cast<decltype (_min_federates)> (_federates.size ()) < _min_federates)
    {
        return false;
    }
    if (static_cast<decltype (_min_brokers)> (_brokers.size ()) < _min_brokers)
    {
        return false;
    }
    // all subBrokers must be requesting init
    for (auto &brk : _brokers)
    {
        if (!brk._initRequested)
        {
            return false;
        }
    }
    return true;
}

bool CoreBroker::allDisconnected () const
{
    // all subBrokers must be disconnected
    for (auto &brk : _brokers)
    {
        if (!brk._disconnected)
        {
            return false;
        }
    }
    return true;
}

bool matchingTypes (const std::string &type1, const std::string &type2)
{
    if (type1 == type2)
    {
        return true;
    }
    if ((type1.empty ()) || (type2.empty ()))
    {
        return true;
    }
    if ((type1 == "block") || (type2 == "block"))
    {
        return true;
    }
    if ((type1 == "string") || (type2 == "string"))
    {
        return true;
    }
    if ((type1 == "data") || (type2 == "data"))
    {
        return true;
    }
    return false;
}
}
