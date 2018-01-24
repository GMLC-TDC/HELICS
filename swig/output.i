
// File: index.xml

// File: classhelics_1_1ActionMessage.xml


%feature("docstring") helics::ActionMessage "
";

%feature("docstring") helics::ActionMessage::ActionMessage "

default constructor
";

%feature("docstring") helics::ActionMessage::ActionMessage "

construct from an action type

this is an implicit constructor
";

%feature("docstring") helics::ActionMessage::ActionMessage "

construct from action, source and destination id's
";

%feature("docstring") helics::ActionMessage::ActionMessage "

move constructor
";

%feature("docstring") helics::ActionMessage::ActionMessage "

build an action message from a message
";

%feature("docstring") helics::ActionMessage::ActionMessage "

construct from a string
";

%feature("docstring") helics::ActionMessage::ActionMessage "

construct from a data vector
";

%feature("docstring") helics::ActionMessage::ActionMessage "

construct from a data pointer and size
";

%feature("docstring") helics::ActionMessage::ActionMessage "

copy constructor
";

%feature("docstring") helics::ActionMessage::~ActionMessage "

destructor
";

%feature("docstring") helics::ActionMessage::action "

get the action of the message
";

%feature("docstring") helics::ActionMessage::setAction "

set the action
";

%feature("docstring") helics::ActionMessage::info "

get a reference to the additional info structure
";

%feature("docstring") helics::ActionMessage::info "

get a const ref to the info structure
";

%feature("docstring") helics::ActionMessage::moveInfo "

move a message data into the actionMessage

take ownership of the message and move the contents out then destroy the message
shell

Parameters
----------
* `message` :
    the message to move.
";

%feature("docstring") helics::ActionMessage::save "
";

%feature("docstring") helics::ActionMessage::load "
";

%feature("docstring") helics::ActionMessage::toByteArray "

convert a command to a raw data bytes

Parameters
----------
* `data` :
    pointer to memory to store the command
* `buffer_size&ndash;` :
    the size of the buffer

Returns
-------
the size of the buffer actually used
";

%feature("docstring") helics::ActionMessage::to_string "

convert to a string using a reference
";

%feature("docstring") helics::ActionMessage::to_string "

convert to a byte string
";

%feature("docstring") helics::ActionMessage::packetize "

packettize the message with a simple header and tail sequence
";

%feature("docstring") helics::ActionMessage::to_vector "

covert to a byte vector using a reference
";

%feature("docstring") helics::ActionMessage::to_vector "

convert a command to a byte vector
";

%feature("docstring") helics::ActionMessage::fromByteArray "

generate a command from a raw data stream
";

%feature("docstring") helics::ActionMessage::depacketize "

load a command from a packetized stream /ref packetize

Returns
-------
the number of bytes used
";

%feature("docstring") helics::ActionMessage::from_string "

read a command from a string
";

%feature("docstring") helics::ActionMessage::from_vector "

read a command from a char vector
";

// File: classhelics_1_1ActionMessage_1_1AdditionalInfo.xml


%feature("docstring") helics::ActionMessage::AdditionalInfo "
";

%feature("docstring") helics::ActionMessage::AdditionalInfo::AdditionalInfo "

constructor
";

%feature("docstring") helics::ActionMessage::AdditionalInfo::AdditionalInfo "

copy constructor
";

%feature("docstring") helics::ActionMessage::AdditionalInfo::AdditionalInfo "

move constructor
";

%feature("docstring") helics::ActionMessage::AdditionalInfo::save "
";

%feature("docstring") helics::ActionMessage::AdditionalInfo::load "
";

// File: classhelics_1_1ArgDescriptor.xml


%feature("docstring") helics::ArgDescriptor "

class to contain a descriptor for a command line argument

C++ includes: argParser.h
";

%feature("docstring") helics::ArgDescriptor::ArgDescriptor "
";

%feature("docstring") helics::ArgDescriptor::ArgDescriptor "
";

// File: classAsioServiceManager.xml


%feature("docstring") AsioServiceManager "

class defining a (potential) singleton Asio Io_service manager for all
boost::asio usage

C++ includes: AsioServiceManager.h
";

%feature("docstring") AsioServiceManager::getServicePointer "

return a pointer to a service manager

the function will search for an existing service manager for the name if it
doesn't find one it will create a new one

Parameters
----------
* `serviceName` :
    the name of the service to find or create
";

%feature("docstring") AsioServiceManager::getExistingServicePointer "

return a pointer to a service manager

the function will search for an existing service manager for the name if it
doesn't find one it will return nullptr

Parameters
----------
* `serviceName` :
    the name of the service to find
";

%feature("docstring") AsioServiceManager::getService "

get the boost io_service associated with the service manager
";

%feature("docstring") AsioServiceManager::getExistingService "

get the boost io_service associated with the service manager but only if the
service exists if it doesn't this will throw and invalid_argument exception
";

%feature("docstring") AsioServiceManager::closeService "
";

%feature("docstring") AsioServiceManager::setServiceToLeakOnDelete "

tell the service to free the pointer and leak the memory on delete

You may ask why, well in windows systems when operating in a DLL if this context
is closed after certain other operations that happen when the DLL is unlinked
bad things can happen, and since in nearly all cases this happens at Shutdown
leaking really doesn't matter that much and if you don't the service could
terminate before some other parts of the program which cause all sorts of odd
errors and issues
";

%feature("docstring") AsioServiceManager::runServiceLoop "

run a single thread for the service manager to execute asynchronous services in

will run a single thread for the io_service, it will not stop the thread until
either the service manager is closed or the haltServiceLoop function is called
and there is no more work

Parameters
----------
* `in` :
    the name of the service
";

%feature("docstring") AsioServiceManager::haltServiceLoop "

halt the service loop thread if the counter==0

decrements the loop request counter and if it is 0 then will halt the service
loop

Parameters
----------
* `in` :
    the name of the service
";

%feature("docstring") AsioServiceManager::~AsioServiceManager "
";

%feature("docstring") AsioServiceManager::getName "

get the name of the current service manager
";

%feature("docstring") AsioServiceManager::getBaseService "

get the underlying boost::io_service reference
";

%feature("docstring") AsioServiceManager::serviceRunLoop "
";

// File: classhelics_1_1AsyncFedCallInfo.xml


%feature("docstring") helics::AsyncFedCallInfo "

helper class for Federate info that holds the futures for async calls

C++ includes: AsyncFedCallInfo.hpp
";

// File: classhelics_1_1Barrier.xml


%feature("docstring") helics::Barrier "
";

%feature("docstring") helics::Barrier::Barrier "
";

%feature("docstring") helics::Barrier::Wait "
";

// File: classhelics_1_1BasicBrokerInfo.xml


%feature("docstring") helics::BasicBrokerInfo "

class defining the common information about a broker federate

C++ includes: CoreBroker.hpp
";

%feature("docstring") helics::BasicBrokerInfo::BasicBrokerInfo "
";

// File: classhelics_1_1BasicFedInfo.xml


%feature("docstring") helics::BasicFedInfo "

class defining the common information for a federate

C++ includes: CoreBroker.hpp
";

%feature("docstring") helics::BasicFedInfo::BasicFedInfo "
";

// File: classhelics_1_1BasicHandleInfo.xml


%feature("docstring") helics::BasicHandleInfo "

class defining and capturing basic information about a handle

C++ includes: BasicHandleInfo.hpp
";

%feature("docstring") helics::BasicHandleInfo::BasicHandleInfo "

default constructor
";

%feature("docstring") helics::BasicHandleInfo::BasicHandleInfo "

construct from the data
";

%feature("docstring") helics::BasicHandleInfo::BasicHandleInfo "

construct from the data for filters
";

// File: classBlockingPriorityQueue.xml


%feature("docstring") BlockingPriorityQueue "

class implementing a blocking queue with a priority channel

this class uses locks one for push and pull it can exhibit longer blocking times
if the internal operations require a swap, however in high usage the two locks
will reduce contention in most cases.

C++ includes: BlockingPriorityQueue.hpp
";

%feature("docstring") BlockingPriorityQueue::BlockingPriorityQueue "

default constructor
";

%feature("docstring") BlockingPriorityQueue::BlockingPriorityQueue "

constructor with the capacity numbers

there are two internal vectors that alternate so the actual reserve is 2x the
capacity numbers in two different vectors

Parameters
----------
* `capacity` :
    the initial reserve capacity for the arrays
";

%feature("docstring") BlockingPriorityQueue::BlockingPriorityQueue "

enable the move constructor not the copy constructor
";

%feature("docstring") BlockingPriorityQueue::BlockingPriorityQueue "

DISABLE_COPY_AND_ASSIGN
";

%feature("docstring") BlockingPriorityQueue::clear "

clear the queue
";

%feature("docstring") BlockingPriorityQueue::~BlockingPriorityQueue "
";

%feature("docstring") BlockingPriorityQueue::reserve "

set the capacity of the queue actually double the requested the size will be
reserved due to the use of two vectors internally

Parameters
----------
* `capacity` :
    the capacity to reserve
";

%feature("docstring") BlockingPriorityQueue::push "

push an element onto the queue val the value to push on the queue
";

%feature("docstring") BlockingPriorityQueue::pushPriority "

push an element onto the queue val the value to push on the queue
";

%feature("docstring") BlockingPriorityQueue::emplace "

construct on object in place on the queue
";

%feature("docstring") BlockingPriorityQueue::emplacePriority "

emplace an element onto the priority queue val the value to push on the queue
";

%feature("docstring") BlockingPriorityQueue::try_peek "

try to peek at an object without popping it from the stack

only available for copy assignable objects

Returns
-------
an optional object with an object of type T if available
";

%feature("docstring") BlockingPriorityQueue::try_pop "

try to pop an object from the queue

Returns
-------
an optional containing the value if successful the optional will be empty if
there is no element in the queue
";

%feature("docstring") BlockingPriorityQueue::pop "

blocking call to wait on an object from the stack
";

%feature("docstring") BlockingPriorityQueue::pop "

blocking call that will call the specified functor if the queue is empty

Parameters
----------
* `callOnWaitFunction` :
    an nullary functor that will be called if the initial query does not return
    a value

after calling the function the call will check again and if still empty will
block and wait.
";

%feature("docstring") BlockingPriorityQueue::empty "

check whether there are any elements in the queue because this is meant for
multi-threaded applications this may or may not have any meaning depending on
the number of consumers
";

// File: classBlockingQueue.xml


%feature("docstring") BlockingQueue "

NOTES:: PT Went with unlocking after signaling on the basis of this page
http://www.domaigne.com/blog/computing/condvars-signal-with-mutex-locked-or-not/
will check performance at a later time class implementing a blocking queue

this class uses locks one for push and pull it can exhibit longer blocking times
if the internal operations require a swap, however in high contention the two
locks will reduce contention in most cases.

C++ includes: BlockingQueue.hpp
";

%feature("docstring") BlockingQueue::BlockingQueue "

default constructor
";

%feature("docstring") BlockingQueue::BlockingQueue "

constructor with the capacity numbers

there are two internal vectors that alternate so the actual reserve is 2x the
capacity numbers in two different vectors

Parameters
----------
* `capacity` :
    the initial reserve capacity for the arrays
";

%feature("docstring") BlockingQueue::BlockingQueue "

enable the move constructor not the copy constructor
";

%feature("docstring") BlockingQueue::BlockingQueue "

DISABLE_COPY_AND_ASSIGN
";

%feature("docstring") BlockingQueue::~BlockingQueue "
";

%feature("docstring") BlockingQueue::clear "

clear the queue
";

%feature("docstring") BlockingQueue::reserve "

set the capacity of the queue actually double the requested the size will be
reserved due to the use of two vectors internally

Parameters
----------
* `capacity` :
    the capacity to reserve
";

%feature("docstring") BlockingQueue::push "

push an element onto the queue val the value to push on the queue
";

%feature("docstring") BlockingQueue::emplace "

construct on object in place on the queue
";

%feature("docstring") BlockingQueue::try_peek "

try to peek at an object without popping it from the stack

only available for copy assignable objects

Returns
-------
an optional object with an object of type T if available
";

%feature("docstring") BlockingQueue::try_pop "

try to pop an object from the queue

Returns
-------
an optional containing the value if successful the optional will be empty if
there is no element in the queue
";

%feature("docstring") BlockingQueue::pop "

blocking call to wait on an object from the stack
";

%feature("docstring") BlockingQueue::pop "

blocking call that will call the specified functor if the queue is empty

Parameters
----------
* `callOnWaitFunction` :
    an nullary functor that will be called if the initial query does not return
    a value

after calling the function the call will check again and if still empty will
block and wait.
";

%feature("docstring") BlockingQueue::empty "

check whether there are any elements in the queue because this is meant for
multi-threaded applications this may or may not have any meaning depending on
the number of consumers
";

%feature("docstring") BlockingQueue::size "

get the current size of the queue

this may or may not have much meaning depending on the number of consumers
";

// File: classhelics_1_1BlockingQueue__old.xml


%feature("docstring") helics::BlockingQueue_old "

a queue that blocks while waiting for an input

C++ includes: blocking_queue.h
";

%feature("docstring") helics::BlockingQueue_old::BlockingQueue_old "

default constructor
";

%feature("docstring") helics::BlockingQueue_old::BlockingQueue_old "

DISABLE_COPY_AND_ASSIGN
";

%feature("docstring") helics::BlockingQueue_old::push "

push an object onto the queue
";

%feature("docstring") helics::BlockingQueue_old::emplace "

construct on object in place on the queue
";

%feature("docstring") helics::BlockingQueue_old::try_pop "

try to pop an object from the queue

Parameters
----------
* `t` :
    the location in which to place the object

Returns
-------
true if the operation was successful
";

%feature("docstring") helics::BlockingQueue_old::pop "
";

%feature("docstring") helics::BlockingQueue_old::try_peek "

try to peek at an object without popping it from the stack

Returns
-------
an optional object with an objec of type T if available
";

%feature("docstring") helics::BlockingQueue_old::size "

get the current size of the queue
";

// File: classhelics_1_1Broker.xml


%feature("docstring") helics::Broker "

virtual class defining a public interface to a broker

C++ includes: Broker.hpp
";

%feature("docstring") helics::Broker::Broker "

default constructor

Parameters
----------
* `setAsRootBroker` :
    set to true to indicate this object is a root broker
";

%feature("docstring") helics::Broker::Broker "
";

%feature("docstring") helics::Broker::Broker "
";

%feature("docstring") helics::Broker::Broker "
";

%feature("docstring") helics::Broker::~Broker "

destructor
";

%feature("docstring") helics::Broker::~Broker "
";

%feature("docstring") helics::Broker::connect "

connect the core to its broker

should be done after initialization has complete
";

%feature("docstring") helics::Broker::disconnect "

disconnect the broker from any other brokers and communications
";

%feature("docstring") helics::Broker::isConnected "

check if the broker is connected
";

%feature("docstring") helics::Broker::isConnected "
";

%feature("docstring") helics::Broker::setAsRoot "

set the broker to be a root broker

only valid before the initialization function is called
";

%feature("docstring") helics::Broker::isRoot "

return true if the broker is a root broker
";

%feature("docstring") helics::Broker::isOpenToNewFederates "

check if the broker is ready to accept new federates or cores
";

%feature("docstring") helics::Broker::initialize "

start up the broker with an initialization string containing commands and
parameters
";

%feature("docstring") helics::Broker::initializeFromArgs "

initialize from command line arguments
";

%feature("docstring") helics::Broker::getIdentifier "

get the local identification for the broker
";

%feature("docstring") helics::Broker::getAddress "

get the connection address for the broker
";

// File: classhelics_1_1BrokerBase.xml


%feature("docstring") helics::BrokerBase "

base class for broker like objects

C++ includes: BrokerBase.hpp
";

%feature("docstring") helics::BrokerBase::displayHelp "
";

%feature("docstring") helics::BrokerBase::BrokerBase "
";

%feature("docstring") helics::BrokerBase::BrokerBase "
";

%feature("docstring") helics::BrokerBase::~BrokerBase "
";

%feature("docstring") helics::BrokerBase::initializeFromCmdArgs "

initialize the core manager with command line arguments

Parameters
----------
* `argc` :
    the number of arguments
* `argv` :
    char pointers to the arguments
";

%feature("docstring") helics::BrokerBase::addActionMessage "

add an action Message to the process queue
";

%feature("docstring") helics::BrokerBase::addActionMessage "

move a action Message into the commandQueue
";

%feature("docstring") helics::BrokerBase::setLoggerFunction "

set the logging callback function

Parameters
----------
* `logFunction` :
    a function with a signature of void(int level,  const std::string &source,
    const std::string &message) the function takes a level indicating the
    logging level string with the source name and a string with the message
";

%feature("docstring") helics::BrokerBase::processDisconnect "

process a disconnect signal
";

%feature("docstring") helics::BrokerBase::isRunning "

check if the main processing loop of a broker is running
";

%feature("docstring") helics::BrokerBase::setLogLevel "

set the logging level
";

%feature("docstring") helics::BrokerBase::setLogLevels "

set the logging levels

Parameters
----------
* `consoleLevel` :
    the logging level for the console display
* `fileLevel` :
    the logging level for the log file
";

%feature("docstring") helics::BrokerBase::joinAllThreads "

close all the threads
";

// File: classhelics_1_1BrokerObject.xml


%feature("docstring") helics::BrokerObject "

object wrapping a broker for the c-api

C++ includes: api_objects.h
";

// File: classhelics_1_1changeOnDestroy.xml


%feature("docstring") helics::changeOnDestroy "
";

%feature("docstring") helics::changeOnDestroy::changeOnDestroy "
";

%feature("docstring") helics::changeOnDestroy::~changeOnDestroy "
";

// File: classutilities_1_1charMapper.xml


%feature("docstring") utilities::charMapper "

small helper class to map characters to values

C++ includes: charMapper.h
";

%feature("docstring") utilities::charMapper::charMapper "

default constructor
";

%feature("docstring") utilities::charMapper::addKey "

update a the value returned from a key query

this is purposely distinct from the [] operator to make it an error to try to
assign something that way
";

%feature("docstring") utilities::charMapper::at "

get the value assigned to a character

Parameters
----------
* `x` :
    the character to test or convert

Returns
-------
the resulting value, 0 if nothing in particular is specified in a given map
";

// File: classhelics_1_1CloneFilterOperation.xml


%feature("docstring") helics::CloneFilterOperation "

filter for rerouting a packet to a particular endpoint

C++ includes: FilterOperations.hpp
";

%feature("docstring") helics::CloneFilterOperation::CloneFilterOperation "

this operation needs a pointer to a core to operate
";

%feature("docstring") helics::CloneFilterOperation::~CloneFilterOperation "
";

%feature("docstring") helics::CloneFilterOperation::set "

set a property on a filter

Parameters
----------
* `property` :
    the name of the property of the filter to change
* `val` :
    the numerical value of the property
";

%feature("docstring") helics::CloneFilterOperation::setString "

set a string property on a filter

Parameters
----------
* `property` :
    the name of the property of the filter to change
* `val` :
    the numerical value of the property
";

%feature("docstring") helics::CloneFilterOperation::getOperator "
";

// File: classhelics_1_1CloneOperator.xml


%feature("docstring") helics::CloneOperator "

class defining an message operator that either passes the message or not

the evaluation function used should return true if the message should be allowed
through false if it should be dropped

C++ includes: MessageOperators.hpp
";

%feature("docstring") helics::CloneOperator::CloneOperator "

default constructor
";

%feature("docstring") helics::CloneOperator::CloneOperator "

set the function to modify the data of the message in the constructor
";

%feature("docstring") helics::CloneOperator::setCloneFunction "

set the function to modify the data of the message
";

// File: classhelics_1_1CloningFilter.xml


%feature("docstring") helics::CloningFilter "

class used to clone message for delivery to other endpoints

C++ includes: Filters.hpp
";

%feature("docstring") helics::CloningFilter::CloningFilter "

construct from a core object
";

%feature("docstring") helics::CloningFilter::CloningFilter "

construct from a Federate
";

%feature("docstring") helics::CloningFilter::addSourceTarget "

add a sourceEndpoint to the list of endpoint to clone
";

%feature("docstring") helics::CloningFilter::addDestinationTarget "

add a destination endpoint to the list of endpoints to clone
";

%feature("docstring") helics::CloningFilter::addDeliveryEndpoint "

add a delivery address this is the name of an endpoint to deliver the message to
";

%feature("docstring") helics::CloningFilter::removeSourceTarget "

remove a sourceEndpoint to the list of endpoint to clone
";

%feature("docstring") helics::CloningFilter::removeDestinationTarget "

remove a destination endpoint to the list of endpoints to clone
";

%feature("docstring") helics::CloningFilter::removeDeliveryEndpoint "

remove a delivery address this is the name of an endpoint to deliver the message
to
";

%feature("docstring") helics::CloningFilter::setString "

set a string property on a filter

Parameters
----------
* `property` :
    the name of the property of the filter to change
* `val` :
    the numerical value of the property
";

// File: classhelics_1_1CombinationFederate.xml


%feature("docstring") helics::CombinationFederate "

class defining a federate that can use both the value and message interfaces

C++ includes: CombinationFederate.hpp
";

%feature("docstring") helics::CombinationFederate::CombinationFederate "

default constructor
";

%feature("docstring") helics::CombinationFederate::CombinationFederate "

constructor taking a federate information structure and using the default core

Parameters
----------
* `fi` :
    a federate information structure
";

%feature("docstring") helics::CombinationFederate::CombinationFederate "

constructor taking a federate information structure and using the given core

Parameters
----------
* `core` :
    a pointer to core object which the federate can join
* `fi` :
    a federate information structure
";

%feature("docstring") helics::CombinationFederate::CombinationFederate "

constructor taking a file with the required information

Parameters
----------
* `file` :
    a file defining the federate information
";

%feature("docstring") helics::CombinationFederate::CombinationFederate "

move construction
";

%feature("docstring") helics::CombinationFederate::CombinationFederate "
";

%feature("docstring") helics::CombinationFederate::CombinationFederate "
";

%feature("docstring") helics::CombinationFederate::~CombinationFederate "

destructor
";

%feature("docstring") helics::CombinationFederate::disconnect "

disconnect a simulation from the core (will also call finalize before
disconnecting if necessary)
";

%feature("docstring") helics::CombinationFederate::registerInterfaces "

register a set of interfaces defined in a file

call is only valid in startup mode

Parameters
----------
* `jsonString` :
    the location of the file or json String to load to generate the interfaces
";

// File: classhelics_1_1CommonCore.xml


%feature("docstring") helics::CommonCore "

base class implementing a standard interaction strategy between federates

the CommonCore is virtual class that manages local federates and handles most of
the interaction between federate it is meant to be instantiated for specific
interfederate communication strategies

C++ includes: CommonCore.hpp
";

%feature("docstring") helics::CommonCore::CommonCore "

default constructor
";

%feature("docstring") helics::CommonCore::CommonCore "

function mainly to match some other object constructors does the same thing as
the default constructor
";

%feature("docstring") helics::CommonCore::CommonCore "

construct from a core name
";

%feature("docstring") helics::CommonCore::~CommonCore "

virtual destructor
";

%feature("docstring") helics::CommonCore::initialize "

Simulator control. Initialize the core.

Should be invoked a single time to initialize the co-simulation core.
";

%feature("docstring") helics::CommonCore::initializeFromArgs "

initialize the core manager with command line arguments

Parameters
----------
* `argc` :
    the number of arguments
* `argv` :
    char pointers to the arguments
";

%feature("docstring") helics::CommonCore::isInitialized "

Returns true if the core has been initialized.
";

%feature("docstring") helics::CommonCore::isOpenToNewFederates "

check if the core is ready to accept new federates
";

%feature("docstring") helics::CommonCore::error "

Federate has encountered an unrecoverable error.
";

%feature("docstring") helics::CommonCore::finalize "

Federate has completed.

Should be invoked a single time to complete the simulation.
";

%feature("docstring") helics::CommonCore::enterInitializingState "

Federates may be in three states.

Federates are in three states.

1.  Startup Configuration of the federate. State begins when registerFederate()
    is invoked and ends when enterInitializingState() is invoked.
2.  Initializing Configure of the simulation state prior to the start of time
    stepping. State begins when enterInitializingState() is invoked and ends
    when enterExecutingState(true) is invoked.
3.  Executing State begins when enterExecutingState() is invoked and ends when
    Finalize() is invoked. Change the federate state to the Initializing state.

May only be invoked in Created state otherwise an error is thrown
";

%feature("docstring") helics::CommonCore::enterExecutingState "

Change the federate state to the Executing state.

May only be invoked in Initializing state.

Parameters
----------
* `federateID` :
    the identifier of the federate
* `iterationCompleted` :
    if true no more iterations on this federate are requested if nonconverged
    the federate requests an iterative update

Returns
-------
nonconverged if the executing state has not been entered and there are updates,
complete if the simulation is ready to move on to the executing state
";

%feature("docstring") helics::CommonCore::registerFederate "

Register a federate.

The returned FederateId is local to invoking process, FederateId's should not be
used as a global identifier.

May only be invoked in initialize state otherwise throws an error
";

%feature("docstring") helics::CommonCore::getFederateName "

Returns the federate name.
";

%feature("docstring") helics::CommonCore::getFederateId "

Returns the federate Id.
";

%feature("docstring") helics::CommonCore::getFederationSize "

Returns the global number of federates that are registered only return
accurately after the initialization state has been entered
";

%feature("docstring") helics::CommonCore::timeRequest "

Time management. Request a new time advancement window for non-reiterative
federates.

RequestTime() blocks until all non-reiterative federates have invoked
requestTime() and all reiterative federates have converged (called
requestTimeIterative() with localConverged value of true). Return time is the
minimum of all supplied times.

May only be invoked in Executing state.

Iterative federates may not invoke this method.

Parameters
----------
* `next` :
";

%feature("docstring") helics::CommonCore::requestTimeIterative "

Request a new time advancement window for reiterative federates.

Reiterative federates block on requestTimeIterative() until all reiterative
federates have invoked requestTimeIterative(). The bool returned a global AND of
all localConverged values. If globalConverged is false, time returned is the
previous granted time. Time should not advance and another iteration attempted.
Federates should recompute state based on newly published values. Time is
advanced only when all reiterative federates have converged. If globalConverged
is True, grantedTime is the minimum of over all next times in both reiterative
and non-reiterative federates.

If a federate determines it cannot converge it should invoke the error() method.

Federates only participate it in reiterations for times that are evenly
divisible by the federates time delta.

May only be invoked in Executing state.

Non-reiterative federates may not invoke this method.

Parameters
----------
* `federateID` :
    the identifier for the federate to process
* `next` :
    the requested time
* `localConverged` :
    has the local federate converged

Returns
-------
an iteration_time object with two field grantedTime and a enumeration indicating
the state of the iteration
";

%feature("docstring") helics::CommonCore::getCurrentTime "

get the most recent granted Time

Parameters
----------
* `federateID`, `the` :
    id of the federate to get the time

Returns
-------
the most recent granted time or the startup time
";

%feature("docstring") helics::CommonCore::getCurrentReiteration "

Returns the current reiteration count for the specified federate.
";

%feature("docstring") helics::CommonCore::setMaximumIterations "

Set the maximum number of iterations allowed.

The minimum value set in any federate is used.

Default value is the maximum allowed value for uint64_t.

May only be invoked in the initialize state.
";

%feature("docstring") helics::CommonCore::setTimeDelta "

Set the minimum time resolution for the specified federate.

The value is used to constrain when the timeRequest methods return to values
that are multiples of the specified delta. This is useful for federates that are
time-stepped and making sub-time-step updates is not meaningful.

Parameters
----------
* `time` :
";

%feature("docstring") helics::CommonCore::setOutputDelay "

Set the outputDelay time for the specified federate.

The value is used to determine the interaction amongst various federates as to
when a specific federate can influence another

Parameters
----------
* `federateID` :
    the identifier for the federate
* `timeoutputDelay` :
";

%feature("docstring") helics::CommonCore::setPeriod "

Set the period for a specified federate.

The value is used to determine the interaction amongst various federates as to
when a specific federate can influence another

Parameters
----------
* `federateID` :
    the identifier for the federate
* `timeoutputDelay` :
";

%feature("docstring") helics::CommonCore::setTimeOffset "

Set the periodic offset for a specified federate.

The value is used as a time shift for calculating the allowable time in a
federate the granted time must one of N*period+offset

Parameters
----------
* `federateID` :
    the identifier for the federate
* `timeOffset` :
    the periodic phase shift
";

%feature("docstring") helics::CommonCore::setInputDelay "

Set the inputDelay time.

The value is used to determine the interaction amongst various federates as to
when a specific federate can influence another

Parameters
----------
* `federateID` :
    the identifier for the federate
* `timeImpact` :
    the length of time it take outside message to propagate into a federate
";

%feature("docstring") helics::CommonCore::setLoggingLevel "

Set the logging level

set the logging level for an individual federate set federateID to 0 for the
core logging level

Parameters
----------
* `federateID` :
    the identifier for the federate
* `loggingLevel` :
    the level of logging to enable <0-no logging, 0 -error only, 1- warnings,
    2-normal, 3-debug, 4-trace
";

%feature("docstring") helics::CommonCore::setFlag "

Set a flag in a a federate

Parameters
----------
* `federateID` :
    the identifier for the federate
* `flag` :
    an index code for the flag to set
* `flagValue` :
    the value to set the flag to
";

%feature("docstring") helics::CommonCore::registerSubscription "
";

%feature("docstring") helics::CommonCore::getSubscription "

get a subscription Handle from its key

Parameters
----------
* `federateID` :
    the identifier for the federate  the tag of the subscription

Returns
-------
a handle to identify the subscription
";

%feature("docstring") helics::CommonCore::registerPublication "

Register a publication.

May only be invoked in the initialize state.

Parameters
----------
* `federateID` :
    the identifier for the federate
* `key` :
    the tag for the publication
* `type` :
    the type of data the publication produces
* `units` :
    the units associated with the publication

Returns
-------
a handle to identify the publication
";

%feature("docstring") helics::CommonCore::getPublication "

get a publication Handle from its key

Parameters
----------
* `federateID` :
    the identifier for the federate  the name of the publication

Returns
-------
a handle to identify the publication
";

%feature("docstring") helics::CommonCore::getHandleName "

Returns the name or identifier for a specified handle
";

%feature("docstring") helics::CommonCore::getTarget "

Returns the target of a specified handle

for publications and subscriptions this is the key for filters this is the
target and for endpoints this will return an empty string
";

%feature("docstring") helics::CommonCore::getUnits "

Returns units for specified handle.
";

%feature("docstring") helics::CommonCore::getType "

Returns type for specified handle.

for endpoints, publications, and filters, this is the input type for
subscriptions this is the type of the publication(if available)

Parameters
----------
* `handle` :
    the handle from the publication, subscription, endpoint or filter
";

%feature("docstring") helics::CommonCore::getOutputType "

Returns output type for specified handle.

for filters this is the outputType, for Subscriptions this is the expected type
for endpoints and publications this is the same as getType();

Parameters
----------
* `handle` :
    the handle from the publication, subscription, endpoint or filter
";

%feature("docstring") helics::CommonCore::setValue "

Publish specified data to the specified key.

Parameters
----------
* `handle` :
    a handle to a publication to use for the value
* `data` :
    the raw data to send
* `len` :
    the size of the data
";

%feature("docstring") helics::CommonCore::getValue "

Return the data for the specified handle.
";

%feature("docstring") helics::CommonCore::getValueUpdates "

Returns vector of subscription handles that received an update during the last
time request. The data remains valid until the next call to getValueUpdates for
the given federateID

Parameters
----------
* `federateID` :
    the identification code of the federate to query

Returns
-------
a reference to the location of an array of handles that have been updated
";

%feature("docstring") helics::CommonCore::registerEndpoint "

Message interface. Designed for point-to-point communication patterns. Register
an endpoint.

May only be invoked in the Initialization state.
";

%feature("docstring") helics::CommonCore::getEndpoint "

get a endpoint Handle from its name or target(this may not be unique so it will
only find the first one)

Parameters
----------
* `federateID` :
    the identifier for the federate
* `name` :
    the name of the endpoint

Returns
-------
a handle to identify the endpoint
";

%feature("docstring") helics::CommonCore::registerSourceFilter "

Register source filter.

May only be invoked in the Initialization state.

Parameters
----------
* `filterName` :
    the name of the filter (may be left blank and one will be automatically
    assigned)
* `source` :
    the target endpoint for the filter
* `type_in` :
    the input type of the filter
* `type_out` :
    the output type of the filter (may be left blank if the filter doesn't
    change type) this is important for ordering in filters with operators

Returns
-------
the handle for the new filter
";

%feature("docstring") helics::CommonCore::registerDestinationFilter "

Register destination filter.

a destination filter will create an additional processing step of messages
before they get to a destination endpoint

May only be invoked in the Initialization state.

Parameters
----------
* `filterName` :
    the name of the filter (may be left blank)
* `dest` :
    the target endpoint for the filter
* `type_in` :
    the input type of the filter (may be left blank, this is for error checking
    and will produce a warning if it doesn't match with the input type of the
    target endpoint

Returns
-------
the handle for the new filter
";

%feature("docstring") helics::CommonCore::getSourceFilter "

get a source filter Handle from its name or target(this may not be unique so it
will only find the first one)

Parameters
----------
* `name` :
    the name of the filter or its target

Returns
-------
a handle to identify the filter
";

%feature("docstring") helics::CommonCore::getDestinationFilter "

get a destination filter Handle from its name or target(this may not be unique
so it will only find the first one)

Parameters
----------
* `name` :
    the name of the filter or its target

Returns
-------
a handle to identify the filter
";

%feature("docstring") helics::CommonCore::addDependency "

add a time dependency between federates

this function is primarily useful for Message federates which do not otherwise
restrict the dependencies adding a dependency gives additional information to
the core that the specified federate(given by id) will be sending Messages to
the named Federate(by federateName)

Parameters
----------
* `federateID` :
    the identifier for the federate
* `federateName` :
    the name of the dependent federate
";

%feature("docstring") helics::CommonCore::registerFrequentCommunicationsPair "

Register known frequently communicating source/destination end points.

May be used for error checking for compatible types and possible optimization by
pre-registering the intent for these endpoints to communicate.
";

%feature("docstring") helics::CommonCore::send "

Send data from source to destination.

Time is implicitly defined as the end of the current time advancement window
(value returned by last call to nextTime().

This send version was designed to enable communication of data between federates
with the possible introduction of source and destination filters to represent
properties of a communication network. This enables simulations to be run
with/without a communications model present.
";

%feature("docstring") helics::CommonCore::sendEvent "

Send data from source to destination with explicit expected delivery time.

Time supplied is the time that will be reported in the message in the receiving
federate.

This send version was designed to enable communication of events between
discrete event federates. For this use case the receiving federate can
deserialize the data and schedule an event for the specified time.

Parameters
----------
* `time` :
    the time the event is scheduled for
* `sourceHandle` :
    the source of the event
* `destination` :
    the target of the event
* `data` :
    the raw data for the event
* `length` :
    the record length of the event
";

%feature("docstring") helics::CommonCore::sendMessage "

Send for filters.

Continues sending the message to the next filter or to final destination.
";

%feature("docstring") helics::CommonCore::receiveCount "

Returns the number of pending receives for the specified destination endpoint or
filter.
";

%feature("docstring") helics::CommonCore::receive "

Returns the next buffered message the specified destination endpoint or filter.

this is a non-blocking call and will return a nullptr if no message are
available
";

%feature("docstring") helics::CommonCore::receiveAny "

Receives a message for any destination.

this is a non-blocking call and will return a nullptr if no messages are
available

Parameters
----------
* `federateID` :
    the identifier for the federate
* `endpoint_id` :
    the endpoint handle related to the message gets stored here
";

%feature("docstring") helics::CommonCore::receiveCountAny "

Returns number of messages for all destinations.
";

%feature("docstring") helics::CommonCore::logMessage "

send a log message to the Core for logging

Parameters
----------
* `federateID` :
    the federate that is sending the log message
* `logLevel` :
    an integer for the log level (0- error, 1- warning, 2-status, 3-debug)
* `logMessage` :
    the message to log
";

%feature("docstring") helics::CommonCore::setFilterOperator "

set the filter callback operator

Parameters
----------
* `filter` :
    the handle of the filter
* `operator` :
    pointer to the operator class executing the filter
";

%feature("docstring") helics::CommonCore::setIdentifier "

set the local identification for the core
";

%feature("docstring") helics::CommonCore::getIdentifier "

get the local identifier for the core
";

%feature("docstring") helics::CommonCore::getFederateNameNoThrow "
";

%feature("docstring") helics::CommonCore::setLoggingCallback "

define a logging function to use for logging message and notices from the
federation and individual federate

Parameters
----------
* `federateID` :
    the identifier for the individual federate or 0 for the Core Logger
* `logFunction` :
    the callback function for doing something with a log message it takes 3
    inputs an integer for logLevel 0-4+ 0 -error, 1- warning 2-status, 3-debug
    44trace A string indicating the source of the message and another string
    with the actual message
";

%feature("docstring") helics::CommonCore::query "

make a query for information from the co-simulation

the format is somewhat unspecified target is the name of an object typically one
of \"federation\", \"broker\", \"core\", or the name of a specific object query
is a broken

Parameters
----------
* `target` :
    the specific target of the query
* `queryStr` :
    the actual query

Returns
-------
a string containing the response to the query. Query is a blocking call and will
not return until the query is answered so use with caution
";

%feature("docstring") helics::CommonCore::setQueryCallback "

supply a query callback function

the intention of the query callback is to allow federates to answer particular
requests through the query interface this allows other federates to make
requests or queries of other federates in an asynchronous fashion.

Parameters
----------
* `federateID` :
    the identifier for the federate
* `queryFunction` :
    a function object that returns a string as a result of a query in the form
    of const string ref.
    This callback will be called when a federate received a query that cannot be
    answered that directed at a particular federate
";

%feature("docstring") helics::CommonCore::getAddress "

get a string representing the connection info to send data to this object
";

%feature("docstring") helics::CommonCore::connect "

connect the core to a broker if needed

Returns
-------
true if the connection was successful
";

%feature("docstring") helics::CommonCore::isConnected "

check if the core is connected properly
";

%feature("docstring") helics::CommonCore::disconnect "

disconnect the core from its broker
";

%feature("docstring") helics::CommonCore::unregister "

unregister the core from any process find functions
";

%feature("docstring") helics::CommonCore::processDisconnect "

process a disconnect signal
";

// File: classhelics_1_1CommsBroker.xml


%feature("docstring") helics::CommsBroker "

helper class defining some common functionality for brokers and cores that use
different communication methods

C++ includes: CommsBroker.hpp
";

%feature("docstring") helics::CommsBroker::CommsBroker "

default constructor
";

%feature("docstring") helics::CommsBroker::CommsBroker "

create from a single argument
";

%feature("docstring") helics::CommsBroker::CommsBroker "

create from an object name
";

%feature("docstring") helics::CommsBroker::~CommsBroker "

destructor
";

%feature("docstring") helics::CommsBroker::transmit "
";

%feature("docstring") helics::CommsBroker::addRoute "
";

// File: classhelics_1_1CommsInterface.xml


%feature("docstring") helics::CommsInterface "

implementation of a generic communications interface

C++ includes: CommsInterface.hpp
";

%feature("docstring") helics::CommsInterface::CommsInterface "

default constructor
";

%feature("docstring") helics::CommsInterface::CommsInterface "

construct from a localTarget and brokerTarget

Parameters
----------
* `localTarget` :
    the interface or specification that should be set to receive incoming
    connections
* `brokerTarget` :
    the target of the broker Interface to link to
";

%feature("docstring") helics::CommsInterface::CommsInterface "

construct from a NetworkBrokerData structure
";

%feature("docstring") helics::CommsInterface::~CommsInterface "

destructor
";

%feature("docstring") helics::CommsInterface::transmit "

transmit a message along a particular route
";

%feature("docstring") helics::CommsInterface::addRoute "

add a new route assigned to the appropriate id
";

%feature("docstring") helics::CommsInterface::connect "

connect the commsInterface

Returns
-------
true if the connection was successful false otherwise
";

%feature("docstring") helics::CommsInterface::disconnect "

disconnected the comms interface
";

%feature("docstring") helics::CommsInterface::reconnect "

try reconnected from a mismatched or disconnection
";

%feature("docstring") helics::CommsInterface::setName "

set the name of the communicator
";

%feature("docstring") helics::CommsInterface::setCallback "

set the callback for processing the messages
";

%feature("docstring") helics::CommsInterface::setMessageSize "

set the max message size and max Queue size
";

%feature("docstring") helics::CommsInterface::isConnected "

check if the commInterface is connected
";

%feature("docstring") helics::CommsInterface::setTimeout "

set the timeout for the initial broker connection

Parameters
----------
* `timeout` :
    the value is in milliseconds
";

// File: classhelics_1_1conditionalChangeOnDestroy.xml


%feature("docstring") helics::conditionalChangeOnDestroy "
";

%feature("docstring") helics::conditionalChangeOnDestroy::conditionalChangeOnDestroy "
";

%feature("docstring") helics::conditionalChangeOnDestroy::~conditionalChangeOnDestroy "
";

// File: classhelics_1_1ConnectionFailure.xml


%feature("docstring") helics::ConnectionFailure "

exception indicating that the registration of an object has failed

C++ includes: core-exceptions.hpp
";

%feature("docstring") helics::ConnectionFailure::ConnectionFailure "
";

// File: classzmq_1_1context__t.xml


%feature("docstring") zmq::context_t "
";

%feature("docstring") zmq::context_t::context_t "
";

%feature("docstring") zmq::context_t::context_t "
";

%feature("docstring") zmq::context_t::~context_t "
";

%feature("docstring") zmq::context_t::close "
";

// File: classhelics_1_1Core.xml


%feature("docstring") helics::Core "

the class defining the core interface through an abstract class

C++ includes: Core.hpp
";

%feature("docstring") helics::Core::Core "

default constructor
";

%feature("docstring") helics::Core::~Core "

virtual destructor
";

%feature("docstring") helics::Core::initialize "

Simulator control. Initialize the core.

Should be invoked a single time to initialize the co-simulation core.
";

%feature("docstring") helics::Core::initializeFromArgs "

Initialize the core from command line arguments.

Should be invoked a single time to initialize the co-simulation core.
";

%feature("docstring") helics::Core::isInitialized "

Returns true if the core has been initialized.
";

%feature("docstring") helics::Core::connect "

connect the core to a broker if needed

Returns
-------
true if the connection was successful
";

%feature("docstring") helics::Core::isConnected "

check if the core is connected properly
";

%feature("docstring") helics::Core::disconnect "

disconnect the core from its broker
";

%feature("docstring") helics::Core::isOpenToNewFederates "

check if the core is ready to accept new federates
";

%feature("docstring") helics::Core::getIdentifier "

get and identifier string for the core
";

%feature("docstring") helics::Core::error "

Federate has encountered an unrecoverable error.
";

%feature("docstring") helics::Core::finalize "

Federate has completed.

Should be invoked a single time to complete the simulation.
";

%feature("docstring") helics::Core::enterInitializingState "

Federates may be in three states.

Federates are in three states.

1.  Startup Configuration of the federate. State begins when registerFederate()
    is invoked and ends when enterInitializingState() is invoked.
2.  Initializing Configure of the simulation state prior to the start of time
    stepping. State begins when enterInitializingState() is invoked and ends
    when enterExecutingState(true) is invoked.
3.  Executing State begins when enterExecutingState() is invoked and ends when
    Finalize() is invoked. Change the federate state to the Initializing state.

May only be invoked in Created state otherwise an error is thrown
";

%feature("docstring") helics::Core::enterExecutingState "

Change the federate state to the Executing state.

May only be invoked in Initializing state.

Parameters
----------
* `federateID` :
    the identifier of the federate
* `iterationCompleted` :
    if true no more iterations on this federate are requested if nonconverged
    the federate requests an iterative update

Returns
-------
nonconverged if the executing state has not been entered and there are updates,
complete if the simulation is ready to move on to the executing state
";

%feature("docstring") helics::Core::registerFederate "

Register a federate.

The returned FederateId is local to invoking process, FederateId's should not be
used as a global identifier.

May only be invoked in initialize state otherwise throws an error
";

%feature("docstring") helics::Core::getFederateName "

Returns the federate name.
";

%feature("docstring") helics::Core::getFederateId "

Returns the federate Id.
";

%feature("docstring") helics::Core::getFederationSize "

Returns the global number of federates that are registered only return
accurately after the initialization state has been entered
";

%feature("docstring") helics::Core::timeRequest "

Time management. Request a new time advancement window for non-reiterative
federates.

RequestTime() blocks until all non-reiterative federates have invoked
requestTime() and all reiterative federates have converged (called
requestTimeIterative() with localConverged value of true). Return time is the
minimum of all supplied times.

May only be invoked in Executing state.

Iterative federates may not invoke this method.

Parameters
----------
* `next` :
";

%feature("docstring") helics::Core::requestTimeIterative "

Request a new time advancement window for reiterative federates.

Reiterative federates block on requestTimeIterative() until all reiterative
federates have invoked requestTimeIterative(). The bool returned a global AND of
all localConverged values. If globalConverged is false, time returned is the
previous granted time. Time should not advance and another iteration attempted.
Federates should recompute state based on newly published values. Time is
advanced only when all reiterative federates have converged. If globalConverged
is True, grantedTime is the minimum of over all next times in both reiterative
and non-reiterative federates.

If a federate determines it cannot converge it should invoke the error() method.

Federates only participate it in reiterations for times that are evenly
divisible by the federates time delta.

May only be invoked in Executing state.

Non-reiterative federates may not invoke this method.

Parameters
----------
* `federateID` :
    the identifier for the federate to process
* `next` :
    the requested time
* `localConverged` :
    has the local federate converged

Returns
-------
an iteration_time object with two field grantedTime and a enumeration indicating
the state of the iteration
";

%feature("docstring") helics::Core::getCurrentReiteration "

Returns the current reiteration count for the specified federate.
";

%feature("docstring") helics::Core::getCurrentTime "

get the most recent granted Time

Parameters
----------
* `federateID`, `the` :
    id of the federate to get the time

Returns
-------
the most recent granted time or the startup time
";

%feature("docstring") helics::Core::setMaximumIterations "

Set the maximum number of iterations allowed.

The minimum value set in any federate is used.

Default value is the maximum allowed value for uint64_t.

May only be invoked in the initialize state.
";

%feature("docstring") helics::Core::setTimeDelta "

Set the minimum time resolution for the specified federate.

The value is used to constrain when the timeRequest methods return to values
that are multiples of the specified delta. This is useful for federates that are
time-stepped and making sub-time-step updates is not meaningful.

Parameters
----------
* `time` :
";

%feature("docstring") helics::Core::setOutputDelay "

Set the outputDelay time for the specified federate.

The value is used to determine the interaction amongst various federates as to
when a specific federate can influence another

Parameters
----------
* `federateID` :
    the identifier for the federate
* `timeoutputDelay` :
";

%feature("docstring") helics::Core::setPeriod "

Set the period for a specified federate.

The value is used to determine the interaction amongst various federates as to
when a specific federate can influence another

Parameters
----------
* `federateID` :
    the identifier for the federate
* `timeoutputDelay` :
";

%feature("docstring") helics::Core::setTimeOffset "

Set the periodic offset for a specified federate.

The value is used as a time shift for calculating the allowable time in a
federate the granted time must one of N*period+offset

Parameters
----------
* `federateID` :
    the identifier for the federate
* `timeOffset` :
    the periodic phase shift
";

%feature("docstring") helics::Core::setInputDelay "

Set the inputDelay time.

The value is used to determine the interaction amongst various federates as to
when a specific federate can influence another

Parameters
----------
* `federateID` :
    the identifier for the federate
* `timeImpact` :
    the length of time it take outside message to propagate into a federate
";

%feature("docstring") helics::Core::setLoggingLevel "

Set the logging level

set the logging level for an individual federate set federateID to 0 for the
core logging level

Parameters
----------
* `federateID` :
    the identifier for the federate
* `loggingLevel` :
    the level of logging to enable <0-no logging, 0 -error only, 1- warnings,
    2-normal, 3-debug, 4-trace
";

%feature("docstring") helics::Core::setFlag "

Set a flag in a a federate

Parameters
----------
* `federateID` :
    the identifier for the federate
* `flag` :
    an index code for the flag to set
* `flagValue` :
    the value to set the flag to
";

%feature("docstring") helics::Core::registerSubscription "

Value interface. Register a subscription for the specified federate.

May only be invoked in the initialize state.

Parameters
----------
* `federateID` :
* `key` :
    the name of the subscription
* `type` :
    a string describing the type of the federate
* `units` :
    a string naming the units of the federate
* `check_mode` :
    if set to required the core will error if the subscription does not have a
    corresponding publication when converting to init mode
";

%feature("docstring") helics::Core::getSubscription "

get a subscription Handle from its key

Parameters
----------
* `federateID` :
    the identifier for the federate  the tag of the subscription

Returns
-------
a handle to identify the subscription
";

%feature("docstring") helics::Core::registerPublication "

Register a publication.

May only be invoked in the initialize state.

Parameters
----------
* `federateID` :
    the identifier for the federate
* `key` :
    the tag for the publication
* `type` :
    the type of data the publication produces
* `units` :
    the units associated with the publication

Returns
-------
a handle to identify the publication
";

%feature("docstring") helics::Core::getPublication "

get a publication Handle from its key

Parameters
----------
* `federateID` :
    the identifier for the federate  the name of the publication

Returns
-------
a handle to identify the publication
";

%feature("docstring") helics::Core::getHandleName "

Returns the name or identifier for a specified handle
";

%feature("docstring") helics::Core::getTarget "

Returns the target of a specified handle

for publications and subscriptions this is the key for filters this is the
target and for endpoints this will return an empty string
";

%feature("docstring") helics::Core::getUnits "

Returns units for specified handle.
";

%feature("docstring") helics::Core::getType "

Returns type for specified handle.

for endpoints, publications, and filters, this is the input type for
subscriptions this is the type of the publication(if available)

Parameters
----------
* `handle` :
    the handle from the publication, subscription, endpoint or filter
";

%feature("docstring") helics::Core::getOutputType "

Returns output type for specified handle.

for filters this is the outputType, for Subscriptions this is the expected type
for endpoints and publications this is the same as getType();

Parameters
----------
* `handle` :
    the handle from the publication, subscription, endpoint or filter
";

%feature("docstring") helics::Core::setValue "

Publish specified data to the specified key.

Parameters
----------
* `handle` :
    a handle to a publication to use for the value
* `data` :
    the raw data to send
* `len` :
    the size of the data
";

%feature("docstring") helics::Core::getValue "

Return the data for the specified handle.
";

%feature("docstring") helics::Core::getValueUpdates "

Returns vector of subscription handles that received an update during the last
time request. The data remains valid until the next call to getValueUpdates for
the given federateID

Parameters
----------
* `federateID` :
    the identification code of the federate to query

Returns
-------
a reference to the location of an array of handles that have been updated
";

%feature("docstring") helics::Core::registerEndpoint "

Message interface. Designed for point-to-point communication patterns. Register
an endpoint.

May only be invoked in the Initialization state.
";

%feature("docstring") helics::Core::getEndpoint "

get a endpoint Handle from its name or target(this may not be unique so it will
only find the first one)

Parameters
----------
* `federateID` :
    the identifier for the federate
* `name` :
    the name of the endpoint

Returns
-------
a handle to identify the endpoint
";

%feature("docstring") helics::Core::registerSourceFilter "

Register source filter.

May only be invoked in the Initialization state.

Parameters
----------
* `filterName` :
    the name of the filter (may be left blank and one will be automatically
    assigned)
* `source` :
    the target endpoint for the filter
* `type_in` :
    the input type of the filter
* `type_out` :
    the output type of the filter (may be left blank if the filter doesn't
    change type) this is important for ordering in filters with operators

Returns
-------
the handle for the new filter
";

%feature("docstring") helics::Core::registerDestinationFilter "

Register destination filter.

a destination filter will create an additional processing step of messages
before they get to a destination endpoint

May only be invoked in the Initialization state.

Parameters
----------
* `filterName` :
    the name of the filter (may be left blank)
* `dest` :
    the target endpoint for the filter
* `type_in` :
    the input type of the filter (may be left blank, this is for error checking
    and will produce a warning if it doesn't match with the input type of the
    target endpoint

Returns
-------
the handle for the new filter
";

%feature("docstring") helics::Core::getSourceFilter "

get a source filter Handle from its name or target(this may not be unique so it
will only find the first one)

Parameters
----------
* `name` :
    the name of the filter or its target

Returns
-------
a handle to identify the filter
";

%feature("docstring") helics::Core::getDestinationFilter "

get a destination filter Handle from its name or target(this may not be unique
so it will only find the first one)

Parameters
----------
* `name` :
    the name of the filter or its target

Returns
-------
a handle to identify the filter
";

%feature("docstring") helics::Core::addDependency "

add a time dependency between federates

this function is primarily useful for Message federates which do not otherwise
restrict the dependencies adding a dependency gives additional information to
the core that the specified federate(given by id) will be sending Messages to
the named Federate(by federateName)

Parameters
----------
* `federateID` :
    the identifier for the federate
* `federateName` :
    the name of the dependent federate
";

%feature("docstring") helics::Core::registerFrequentCommunicationsPair "

Register known frequently communicating source/destination end points.

May be used for error checking for compatible types and possible optimization by
pre-registering the intent for these endpoints to communicate.
";

%feature("docstring") helics::Core::send "

Send data from source to destination.

Time is implicitly defined as the end of the current time advancement window
(value returned by last call to nextTime().

This send version was designed to enable communication of data between federates
with the possible introduction of source and destination filters to represent
properties of a communication network. This enables simulations to be run
with/without a communications model present.
";

%feature("docstring") helics::Core::sendEvent "

Send data from source to destination with explicit expected delivery time.

Time supplied is the time that will be reported in the message in the receiving
federate.

This send version was designed to enable communication of events between
discrete event federates. For this use case the receiving federate can
deserialize the data and schedule an event for the specified time.

Parameters
----------
* `time` :
    the time the event is scheduled for
* `sourceHandle` :
    the source of the event
* `destination` :
    the target of the event
* `data` :
    the raw data for the event
* `length` :
    the record length of the event
";

%feature("docstring") helics::Core::sendMessage "

Send for filters.

Continues sending the message to the next filter or to final destination.
";

%feature("docstring") helics::Core::receiveCount "

Returns the number of pending receives for the specified destination endpoint or
filter.
";

%feature("docstring") helics::Core::receive "

Returns the next buffered message the specified destination endpoint or filter.

this is a non-blocking call and will return a nullptr if no message are
available
";

%feature("docstring") helics::Core::receiveAny "

Receives a message for any destination.

this is a non-blocking call and will return a nullptr if no messages are
available

Parameters
----------
* `federateID` :
    the identifier for the federate
* `endpoint_id` :
    the endpoint handle related to the message gets stored here
";

%feature("docstring") helics::Core::receiveCountAny "

Returns number of messages for all destinations.
";

%feature("docstring") helics::Core::logMessage "

send a log message to the Core for logging

Parameters
----------
* `federateID` :
    the federate that is sending the log message
* `logLevel` :
    an integer for the log level (0- error, 1- warning, 2-status, 3-debug)
* `logMessage` :
    the message to log
";

%feature("docstring") helics::Core::setFilterOperator "

set the filter callback operator

Parameters
----------
* `filter` :
    the handle of the filter
* `operator` :
    pointer to the operator class executing the filter
";

%feature("docstring") helics::Core::setLoggingCallback "

define a logging function to use for logging message and notices from the
federation and individual federate

Parameters
----------
* `federateID` :
    the identifier for the individual federate or 0 for the Core Logger
* `logFunction` :
    the callback function for doing something with a log message it takes 3
    inputs an integer for logLevel 0-4+ 0 -error, 1- warning 2-status, 3-debug
    44trace A string indicating the source of the message and another string
    with the actual message
";

%feature("docstring") helics::Core::query "

make a query for information from the co-simulation

the format is somewhat unspecified target is the name of an object typically one
of \"federation\", \"broker\", \"core\", or the name of a specific object query
is a broken

Parameters
----------
* `target` :
    the specific target of the query
* `queryStr` :
    the actual query

Returns
-------
a string containing the response to the query. Query is a blocking call and will
not return until the query is answered so use with caution
";

%feature("docstring") helics::Core::setQueryCallback "

supply a query callback function

the intention of the query callback is to allow federates to answer particular
requests through the query interface this allows other federates to make
requests or queries of other federates in an asynchronous fashion.

Parameters
----------
* `federateID` :
    the identifier for the federate
* `queryFunction` :
    a function object that returns a string as a result of a query in the form
    of const string ref.
    This callback will be called when a federate received a query that cannot be
    answered that directed at a particular federate
";

// File: classhelics_1_1CoreBroker.xml


%feature("docstring") helics::CoreBroker "

class implementing most of the functionality of a generic broker Basically acts
as a router for information, deals with stuff internally if it can and sends
higher up if it can't or does something else if it is the root of the tree

C++ includes: CoreBroker.hpp
";

%feature("docstring") helics::CoreBroker::connect "

connect the core to its broker

should be done after initialization has complete
";

%feature("docstring") helics::CoreBroker::disconnect "

disconnect the broker from any other brokers and communications
";

%feature("docstring") helics::CoreBroker::unregister "

unregister the broker from the factory find methods
";

%feature("docstring") helics::CoreBroker::processDisconnect "

disconnect the broker from any other brokers and communications if the flag is
set it should not do the unregister step of the disconnection, if this is set it
is presumed the unregistration has already happened or it will be taken care of
manually
";

%feature("docstring") helics::CoreBroker::isConnected "

check if the broker is connected
";

%feature("docstring") helics::CoreBroker::setAsRoot "

set the broker to be a root broker

only valid before the initialization function is called
";

%feature("docstring") helics::CoreBroker::isRoot "

return true if the broker is a root broker
";

%feature("docstring") helics::CoreBroker::isOpenToNewFederates "

check if the broker is ready to accept new federates or cores
";

%feature("docstring") helics::CoreBroker::CoreBroker "

default constructor

Parameters
----------
* `setAsRootBroker` :
    set to true to indicate this object is a root broker
";

%feature("docstring") helics::CoreBroker::CoreBroker "

constructor to set the name of the broker
";

%feature("docstring") helics::CoreBroker::~CoreBroker "

destructor
";

%feature("docstring") helics::CoreBroker::initialize "

start up the broker with an initialization string containing commands and
parameters
";

%feature("docstring") helics::CoreBroker::initializeFromArgs "

initialize from command line arguments
";

%feature("docstring") helics::CoreBroker::allInitReady "

check if all the local federates are ready to be initialized

Returns
-------
true if everyone is ready, false otherwise
";

%feature("docstring") helics::CoreBroker::allDisconnected "
";

%feature("docstring") helics::CoreBroker::setIdentifier "

set the local identification string for the broker
";

%feature("docstring") helics::CoreBroker::getIdentifier "

get the local identification for the broker
";

%feature("docstring") helics::CoreBroker::displayHelp "

display the help for command line arguments on the broker
";

// File: classhelics_1_1CoreFederateInfo.xml


%feature("docstring") helics::CoreFederateInfo "

class defining some required information about the federate

C++ includes: CoreFederateInfo.hpp
";

// File: classhelics_1_1CoreObject.xml


%feature("docstring") helics::CoreObject "

object wrapping a core for the c-api

C++ includes: api_objects.h
";

%feature("docstring") helics::CoreObject::CoreObject "
";

%feature("docstring") helics::CoreObject::~CoreObject "
";

// File: classcount__time.xml


%feature("docstring") count_time "

a time counter that converts time to a a 64 bit integer by powers of 10

templateparam
-------------
* `N` :
    implying 10^N ticks per second
* `base` :
    the type to use as a base

C++ includes: timeRepresentation.hpp
";

%feature("docstring") count_time::maxVal "
";

%feature("docstring") count_time::minVal "
";

%feature("docstring") count_time::zeroVal "
";

%feature("docstring") count_time::epsilon "
";

%feature("docstring") count_time::convert "
";

%feature("docstring") count_time::toDouble "
";

%feature("docstring") count_time::toCount "
";

%feature("docstring") count_time::fromCount "
";

%feature("docstring") count_time::seconds "
";

// File: classhelics_1_1data__block.xml


%feature("docstring") helics::data_block "

basic data object for use in the user API layer

an adapter over a string, many objects will be strings actually so this is just
a wrapper for that common use case, and many other objects are small, so the
small string optimization takes advantage of that

C++ includes: core-data.hpp
";

%feature("docstring") helics::data_block::data_block "

default constructor
";

%feature("docstring") helics::data_block::data_block "

size allocation constructor
";

%feature("docstring") helics::data_block::data_block "

size and data
";

%feature("docstring") helics::data_block::data_block "

copy constructor
";

%feature("docstring") helics::data_block::data_block "

move constructor
";

%feature("docstring") helics::data_block::data_block "

construct from char *
";

%feature("docstring") helics::data_block::data_block "

construct from string
";

%feature("docstring") helics::data_block::data_block "

move from string
";

%feature("docstring") helics::data_block::data_block "

char * and length
";

%feature("docstring") helics::data_block::data_block "

construct from a vector object
";

%feature("docstring") helics::data_block::data_block "

construct from an arbitrary vector
";

%feature("docstring") helics::data_block::assign "

assignment from string and length
";

%feature("docstring") helics::data_block::swap "

swap function
";

%feature("docstring") helics::data_block::append "

append the existing data with a additional data
";

%feature("docstring") helics::data_block::append "

append the existing data with a string
";

%feature("docstring") helics::data_block::data "

return a pointer to the data
";

%feature("docstring") helics::data_block::data "

if the object is const return a const pointer
";

%feature("docstring") helics::data_block::empty "

check if the block is empty
";

%feature("docstring") helics::data_block::size "

get the size of the data block
";

%feature("docstring") helics::data_block::resize "

resize the data storage
";

%feature("docstring") helics::data_block::resize "

resize the data storage
";

%feature("docstring") helics::data_block::reserve "

reserve space in a data_block
";

%feature("docstring") helics::data_block::to_string "

get a string reference
";

%feature("docstring") helics::data_block::begin "

non const iterator
";

%feature("docstring") helics::data_block::end "

non const iterator end
";

%feature("docstring") helics::data_block::cbegin "

const iterator
";

%feature("docstring") helics::data_block::cend "

const iterator end
";

%feature("docstring") helics::data_block::push_back "

add a character to the data
";

// File: structdata__t.xml


%feature("docstring") data_t "

Data to be communicated.

Core operates on opaque byte buffers.

C++ includes: api-data.h
";

// File: classhelics_1_1data__view.xml


%feature("docstring") helics::data_view "

class containing a constant view of data block

C++ includes: Message.hpp
";

%feature("docstring") helics::data_view::data_view "

default constructor
";

%feature("docstring") helics::data_view::data_view "

construct from a shared_ptr to a data_block
";

%feature("docstring") helics::data_view::data_view "

construct from a regular data_block
";

%feature("docstring") helics::data_view::data_view "

copy constructor
";

%feature("docstring") helics::data_view::data_view "

move constructor
";

%feature("docstring") helics::data_view::data_view "

construct from a string
";

%feature("docstring") helics::data_view::data_view "

construct from a char Pointer and length
";

%feature("docstring") helics::data_view::data_view "

construct from a string
";

%feature("docstring") helics::data_view::data_view "

construct from a rValue to a string
";

%feature("docstring") helics::data_view::data_view "

construct from a char vector
";

%feature("docstring") helics::data_view::data_view "

construct from a string_view
";

%feature("docstring") helics::data_view::to_data_block "

create a new datablock from the data
";

%feature("docstring") helics::data_view::swap "

swap function
";

%feature("docstring") helics::data_view::data "

get the data block
";

%feature("docstring") helics::data_view::size "

get the length
";

%feature("docstring") helics::data_view::empty "

check if the view is empty
";

%feature("docstring") helics::data_view::string "

return a string of the data

this actually does a copy to a new string
";

%feature("docstring") helics::data_view::begin "

begin iterator
";

%feature("docstring") helics::data_view::end "

end iterator
";

%feature("docstring") helics::data_view::cbegin "

begin const iterator
";

%feature("docstring") helics::data_view::cend "

end const iterator
";

// File: classDelayedDestructor.xml


%feature("docstring") DelayedDestructor "

helper class to destroy objects at a late time when it is convenient and there
are no more possibilities of threading issues

C++ includes: delayedDestructor.hpp
";

%feature("docstring") DelayedDestructor::DelayedDestructor "
";

%feature("docstring") DelayedDestructor::DelayedDestructor "
";

%feature("docstring") DelayedDestructor::DelayedDestructor "
";

%feature("docstring") DelayedDestructor::~DelayedDestructor "
";

%feature("docstring") DelayedDestructor::destroyObjects "
";

%feature("docstring") DelayedDestructor::destroyObjects "
";

%feature("docstring") DelayedDestructor::addObjectsToBeDestroyed "
";

%feature("docstring") DelayedDestructor::addObjectsToBeDestroyed "
";

// File: classDelayedObjects.xml


%feature("docstring") DelayedObjects "

class holding a map of delayed object

C++ includes: DelayedObjects.hpp
";

%feature("docstring") DelayedObjects::DelayedObjects "
";

%feature("docstring") DelayedObjects::DelayedObjects "
";

%feature("docstring") DelayedObjects::DelayedObjects "
";

%feature("docstring") DelayedObjects::~DelayedObjects "
";

%feature("docstring") DelayedObjects::setDelayedValue "
";

%feature("docstring") DelayedObjects::setDelayedValue "
";

%feature("docstring") DelayedObjects::getFuture "
";

%feature("docstring") DelayedObjects::getFuture "
";

%feature("docstring") DelayedObjects::finishedWithValue "
";

%feature("docstring") DelayedObjects::finishedWithValue "
";

// File: classhelics_1_1DelayFilterOperation.xml


%feature("docstring") helics::DelayFilterOperation "

filter for delaying a message in time

C++ includes: FilterOperations.hpp
";

%feature("docstring") helics::DelayFilterOperation::DelayFilterOperation "
";

%feature("docstring") helics::DelayFilterOperation::set "

set a property on a filter

Parameters
----------
* `property` :
    the name of the property of the filter to change
* `val` :
    the numerical value of the property
";

%feature("docstring") helics::DelayFilterOperation::getOperator "
";

// File: classhelics_1_1DependencyInfo.xml


%feature("docstring") helics::DependencyInfo "

data class containing information about inter-federate dependencies

C++ includes: TimeDependencies.hpp
";

%feature("docstring") helics::DependencyInfo::DependencyInfo "

default constructor
";

%feature("docstring") helics::DependencyInfo::DependencyInfo "

construct from a federate id
";

%feature("docstring") helics::DependencyInfo::ProcessMessage "
";

// File: classhelics_1_1DestinationFilter.xml


%feature("docstring") helics::DestinationFilter "

class wrapping a destination filter

C++ includes: Filters.hpp
";

%feature("docstring") helics::DestinationFilter::DestinationFilter "

constructor to build an destination filter object

Parameters
----------
* `fed` :
    the MessageFederate to use
* `target` :
    the endpoint the filter is targeting
* `name` :
    the name of the filter
* `input_type` :
    the type of data the filter is expecting
* `output_type` :
    the type of data the filter is generating
";

%feature("docstring") helics::DestinationFilter::DestinationFilter "

constructor to build an destination filter object

Parameters
----------
* `cr` :
    the Core to register the filter with
* `target` :
    the endpoint the filter is targeting
* `name` :
    the name of the filter
* `input_type` :
    the type of data the filter is expecting
* `output_type` :
    the type of data the filter is generating
";

%feature("docstring") helics::DestinationFilter::~DestinationFilter "
";

// File: classdouble__time.xml


%feature("docstring") double_time "

class representing time as a floating point value

C++ includes: timeRepresentation.hpp
";

%feature("docstring") double_time::convert "
";

%feature("docstring") double_time::toDouble "
";

%feature("docstring") double_time::maxVal "
";

%feature("docstring") double_time::minVal "
";

%feature("docstring") double_time::zeroVal "
";

%feature("docstring") double_time::epsilon "
";

%feature("docstring") double_time::toCount "
";

%feature("docstring") double_time::fromCount "
";

%feature("docstring") double_time::seconds "
";

// File: classDualMappedVector.xml


%feature("docstring") DualMappedVector "
";

%feature("docstring") DualMappedVector::insert "
";

%feature("docstring") DualMappedVector::find "
";

%feature("docstring") DualMappedVector::find "
";

%feature("docstring") DualMappedVector::find "
";

%feature("docstring") DualMappedVector::find "
";

%feature("docstring") DualMappedVector::begin "
";

%feature("docstring") DualMappedVector::end "
";

%feature("docstring") DualMappedVector::cbegin "
";

%feature("docstring") DualMappedVector::cend "
";

%feature("docstring") DualMappedVector::size "
";

%feature("docstring") DualMappedVector::clear "
";

// File: classhelics_1_1Echo.xml


%feature("docstring") helics::Echo "

class implementing a Echo object, which will generate endpoint interfaces and
send a data message back to the source at the with a specified delay

the Echo class is not threadsafe, don't try to use it from multiple threads
without external protection, that will result in undefined behavior

C++ includes: echo.h
";

%feature("docstring") helics::Echo::Echo "

default constructor
";

%feature("docstring") helics::Echo::Echo "

construct from command line arguments

Parameters
----------
* `argc` :
    the number of arguments
* `argv` :
    the strings in the input
";

%feature("docstring") helics::Echo::Echo "

construct from a federate info object

Parameters
----------
* `fi` :
    a pointer info object containing information on the desired federate
    configuration
";

%feature("docstring") helics::Echo::Echo "

constructor taking a federate information structure and using the given core

Parameters
----------
* `core` :
    a pointer to core object which the federate can join
* `fi` :
    a federate information structure
";

%feature("docstring") helics::Echo::Echo "

constructor taking a file with the required information

Parameters
----------
* `jsonString` :
    file or json string defining the federate information and other
    configuration
";

%feature("docstring") helics::Echo::Echo "

move construction
";

%feature("docstring") helics::Echo::Echo "

don't allow the copy constructor
";

%feature("docstring") helics::Echo::~Echo "
";

%feature("docstring") helics::Echo::loadFile "

load a file containing publication information

Parameters
----------
* `filename` :
    the file containing the configuration and Player data accepted format are
    json, xml, and a Player format which is tab delimited or comma delimited
";

%feature("docstring") helics::Echo::initialize "

initialize the Player federate

generate all the publications and organize the points, the final publication
count will be available after this time and the Player will enter the
initialization mode, which means it will not be possible to add more
publications calling run will automatically do this if necessary
";

%feature("docstring") helics::Echo::run "
";

%feature("docstring") helics::Echo::run "

run the Echo federate until the specified time

Parameters
----------
* `stopTime_input` :
    the desired stop time
";

%feature("docstring") helics::Echo::addEndpoint "

add an endpoint to the Player

Parameters
----------
* `endpointName` :
    the name of the endpoint
* `endpointType` :
    the named type of the endpoint
";

%feature("docstring") helics::Echo::echoCount "

get the number of points loaded
";

%feature("docstring") helics::Echo::endpointCount "

get the number of endpoints
";

// File: classhelics_1_1Endpoint.xml


%feature("docstring") helics::Endpoint "

class to manage an endpoint

C++ includes: Endpoints.hpp
";

%feature("docstring") helics::Endpoint::Endpoint "

constructor to build an endpoint object

Parameters
----------
* `mFed` :
    the MessageFederate to use
* `name` :
    the name of the endpoint
* `type` :
    a named type associated with the endpoint
";

%feature("docstring") helics::Endpoint::Endpoint "

constructor to build an endpoint object

Parameters
----------
* `mFed` :
    the MessageFederate to use
* `name` :
    the name of the endpoint
* `type` :
    a named type associated with the endpoint
";

%feature("docstring") helics::Endpoint::Endpoint "
";

%feature("docstring") helics::Endpoint::send "

send a data block and length

Parameters
----------
* `dest` :
    string name of the destination
* `data` :
    pointer to data location
* `data_size` :
    the length of the data
";

%feature("docstring") helics::Endpoint::send "

send a data block and length

Parameters
----------
* `dest` :
    string name of the destination
* `data` :
    pointer to data location
* `data_size` :
    the length of the data
";

%feature("docstring") helics::Endpoint::send "

send a data block and length

Parameters
----------
* `data` :
    pointer to data location
* `data_size` :
    the length of the data
* `sendTime` :
    the time to send the message
";

%feature("docstring") helics::Endpoint::send "

send a data_view

a data view can convert from many different formats so this function should be
catching many of the common use cases

Parameters
----------
* `dest` :
    string name of the destination
* `data` :
    the information to send
";

%feature("docstring") helics::Endpoint::send "

send a data_view

a data view can convert from many different formats so this function should be
catching many of the common use cases

Parameters
----------
* `dest` :
    string name of the destination
* `data` :
    data representation to send
* `sendTime` :
    the time the message should be sent
";

%feature("docstring") helics::Endpoint::send "

send a data block and length to the target destination

Parameters
----------
* `data` :
    pointer to data location
* `data_size` :
    the length of the data
";

%feature("docstring") helics::Endpoint::send "

send a data_view to the target destination

a data view can convert from many different formats so this function should be
catching many of the common use cases

Parameters
----------
* `data` :
    the information to send
";

%feature("docstring") helics::Endpoint::send "

send a data_view to the specified target destination

a data view can convert from many different formats so this function should be
catching many of the common use cases

Parameters
----------
* `data` :
    a representation to send
* `sendTime` :
    the time the message should be sent
";

%feature("docstring") helics::Endpoint::send "

send a message object

this is to send a pre-built message

Parameters
----------
* `mess` :
    a reference to an actual message object
";

%feature("docstring") helics::Endpoint::subscribe "

subscribe the endpoint to a particular publication
";

%feature("docstring") helics::Endpoint::getMessage "

get an available message if there is no message the returned object is empty
";

%feature("docstring") helics::Endpoint::hasMessage "

check if there is a message available
";

%feature("docstring") helics::Endpoint::receiveCount "

check if there is a message available
";

%feature("docstring") helics::Endpoint::setCallback "

register a callback for an update notification

the callback is called in the just before the time request function returns

Parameters
----------
* `callback` :
    a function with signature void(endpoint_id_t, Time) time is the time the
    value was updated This callback is a notification callback and doesn't
    return the value
";

%feature("docstring") helics::Endpoint::setTargetDestination "

set a target destination for unspecified messages
";

%feature("docstring") helics::Endpoint::getName "

get the name of the endpoint
";

%feature("docstring") helics::Endpoint::getType "

get the specified type of the endpoint
";

%feature("docstring") helics::Endpoint::getID "

get the actual endpoint id for the fed
";

// File: structhelics_1_1endpoint__info.xml


%feature("docstring") helics::endpoint_info "

structure containing information about an endpoint

C++ includes: MessageFederateManager.hpp
";

%feature("docstring") helics::endpoint_info::endpoint_info "
";

// File: classhelics_1_1EndpointInfo.xml


%feature("docstring") helics::EndpointInfo "

data class containing the information about an endpoint

C++ includes: EndpointInfo.hpp
";

%feature("docstring") helics::EndpointInfo::EndpointInfo "

constructor from all data
";

%feature("docstring") helics::EndpointInfo::getMessage "

get the next message up to the specified time
";

%feature("docstring") helics::EndpointInfo::queueSize "

get the number of messages in the queue up to the specified time
";

%feature("docstring") helics::EndpointInfo::addMessage "

add a message to the queue
";

%feature("docstring") helics::EndpointInfo::firstMessageTime "

get the timestamp of the first message in the queue
";

// File: classhelics_1_1EndpointObject.xml


%feature("docstring") helics::EndpointObject "

object wrapping and endpoint

C++ includes: api_objects.h
";

// File: classzmq_1_1error__t.xml


%feature("docstring") zmq::error_t "
";

%feature("docstring") zmq::error_t::error_t "
";

%feature("docstring") zmq::error_t::what "
";

%feature("docstring") zmq::error_t::num "
";

// File: classhelics_1_1Federate.xml


%feature("docstring") helics::Federate "

base class for a federate in the application API

C++ includes: Federate.hpp
";

%feature("docstring") helics::Federate::Federate "

constructor taking a federate information structure

Parameters
----------
* `fi` :
    a federate information structure

make sure the core is connected
";

%feature("docstring") helics::Federate::Federate "

constructor taking a core and a federate information structure

Parameters
----------
* `fi` :
    a federate information structure

make sure the core is connected
";

%feature("docstring") helics::Federate::Federate "

constructor taking a file with the required information

Parameters
----------
* `jsonString` :
    can be either a JSON file or a string containing JSON code
";

%feature("docstring") helics::Federate::Federate "
";

%feature("docstring") helics::Federate::Federate "
";

%feature("docstring") helics::Federate::Federate "
";

%feature("docstring") helics::Federate::Federate "
";

%feature("docstring") helics::Federate::~Federate "

virtual destructor function
";

%feature("docstring") helics::Federate::~Federate "
";

%feature("docstring") helics::Federate::enterInitializationState "

enter the initialization mode after all interfaces have been defined

the call will block until all federates have entered initialization mode
";

%feature("docstring") helics::Federate::enterInitializationState "
";

%feature("docstring") helics::Federate::enterInitializationStateAsync "

enter the initialization mode after all interfaces have been defined

the call will not block
";

%feature("docstring") helics::Federate::enterInitializationStateAsync "
";

%feature("docstring") helics::Federate::isAsyncOperationCompleted "

called after one of the async calls and will indicate true if an async operation
has completed

only call from the same thread as the one that called the initial async call and
will return false if called when no aysnc operation is in flight
";

%feature("docstring") helics::Federate::isAsyncOperationCompleted "
";

%feature("docstring") helics::Federate::enterInitializationStateComplete "

second part of the async process for entering initializationState call after a
call to enterInitializationStateAsync if call any other time it will throw an
InvalidFunctionCall exception
";

%feature("docstring") helics::Federate::enterInitializationStateComplete "
";

%feature("docstring") helics::Federate::enterExecutionState "

enter the normal execution mode

call will block until all federates have entered this mode
";

%feature("docstring") helics::Federate::enterExecutionState "
";

%feature("docstring") helics::Federate::enterExecutionStateAsync "

enter the normal execution mode

call will block until all federates have entered this mode
";

%feature("docstring") helics::Federate::enterExecutionStateAsync "
";

%feature("docstring") helics::Federate::enterExecutionStateComplete "

complete the async call for entering Execution state

call will not block but will return quickly. The
enterInitializationStateFinalize must be called before doing other operations
";

%feature("docstring") helics::Federate::enterExecutionStateComplete "
";

%feature("docstring") helics::Federate::finalize "

terminate the simulation

call is normally non-blocking, but may block if called in the midst of an
asynchronous call sequence, no core calling commands may be called after
completion of this function
";

%feature("docstring") helics::Federate::finalize "
";

%feature("docstring") helics::Federate::disconnect "

disconnect a simulation from the core (will also call finalize before
disconnecting if necessary)
";

%feature("docstring") helics::Federate::error "

specify the simulator had an unrecoverable error
";

%feature("docstring") helics::Federate::error "

specify the simulator had an error with error code and message
";

%feature("docstring") helics::Federate::setSeparator "
";

%feature("docstring") helics::Federate::requestTime "

request a time advancement

Parameters
----------
* `the` :
    next requested time step

Returns
-------
the granted time step
";

%feature("docstring") helics::Federate::requestTime "
";

%feature("docstring") helics::Federate::requestTimeIterative "

request a time advancement

Parameters
----------
* `the` :
    next requested time step

Returns
-------
the granted time step
";

%feature("docstring") helics::Federate::requestTimeIterative "
";

%feature("docstring") helics::Federate::requestTimeAsync "

request a time advancement

Parameters
----------
* `the` :
    next requested time step
";

%feature("docstring") helics::Federate::requestTimeAsync "
";

%feature("docstring") helics::Federate::requestTimeIterativeAsync "

request a time advancement

Parameters
----------
* `the` :
    next requested time step
* `iterate` :
    a requested iteration level (none, require, optional)

Returns
-------
the granted time step

request a time advancement

Parameters
----------
* `the` :
    next requested time step

Returns
-------
the granted time step
";

%feature("docstring") helics::Federate::requestTimeIterativeAsync "
";

%feature("docstring") helics::Federate::requestTimeComplete "

request a time advancement

Parameters
----------
* `the` :
    next requested time step

Returns
-------
the granted time step
";

%feature("docstring") helics::Federate::requestTimeComplete "
";

%feature("docstring") helics::Federate::requestTimeIterativeComplete "

finalize the time advancement request

Returns
-------
the granted time step in an iteration_time structure which contains a time and
iteration result

finalize the time advancement request

Returns
-------
the granted time step
";

%feature("docstring") helics::Federate::requestTimeIterativeComplete "
";

%feature("docstring") helics::Federate::setTimeDelta "

set the minimum time delta for the federate

Parameters
----------
* `tdelta` :
    the minimum time delta to return from a time request function
";

%feature("docstring") helics::Federate::setOutputDelay "

set the look ahead time or output delay

the look ahead is the propagation time for messages/event to propagate from the
Federate to the outside federation

Parameters
----------
* `outputDelay` :
    the value of the time delay (must be >=0)

Exceptions
----------
* `invalid_value` :
    when using a time <0
";

%feature("docstring") helics::Federate::setInputDelay "

set the impact Window time

the impact window is the time window around the time request in which other
federates cannot affect the federate

Parameters
----------
* `inputDelay` :
    the look ahead time

Exceptions
----------
* `invalid_value` :
    when using a time <0
";

%feature("docstring") helics::Federate::setPeriod "

set the period and offset of the federate

the federate will on grant time on N*period+offset interval

Parameters
----------
* `period` :
    the length of time between each subsequent grants
* `offset` :
    the shift of the period from 0 offset must be < period
";

%feature("docstring") helics::Federate::setFlag "

set a flag for the federate

Parameters
----------
* `flag` :
    an index into the flag /ref flag-definitions.h
* `flagvalue` :
    the value of the flag defaults to true
";

%feature("docstring") helics::Federate::setLoggingLevel "

set the logging level for the federate @ details debug and trace only do
anything if they were enabled in the compilation

Parameters
----------
* `loggingLevel` :
    (-1: none, 0: error_only, 1: warnings, 2: normal, 3: debug, 4: trace)
";

%feature("docstring") helics::Federate::query "

make a query of the core

this call is blocking until the value is returned which make take some time
depending on the size of the federation and the specific string being queried

Parameters
----------
* `target` :
    the target of the query can be \"federation\", \"federate\", \"broker\",
    \"core\", or a specific name of a federate, core, or broker
* `queryStr` :
    a string with the query see other documentation for specific properties to
    query, can be defined by the federate

Returns
-------
a string with the value requested. this is either going to be a vector of
strings value or a json string stored in the first element of the vector. The
string \"#invalid\" is returned if the query was not valid
";

%feature("docstring") helics::Federate::query "

make a query of the core

this call is blocking until the value is returned which make take some time
depending on the size of the federation and the specific string being queried

Parameters
----------
* `queryStr` :
    a string with the query see other documentation for specific properties to
    query, can be defined by the federate if the local federate does not
    recognize the query it sends it on to the federation

Returns
-------
a string with the value requested. this is either going to be a vector of
strings value or a json string stored in the first element of the vector. The
string \"#invalid\" is returned if the query was not valid
";

%feature("docstring") helics::Federate::query "

make a query of the core

this call is blocking until the value is returned which make take some time
depending on the size of the federation and the specific string being queried

Parameters
----------
* `target` :
    the target of the query can be \"federation\", \"federate\", \"broker\",
    \"core\", or a specific name of a federate, core, or broker
* `queryStr` :
    a string with the query see other documentation for specific properties to
    query, can be defined by the federate

Returns
-------
a string with the value requested. this is either going to be a vector of
strings value or a json string stored in the first element of the vector. The
string \"#invalid\" is returned if the query was not valid
";

%feature("docstring") helics::Federate::queryAsync "

make a query of the core in an async fashion

this call is blocking until the value is returned which make take some time
depending on the size of the federation and the specific string being queried

Parameters
----------
* `target` :
    the target of the query can be \"federation\", \"federate\", \"broker\",
    \"core\", or a specific name of a federate, core, or broker
* `queryStr` :
    a string with the query see other documentation for specific properties to
    query, can be defined by the federate

Returns
-------
a query_id_t to use for returning the result
";

%feature("docstring") helics::Federate::queryAsync "

make a query of the core in an async fashion

this call is blocking until the value is returned which make take some time
depending on the size of the federation and the specific string being queried

Parameters
----------
* `queryStr` :
    a string with the query see other documentation for specific properties to
    query, can be defined by the federate

Returns
-------
a query_id_t used to get the results of the query in the future
";

%feature("docstring") helics::Federate::queryComplete "

get the results of an async query

the call will block until the results are returned inquiry of queryCompleted()
to check if the results have been returned or not yet

Parameters
----------
* `queryIndex` :
    the int value returned from the queryAsync call

Returns
-------
a string with the value requested. the format of the string will be either a
single string a string vector like \"[string1; string2]\" or json The string
\"#invalid\" is returned if the query was not valid
";

%feature("docstring") helics::Federate::isQueryCompleted "

check if an asynchronous query call has been completed

Returns
-------
true if the results are ready for
";

%feature("docstring") helics::Federate::registerSourceFilter "

define a filter interface on a source

a source filter will be sent any packets that come from a particular source if
multiple filters are defined on the same source, they will be placed in some
order defined by the core

Parameters
----------
* `the` :
    name of the endpoint
* `the` :
    inputType which the source filter can receive
";

%feature("docstring") helics::Federate::registerSourceFilter "

define a filter interface on a source

a source filter will be sent any packets that come from a particular source if
multiple filters are defined on the same source, they will be placed in some
order defined by the core

Parameters
----------
* `the` :
    name of the endpoint
* `the` :
    inputType which the source filter can receive
";

%feature("docstring") helics::Federate::registerDestinationFilter "

define a filter interface for a destination

a destination filter will be sent any packets that are going to a particular
destination multiple filters are not allowed to specify the same destination

Parameters
----------
* `the` :
    name of the destination endpoint
* `the` :
    inputType which the destination filter can receive
";

%feature("docstring") helics::Federate::registerDestinationFilter "

define a filter interface for a destination

a destination filter will be sent any packets that are going to a particular
destination multiple filters are not allowed to specify the same destination

Parameters
----------
* `the` :
    name of the destination endpoint
* `the` :
    inputType which the destination filter can receive
";

%feature("docstring") helics::Federate::getFilterName "

get the name of a filter

Parameters
----------
* `id` :
    the filter to query

Returns
-------
empty string if an invalid id is passed
";

%feature("docstring") helics::Federate::getFilterEndpoint "

get the name of the endpoint that a filter is associated with

Parameters
----------
* `id` :
    the filter to query

Returns
-------
empty string if an invalid id is passed
";

%feature("docstring") helics::Federate::getFilterInputType "

get the input type of a filter from its id

Parameters
----------
* `id` :
    the endpoint to query

Returns
-------
empty string if an invalid id is passed
";

%feature("docstring") helics::Federate::getFilterOutputType "

get the output type of a filter from its id

Parameters
----------
* `id` :
    the endpoint to query

Returns
-------
empty string if an invalid id is passed
";

%feature("docstring") helics::Federate::getFilterId "

get the id of a source filter from the name of the endpoint

Parameters
----------
* `filterName` :
    the name of the filter

Returns
-------
invalid_filter_id if name is not recognized otherwise returns the filter id
";

%feature("docstring") helics::Federate::getSourceFilterId "

get the id of a source filter from the name of the filter

Parameters
----------
* `filterName` :
    the publication id

Returns
-------
invalid_filter_id if name is not recognized otherwise returns the filter id
";

%feature("docstring") helics::Federate::getDestFilterId "

get the id of a destination filter from the name of the endpoint

Parameters
----------
* `filterName` :
    the publication id

Returns
-------
invalid_filter_id if name is not recognized otherwise returns the filter id
";

%feature("docstring") helics::Federate::setFilterOperator "

register a operator for the specified filter

for time_agnostic federates only, all other settings would trigger an error The
MessageOperator gets called when there is a message to filter, There is no order
or state to this messages can come in any order.

Parameters
----------
* `filter` :
    the identifier for the filter to trigger
* `op` :
    A shared_ptr to a message operator
";

%feature("docstring") helics::Federate::setFilterOperator "

register a operator for the specified filters

for time_agnostic federates only, all other settings would trigger an error The
MessageOperator gets called when there is a message to filter, There is no order
or state to this message can come in any order.

Parameters
----------
* `filters` :
    the identifier for the filter to trigger
* `op` :
    A shared_ptr to a message operator
";

%feature("docstring") helics::Federate::registerInterfaces "

register a set of interfaces defined in a file

call is only valid in startup mode

Parameters
----------
* `jsonString` :
    the location of the file or json String to load to generate the interfaces
";

%feature("docstring") helics::Federate::getID "

get the underlying federateID for the core
";

%feature("docstring") helics::Federate::getCurrentState "

get the current state of the federate
";

%feature("docstring") helics::Federate::getCurrentTime "

get the current Time

the most recent granted time of the federate
";

%feature("docstring") helics::Federate::getName "

get the federate name
";

%feature("docstring") helics::Federate::getCorePointer "

get a pointer to the core object used by the federate
";

// File: classhelics_1_1FederateInfo.xml


%feature("docstring") helics::FederateInfo "

data class defining federate properties and information

C++ includes: Federate.hpp
";

%feature("docstring") helics::FederateInfo::FederateInfo "

default constructor
";

%feature("docstring") helics::FederateInfo::FederateInfo "

construct from the federate name
";

%feature("docstring") helics::FederateInfo::FederateInfo "

construct from the name and type
";

%feature("docstring") helics::FederateInfo::FederateInfo "

load a federateInfo object from command line arguments

Parameters
----------
* `argc` :
    the number of arguments
* `argv` :
    an array of char * pointers to the arguments
";

%feature("docstring") helics::FederateInfo::FederateInfo "
";

%feature("docstring") helics::FederateInfo::FederateInfo "
";

%feature("docstring") helics::FederateInfo::FederateInfo "
";

%feature("docstring") helics::FederateInfo::loadInfoFromArgs "

load a federateInfo object from command line arguments

Parameters
----------
* `argc` :
    the number of arguments
* `argv` :
    an array of char * pointers to the arguments
";

%feature("docstring") helics::FederateInfo::~FederateInfo "
";

%feature("docstring") helics::FederateInfo::setFederateName "
";

%feature("docstring") helics::FederateInfo::setCoreName "
";

%feature("docstring") helics::FederateInfo::setCoreInitString "
";

%feature("docstring") helics::FederateInfo::setCoreTypeFromString "
";

%feature("docstring") helics::FederateInfo::setCoreType "
";

%feature("docstring") helics::FederateInfo::setFlag "
";

%feature("docstring") helics::FederateInfo::setOutputDelay "
";

%feature("docstring") helics::FederateInfo::setTimeDelta "
";

%feature("docstring") helics::FederateInfo::setInputDelay "
";

%feature("docstring") helics::FederateInfo::setTimeOffset "
";

%feature("docstring") helics::FederateInfo::setPeriod "
";

%feature("docstring") helics::FederateInfo::setMaxIterations "
";

%feature("docstring") helics::FederateInfo::setLoggingLevel "
";

%feature("docstring") helics::FederateInfo::getInfo "
";

// File: classhelics_1_1FederateState.xml


%feature("docstring") helics::FederateState "

class managing the information about a single federate

C++ includes: FederateState.hpp
";

%feature("docstring") helics::FederateState::FederateState "

constructor from name and information structure
";

%feature("docstring") helics::FederateState::~FederateState "

destructor
";

%feature("docstring") helics::FederateState::reset "

reset the federate to created state
";

%feature("docstring") helics::FederateState::reInit "

reset the federate to the initializing state
";

%feature("docstring") helics::FederateState::getIdentifier "

get the name of the federate
";

%feature("docstring") helics::FederateState::getState "
";

%feature("docstring") helics::FederateState::getSubscription "
";

%feature("docstring") helics::FederateState::getSubscription "
";

%feature("docstring") helics::FederateState::getPublication "
";

%feature("docstring") helics::FederateState::getPublication "
";

%feature("docstring") helics::FederateState::getEndpoint "
";

%feature("docstring") helics::FederateState::getEndpoint "
";

%feature("docstring") helics::FederateState::createSubscription "
";

%feature("docstring") helics::FederateState::createPublication "
";

%feature("docstring") helics::FederateState::createEndpoint "
";

%feature("docstring") helics::FederateState::getQueueSize "

get the size of a message queue for a specific endpoint or filter handle
";

%feature("docstring") helics::FederateState::getQueueSize "

get the sum of all message queue sizes i.e. the total number of messages
available in all endpoints
";

%feature("docstring") helics::FederateState::getCurrentIteration "

get the current iteration counter for an iterative call

this will work properly even when a federate is processing
";

%feature("docstring") helics::FederateState::receive "

get the next available message for an endpoint

Parameters
----------
* `id` :
    the handle of an endpoint or filter

Returns
-------
a pointer to a message -the ownership of the message is transfered to the caller
";

%feature("docstring") helics::FederateState::receiveAny "

get any message ready for reception

Parameters
----------
* `id` :
    the endpoint related to the message
";

%feature("docstring") helics::FederateState::setParent "

set the CommonCore object that is managing this Federate
";

%feature("docstring") helics::FederateState::getInfo "

specify the core object that manages this federate get the info structure for
the federate
";

%feature("docstring") helics::FederateState::updateFederateInfo "

update the info structure

public call so it also calls the federate lock before calling private update
function the action Message should be CMD_FED_CONFIGURE
";

%feature("docstring") helics::FederateState::grantedTime "

get the granted time of a federate
";

%feature("docstring") helics::FederateState::getEvents "

get a reference to the handles of subscriptions with value updates

< if we are processing this vector is in an undefined state
";

%feature("docstring") helics::FederateState::getDependents "

get a reference to the global ids of dependent federates
";

%feature("docstring") helics::FederateState::setCoreObject "
";

%feature("docstring") helics::FederateState::waitSetup "

process until the federate has verified its membership and assigned a global id
number
";

%feature("docstring") helics::FederateState::enterInitState "

process until the init state has been entered or there is a failure
";

%feature("docstring") helics::FederateState::enterExecutingState "

function to call when entering execution state

Parameters
----------
* `converged` :
    indicator of whether the fed should iterate if need be or not returns either
    converged or nonconverged depending on whether an iteration is needed
";

%feature("docstring") helics::FederateState::requestTime "

request a time advancement

Parameters
----------
* `nextTime` :
    the time of the requested advancement
* `converged` :
    set to complete to end dense time step iteration, nonconverged to continue
    iterating if need be

Returns
-------
an iteration time with two elements the granted time and the convergence state
";

%feature("docstring") helics::FederateState::genericUnspecifiedQueueProcess "

function to process the queue in a generic fashion used to just process messages
with no specific end in mind
";

%feature("docstring") helics::FederateState::addAction "

add an action message to the queue
";

%feature("docstring") helics::FederateState::addAction "

move a message to the queue
";

%feature("docstring") helics::FederateState::logMessage "

log a message to the federate Logger

Parameters
----------
* `level` :
    the logging level of the message
* `logMessageSource-` :
    the name of the object that sent the message
* `message` :
    the message to log
";

%feature("docstring") helics::FederateState::setLogger "

set the logging function

function must have signature void(int level, const std::string &sourceName,
const std::string &message)
";

%feature("docstring") helics::FederateState::setQueryCallback "

set the query callback function

function must have signature std::string(const std::string &query)
";

%feature("docstring") helics::FederateState::processQuery "

generate the result of a query string

Parameters
----------
* `query` :
    a query string

Returns
-------
the resulting string from the query
";

%feature("docstring") helics::FederateState::checkSetValue "

check if a value should be published or not

Parameters
----------
* `pub_id` :
    the handle of the publication
* `data` :
    the raw data to check
* `len` :
    the length of the data

Returns
-------
true if it should be published, false if not
";

// File: classhelics_1_1FedObject.xml


%feature("docstring") helics::FedObject "

object wrapping a federate for the c-api

C++ includes: api_objects.h
";

%feature("docstring") helics::FedObject::FedObject "
";

%feature("docstring") helics::FedObject::~FedObject "
";

// File: classhelics_1_1Filter.xml


%feature("docstring") helics::Filter "

class for managing a particular filter

C++ includes: Filters.hpp
";

%feature("docstring") helics::Filter::Filter "

default constructor
";

%feature("docstring") helics::Filter::Filter "

construct through a federate
";

%feature("docstring") helics::Filter::Filter "

construct through a core object
";

%feature("docstring") helics::Filter::~Filter "

virtual destructor
";

%feature("docstring") helics::Filter::setOperator "

set a message operator to process the message
";

%feature("docstring") helics::Filter::getID "
";

%feature("docstring") helics::Filter::getCoreHandle "
";

%feature("docstring") helics::Filter::getTarget "

get the target of the filter
";

%feature("docstring") helics::Filter::getName "

get the name for the filter
";

%feature("docstring") helics::Filter::getInputType "

get the specified input type of the filter
";

%feature("docstring") helics::Filter::getOutputType "

get the specified output type of the filter
";

%feature("docstring") helics::Filter::set "

set a property on a filter

Parameters
----------
* `property` :
    the name of the property of the filter to change
* `val` :
    the numerical value of the property
";

%feature("docstring") helics::Filter::setString "

set a string property on a filter

Parameters
----------
* `property` :
    the name of the property of the filter to change
* `val` :
    the numerical value of the property
";

// File: classhelics_1_1FilterCoordinator.xml


%feature("docstring") helics::FilterCoordinator "

class to manage the ordering of filter operations for an endpoint

C++ includes: FilterFunctions.hpp
";

// File: classhelics_1_1FilterInfo.xml


%feature("docstring") helics::FilterInfo "

data class defining the information about a filter

C++ includes: FilterInfo.hpp
";

%feature("docstring") helics::FilterInfo::FilterInfo "

constructor from all fields
";

// File: classhelics_1_1FilterObject.xml


%feature("docstring") helics::FilterObject "

object wrapping a source filter

C++ includes: api_objects.h
";

// File: classhelics_1_1FilterOperations.xml


%feature("docstring") helics::FilterOperations "

class for managing filter operations

C++ includes: FilterOperations.hpp
";

%feature("docstring") helics::FilterOperations::FilterOperations "
";

%feature("docstring") helics::FilterOperations::FilterOperations "
";

%feature("docstring") helics::FilterOperations::FilterOperations "
";

%feature("docstring") helics::FilterOperations::~FilterOperations "
";

%feature("docstring") helics::FilterOperations::set "

set a property on a filter

Parameters
----------
* `property` :
    the name of the property of the filter to change
* `val` :
    the numerical value of the property
";

%feature("docstring") helics::FilterOperations::setString "

set a string property on a filter

Parameters
----------
* `property` :
    the name of the property of the filter to change
* `val` :
    the numerical value of the property
";

%feature("docstring") helics::FilterOperations::getOperator "
";

// File: classhelics_1_1FilterOperator.xml


%feature("docstring") helics::FilterOperator "

FilterOperator abstract class

FilterOperators will transform a message in some way in a direct fashion

C++ includes: core-data.hpp
";

%feature("docstring") helics::FilterOperator::FilterOperator "

default constructor
";

%feature("docstring") helics::FilterOperator::~FilterOperator "

virtual destructor
";

%feature("docstring") helics::FilterOperator::process "

filter the message either modify the message or generate a new one
";

// File: classhelics_1_1FunctionExecutionFailure.xml


%feature("docstring") helics::FunctionExecutionFailure "

exception class indicating that a function has failed for some reason

C++ includes: core-exceptions.hpp
";

%feature("docstring") helics::FunctionExecutionFailure::FunctionExecutionFailure "
";

// File: structhelics_1_1helics__iteration__time.xml


%feature("docstring") helics::helics_iteration_time "
";

// File: classhelics_1_1HelicsException.xml


%feature("docstring") helics::HelicsException "

base exception class for helics

C++ includes: core-exceptions.hpp
";

%feature("docstring") helics::HelicsException::HelicsException "
";

%feature("docstring") helics::HelicsException::HelicsException "
";

%feature("docstring") helics::HelicsException::what "
";

// File: classhelics_1_1HelicsTerminated.xml


%feature("docstring") helics::HelicsTerminated "

severe exception indicating HELICS has terminated

C++ includes: core-exceptions.hpp
";

%feature("docstring") helics::HelicsTerminated::HelicsTerminated "
";

// File: classhelics_1_1identifier__id__t.xml


%feature("docstring") helics::identifier_id_t "

class defining an identifier type

the intent of this class is to limit the operations available on a publication
identifier to those that are a actually required and make sense, and make it as
low impact as possible. it also acts to limit any mistakes of a publication_id
for a subscription_id or other types of interfaces

C++ includes: helicsTypes.hpp
";

%feature("docstring") helics::identifier_id_t::identifier_id_t "

default constructor
";

%feature("docstring") helics::identifier_id_t::identifier_id_t "

value based constructor
";

%feature("docstring") helics::identifier_id_t::identifier_id_t "

copy constructor
";

%feature("docstring") helics::identifier_id_t::value "

get the underlying value
";

// File: classinteger__time.xml


%feature("docstring") integer_time "

prototype class for representing time

implements time as a count of 1/2^N seconds this is done for performance because
many mathematical operations are needed on the time and this way it could be
implemented using shift and masks for some conversions to floating point
operations

C++ includes: timeRepresentation.hpp
";

%feature("docstring") integer_time::maxVal "
";

%feature("docstring") integer_time::minVal "
";

%feature("docstring") integer_time::zeroVal "
";

%feature("docstring") integer_time::epsilon "
";

%feature("docstring") integer_time::convert "

convert to a base type representation
";

%feature("docstring") integer_time::toDouble "

convert the value to a double representation in seconds
";

%feature("docstring") integer_time::toCount "

convert the val to a count of the specified time units

really kind of awkward to do with this time representation so I just convert to
a double first
";

%feature("docstring") integer_time::fromCount "
";

%feature("docstring") integer_time::seconds "

convert to an integer count in seconds
";

// File: classhelics_1_1InvalidFunctionCall.xml


%feature("docstring") helics::InvalidFunctionCall "

exception thrown when a function call was made at an inappropriate time

defining an exception class for invalid function calls

C++ includes: core-exceptions.hpp
";

%feature("docstring") helics::InvalidFunctionCall::InvalidFunctionCall "
";

%feature("docstring") helics::InvalidFunctionCall::InvalidFunctionCall "
";

// File: classhelics_1_1InvalidIdentifier.xml


%feature("docstring") helics::InvalidIdentifier "

exception for an invalid identification Handle

C++ includes: core-exceptions.hpp
";

%feature("docstring") helics::InvalidIdentifier::InvalidIdentifier "
";

// File: classhelics_1_1InvalidParameter.xml


%feature("docstring") helics::InvalidParameter "

exception when one or more of the parameters in the function call were invalid

C++ includes: core-exceptions.hpp
";

%feature("docstring") helics::InvalidParameter::InvalidParameter "
";

// File: classhelics_1_1InvalidParameterValue.xml


%feature("docstring") helics::InvalidParameterValue "

defining an exception class for invalid parameter values

C++ includes: Federate.hpp
";

%feature("docstring") helics::InvalidParameterValue::InvalidParameterValue "
";

// File: classhelics_1_1InvalidStateTransition.xml


%feature("docstring") helics::InvalidStateTransition "

defining an exception class for state transition errors

C++ includes: Federate.hpp
";

%feature("docstring") helics::InvalidStateTransition::InvalidStateTransition "
";

// File: classhelics_1_1IpcBroker.xml


%feature("docstring") helics::IpcBroker "
";

%feature("docstring") helics::IpcBroker::IpcBroker "

default constructor
";

%feature("docstring") helics::IpcBroker::IpcBroker "
";

%feature("docstring") helics::IpcBroker::initializeFromArgs "

initialize from command line arguments
";

%feature("docstring") helics::IpcBroker::~IpcBroker "

destructor
";

%feature("docstring") helics::IpcBroker::getAddress "

get the connection address for the broker
";

%feature("docstring") helics::IpcBroker::displayHelp "
";

// File: classhelics_1_1IpcComms.xml


%feature("docstring") helics::IpcComms "

implementation for the core that uses boost interprocess messages to communicate

C++ includes: IpcComms.h
";

%feature("docstring") helics::IpcComms::IpcComms "

default constructor
";

%feature("docstring") helics::IpcComms::IpcComms "
";

%feature("docstring") helics::IpcComms::~IpcComms "

destructor
";

// File: classhelics_1_1IpcCore.xml


%feature("docstring") helics::IpcCore "

implementation for the core that uses Interprocess messages to communicate

C++ includes: IpcCore.h
";

%feature("docstring") helics::IpcCore::IpcCore "

default constructor
";

%feature("docstring") helics::IpcCore::IpcCore "
";

%feature("docstring") helics::IpcCore::~IpcCore "

destructor
";

%feature("docstring") helics::IpcCore::initializeFromArgs "

initialize the core manager with command line arguments

Parameters
----------
* `argc` :
    the number of arguments
* `argv` :
    char pointers to the arguments
";

%feature("docstring") helics::IpcCore::getAddress "

get a string representing the connection info to send data to this object
";

// File: structhelics_1_1is__iterable.xml


%feature("docstring") helics::is_iterable "

template trait for figuring out if something is an iterable container

C++ includes: ValueConverter_impl.hpp
";

// File: structhelics_1_1is__iterable_3_01T_00_01typename_01std_1_1enable__if__t_3_01std_1_1is__same_3_012393cea268135d6212d4810890e5a85a.xml


// File: structhelics_1_1is__vector.xml


%feature("docstring") helics::is_vector "

template trait for figuring out if something is a vector of objects

C++ includes: ValueConverter_impl.hpp
";

// File: structhelics_1_1is__vector_3_01T_00_01typename_01std_1_1enable__if__t_3_01std_1_1is__same_3_01T_ba576651aa0fcfda53315bc29640e40e.xml


%feature("docstring") helics::is_vector< T, typename std::enable_if_t< std::is_same< T, std::vector< typename T::value_type, typename T::allocator_type > >::value > > "
";

// File: structhelics_1_1iteration__time.xml


%feature("docstring") helics::iteration_time "

simple struct with the time and completion marker for iterations or dense time
steps

C++ includes: helics-time.hpp
";

%feature("docstring") helics::iteration_time::iteration_time "

default constructor
";

%feature("docstring") helics::iteration_time::iteration_time "

construct from properties
";

// File: classhelics_1_1Logger.xml


%feature("docstring") helics::Logger "

class implementing a thread safe Logger

the Logger uses a queuing mechanism and condition variable to store messages to
a queue and print/display them in a single thread allowing for asynchronous
logging

C++ includes: logger.h
";

%feature("docstring") helics::Logger::Logger "

default constructor
";

%feature("docstring") helics::Logger::Logger "

construct and link to the specified logging Core
";

%feature("docstring") helics::Logger::~Logger "

destructor
";

%feature("docstring") helics::Logger::openFile "

open a file to write the log messages

Parameters
----------
* `file` :
    the name of the file to write messages to
";

%feature("docstring") helics::Logger::startLogging "

function to start the logging thread

Parameters
----------
* `cLevel` :
    the console print level
* `fLevel` :
    the file print level messages coming in below these levels will be printed
";

%feature("docstring") helics::Logger::startLogging "

overload of

See also: startLogging with unspecified logging levels
";

%feature("docstring") helics::Logger::haltLogging "

stop logging for a time messages received while halted are ignored
";

%feature("docstring") helics::Logger::log "

log a message at a particular level

Parameters
----------
* `level` :
    the level of the message
* `logMessage` :
    the actual message to log
";

%feature("docstring") helics::Logger::log "

message to log without regard for levels*

Parameters
----------
* `logMessage` :
    the message to log
";

%feature("docstring") helics::Logger::flush "

flush the log queue
";

%feature("docstring") helics::Logger::isRunning "

check if the Logger is running
";

%feature("docstring") helics::Logger::changeLevels "

alter the printing levels

Parameters
----------
* `cLevel` :
    the level to print to the console
* `fLevel` :
    the level to print to the file if it is open
";

// File: classhelics_1_1LoggerManager.xml


%feature("docstring") helics::LoggerManager "

class defining a singleton manager for all logging use

C++ includes: logger.h
";

%feature("docstring") helics::LoggerManager::getLoggerManager "

get a pointer to a logging manager so it cannot go out of scope
";

%feature("docstring") helics::LoggerManager::getLoggerCore "

get a pointer to a logging core
";

%feature("docstring") helics::LoggerManager::closeLogger "

close the named Logger

prevents the Logger from being retrieved through this class but does not
necessarily destroy the Logger
";

%feature("docstring") helics::LoggerManager::logMessage "

sends a message to the default Logger
";

%feature("docstring") helics::LoggerManager::~LoggerManager "
";

%feature("docstring") helics::LoggerManager::getName "
";

// File: classhelics_1_1LoggerNoThread.xml


%feature("docstring") helics::LoggerNoThread "

logging class that handle the logs immediately with no threading or
synchronization

C++ includes: logger.h
";

%feature("docstring") helics::LoggerNoThread::LoggerNoThread "

default constructor
";

%feature("docstring") helics::LoggerNoThread::LoggerNoThread "

this does nothing with the argument since it is not threaded here to match the
API of Logger
";

%feature("docstring") helics::LoggerNoThread::openFile "

open a file to write the log messages

Parameters
----------
* `file` :
    the name of the file to write messages to
";

%feature("docstring") helics::LoggerNoThread::startLogging "

function to start the logging thread

Parameters
----------
* `cLevel` :
    the console print level
* `fLevel` :
    the file print level messages coming in below these levels will be printed
";

%feature("docstring") helics::LoggerNoThread::startLogging "

overload of ::startLogging with unspecified logging levels
";

%feature("docstring") helics::LoggerNoThread::log "

log a message at a particular level

Parameters
----------
* `level` :
    the level of the message
* `logMessage` :
    the actual message to log
";

%feature("docstring") helics::LoggerNoThread::log "

message to log without regard for levels*

Parameters
----------
* `logMessage` :
    the message to log
";

%feature("docstring") helics::LoggerNoThread::isRunning "

check if the logging thread is running
";

%feature("docstring") helics::LoggerNoThread::flush "

flush the log queue
";

%feature("docstring") helics::LoggerNoThread::changeLevels "

alter the printing levels

Parameters
----------
* `cLevel` :
    the level to print to the console
* `fLevel` :
    the level to print to the file if it is open
";

// File: classhelics_1_1LoggingCore.xml


%feature("docstring") helics::LoggingCore "

class to manage a single thread for all logging

C++ includes: logger.h
";

%feature("docstring") helics::LoggingCore::LoggingCore "

default constructor
";

%feature("docstring") helics::LoggingCore::~LoggingCore "

destructor
";

%feature("docstring") helics::LoggingCore::addMessage "

add a message for the LoggingCore or just general console print
";

%feature("docstring") helics::LoggingCore::addMessage "

move a message for the LoggingCore or just general console print
";

%feature("docstring") helics::LoggingCore::addMessage "

add a message for a specific Logger

Parameters
----------
* `index` :
    the index of the function callback to use
* `message` :
    the message to send
";

%feature("docstring") helics::LoggingCore::addMessage "

add a message for a specific Logger

Parameters
----------
* `index` :
    the index of the function callback to use
* `message` :
    the message to send
";

%feature("docstring") helics::LoggingCore::addFileProcessor "

add a file processing callback (not just files)

Parameters
----------
* `newFunction` :
    the callback to call on receipt of a message
";

%feature("docstring") helics::LoggingCore::haltOperations "

remove a function callback
";

%feature("docstring") helics::LoggingCore::updateProcessingFunction "

update a callback for a particular instance
";

// File: classMappedPointerVector.xml


%feature("docstring") MappedPointerVector "
";

%feature("docstring") MappedPointerVector::insert "
";

%feature("docstring") MappedPointerVector::find "
";

%feature("docstring") MappedPointerVector::begin "
";

%feature("docstring") MappedPointerVector::end "
";

%feature("docstring") MappedPointerVector::cbegin "
";

%feature("docstring") MappedPointerVector::cend "
";

%feature("docstring") MappedPointerVector::size "
";

%feature("docstring") MappedPointerVector::clear "
";

// File: classMappedVector.xml


%feature("docstring") MappedVector "
";

%feature("docstring") MappedVector::insert "
";

%feature("docstring") MappedVector::find "
";

%feature("docstring") MappedVector::find "
";

%feature("docstring") MappedVector::begin "
";

%feature("docstring") MappedVector::end "
";

%feature("docstring") MappedVector::cbegin "
";

%feature("docstring") MappedVector::cend "
";

%feature("docstring") MappedVector::size "
";

%feature("docstring") MappedVector::clear "
";

// File: classMasterObjectHolder.xml


%feature("docstring") MasterObjectHolder "

class for containing all the objects associated with a federation

C++ includes: api_objects.h
";

%feature("docstring") MasterObjectHolder::MasterObjectHolder "
";

%feature("docstring") MasterObjectHolder::~MasterObjectHolder "
";

%feature("docstring") MasterObjectHolder::addBroker "
";

%feature("docstring") MasterObjectHolder::addCore "
";

%feature("docstring") MasterObjectHolder::addFed "
";

%feature("docstring") MasterObjectHolder::clearBroker "
";

%feature("docstring") MasterObjectHolder::clearCore "
";

%feature("docstring") MasterObjectHolder::clearFed "
";

%feature("docstring") MasterObjectHolder::deleteAll "
";

// File: classhelics_1_1Message.xml


%feature("docstring") helics::Message "

class containing a message structure

C++ includes: core-data.hpp
";

%feature("docstring") helics::Message::Message "

default constructor
";

%feature("docstring") helics::Message::Message "

move constructor
";

%feature("docstring") helics::Message::Message "

copy constructor
";

%feature("docstring") helics::Message::swap "

swap operation for the Message
";

%feature("docstring") helics::Message::isValid "

check if the Message contains an actual Message

Returns
-------
false if there is no Message data
";

// File: structmessage__t.xml


%feature("docstring") message_t "

Message_t mapped to a c compatible structure

C++ includes: api-data.h
";

// File: classzmq_1_1message__t.xml


%feature("docstring") zmq::message_t "
";

%feature("docstring") zmq::message_t::message_t "
";

%feature("docstring") zmq::message_t::message_t "
";

%feature("docstring") zmq::message_t::message_t "
";

%feature("docstring") zmq::message_t::message_t "
";

%feature("docstring") zmq::message_t::message_t "
";

%feature("docstring") zmq::message_t::~message_t "
";

%feature("docstring") zmq::message_t::rebuild "
";

%feature("docstring") zmq::message_t::rebuild "
";

%feature("docstring") zmq::message_t::rebuild "
";

%feature("docstring") zmq::message_t::rebuild "
";

%feature("docstring") zmq::message_t::move "
";

%feature("docstring") zmq::message_t::copy "
";

%feature("docstring") zmq::message_t::more "
";

%feature("docstring") zmq::message_t::data "
";

%feature("docstring") zmq::message_t::data "
";

%feature("docstring") zmq::message_t::data "
";

%feature("docstring") zmq::message_t::data "
";

%feature("docstring") zmq::message_t::size "
";

%feature("docstring") zmq::message_t::equal "
";

%feature("docstring") zmq::message_t::gets "
";

// File: classhelics_1_1MessageConditionalOperator.xml


%feature("docstring") helics::MessageConditionalOperator "

class defining an message operator that either passes the message or not

the evaluation function used should return true if the message should be allowed
through false if it should be dropped

C++ includes: MessageOperators.hpp
";

%feature("docstring") helics::MessageConditionalOperator::MessageConditionalOperator "

default constructor
";

%feature("docstring") helics::MessageConditionalOperator::MessageConditionalOperator "

set the function to modify the data of the message in the constructor
";

%feature("docstring") helics::MessageConditionalOperator::setConditionFunction "

set the function to modify the data of the message
";

// File: classhelics_1_1MessageDataOperator.xml


%feature("docstring") helics::MessageDataOperator "

class defining an message operator that operates purely on the data aspect of a
message

C++ includes: MessageOperators.hpp
";

%feature("docstring") helics::MessageDataOperator::MessageDataOperator "

default constructor
";

%feature("docstring") helics::MessageDataOperator::MessageDataOperator "

set the function to modify the data of the message in the constructor
";

%feature("docstring") helics::MessageDataOperator::setDataFunction "

set the function to modify the data of the message
";

// File: classhelics_1_1MessageDestOperator.xml


%feature("docstring") helics::MessageDestOperator "

class defining an message operator that operates purely on the destination
aspect of a message

C++ includes: MessageOperators.hpp
";

%feature("docstring") helics::MessageDestOperator::MessageDestOperator "

default constructor
";

%feature("docstring") helics::MessageDestOperator::MessageDestOperator "

set the function to modify the time of the message in the constructor
";

%feature("docstring") helics::MessageDestOperator::setDestFunction "

set the function to modify the time of the message
";

// File: classhelics_1_1MessageFederate.xml


%feature("docstring") helics::MessageFederate "

class defining the block communication based interface

C++ includes: MessageFederate.hpp
";

%feature("docstring") helics::MessageFederate::MessageFederate "

constructor taking a federate information structure and using the default core

Parameters
----------
* `fi` :
    a federate information structure
";

%feature("docstring") helics::MessageFederate::MessageFederate "

constructor taking a core and a federate information structure, sore information
in fi is ignored

Parameters
----------
* `core` :
    a shared ptr to a core to join
* `fi` :
    a federate information structure
";

%feature("docstring") helics::MessageFederate::MessageFederate "

constructor taking a string with the required information

Parameters
----------
* `jsonString` :
    can be either a json file or a string containing json code
";

%feature("docstring") helics::MessageFederate::MessageFederate "

move constructor
";

%feature("docstring") helics::MessageFederate::MessageFederate "

default constructor
";

%feature("docstring") helics::MessageFederate::MessageFederate "

special constructor should only be used by child classes in constructor due to
virtual inheritance
";

%feature("docstring") helics::MessageFederate::MessageFederate "
";

%feature("docstring") helics::MessageFederate::MessageFederate "
";

%feature("docstring") helics::MessageFederate::MessageFederate "
";

%feature("docstring") helics::MessageFederate::~MessageFederate "

destructor
";

%feature("docstring") helics::MessageFederate::~MessageFederate "
";

%feature("docstring") helics::MessageFederate::registerEndpoint "

register an endpoint

call is only valid in startup mode

Parameters
----------
* `name` :
    the name of the endpoint
* `type` :
    the defined type of the interface for endpoint checking if requested
";

%feature("docstring") helics::MessageFederate::registerEndpoint "

Methods for registering endpoints
";

%feature("docstring") helics::MessageFederate::registerGlobalEndpoint "

register an endpoint directly without prepending the federate name

call is only valid in startup mode

Parameters
----------
* `name` :
    the name of the endpoint
* `type` :
    the defined type of the interface for endpoint checking if requested
";

%feature("docstring") helics::MessageFederate::registerGlobalEndpoint "
";

%feature("docstring") helics::MessageFederate::registerInterfaces "

register a set of interfaces defined in a file

call is only valid in startup mode

Parameters
----------
* `jsonString` :
    the location of the file or json String to load to generate the interfaces
";

%feature("docstring") helics::MessageFederate::registerKnownCommunicationPath "

give the core a hint for known communication paths

the function will generate an error in the core if a communication path is not
present once the simulation is initialized

Parameters
----------
* `localEndpoint` :
    the local endpoint of a known communication pair
* `remoteEndpoint` :
    of a communication pair
";

%feature("docstring") helics::MessageFederate::subscribe "

subscribe to valueFederate publication to be delivered as Messages to the given
endpoint

Parameters
----------
* `endpoint` :
    the specified endpoint to deliver the values
* `name` :
    the name of the publication to subscribe
* `type` :
    the type of publication
";

%feature("docstring") helics::MessageFederate::subscribe "

Subscribe to an endpoint
";

%feature("docstring") helics::MessageFederate::hasMessage "

check if the federate has any outstanding messages
";

%feature("docstring") helics::MessageFederate::hasMessage "
";

%feature("docstring") helics::MessageFederate::hasMessage "

Checks if federate has any messages
";

%feature("docstring") helics::MessageFederate::hasMessage "
";

%feature("docstring") helics::MessageFederate::receiveCount "

Returns the number of pending receives for the specified destination endpoint.
";

%feature("docstring") helics::MessageFederate::receiveCount "

Returns the number of pending receives for all endpoints.

Returns the number of pending receives for the specified destination endpoint.

this function is not preferred in multithreaded contexts due to the required
locking prefer to just use getMessage until it returns an invalid Message.
";

%feature("docstring") helics::MessageFederate::receiveCount "

Returns the number of pending receives for endpoint
";

%feature("docstring") helics::MessageFederate::receiveCount "

Returns the number of pending receives for all endpoints.
";

%feature("docstring") helics::MessageFederate::getMessage "

receive a packet from a particular endpoint

Parameters
----------
* `endpoint` :
    the identifier for the endpoint

Returns
-------
a message object
";

%feature("docstring") helics::MessageFederate::getMessage "

receive a communication message for any endpoint in the federate

the return order will be in order of endpoint creation then order of arrival all
messages for the first endpoint, then all for the second, and so on

Returns
-------
a unique_ptr to a Message object containing the message data
";

%feature("docstring") helics::MessageFederate::getMessage "

Get a packet from an endpoint
";

%feature("docstring") helics::MessageFederate::getMessage "

Get a packet for any endpoints in the federate
";

%feature("docstring") helics::MessageFederate::sendMessage "

send a message

send a message to a specific destination

Parameters
----------
* `source` :
    the source endpoint
* `dest` :
    a string naming the destination
* `data` :
    a buffer containing the data
* `len` :
    the length of the data buffer
";

%feature("docstring") helics::MessageFederate::sendMessage "

send a message

send a message to a specific destination

Parameters
----------
* `source` :
    the source endpoint
* `dest` :
    a string naming the destination
* `message` :
    a data_view of the message
";

%feature("docstring") helics::MessageFederate::sendMessage "

send an event message at a particular time

send a message to a specific destination

Parameters
----------
* `source` :
    the source endpoint
* `dest` :
    a string naming the destination
* `data` :
    a buffer containing the data
* `len` :
    the length of the data buffer
* `Time` :
    the time the message should be sent
";

%feature("docstring") helics::MessageFederate::sendMessage "

send an event message at a particular time

send a message to a specific destination

Parameters
----------
* `source` :
    the source endpoint
* `dest` :
    a string naming the destination
* `message` :
    a data_view of the message data to send
* `Time` :
    the time the message should be sent
";

%feature("docstring") helics::MessageFederate::sendMessage "

send an event message at a particular time

send a message to a specific destination

Parameters
----------
* `source` :
    the source endpoint
* `message` :
    a pointer to the message
";

%feature("docstring") helics::MessageFederate::sendMessage "

send an event message at a particular time

send a message to a specific destination

Parameters
----------
* `source` :
    the source endpoint
* `message` :
    a message object
";

%feature("docstring") helics::MessageFederate::sendMessage "

Methods for sending a message
";

%feature("docstring") helics::MessageFederate::sendMessage "
";

%feature("docstring") helics::MessageFederate::sendMessage "
";

%feature("docstring") helics::MessageFederate::getEndpointName "

get the name of an endpoint from its id

Parameters
----------
* `id` :
    the endpoint to query

Returns
-------
empty string if an invalid id is passed
";

%feature("docstring") helics::MessageFederate::getEndpointName "
";

%feature("docstring") helics::MessageFederate::getEndpointId "

get the id of a registered publication from its id

Parameters
----------
* `name` :
    the publication id

Returns
-------
ivalid_publication_id if name is not recognized otherwise returns the
publication_id
";

%feature("docstring") helics::MessageFederate::getEndpointType "

get the type associated with an endpoint

Parameters
----------
* `ep` :
    the endpoint identifier

Returns
-------
a string containing the endpoint type
";

%feature("docstring") helics::MessageFederate::getEndpointType "
";

%feature("docstring") helics::MessageFederate::registerEndpointCallback "

register a callback for all endpoints

Parameters
----------
* `callback` :
    the function to execute upon receipt of a message for any endpoint
";

%feature("docstring") helics::MessageFederate::registerEndpointCallback "

register a callback for a specific endpoint

Parameters
----------
* `ep` :
    the endpoint to associate with the specified callback
* `callback` :
    the function to execute upon receipt of a message for the given endpoint
";

%feature("docstring") helics::MessageFederate::registerEndpointCallback "

register a callback for a set of specific endpoint

Parameters
----------
* `ep` :
    a vector of endpoints to associate with the specified callback
* `callback` :
    the function to execute upon receipt of a message for the given endpoint
";

%feature("docstring") helics::MessageFederate::disconnect "

disconnect a simulation from the core (will also call finalize before
disconnecting if necessary)
";

%feature("docstring") helics::MessageFederate::getEndpointCount "

get the number of registered endpoints

get a count of the number endpoints registered
";

// File: classhelics_1_1MessageFederateManager.xml


%feature("docstring") helics::MessageFederateManager "

class handling the implementation details of a value Federate

the functions will parallel those in message Federate and contain the actual
implementation details

C++ includes: MessageFederateManager.hpp
";

%feature("docstring") helics::MessageFederateManager::MessageFederateManager "

construct from a pointer to a core and a specified federate id
";

%feature("docstring") helics::MessageFederateManager::~MessageFederateManager "
";

%feature("docstring") helics::MessageFederateManager::registerEndpoint "

register an endpoint

call is only valid in startup mode

Parameters
----------
* `name` :
    the name of the endpoint
* `type` :
    the defined type of the interface for endpoint checking if requested
";

%feature("docstring") helics::MessageFederateManager::registerKnownCommunicationPath "

give the core a hint for known communication paths Specifying a path that is not
present will cause the simulation to abort with an error message

Parameters
----------
* `localEndpoint` :
    the local endpoint of a known communication pair
* `remoteEndpoint` :
    of a communication pair
";

%feature("docstring") helics::MessageFederateManager::subscribe "

subscribe to valueFederate publication to be delivered as Messages to the given
endpoint

Parameters
----------
* `endpoint` :
    the specified endpoint to deliver the values
* `name` :
    the name of the publication to subscribe
* `type` :
    the type of publication
";

%feature("docstring") helics::MessageFederateManager::hasMessage "

check if the federate has any outstanding messages
";

%feature("docstring") helics::MessageFederateManager::hasMessage "
";

%feature("docstring") helics::MessageFederateManager::receiveCount "

Returns the number of pending receives for the specified destination endpoint.
";

%feature("docstring") helics::MessageFederateManager::receiveCount "

Returns the number of pending receives for the specified destination endpoint.

Returns the number of pending receives for the specified destination endpoint.

this function is not preferred in multi-threaded contexts due to the required
locking prefer to just use getMessage until it returns an invalid Message.
";

%feature("docstring") helics::MessageFederateManager::getMessage "

receive a packet from a particular endpoint

Parameters
----------
* `endpoint` :
    the identifier for the endpoint

Returns
-------
a message object
";

%feature("docstring") helics::MessageFederateManager::getMessage "
";

%feature("docstring") helics::MessageFederateManager::sendMessage "
";

%feature("docstring") helics::MessageFederateManager::sendMessage "
";

%feature("docstring") helics::MessageFederateManager::sendMessage "
";

%feature("docstring") helics::MessageFederateManager::updateTime "

update the time from oldTime to newTime

Parameters
----------
* `newTime` :
    the newTime of the federate
* `oldTime` :
    the oldTime of the federate

find the id
";

%feature("docstring") helics::MessageFederateManager::startupToInitializeStateTransition "

transition from Startup To the Initialize State
";

%feature("docstring") helics::MessageFederateManager::initializeToExecuteStateTransition "

transition from initialize to execution State
";

%feature("docstring") helics::MessageFederateManager::getEndpointName "

get the name of an endpoint from its id

Parameters
----------
* `id` :
    the endpoint to query

Returns
-------
empty string if an invalid id is passed
";

%feature("docstring") helics::MessageFederateManager::getEndpointId "

get the id of a registered publication from its id

Parameters
----------
* `name` :
    the publication id

Returns
-------
ivalid_publication_id if name is not recognized otherwise returns the
publication_id
";

%feature("docstring") helics::MessageFederateManager::getEndpointType "

get the type of an endpoint from its id

Parameters
----------
* `id` :
    the endpoint to query

Returns
-------
empty string if an invalid id is passed or no type was specified
";

%feature("docstring") helics::MessageFederateManager::registerCallback "

register a callback function to call when any endpoint receives a message

there can only be one generic callback

Parameters
----------
* `callback` :
    the function to call
";

%feature("docstring") helics::MessageFederateManager::registerCallback "

register a callback function to call when the specified endpoint receives a
message

Parameters
----------
* `id` :
    the endpoint id to register the callback for
* `callback` :
    the function to call
";

%feature("docstring") helics::MessageFederateManager::registerCallback "

register a callback function to call when one of the specified endpoint ids
receives a message

Parameters
----------
* `ids` :
    the set of ids to register the callback for
* `callback` :
    the function to call
";

%feature("docstring") helics::MessageFederateManager::disconnect "

disconnect from the coreObject
";

%feature("docstring") helics::MessageFederateManager::getEndpointCount "

get the number of registered endpoints
";

// File: classhelics_1_1MessageHolder.xml


%feature("docstring") helics::MessageHolder "
";

// File: classhelics_1_1MessageTimeOperator.xml


%feature("docstring") helics::MessageTimeOperator "

class defining an message operator that operates purely on the time aspect of a
message

C++ includes: MessageOperators.hpp
";

%feature("docstring") helics::MessageTimeOperator::MessageTimeOperator "

default constructor
";

%feature("docstring") helics::MessageTimeOperator::MessageTimeOperator "

set the function to modify the time of the message in the constructor
";

%feature("docstring") helics::MessageTimeOperator::setTimeFunction "

set the function to modify the time of the message
";

// File: classzmq_1_1monitor__t.xml


%feature("docstring") zmq::monitor_t "
";

%feature("docstring") zmq::monitor_t::monitor_t "
";

%feature("docstring") zmq::monitor_t::~monitor_t "
";

%feature("docstring") zmq::monitor_t::monitor "
";

%feature("docstring") zmq::monitor_t::monitor "
";

%feature("docstring") zmq::monitor_t::init "
";

%feature("docstring") zmq::monitor_t::init "
";

%feature("docstring") zmq::monitor_t::check_event "
";

%feature("docstring") zmq::monitor_t::on_monitor_started "
";

%feature("docstring") zmq::monitor_t::on_event_connected "
";

%feature("docstring") zmq::monitor_t::on_event_connect_delayed "
";

%feature("docstring") zmq::monitor_t::on_event_connect_retried "
";

%feature("docstring") zmq::monitor_t::on_event_listening "
";

%feature("docstring") zmq::monitor_t::on_event_bind_failed "
";

%feature("docstring") zmq::monitor_t::on_event_accepted "
";

%feature("docstring") zmq::monitor_t::on_event_accept_failed "
";

%feature("docstring") zmq::monitor_t::on_event_closed "
";

%feature("docstring") zmq::monitor_t::on_event_close_failed "
";

%feature("docstring") zmq::monitor_t::on_event_disconnected "
";

%feature("docstring") zmq::monitor_t::on_event_handshake_failed_no_detail "
";

%feature("docstring") zmq::monitor_t::on_event_handshake_failed_protocol "
";

%feature("docstring") zmq::monitor_t::on_event_handshake_failed_auth "
";

%feature("docstring") zmq::monitor_t::on_event_handshake_succeeded "
";

%feature("docstring") zmq::monitor_t::on_event_unknown "
";

// File: classhelics_1_1MpiBroker.xml


%feature("docstring") helics::MpiBroker "
";

%feature("docstring") helics::MpiBroker::MpiBroker "

default constructor
";

%feature("docstring") helics::MpiBroker::MpiBroker "
";

%feature("docstring") helics::MpiBroker::InitializeFromArgs "
";

%feature("docstring") helics::MpiBroker::~MpiBroker "

destructor
";

%feature("docstring") helics::MpiBroker::getAddress "

get the connection address for the broker
";

// File: classhelics_1_1MpiComms.xml


%feature("docstring") helics::MpiComms "

implementation for the core that uses zmq messages to communicate

C++ includes: MpiComms.h
";

%feature("docstring") helics::MpiComms::MpiComms "

default constructor
";

%feature("docstring") helics::MpiComms::MpiComms "
";

%feature("docstring") helics::MpiComms::~MpiComms "

destructor
";

// File: classhelics_1_1MpiCore.xml


%feature("docstring") helics::MpiCore "

implementation for the core that uses zmq messages to communicate

C++ includes: MpiCore.h
";

%feature("docstring") helics::MpiCore::MpiCore "

default constructor
";

%feature("docstring") helics::MpiCore::MpiCore "
";

%feature("docstring") helics::MpiCore::~MpiCore "

destructor
";

%feature("docstring") helics::MpiCore::InitializeFromArgs "
";

%feature("docstring") helics::MpiCore::getAddress "

get a string representing the connection info to send data to this object
";

// File: classhelics_1_1NetworkBrokerData.xml


%feature("docstring") helics::NetworkBrokerData "

helper class designed to contain the common elements between networking brokers
and cores

C++ includes: NetworkBrokerData.hpp
";

%feature("docstring") helics::NetworkBrokerData::NetworkBrokerData "
";

%feature("docstring") helics::NetworkBrokerData::NetworkBrokerData "

constructor from the allowed type
";

%feature("docstring") helics::NetworkBrokerData::initializeFromArgs "
";

%feature("docstring") helics::NetworkBrokerData::setInterfaceType "
";

%feature("docstring") helics::NetworkBrokerData::displayHelp "
";

// File: classhelics_1_1NullFilterOperator.xml


%feature("docstring") helics::NullFilterOperator "

special filter operator defining no operation the original message is simply
returned

C++ includes: core-data.hpp
";

%feature("docstring") helics::NullFilterOperator::NullFilterOperator "

default constructor
";

%feature("docstring") helics::NullFilterOperator::process "

filter the message either modify the message or generate a new one
";

// File: classhelics_1_1ownedQueue.xml


%feature("docstring") helics::ownedQueue "

class implementing a queue owned by a particular object

C++ includes: IpcQueueHelper.h
";

%feature("docstring") helics::ownedQueue::ownedQueue "
";

%feature("docstring") helics::ownedQueue::~ownedQueue "
";

%feature("docstring") helics::ownedQueue::connect "
";

%feature("docstring") helics::ownedQueue::changeState "
";

%feature("docstring") helics::ownedQueue::getMessage "
";

%feature("docstring") helics::ownedQueue::getMessage "
";

%feature("docstring") helics::ownedQueue::getError "
";

// File: classhelics_1_1Player.xml


%feature("docstring") helics::Player "

class implementing a Player object, which is capable of reading a file and
generating interfaces and sending signals at the appropriate times

the Player class is not thread-safe, don't try to use it from multiple threads
without external protection, that will result in undefined behavior

C++ includes: player.h
";

%feature("docstring") helics::Player::Player "

default constructor
";

%feature("docstring") helics::Player::Player "

construct from command line arguments

Parameters
----------
* `argc` :
    the number of arguments
* `argv` :
    the strings in the input
";

%feature("docstring") helics::Player::Player "

construct from a federate info object

Parameters
----------
* `fi` :
    a pointer info object containing information on the desired federate
    configuration
";

%feature("docstring") helics::Player::Player "

constructor taking a federate information structure and using the given core

Parameters
----------
* `core` :
    a pointer to core object which the federate can join
* `fi` :
    a federate information structure
";

%feature("docstring") helics::Player::Player "

constructor taking a file with the required information

Parameters
----------
* `jsonString` :
    file or JSON string defining the federate information and other
    configuration
";

%feature("docstring") helics::Player::Player "

move construction
";

%feature("docstring") helics::Player::Player "

don't allow the copy constructor
";

%feature("docstring") helics::Player::~Player "
";

%feature("docstring") helics::Player::loadFile "

load a file containing publication information

Parameters
----------
* `filename` :
    the file containing the configuration and Player data accepted format are
    JSON, xml, and a Player format which is tab delimited or comma delimited
";

%feature("docstring") helics::Player::initialize "

initialize the Player federate

generate all the publications and organize the points, the final publication
count will be available after this time and the Player will enter the
initialization mode, which means it will not be possible to add more
publications calling run will automatically do this if necessary
";

%feature("docstring") helics::Player::run "
";

%feature("docstring") helics::Player::run "

run the Player until the specified time

Parameters
----------
* `stopTime_input` :
    the desired stop time
";

%feature("docstring") helics::Player::addPublication "

add a publication to a Player

Parameters
----------
* `key` :
    the key of the publication to add
* `type` :
    the type of the publication
* `units` :
    the units associated with the publication
";

%feature("docstring") helics::Player::addPublication "

add a publication to a Player

Parameters
----------
* `key` :
    the key of the publication to add
* `type` :
    the type of the publication
* `units` :
    the units associated with the publication
";

%feature("docstring") helics::Player::addEndpoint "

add an endpoint to the Player

Parameters
----------
* `endpointName` :
    the name of the endpoint
* `endpointType` :
    the named type of the endpoint
";

%feature("docstring") helics::Player::addPoint "

add a data point to publish through a Player

Parameters
----------
* `pubTime` :
    the time of the publication
* `key` :
    the key for the publication
* `val` :
    the value to publish
";

%feature("docstring") helics::Player::addMessage "

add a message to a Player queue

Parameters
----------
* `pubTime` :
    the time the message should be sent
* `src` :
    the source endpoint of the message
* `dest` :
    the destination endpoint of the message
* `payload` :
    the payload of the message
";

%feature("docstring") helics::Player::addMessage "

add an event for a specific time to a Player queue

Parameters
----------
* `sendTime` :
    the time the message should be sent
* `actionTime` :
    the eventTime listed for the message
* `src` :
    the source endpoint of the message
* `dest` :
    the destination endpoint of the message
* `payload` :
    the payload of the message
";

%feature("docstring") helics::Player::pointCount "

get the number of points loaded
";

%feature("docstring") helics::Player::messageCount "

get the number of messages loaded
";

%feature("docstring") helics::Player::publicationCount "

get the number of publications
";

%feature("docstring") helics::Player::endpointCount "

get the number of endpoints
";

%feature("docstring") helics::Player::finalize "

finalize the Player federate
";

%feature("docstring") helics::Player::isActive "

check if the Player is ready to run
";

// File: classhelics_1_1Publication.xml


%feature("docstring") helics::Publication "
";

%feature("docstring") helics::Publication::Publication "
";

%feature("docstring") helics::Publication::Publication "

constructor to build a publication object

Parameters
----------
* `valueFed` :
    the ValueFederate to use
* `type_` :
    the defined type of the publication
* `name` :
    the name of the subscription
* `units` :
    the units associated with a Federate
";

%feature("docstring") helics::Publication::Publication "

constructor to build a publication object

Parameters
----------
* `locality` :
    set to global for a global publication or local for a local one
* `valueFed` :
    the ValueFederate to use
* `type_` :
    the defined type of the publication
* `name` :
    the name of the subscription
* `units` :
    the units associated with a Federate
";

%feature("docstring") helics::Publication::Publication "

generate a publication object from a preexisting publication

Parameters
----------
* `valueFed` :
    a pointer to the appropriate value Federate
* `pubIndex` :
    the index of the subscription
";

%feature("docstring") helics::Publication::publish "

send a value for publication

Parameters
----------
* `val` :
    the value to publish
";

%feature("docstring") helics::Publication::publish "
";

%feature("docstring") helics::Publication::publish "
";

%feature("docstring") helics::Publication::publish "
";

%feature("docstring") helics::Publication::publish "
";

%feature("docstring") helics::Publication::publish "
";

%feature("docstring") helics::Publication::publish "
";

%feature("docstring") helics::Publication::publish "
";

%feature("docstring") helics::Publication::publish "
";

%feature("docstring") helics::Publication::publish "

secondary publish function to allow unit conversion before publication

Parameters
----------
* `val` :
    the value to publish
* `units` :
    the units association with the publication
";

%feature("docstring") helics::Publication::setMinimumChange "
";

%feature("docstring") helics::Publication::enableChangeDetection "
";

// File: structhelics_1_1publication__info.xml


%feature("docstring") helics::publication_info "

structure used to contain information about a publication

C++ includes: ValueFederateManager.hpp
";

%feature("docstring") helics::publication_info::publication_info "
";

// File: classhelics_1_1PublicationBase.xml


%feature("docstring") helics::PublicationBase "
";

%feature("docstring") helics::PublicationBase::PublicationBase "
";

%feature("docstring") helics::PublicationBase::PublicationBase "
";

%feature("docstring") helics::PublicationBase::PublicationBase "
";

%feature("docstring") helics::PublicationBase::PublicationBase "

generate a publication object from an existing publication in a federate  useful
for creating publication objects from publications generated by a configuration
script
";

%feature("docstring") helics::PublicationBase::~PublicationBase "

default destructor
";

%feature("docstring") helics::PublicationBase::getID "
";

%feature("docstring") helics::PublicationBase::getKey "

get the key for the subscription
";

%feature("docstring") helics::PublicationBase::getName "

get the key for the subscription
";

%feature("docstring") helics::PublicationBase::getType "

get the key for the subscription
";

%feature("docstring") helics::PublicationBase::getUnits "
";

// File: classhelics_1_1PublicationInfo.xml


%feature("docstring") helics::PublicationInfo "

data class containing the information about a publication

C++ includes: PublicationInfo.hpp
";

%feature("docstring") helics::PublicationInfo::PublicationInfo "

constructor from the basic information
";

%feature("docstring") helics::PublicationInfo::CheckSetValue "

check the value if it is the same as the most recent data and if changed store
it
";

// File: classhelics_1_1PublicationObject.xml


%feature("docstring") helics::PublicationObject "

object wrapping a publication

C++ includes: api_objects.h
";

// File: classhelics_1_1PublicationOnChange.xml


%feature("docstring") helics::PublicationOnChange "

class to handle a publication but the value is only published in the change is
greater than a certain level

C++ includes: Publications.hpp
";

%feature("docstring") helics::PublicationOnChange::PublicationOnChange "
";

%feature("docstring") helics::PublicationOnChange::PublicationOnChange "

constructor to build a publishOnChange object

Parameters
----------
* `valueFed` :
    the ValueFederate to use
* `name` :
    the name of the subscription
* `minChange` :
    the minimum change required to actually publish the value
* `units` :
    the units associated with a Federate
";

%feature("docstring") helics::PublicationOnChange::publish "

send a value for publication

the value is only published if it exceeds the specified level

Parameters
----------
* `val` :
    the value to publish
";

// File: classhelics_1_1PublicationT.xml


%feature("docstring") helics::PublicationT "

class to handle a publication

C++ includes: Publications.hpp
";

%feature("docstring") helics::PublicationT::PublicationT "
";

%feature("docstring") helics::PublicationT::PublicationT "

constructor to build a publication object

Parameters
----------
* `valueFed` :
    the ValueFederate to use
* `name` :
    the name of the subscription
* `units` :
    the units associated with a Federate
";

%feature("docstring") helics::PublicationT::PublicationT "

constructor to build a publication object

Parameters
----------
* `valueFed` :
    the ValueFederate to use
* `name` :
    the name of the subscription
* `units` :
    the units associated with a Federate
";

%feature("docstring") helics::PublicationT::publish "

send a value for publication

Parameters
----------
* `val` :
    the value to publish
";

%feature("docstring") helics::PublicationT::publish "

secondary publish function to allow unit conversion before publication

Parameters
----------
* `val` :
    the value to publish
* `units` :
    the units association with the publication
";

// File: classhelics_1_1queryObject.xml


%feature("docstring") helics::queryObject "

object representing a query

C++ includes: api_objects.h
";

// File: classhelics_1_1RandomDelayFilterOperation.xml


%feature("docstring") helics::RandomDelayFilterOperation "

filter for generating a random delay time for a message

C++ includes: FilterOperations.hpp
";

%feature("docstring") helics::RandomDelayFilterOperation::RandomDelayFilterOperation "
";

%feature("docstring") helics::RandomDelayFilterOperation::~RandomDelayFilterOperation "
";

%feature("docstring") helics::RandomDelayFilterOperation::set "

set a property on a filter

Parameters
----------
* `property` :
    the name of the property of the filter to change
* `val` :
    the numerical value of the property
";

%feature("docstring") helics::RandomDelayFilterOperation::setString "

set a string property on a filter

Parameters
----------
* `property` :
    the name of the property of the filter to change
* `val` :
    the numerical value of the property
";

%feature("docstring") helics::RandomDelayFilterOperation::getOperator "
";

// File: classhelics_1_1randomDelayGenerator.xml


%feature("docstring") helics::randomDelayGenerator "

class wrapping the distribution generation functions and parameters
";

%feature("docstring") helics::randomDelayGenerator::generate "
";

// File: classhelics_1_1RandomDropFilterOperation.xml


%feature("docstring") helics::RandomDropFilterOperation "

filter for randomly dropping a packet

C++ includes: FilterOperations.hpp
";

%feature("docstring") helics::RandomDropFilterOperation::RandomDropFilterOperation "
";

%feature("docstring") helics::RandomDropFilterOperation::~RandomDropFilterOperation "
";

%feature("docstring") helics::RandomDropFilterOperation::set "

set a property on a filter

Parameters
----------
* `property` :
    the name of the property of the filter to change
* `val` :
    the numerical value of the property
";

%feature("docstring") helics::RandomDropFilterOperation::setString "

set a string property on a filter

Parameters
----------
* `property` :
    the name of the property of the filter to change
* `val` :
    the numerical value of the property
";

%feature("docstring") helics::RandomDropFilterOperation::getOperator "
";

// File: classhelics_1_1Recorder.xml


%feature("docstring") helics::Recorder "

class designed to capture data points from a set of subscriptions or endpoints

C++ includes: recorder.h
";

%feature("docstring") helics::Recorder::Recorder "

construct from a FederateInfo structure
";

%feature("docstring") helics::Recorder::Recorder "

construct from command line arguments
";

%feature("docstring") helics::Recorder::Recorder "

constructor taking a federate information structure and using the given core

Parameters
----------
* `core` :
    a pointer to core object which the federate can join
* `fi` :
    a federate information structure
";

%feature("docstring") helics::Recorder::Recorder "

constructor taking a file with the required information

Parameters
----------
* `file` :
    a file defining the federate information
";

%feature("docstring") helics::Recorder::Recorder "

move construction
";

%feature("docstring") helics::Recorder::Recorder "

don't allow the copy constructor
";

%feature("docstring") helics::Recorder::~Recorder "

destructor
";

%feature("docstring") helics::Recorder::loadFile "

load a file containing subscription information

Parameters
----------
* `filename` :
    the name of the file to load (.txt, .json, or .xml

Returns
-------
0 on success <0 on failure
";

%feature("docstring") helics::Recorder::run "
";

%feature("docstring") helics::Recorder::run "

run the Player until the specified time
";

%feature("docstring") helics::Recorder::addSubscription "

add a subscription to capture

add a subscription to record
";

%feature("docstring") helics::Recorder::addEndpoint "

add an endpoint
";

%feature("docstring") helics::Recorder::addSourceEndpointClone "

copy all messages that come from a specified endpoint
";

%feature("docstring") helics::Recorder::addDestEndpointClone "

copy all messages that are going to a specific endpoint
";

%feature("docstring") helics::Recorder::addCapture "

add a capture interface

Parameters
----------
* `captureDesc` :
    describes a federate to capture all the interfaces for
";

%feature("docstring") helics::Recorder::saveFile "

save the data to a file
";

%feature("docstring") helics::Recorder::pointCount "

get the number of captured points
";

%feature("docstring") helics::Recorder::messageCount "

get the number of captured messages
";

%feature("docstring") helics::Recorder::getValue "

get a string with the value of point index

Parameters
----------
* `index` :
    the number of the point to retrieve

Returns
-------
a pair with the tag as the first element and the value as the second
";

%feature("docstring") helics::Recorder::getMessage "

get a message

makes a copy of a message and returns it in a unique_ptr

Parameters
----------
* `index` :
    the number of the message to retrieve
";

%feature("docstring") helics::Recorder::finalize "

finalize the federate
";

%feature("docstring") helics::Recorder::isActive "

check if the Recorder is ready to run
";

// File: classhelics_1_1RegistrationFailure.xml


%feature("docstring") helics::RegistrationFailure "

exception indicating that the registration of an object has failed

C++ includes: core-exceptions.hpp
";

%feature("docstring") helics::RegistrationFailure::RegistrationFailure "
";

// File: classhelics_1_1RerouteFilterOperation.xml


%feature("docstring") helics::RerouteFilterOperation "

filter for rerouting a packet to a particular endpoint

C++ includes: FilterOperations.hpp
";

%feature("docstring") helics::RerouteFilterOperation::RerouteFilterOperation "
";

%feature("docstring") helics::RerouteFilterOperation::~RerouteFilterOperation "
";

%feature("docstring") helics::RerouteFilterOperation::set "

set a property on a filter

Parameters
----------
* `property` :
    the name of the property of the filter to change
* `val` :
    the numerical value of the property
";

%feature("docstring") helics::RerouteFilterOperation::setString "

set a string property on a filter

Parameters
----------
* `property` :
    the name of the property of the filter to change
* `val` :
    the numerical value of the property
";

%feature("docstring") helics::RerouteFilterOperation::getOperator "
";

// File: classSearchableObjectHolder.xml


%feature("docstring") SearchableObjectHolder "

helper class to destroy objects at a late time when it is convenient and there
are no more possibilities of threading issues

C++ includes: searchableObjectHolder.hpp
";

%feature("docstring") SearchableObjectHolder::SearchableObjectHolder "
";

%feature("docstring") SearchableObjectHolder::SearchableObjectHolder "
";

%feature("docstring") SearchableObjectHolder::~SearchableObjectHolder "
";

%feature("docstring") SearchableObjectHolder::addObject "
";

%feature("docstring") SearchableObjectHolder::removeObject "
";

%feature("docstring") SearchableObjectHolder::removeObject "
";

%feature("docstring") SearchableObjectHolder::copyObject "
";

%feature("docstring") SearchableObjectHolder::findObject "
";

%feature("docstring") SearchableObjectHolder::findObject "
";

// File: classhelics_1_1sendToQueue.xml


%feature("docstring") helics::sendToQueue "

class implementing interactions with a queue to transmit data

C++ includes: IpcQueueHelper.h
";

%feature("docstring") helics::sendToQueue::sendToQueue "
";

%feature("docstring") helics::sendToQueue::connect "
";

%feature("docstring") helics::sendToQueue::sendMessage "
";

%feature("docstring") helics::sendToQueue::getError "
";

// File: classhelics_1_1shared__queue__state.xml


%feature("docstring") helics::shared_queue_state "

class defining a shared queue state meaning interaction with a queue the object
is not the owner of

C++ includes: IpcQueueHelper.h
";

%feature("docstring") helics::shared_queue_state::getState "
";

%feature("docstring") helics::shared_queue_state::setState "
";

// File: classSimpleQueue.xml


%feature("docstring") SimpleQueue "

class for very simple thread safe queue

uses two vectors for the operations, once the pull vector is empty it swaps the
vectors and reverses it so it can pop from the back

templateparam
-------------
* `X` :
    the base class of the queue

C++ includes: simpleQueue.hpp
";

%feature("docstring") SimpleQueue::SimpleQueue "

default constructor
";

%feature("docstring") SimpleQueue::SimpleQueue "

constructor with a reservation size

Parameters
----------
* `capacity` :
    the initial storage capacity of the queue
";

%feature("docstring") SimpleQueue::SimpleQueue "

enable the move constructor not the copy constructor
";

%feature("docstring") SimpleQueue::SimpleQueue "

DISABLE_COPY_AND_ASSIGN
";

%feature("docstring") SimpleQueue::~SimpleQueue "
";

%feature("docstring") SimpleQueue::empty "

check whether there are any elements in the queue because this is meant for
multi threaded applications this may or may not have any meaning depending on
the number of consumers
";

%feature("docstring") SimpleQueue::size "

get the current size of the queue
";

%feature("docstring") SimpleQueue::clear "

clear the queue
";

%feature("docstring") SimpleQueue::reserve "

set the capacity of the queue actually double the requested the size will be
reserved due to the use of two vectors internally

Parameters
----------
* `capacity` :
    the capacity to reserve
";

%feature("docstring") SimpleQueue::push "

push an element onto the queue val the value to push on the queue
";

%feature("docstring") SimpleQueue::emplace "

push an element onto the queue val the value to push on the queue
";

%feature("docstring") SimpleQueue::pop "

extract the first element from the queue

Returns
-------
an empty optional if there is no element otherwise the optional will contain a
value
";

%feature("docstring") SimpleQueue::peek "

try to peek at an object without popping it from the stack

only available for copy assignable objects

Returns
-------
an optional object with an object of type T if available
";

// File: classzmq_1_1socket__t.xml


%feature("docstring") zmq::socket_t "
";

%feature("docstring") zmq::socket_t::socket_t "
";

%feature("docstring") zmq::socket_t::~socket_t "
";

%feature("docstring") zmq::socket_t::close "
";

%feature("docstring") zmq::socket_t::setsockopt "
";

%feature("docstring") zmq::socket_t::setsockopt "
";

%feature("docstring") zmq::socket_t::setsockopt "
";

%feature("docstring") zmq::socket_t::getsockopt "
";

%feature("docstring") zmq::socket_t::getsockopt "
";

%feature("docstring") zmq::socket_t::getsockopt "
";

%feature("docstring") zmq::socket_t::bind "
";

%feature("docstring") zmq::socket_t::bind "
";

%feature("docstring") zmq::socket_t::unbind "
";

%feature("docstring") zmq::socket_t::unbind "
";

%feature("docstring") zmq::socket_t::connect "
";

%feature("docstring") zmq::socket_t::connect "
";

%feature("docstring") zmq::socket_t::disconnect "
";

%feature("docstring") zmq::socket_t::disconnect "
";

%feature("docstring") zmq::socket_t::connected "
";

%feature("docstring") zmq::socket_t::send "
";

%feature("docstring") zmq::socket_t::send "
";

%feature("docstring") zmq::socket_t::send "
";

%feature("docstring") zmq::socket_t::send "
";

%feature("docstring") zmq::socket_t::recv "
";

%feature("docstring") zmq::socket_t::recv "
";

// File: classhelics_1_1Source.xml


%feature("docstring") helics::Source "

class implementing a source federate, which is capable of generating signals of
various kinds and sending signals at the appropriate times

the source class is not threadsafe, don't try to use it from multiple threads
without external protection, that will result in undefined behavior

C++ includes: source.h
";

%feature("docstring") helics::Source::Source "

default constructor
";

%feature("docstring") helics::Source::Source "

construct from command line arguments

Parameters
----------
* `argc` :
    the number of arguments
* `argv` :
    the strings in the input
";

%feature("docstring") helics::Source::Source "

construct from a federate info object

Parameters
----------
* `fi` :
    a pointer info object containing information on the desired federate
    configuration
";

%feature("docstring") helics::Source::Source "

constructor taking a federate information structure and using the given core

Parameters
----------
* `core` :
    a pointer to core object which the federate can join
* `fi` :
    a federate information structure
";

%feature("docstring") helics::Source::Source "

constructor taking a file with the required information

Parameters
----------
* `jsonString` :
    file or json string defining the federate information and other
    configuration
";

%feature("docstring") helics::Source::Source "

move construction
";

%feature("docstring") helics::Source::Source "

don't allow the copy constructor
";

%feature("docstring") helics::Source::~Source "
";

%feature("docstring") helics::Source::loadFile "

load a file containing publication information

Parameters
----------
* `filename` :
    the file containing the configuration and source data accepted format are
    json, xml, and a source format which is tab delimited or comma delimited
";

%feature("docstring") helics::Source::initialize "

initialize the source federate

generate all the publications and organize the points, the final publication
count will be available after this time and the source will enter the
initialization mode, which means it will not be possible to add more
publications calling run will automatically do this if necessary
";

%feature("docstring") helics::Source::run "
";

%feature("docstring") helics::Source::run "

run the source until the specified time

Parameters
----------
* `stopTime_input` :
    the desired stop time
";

%feature("docstring") helics::Source::addSource "

add a publication to a source

Parameters
----------
* `key` :
    the key of the publication to add
* `type` :
    the type of the publication
* `units` :
    the units associated with the publication
";

// File: classhelics_1_1SourceFilter.xml


%feature("docstring") helics::SourceFilter "

class wrapping a source filter

C++ includes: Filters.hpp
";

%feature("docstring") helics::SourceFilter::SourceFilter "

constructor to build an source filter object

Parameters
----------
* `fed` :
    the Federate to use
* `target` :
    the endpoint the filter is targeting
* `name` :
    the name of the filter
* `input_type` :
    the type of data the filter is expecting
* `output_type` :
    the type of data the filter is generating
";

%feature("docstring") helics::SourceFilter::SourceFilter "

constructor to build an source filter object

Parameters
----------
* `fed` :
    the Federate to use
* `target` :
    the endpoint the filter is targeting
* `name` :
    the name of the filter
* `input_type` :
    the type of data the filter is expecting
* `output_type` :
    the type of data the filter is generating
";

%feature("docstring") helics::SourceFilter::~SourceFilter "
";

// File: classhelics_1_1SourceObject.xml


%feature("docstring") helics::SourceObject "
";

// File: classStringToCmdLine.xml


%feature("docstring") StringToCmdLine "

class used to convert a string into command line arguments

C++ includes: stringToCmdLine.h
";

%feature("docstring") StringToCmdLine::StringToCmdLine "

construct from a string
";

%feature("docstring") StringToCmdLine::load "

load a string

Parameters
----------
* `cmdString` :
    a single string containing command line arguments
";

%feature("docstring") StringToCmdLine::getArgCount "

get the number of separate arguments corresponding to argc
";

%feature("docstring") StringToCmdLine::getArgV "

get the argument values corresponding to char *argv[]
";

// File: classhelics_1_1Subscription.xml


%feature("docstring") helics::Subscription "

primary subscription object class

can convert between the helics primary base class types

C++ includes: Subscriptions.hpp
";

%feature("docstring") helics::Subscription::Subscription "
";

%feature("docstring") helics::Subscription::Subscription "
";

%feature("docstring") helics::Subscription::Subscription "
";

%feature("docstring") helics::Subscription::Subscription "
";

%feature("docstring") helics::Subscription::Subscription "
";

%feature("docstring") helics::Subscription::Subscription "

generate a subscription object from a preexisting subscription

Parameters
----------
* `valueFed` :
    a pointer to the appropriate value Federate
* `subIndex` :
    the index of the subscription
";

%feature("docstring") helics::Subscription::isUpdated "

check if the value has been updated
";

%feature("docstring") helics::Subscription::getValue "

get the latest value for the subscription

Parameters
----------
* `out` :
    the location to store the value
";

%feature("docstring") helics::Subscription::getValue "

get the most recent value

Returns
-------
the value
";

%feature("docstring") helics::Subscription::getValueAs "

get the most recent value

Returns
-------
the value
";

%feature("docstring") helics::Subscription::getValueAs "

get the most recent calculation with the result as a convertible type
";

%feature("docstring") helics::Subscription::registerCallback "

register a callback for the update

the callback is called in the just before the time request function returns

Parameters
----------
* `callback` :
    a function with signature void(X val, Time time) val is the new value and
    time is the time the value was updated
";

%feature("docstring") helics::Subscription::registerCallback "

register a callback for an update notification

the callback is called in the just before the time request function returns

Parameters
----------
* `callback` :
    a function with signature void( Time time) time is the time the value was
    updated This callback is a notification callback and doesn't return the
    value
";

%feature("docstring") helics::Subscription::setDefault "

set the default value to use before any update has been published
";

%feature("docstring") helics::Subscription::setMinimumChange "

set the minimum delta for change detection

Parameters
----------
* `detltaV` :
    a double with the change in a value in order to register a different value
";

%feature("docstring") helics::Subscription::enableChangeDetection "

enable change detection

Parameters
----------
* `enabled` :
    (optional) set to false to disable change detection true(default) to enable
    it
";

// File: structhelics_1_1subscription__info.xml


%feature("docstring") helics::subscription_info "

structure used to contain information about a subscription

C++ includes: ValueFederateManager.hpp
";

%feature("docstring") helics::subscription_info::subscription_info "
";

// File: classhelics_1_1SubscriptionBase.xml


%feature("docstring") helics::SubscriptionBase "

base class for a subscription object

C++ includes: Subscriptions.hpp
";

%feature("docstring") helics::SubscriptionBase::SubscriptionBase "
";

%feature("docstring") helics::SubscriptionBase::SubscriptionBase "
";

%feature("docstring") helics::SubscriptionBase::SubscriptionBase "
";

%feature("docstring") helics::SubscriptionBase::SubscriptionBase "
";

%feature("docstring") helics::SubscriptionBase::SubscriptionBase "
";

%feature("docstring") helics::SubscriptionBase::~SubscriptionBase "
";

%feature("docstring") helics::SubscriptionBase::getLastUpdate "

get the time of the last update

Returns
-------
the time of the last update
";

%feature("docstring") helics::SubscriptionBase::isUpdated "

check if the value has subscription has been updated
";

%feature("docstring") helics::SubscriptionBase::getID "
";

%feature("docstring") helics::SubscriptionBase::registerCallback "

register a callback for an update notification

the callback is called in the just before the time request function returns

Parameters
----------
* `callback` :
    a function with signature void( Time time) time is the time the value was
    updated This callback is a notification callback and doesn't return the
    value
";

%feature("docstring") helics::SubscriptionBase::getKey "

get the key for the subscription
";

%feature("docstring") helics::SubscriptionBase::getName "

get the key for the subscription
";

%feature("docstring") helics::SubscriptionBase::getType "

get the key for the subscription
";

%feature("docstring") helics::SubscriptionBase::getUnits "

get the units associated with a subscription
";

// File: classhelics_1_1SubscriptionInfo.xml


%feature("docstring") helics::SubscriptionInfo "

data class for managing information about a subscription

C++ includes: SubscriptionInfo.hpp
";

%feature("docstring") helics::SubscriptionInfo::SubscriptionInfo "

constructor with all the information
";

%feature("docstring") helics::SubscriptionInfo::getData "

get the current data
";

%feature("docstring") helics::SubscriptionInfo::addData "

add a data block into the queue
";

%feature("docstring") helics::SubscriptionInfo::updateTime "

update current status to new time

Parameters
----------
* `newTime` :
    the time to move the subscription to

Returns
-------
true if the value has changed
";

%feature("docstring") helics::SubscriptionInfo::nextValueTime "
";

// File: classhelics_1_1SubscriptionObject.xml


%feature("docstring") helics::SubscriptionObject "

object wrapping a subscription

C++ includes: api_objects.h
";

// File: classhelics_1_1SubscriptionT.xml


%feature("docstring") helics::SubscriptionT "

class to handle a subscription

templateparam
-------------
* `X` :
    the class of the value associated with a subscription

C++ includes: Subscriptions.hpp
";

%feature("docstring") helics::SubscriptionT::SubscriptionT "
";

%feature("docstring") helics::SubscriptionT::SubscriptionT "

constructor to build a subscription object

Parameters
----------
* `valueFed` :
    the ValueFederate to use
* `name` :
    the name of the subscription
* `units` :
    the units associated with a Federate
";

%feature("docstring") helics::SubscriptionT::SubscriptionT "

constructor to build a subscription object

Parameters
----------
* `required` :
    a flag indicating that the subscription is required to have a matching
    publication
* `valueFed` :
    the ValueFederate to use
* `name` :
    the name of the subscription
* `units` :
    the units associated with a Federate
";

%feature("docstring") helics::SubscriptionT::getValue "

get the most recent value

Returns
-------
the value
";

%feature("docstring") helics::SubscriptionT::getValue "

store the value in the given variable

Parameters
----------
* `out` :
    the location to store the value
";

%feature("docstring") helics::SubscriptionT::registerCallback "

register a callback for the update

the callback is called in the just before the time request function returns

Parameters
----------
* `callback` :
    a function with signature void(X val, Time time) val is the new value and
    time is the time the value was updated
";

%feature("docstring") helics::SubscriptionT::registerCallback "

register a callback for an update notification

the callback is called in the just before the time request function returns

Parameters
----------
* `callback` :
    a function with signature void( Time time) time is the time the value was
    updated This callback is a notification callback and doesn't return the
    value
";

%feature("docstring") helics::SubscriptionT::setDefault "

set a default value

Parameters
----------
* `val` :
    the value to set as the default
";

%feature("docstring") helics::SubscriptionT::setMinimumChange "

set a minimum change value
";

%feature("docstring") helics::SubscriptionT::enableChangeDetection "
";

// File: classtcp__connection.xml


%feature("docstring") tcp_connection "

tcp socket connection for connecting to a server

C++ includes: TcpHelperClasses.h
";

%feature("docstring") tcp_connection::create "
";

%feature("docstring") tcp_connection::socket "
";

%feature("docstring") tcp_connection::cancel "
";

%feature("docstring") tcp_connection::close "

close the socket connection

Parameters
----------
*  :
";

%feature("docstring") tcp_connection::send "

send raw data

Exceptions
----------
* `boost::system::system_error` :
    on failure
";

%feature("docstring") tcp_connection::send "

send a string

Exceptions
----------
* `boost::system::system_error` :
    on failure
";

%feature("docstring") tcp_connection::receive "

do a blocking receive on the socket

Exceptions
----------
* `boost::system::system_error` :
    on failure

Returns
-------
the number of bytes received
";

%feature("docstring") tcp_connection::send_async "

perform an asynchronous send operation

Parameters
----------
* `buffer` :
    the data to send
* `dataLength` :
    the length of the data
* `callback` :
    a callback function of the form void handler( const
    boost::system::error_code& error, // Result of operation. std::size_t
    bytes_transferred // Number of bytes received. );
";

%feature("docstring") tcp_connection::async_receive "

perform an asynchronous receive operation

Parameters
----------
* `buffer` :
    the data to send
* `dataLength` :
    the length of the data
* `callback` :
    a callback function of the form void handler( const
    boost::system::error_code& error, // Result of operation. std::size_t
    bytes_transferred // Number of bytes received. );
";

%feature("docstring") tcp_connection::async_receive "

perform an asynchronous receive operation

Parameters
----------
* `buffer` :
    the data to send
* `dataLength` :
    the length of the data
* `callback` :
    a callback function of the form void handler( const
    boost::system::error_code& error, // Result of operation. std::size_t
    bytes_transferred // Number of bytes received. );
";

%feature("docstring") tcp_connection::isConnected "

check if the socket has finished the connection process
";

%feature("docstring") tcp_connection::waitUntilConnected "

wait until the socket has finished the connection process

Parameters
----------
* `timeOut` :
    the number of ms to wait for the connection process to finish (-1) for no
    limit

Returns
-------
0 if connected, -1 if the timeout was reached, -2 if error
";

// File: classtcp__rx__connection.xml


%feature("docstring") tcp_rx_connection "

tcp socket generation for a receiving server

C++ includes: TcpHelperClasses.h
";

%feature("docstring") tcp_rx_connection::create "
";

%feature("docstring") tcp_rx_connection::socket "
";

%feature("docstring") tcp_rx_connection::start "
";

%feature("docstring") tcp_rx_connection::stop "
";

%feature("docstring") tcp_rx_connection::close "
";

%feature("docstring") tcp_rx_connection::setDataCall "
";

%feature("docstring") tcp_rx_connection::setErrorCall "
";

%feature("docstring") tcp_rx_connection::send "

send raw data

Exceptions
----------
* `boost::system::system_error` :
    on failure
";

%feature("docstring") tcp_rx_connection::send "

send a string

Exceptions
----------
* `boost::system::system_error` :
    on failure
";

// File: classtcp__server.xml


%feature("docstring") tcp_server "

helper class for a server

C++ includes: TcpHelperClasses.h
";

%feature("docstring") tcp_server::tcp_server "
";

%feature("docstring") tcp_server::stop "
";

%feature("docstring") tcp_server::start "
";

%feature("docstring") tcp_server::close "
";

%feature("docstring") tcp_server::setDataCall "
";

%feature("docstring") tcp_server::setErrorCall "
";

// File: classhelics_1_1TcpBroker.xml


%feature("docstring") helics::TcpBroker "
";

%feature("docstring") helics::TcpBroker::TcpBroker "

default constructor
";

%feature("docstring") helics::TcpBroker::TcpBroker "
";

%feature("docstring") helics::TcpBroker::initializeFromArgs "

initialize from command line arguments
";

%feature("docstring") helics::TcpBroker::~TcpBroker "

destructor
";

%feature("docstring") helics::TcpBroker::getAddress "

get the connection address for the broker
";

%feature("docstring") helics::TcpBroker::displayHelp "
";

// File: classhelics_1_1TcpComms.xml


%feature("docstring") helics::TcpComms "

implementation for the communication interface that uses TCP messages to
communicate

C++ includes: TcpComms.h
";

%feature("docstring") helics::TcpComms::TcpComms "

default constructor
";

%feature("docstring") helics::TcpComms::TcpComms "
";

%feature("docstring") helics::TcpComms::TcpComms "
";

%feature("docstring") helics::TcpComms::~TcpComms "

destructor
";

%feature("docstring") helics::TcpComms::setBrokerPort "

set the port numbers for the local ports
";

%feature("docstring") helics::TcpComms::setPortNumber "
";

%feature("docstring") helics::TcpComms::setAutomaticPortStartPort "
";

%feature("docstring") helics::TcpComms::getPort "

get the port number of the comms object to push message to
";

%feature("docstring") helics::TcpComms::getAddress "
";

// File: classhelics_1_1TcpCore.xml


%feature("docstring") helics::TcpCore "

implementation for the core that uses tcp messages to communicate

C++ includes: TcpCore.h
";

%feature("docstring") helics::TcpCore::TcpCore "

default constructor
";

%feature("docstring") helics::TcpCore::TcpCore "
";

%feature("docstring") helics::TcpCore::~TcpCore "
";

%feature("docstring") helics::TcpCore::initializeFromArgs "

initialize the core manager with command line arguments

Parameters
----------
* `argc` :
    the number of arguments
* `argv` :
    char pointers to the arguments
";

%feature("docstring") helics::TcpCore::getAddress "

get a string representing the connection info to send data to this object
";

// File: classhelics_1_1TestBroker.xml


%feature("docstring") helics::TestBroker "

class implementing a basic broker that links to other brokers in process memory

C++ includes: TestBroker.h
";

%feature("docstring") helics::TestBroker::TestBroker "

default constructor
";

%feature("docstring") helics::TestBroker::TestBroker "
";

%feature("docstring") helics::TestBroker::TestBroker "

construct with a pointer to a broker
";

%feature("docstring") helics::TestBroker::~TestBroker "
";

%feature("docstring") helics::TestBroker::initializeFromArgs "

initialize from command line arguments
";

%feature("docstring") helics::TestBroker::getAddress "

get the connection address for the broker
";

%feature("docstring") helics::TestBroker::displayHelp "

static method to display the help message
";

// File: classhelics_1_1TestCore.xml


%feature("docstring") helics::TestCore "

an object implementing a local core object that can communicate in process

C++ includes: TestCore.h
";

%feature("docstring") helics::TestCore::TestCore "

default constructor
";

%feature("docstring") helics::TestCore::TestCore "

construct from a core name
";

%feature("docstring") helics::TestCore::TestCore "

construct with a pointer to a broker
";

%feature("docstring") helics::TestCore::~TestCore "

destructor
";

%feature("docstring") helics::TestCore::initializeFromArgs "

initialize the core manager with command line arguments

Parameters
----------
* `argc` :
    the number of arguments
* `argv` :
    char pointers to the arguments
";

%feature("docstring") helics::TestCore::getAddress "

get a string representing the connection info to send data to this object
";

// File: classhelics_1_1TimeCoordinator.xml


%feature("docstring") helics::TimeCoordinator "
";

%feature("docstring") helics::TimeCoordinator::TimeCoordinator "
";

%feature("docstring") helics::TimeCoordinator::TimeCoordinator "
";

%feature("docstring") helics::TimeCoordinator::getFedInfo "
";

%feature("docstring") helics::TimeCoordinator::getFedInfo "
";

%feature("docstring") helics::TimeCoordinator::setInfo "
";

%feature("docstring") helics::TimeCoordinator::setMessageSender "
";

%feature("docstring") helics::TimeCoordinator::getGrantedTime "
";

%feature("docstring") helics::TimeCoordinator::getDependencies "
";

%feature("docstring") helics::TimeCoordinator::getDependents "

get a reference to the dependents vector
";

%feature("docstring") helics::TimeCoordinator::getCurrentIteration "

get the current iteration counter for an iterative call

this will work properly even when a federate is processing
";

%feature("docstring") helics::TimeCoordinator::updateTimeFactors "

compute updates to time values

Returns
-------
true if they have been modified
";

%feature("docstring") helics::TimeCoordinator::updateValueTime "

update the time_value variable with a new value if needed
";

%feature("docstring") helics::TimeCoordinator::updateMessageTime "

update the time_message variable with a new value if needed
";

%feature("docstring") helics::TimeCoordinator::getDependencyInfo "

take a global id and get a pointer to the dependencyInfo for the other fed will
be nullptr if it doesn't exist
";

%feature("docstring") helics::TimeCoordinator::isDependency "

check whether a federate is a dependency
";

%feature("docstring") helics::TimeCoordinator::processTimeMessage "

process a message related to time

Returns
-------
true if it did anything
";

%feature("docstring") helics::TimeCoordinator::processConfigUpdateMessage "

process a message related to configuration

Parameters
----------
* `cmd` :
    the update command
* `initMode` :
    set to true to allow init only updates
";

%feature("docstring") helics::TimeCoordinator::processDependencyUpdateMessage "

process a dependency update message
";

%feature("docstring") helics::TimeCoordinator::addDependency "

add a federate dependency

Returns
-------
true if it was actually added, false if the federate was already present
";

%feature("docstring") helics::TimeCoordinator::addDependent "

add a dependent federate

Returns
-------
true if it was actually added, false if the federate was already present
";

%feature("docstring") helics::TimeCoordinator::removeDependency "
";

%feature("docstring") helics::TimeCoordinator::removeDependent "
";

%feature("docstring") helics::TimeCoordinator::checkExecEntry "

check if entry to the executing state can be granted
";

%feature("docstring") helics::TimeCoordinator::timeRequest "
";

%feature("docstring") helics::TimeCoordinator::enteringExecMode "
";

%feature("docstring") helics::TimeCoordinator::checkTimeGrant "

check if it is valid to grant a time
";

%feature("docstring") helics::TimeCoordinator::printTimeStatus "

generate a string with the current time status
";

// File: classhelics_1_1TimeDependencies.xml


%feature("docstring") helics::TimeDependencies "

class for managing a set of dependencies

C++ includes: TimeDependencies.hpp
";

%feature("docstring") helics::TimeDependencies::TimeDependencies "

default constructor
";

%feature("docstring") helics::TimeDependencies::isDependency "

return true if the given federate is already a member
";

%feature("docstring") helics::TimeDependencies::addDependency "

insert a dependency into the structure

Returns
-------
true if the dependency was added, false if it existed already
";

%feature("docstring") helics::TimeDependencies::removeDependency "

remove dependency from consideration
";

%feature("docstring") helics::TimeDependencies::updateTime "

update the info about a dependency based on a message
";

%feature("docstring") helics::TimeDependencies::begin "

iterator to first dependency
";

%feature("docstring") helics::TimeDependencies::begin "

const iterator to first dependency
";

%feature("docstring") helics::TimeDependencies::end "

iterator to end point
";

%feature("docstring") helics::TimeDependencies::end "

const iterator to end point
";

%feature("docstring") helics::TimeDependencies::cbegin "

const iterator to first dependency
";

%feature("docstring") helics::TimeDependencies::cend "

const iterator to first dependency
";

%feature("docstring") helics::TimeDependencies::getDependencyInfo "
";

%feature("docstring") helics::TimeDependencies::checkIfReadyForExecEntry "
";

%feature("docstring") helics::TimeDependencies::checkIfReadyForTimeGrant "
";

%feature("docstring") helics::TimeDependencies::resetIteratingExecRequests "
";

%feature("docstring") helics::TimeDependencies::resetIteratingTimeRequests "
";

// File: classTimeRepresentation.xml


%feature("docstring") TimeRepresentation "

prototype class for representing time

time representation class that has as a template argument a class that can
define time as a number and has some required features

C++ includes: timeRepresentation.hpp
";

%feature("docstring") TimeRepresentation::TimeRepresentation "

default constructor
";

%feature("docstring") TimeRepresentation::TimeRepresentation "

normal time constructor from a double representation of seconds
";

%feature("docstring") TimeRepresentation::TimeRepresentation "
";

%feature("docstring") TimeRepresentation::TimeRepresentation "
";

%feature("docstring") TimeRepresentation::seconds "

generate the time in seconds
";

%feature("docstring") TimeRepresentation::toCount "
";

%feature("docstring") TimeRepresentation::getBaseTimeCode "

get the underlying time code value
";

%feature("docstring") TimeRepresentation::setBaseTimeCode "

set the underlying base representation of a time directly

this is not recommended for normal use
";

%feature("docstring") TimeRepresentation::maxVal "

generate a TimeRepresentation of the maximum representative value
";

%feature("docstring") TimeRepresentation::minVal "

generate a TimeRepresentation of the minimum representative value
";

%feature("docstring") TimeRepresentation::zeroVal "

generate a TimeRepresentation of 0
";

%feature("docstring") TimeRepresentation::epsilon "

generate a TimeRepresentation of 0
";

// File: classhelics_1_1UdpBroker.xml


%feature("docstring") helics::UdpBroker "
";

%feature("docstring") helics::UdpBroker::UdpBroker "

default constructor
";

%feature("docstring") helics::UdpBroker::UdpBroker "
";

%feature("docstring") helics::UdpBroker::initializeFromArgs "

initialize from command line arguments
";

%feature("docstring") helics::UdpBroker::~UdpBroker "

destructor
";

%feature("docstring") helics::UdpBroker::getAddress "

get the connection address for the broker
";

%feature("docstring") helics::UdpBroker::displayHelp "
";

// File: classhelics_1_1UdpComms.xml


%feature("docstring") helics::UdpComms "

implementation for the communication interface that uses ZMQ messages to
communicate

C++ includes: UdpComms.h
";

%feature("docstring") helics::UdpComms::UdpComms "

default constructor
";

%feature("docstring") helics::UdpComms::UdpComms "
";

%feature("docstring") helics::UdpComms::UdpComms "
";

%feature("docstring") helics::UdpComms::~UdpComms "

destructor
";

%feature("docstring") helics::UdpComms::setBrokerPort "

set the port numbers for the local ports
";

%feature("docstring") helics::UdpComms::setPortNumber "
";

%feature("docstring") helics::UdpComms::setAutomaticPortStartPort "
";

%feature("docstring") helics::UdpComms::getPort "

get the port number of the comms object to push message to
";

%feature("docstring") helics::UdpComms::getAddress "
";

// File: classhelics_1_1UdpCore.xml


%feature("docstring") helics::UdpCore "

implementation for the core that uses udp messages to communicate

C++ includes: UdpCore.h
";

%feature("docstring") helics::UdpCore::UdpCore "

default constructor
";

%feature("docstring") helics::UdpCore::UdpCore "
";

%feature("docstring") helics::UdpCore::~UdpCore "
";

%feature("docstring") helics::UdpCore::initializeFromArgs "

initialize the core manager with command line arguments

Parameters
----------
* `argc` :
    the number of arguments
* `argv` :
    char pointers to the arguments
";

%feature("docstring") helics::UdpCore::getAddress "

get a string representing the connection info to send data to this object
";

// File: classhelics_1_1ValueCapture.xml


%feature("docstring") helics::ValueCapture "

helper class for capturing data points

C++ includes: recorder.h
";

%feature("docstring") helics::ValueCapture::ValueCapture "
";

%feature("docstring") helics::ValueCapture::ValueCapture "
";

// File: classhelics_1_1ValueConverter.xml


%feature("docstring") helics::ValueConverter "

converter for a basic value

C++ includes: ValueConverter.hpp
";

%feature("docstring") helics::ValueConverter::convert "

convert the value to a block of data

converter for a basic value
";

%feature("docstring") helics::ValueConverter::convert "

convert the value and store to a specific block of data
";

%feature("docstring") helics::ValueConverter::convert "

convert a raw vector of objects and store to a specific block
";

%feature("docstring") helics::ValueConverter::convert "

convert a raw vector of objects and store to a specific block
";

%feature("docstring") helics::ValueConverter::interpret "

interpret a view of the data and convert back to a val
";

%feature("docstring") helics::ValueConverter::interpret "

interpret a view of the data block and store to the specified value
";

%feature("docstring") helics::ValueConverter::type "

get the type of the value
";

// File: classhelics_1_1ValueConverter_3_01std_1_1string_01_4.xml


%feature("docstring") helics::ValueConverter< std::string > "

converter for a single string value

C++ includes: ValueConverter.hpp
";

%feature("docstring") helics::ValueConverter< std::string >::convert "
";

%feature("docstring") helics::ValueConverter< std::string >::convert "
";

%feature("docstring") helics::ValueConverter< std::string >::convert "
";

%feature("docstring") helics::ValueConverter< std::string >::interpret "
";

%feature("docstring") helics::ValueConverter< std::string >::interpret "
";

%feature("docstring") helics::ValueConverter< std::string >::type "
";

// File: classhelics_1_1ValueFederate.xml


%feature("docstring") helics::ValueFederate "

class defining the value based interface

C++ includes: ValueFederate.hpp
";

%feature("docstring") helics::ValueFederate::helics::FederateInfo "
";

%feature("docstring") helics::ValueFederate::ValueFederate "

constructor taking a federate information structure and using the default core

Parameters
----------
* `fi` :
    a federate information structure

constructor taking a core engine and federate info structure
";

%feature("docstring") helics::ValueFederate::ValueFederate "

constructor taking a core and a federate information structure, sore information
in fi is ignored

Parameters
----------
* `core` :
    a shared ptr to a core to join
* `fi` :
    a federate information structure
";

%feature("docstring") helics::ValueFederate::ValueFederate "

constructor taking a string with the required information

Parameters
----------
* `jsonString` :
    can be either a json file or a string containing json code
";

%feature("docstring") helics::ValueFederate::ValueFederate "

default constructor
";

%feature("docstring") helics::ValueFederate::ValueFederate "
";

%feature("docstring") helics::ValueFederate::ValueFederate "
";

%feature("docstring") helics::ValueFederate::ValueFederate "
";

%feature("docstring") helics::ValueFederate::ValueFederate "
";

%feature("docstring") helics::ValueFederate::ValueFederate "
";

%feature("docstring") helics::ValueFederate::ValueFederate "
";

%feature("docstring") helics::ValueFederate::~ValueFederate "

destructor
";

%feature("docstring") helics::ValueFederate::~ValueFederate "
";

%feature("docstring") helics::ValueFederate::registerPublication "

register a publication

call is only valid in startup mode

Parameters
----------
* `key` :
    the name of the publication
* `type` :
    a string defining the type of the publication
* `units` :
    a string defining the units of the publication [optional]

Returns
-------
a publication id object for use as an identifier
";

%feature("docstring") helics::ValueFederate::registerPublication "

register a publication

call is only valid in startup mode by default prepends the name with the
federate name

Parameters
----------
* `key` :
    the name of the publication
* `units` :
    the optional units of the publication

Returns
-------
an identifier for use with this publication
";

%feature("docstring") helics::ValueFederate::registerPublication "

Methods to register publications
";

%feature("docstring") helics::ValueFederate::registerGlobalPublication "

register a publication

call is only valid in startup mode

Parameters
----------
* `key` :
    the name of the publication
* `type` :
    a string defining the type of the publication
* `units` :
    a string defining the units of the publication [optional]

Returns
-------
a publication id object for use as an identifier
";

%feature("docstring") helics::ValueFederate::registerGlobalPublication "

register a publication

call is only valid in startup mode by default prepends the name with the
federate name

Parameters
----------
* `key` :
    the name of the publication
* `units` :
    the optional units of the publication

Returns
-------
an identifier for use with this publication
";

%feature("docstring") helics::ValueFederate::registerGlobalPublication "
";

%feature("docstring") helics::ValueFederate::registerPublicationIndexed "

register a publication as part of an indexed structure

call is only valid in startup mode by default prepends the name with the
federate name the name is registered as a global structure with the index
appended

Parameters
----------
* `key` :
    the name of the publication
* `index1` :
    an index associated with the publication
* `units` :
    the optional units of the publication

Returns
-------
an identifier for use with this publication
";

%feature("docstring") helics::ValueFederate::registerPublicationIndexed "

register a publication as part of a 2 dimensional indexed structure

call is only valid in startup mode by default prepends the name with the
federate name the name is registered as a global structure with the indices
appended

Parameters
----------
* `key` :
    the name of the publication
* `index1` :
    an index associated with the publication
* `index2` :
    a second index
* `units` :
    the optional units of the publication

Returns
-------
an identifier for use with this publication
";

%feature("docstring") helics::ValueFederate::registerPublicationIndexed "
";

%feature("docstring") helics::ValueFederate::registerPublicationIndexed "
";

%feature("docstring") helics::ValueFederate::registerRequiredSubscription "

register a subscription

call is only valid in startup mode register a subscription with name type and
units

Parameters
----------
* `key` :
    the name of the publication to subscribe to
* `type` :
    a string describing the type of the publication
* `units` :
    a string describing the units on the publication
";

%feature("docstring") helics::ValueFederate::registerRequiredSubscription "

register a subscription

call is only valid in startup mode
";

%feature("docstring") helics::ValueFederate::registerRequiredSubscriptionIndexed "

register a required subscription

call is only valid in startup mode, register an optional subscription for a 1D
array of values

Parameters
----------
* `key` :
    the name of the subscription
* `index1` :
    the index into a 1 dimensional array of values
* `units` :
    the optional units on the subscription
";

%feature("docstring") helics::ValueFederate::registerRequiredSubscriptionIndexed "

register a required subscription for a 2-D array of values

call is only valid in startup mode

Parameters
----------
* `key` :
    the name of the subscription
* `index1` :
    the first index of a 2-D value structure
* `index2` :
    the 2nd index of a 2-D value structure
* `units` :
    the optional units on the subscription
";

%feature("docstring") helics::ValueFederate::registerOptionalSubscription "

register a subscription

call is only valid in startup mode

Parameters
----------
* `key` :
    the name of the publication to subscribe to
* `type` :
    the type of the subscription
* `units` :
    the units associated with the desired output
";

%feature("docstring") helics::ValueFederate::registerOptionalSubscription "

register a subscription

call is only valid in startup mode

Parameters
----------
* `key` :
    the name of the subscription
* `units` :
    the optional units on the subscription
";

%feature("docstring") helics::ValueFederate::registerOptionalSubscriptionIndexed "

register an optional subscription

call is only valid in startup mode, register an optional subscription for a 1D
array of values

Parameters
----------
* `key` :
    the name of the subscription
* `index1` :
    the index into a 1 dimensional array of values
* `units` :
    the optional units on the subscription
";

%feature("docstring") helics::ValueFederate::registerOptionalSubscriptionIndexed "

register an optional subscription for a 2-D array of values

call is only valid in startup mode

Parameters
----------
* `key` :
    the name of the subscription
* `index1` :
    the first index of a 2-D value structure
* `index2` :
    the 2nd index of a 2-D value structure
* `units` :
    the optional units on the subscription
";

%feature("docstring") helics::ValueFederate::addSubscriptionShortcut "

add a shortcut for locating a subscription

primarily for use in looking up an id from a different location creates a local
shortcut for referring to a subscription which may have a long actual name

Parameters
----------
* `the` :
    subscription identifier
* `shortcutName` :
    the name of the shortcut
";

%feature("docstring") helics::ValueFederate::setDefaultValue "

set the default value for a subscription

this is the value returned prior to any publications

Parameters
----------
* `id` :
    the subscription identifier
* `block` :
    the data view representing the default value

Exceptions
----------
* `std::invalid_argument` :
    if id is invalid
";

%feature("docstring") helics::ValueFederate::setDefaultValue "

set the default value for a subscription

this is the value returned prior to any publications

Parameters
----------
* `id` :
    the subscription identifier
* `block` :
    the data block representing the default value

Exceptions
----------
* `std::invalid_argument` :
    if id is invalid
";

%feature("docstring") helics::ValueFederate::setDefaultValue "

set a default value for a subscription

Parameters
----------
* `id` :
    the identifier for the subscription
* `val` :
    the default value

Exceptions
----------
* `std::invalid_argument` :
    if id is invalid
";

%feature("docstring") helics::ValueFederate::setDefaultValue "

Methods to set default values for subscriptions
";

%feature("docstring") helics::ValueFederate::setDefaultValue "
";

%feature("docstring") helics::ValueFederate::setDefaultValue "
";

%feature("docstring") helics::ValueFederate::setDefaultValue "
";

%feature("docstring") helics::ValueFederate::setDefaultValue "
";

%feature("docstring") helics::ValueFederate::setDefaultValue "
";

%feature("docstring") helics::ValueFederate::registerInterfaces "

register a set of interfaces defined in a file

call is only valid in startup mode

Parameters
----------
* `jsonString` :
    the location of the file to load to generate the interfaces
";

%feature("docstring") helics::ValueFederate::getValueRaw "

get a value as raw data block from the system

Parameters
----------
* `id` :
    the identifier for the subscription

Returns
-------
a constant data block

Exceptions
----------
* `std::invalid_argument` :
    if id is invalid
";

%feature("docstring") helics::ValueFederate::getValue "

get a value as raw data block from the system

Parameters
----------
* `id` :
    the identifier for the subscription
* `the` :
    value translated to a specific object

Returns
-------
a constant data block

Exceptions
----------
* `std::invalid_argument` :
    if id is invalid
";

%feature("docstring") helics::ValueFederate::getValue "

get a value as raw data block from the system

Parameters
----------
* `id` :
    the identifier for the subscription
* `the` :
    value translated to a specific object

Returns
-------
a constant data block

Exceptions
----------
* `std::invalid_argument` :
    if id is invalid
";

%feature("docstring") helics::ValueFederate::getValue "

Methods to get subscription values
";

%feature("docstring") helics::ValueFederate::publish "

publish a value

Parameters
----------
* `id` :
    the publication identifier
* `a` :
    data block containing the data

Exceptions
----------
* `invalid_argument` :
    if the publication id is invalid
";

%feature("docstring") helics::ValueFederate::publish "

publish a data block

this function is primarily to prevent data blocks from falling through to the
template

Parameters
----------
* `id` :
    the publication identifier
* `a` :
    data block containing the data

Exceptions
----------
* `invalid_argument` :
    if the publication id is invalid
";

%feature("docstring") helics::ValueFederate::publish "

publish a string

Parameters
----------
* `id` :
    the publication identifier
* `data` :
    a const char pointer to a string

Exceptions
----------
* `invalid_argument` :
    if the publication id is invalid
";

%feature("docstring") helics::ValueFederate::publish "

publish data

Parameters
----------
* `id` :
    the publication identifier
* `data` :
    a const char pointer to raw data
* `data_size` :
    the length of the data

Exceptions
----------
* `invalid_argument` :
    if the publication id is invalid
";

%feature("docstring") helics::ValueFederate::publish "

publish a value

templateparam
-------------
* `X` :
    the type of the value to publish

Parameters
----------
* `id` :
    the publication identifier
* `value` :
    a reference to a value holding the data

Exceptions
----------
* `invalid_argument` :
    if the publication id is invalid
";

%feature("docstring") helics::ValueFederate::publish "

Methods to publish values
";

%feature("docstring") helics::ValueFederate::publish "
";

%feature("docstring") helics::ValueFederate::publish "
";

%feature("docstring") helics::ValueFederate::publish "
";

%feature("docstring") helics::ValueFederate::publish "
";

%feature("docstring") helics::ValueFederate::publish "
";

%feature("docstring") helics::ValueFederate::isUpdated "

check if a given subscription has an update

Returns
-------
true if the subscription id is valid and has an update
";

%feature("docstring") helics::ValueFederate::isUpdated "

Check if a subscription is updated
";

%feature("docstring") helics::ValueFederate::getLastUpdateTime "

get the time of the last update
";

%feature("docstring") helics::ValueFederate::getLastUpdateTime "

Get the last time a subscription was updated
";

%feature("docstring") helics::ValueFederate::disconnect "

disconnect a simulation from the core (will also call finalize before
disconnecting if necessary)
";

%feature("docstring") helics::ValueFederate::queryUpdates "

get a list of all the values that have been updated since the last call

Returns
-------
a vector of subscription_ids with all the values that have not been retrieved
since updated
";

%feature("docstring") helics::ValueFederate::queryUpdates "

Get a list of all subscriptions with updates since the last call
";

%feature("docstring") helics::ValueFederate::getSubscriptionKey "

get the key or the string identifier of a subscription from its id

Returns
-------
empty string if an invalid id is passed
";

%feature("docstring") helics::ValueFederate::getSubscriptionId "

get the id of a subscription

Returns
-------
ivalid_subscription_id if name is not a recognized
";

%feature("docstring") helics::ValueFederate::getSubscriptionId "

get the id of a subscription from a vector of subscriptions

Returns
-------
ivalid_subscription_id if name is not a recognized
";

%feature("docstring") helics::ValueFederate::getSubscriptionId "

get the id of a subscription from a 2-d vector of subscriptions

Returns
-------
ivalid_subscription_id if name is not a recognized
";

%feature("docstring") helics::ValueFederate::getPublicationKey "

get the name of a publication from its id

Returns
-------
empty string if an invalid id is passed
";

%feature("docstring") helics::ValueFederate::getPublicationId "

get the id of a registered publication from its id

Parameters
----------
* `name` :
    the name of the publication

Returns
-------
ivalid_publication_id if name is not recognized otherwise returns the
publication_id
";

%feature("docstring") helics::ValueFederate::getPublicationId "

get the id of a registered publication from its id

Parameters
----------
* `name` :
    the name of the publication

Returns
-------
ivalid_publication_id if name is not recognized otherwise returns the
publication_id
";

%feature("docstring") helics::ValueFederate::getPublicationId "

get the id of a registered publication from its id

Parameters
----------
* `name` :
    the name of the publication

Returns
-------
ivalid_publication_id if name is not recognized otherwise returns the
publication_id
";

%feature("docstring") helics::ValueFederate::getSubscriptionUnits "

get the units of a subscriptions from its id

Parameters
----------
* `id` :
    the subscription id to query

Returns
-------
the name or empty string on unrecognized id
";

%feature("docstring") helics::ValueFederate::getSubscriptionUnits "
";

%feature("docstring") helics::ValueFederate::getPublicationUnits "

get the units of a publication from its id

Parameters
----------
* `id` :
    the publication id to query

Returns
-------
the units or empty string on unrecognized id
";

%feature("docstring") helics::ValueFederate::getPublicationUnits "
";

%feature("docstring") helics::ValueFederate::getSubscriptionType "

get the type of a subscription from its id

Parameters
----------
* `id` :
    the subscription id to query

Returns
-------
the type or empty string on unrecognized id
";

%feature("docstring") helics::ValueFederate::getSubscriptionType "
";

%feature("docstring") helics::ValueFederate::getPublicationType "

get the type of a publication from its id

Parameters
----------
* `id` :
    the publication id to query

Returns
-------
the type or empty string on unrecognized id
";

%feature("docstring") helics::ValueFederate::getPublicationType "

get the type of the publication of a particular subscription

Parameters
----------
* `id` :
    the subscription id to query

Returns
-------
the type or empty string on unrecognized id
";

%feature("docstring") helics::ValueFederate::getPublicationType "
";

%feature("docstring") helics::ValueFederate::registerSubscriptionNotificationCallback "

register a callback function to call when any subscribed value is updated

there can only be one generic callback

Parameters
----------
* `callback` :
    the function to call signature void(subscription_id_t, Time)
";

%feature("docstring") helics::ValueFederate::registerSubscriptionNotificationCallback "

register a callback function to call when the specified subscription is updated

Parameters
----------
* `id` :
    the id to register the callback for
* `callback` :
    the function to call
";

%feature("docstring") helics::ValueFederate::registerSubscriptionNotificationCallback "

register a callback function to call when the specified subscription is updated

Parameters
----------
* `ids` :
    the set of ids to register the callback for
* `callback` :
    the function to call
";

%feature("docstring") helics::ValueFederate::getPublicationCount "

get a count of the number publications registered
";

%feature("docstring") helics::ValueFederate::getSubscriptionCount "

get a count of the number subscriptions registered
";

%feature("docstring") helics::ValueFederate::registerTypePublication "
";

%feature("docstring") helics::ValueFederate::registerGlobalTypePublication "
";

%feature("docstring") helics::ValueFederate::registerSubscription "

Methods to register subscriptions
";

%feature("docstring") helics::ValueFederate::registerTypeSubscription "
";

%feature("docstring") helics::ValueFederate::registerSubscriptionIndexed "
";

%feature("docstring") helics::ValueFederate::registerSubscriptionIndexed "
";

%feature("docstring") helics::ValueFederate::getString "
";

%feature("docstring") helics::ValueFederate::getInteger "
";

%feature("docstring") helics::ValueFederate::getDouble "
";

%feature("docstring") helics::ValueFederate::getComplex "
";

%feature("docstring") helics::ValueFederate::getVector "
";

%feature("docstring") helics::ValueFederate::getSubscriptionName "
";

%feature("docstring") helics::ValueFederate::getPublicationName "
";

// File: classhelics_1_1ValueFederateManager.xml


%feature("docstring") helics::ValueFederateManager "

class handling the implementation details of a value Federate

C++ includes: ValueFederateManager.hpp
";

%feature("docstring") helics::ValueFederateManager::ValueFederateManager "
";

%feature("docstring") helics::ValueFederateManager::~ValueFederateManager "
";

%feature("docstring") helics::ValueFederateManager::registerPublication "
";

%feature("docstring") helics::ValueFederateManager::registerRequiredSubscription "

register a subscription

call is only valid in startup mode
";

%feature("docstring") helics::ValueFederateManager::registerOptionalSubscription "

register a subscription

call is only valid in startup mode
";

%feature("docstring") helics::ValueFederateManager::addSubscriptionShortcut "

add a shortcut for locating a subscription

primarily for use in looking up an id from a different location creates a local
shortcut for referring to a subscription which may have a long actual name

Parameters
----------
* `the` :
    subscription identifier
* `shortcutName` :
    the name of the shortcut
";

%feature("docstring") helics::ValueFederateManager::setDefaultValue "

set the default value for a subscription

this is the value returned prior to any publications

Parameters
----------
* `id` :
    the subscription identifier
* `block` :
    the data block representing the default value

copy the data first since we are not entirely sure of the lifetime of the
data_view
";

%feature("docstring") helics::ValueFederateManager::getValue "

get a value as raw data block from the system

Parameters
----------
* `id` :
    the identifier for the subscription

Returns
-------
a constant data block
";

%feature("docstring") helics::ValueFederateManager::publish "

publish a value
";

%feature("docstring") helics::ValueFederateManager::queryUpdate "

check if a given subscription has and update
";

%feature("docstring") helics::ValueFederateManager::queryLastUpdate "

get the time of the last update
";

%feature("docstring") helics::ValueFederateManager::updateTime "

update the time from oldTime to newTime

Parameters
----------
* `newTime` :
    the newTime of the federate
* `oldTime` :
    the oldTime of the federate

find the id
";

%feature("docstring") helics::ValueFederateManager::startupToInitializeStateTransition "

transition from Startup To the Initialize State
";

%feature("docstring") helics::ValueFederateManager::initializeToExecuteStateTransition "

transition from initialize to execution State
";

%feature("docstring") helics::ValueFederateManager::queryUpdates "

get a list of all the values that have been updated since the last call

Returns
-------
a vector of subscription_ids with all the values that have not been retrieved
since updated
";

%feature("docstring") helics::ValueFederateManager::getSubscriptionKey "

get the key of a subscription from its id

Returns
-------
empty string if an invalid id is passed
";

%feature("docstring") helics::ValueFederateManager::getSubscriptionId "

get the id of a subscription

Returns
-------
ivalid_subscription_id if name is not a recognized
";

%feature("docstring") helics::ValueFederateManager::getPublicationKey "

get the key of a publication from its id

Returns
-------
empty string if an invalid id is passed
";

%feature("docstring") helics::ValueFederateManager::getPublicationId "

get the id of a registered publication from its id

Parameters
----------
* `name` :
    the publication id

Returns
-------
ivalid_publication_id if name is not recognized otherwise returns the
publication_id
";

%feature("docstring") helics::ValueFederateManager::getSubscriptionUnits "

get the units of a subscriptions from its id

Parameters
----------
* `id` :
    the subscription id to query

Returns
-------
the name or empty string on unrecognized id
";

%feature("docstring") helics::ValueFederateManager::getPublicationUnits "

get the units of a publication from its id

Parameters
----------
* `id` :
    the publication id to query

Returns
-------
the units or empty string on unrecognized id
";

%feature("docstring") helics::ValueFederateManager::getSubscriptionType "

get the type of a subscription from its id

Parameters
----------
* `id` :
    the subscription id to query

Returns
-------
the type or empty string on unrecognized id
";

%feature("docstring") helics::ValueFederateManager::getPublicationType "

get the type of a publication from its id

Parameters
----------
* `id` :
    the publication id to query

Returns
-------
the type or empty string on unrecognized id
";

%feature("docstring") helics::ValueFederateManager::getPublicationType "

get the type of a publication from its subscription

Parameters
----------
* `id` :
    the subscription id to query

Returns
-------
the type or empty string on unrecognized id
";

%feature("docstring") helics::ValueFederateManager::registerCallback "

register a callback function to call when any subscribed value is updated

there can only be one generic callback

Parameters
----------
* `callback` :
    the function to call
";

%feature("docstring") helics::ValueFederateManager::registerCallback "

register a callback function to call when the specified subscription is updated

Parameters
----------
* `id` :
    the id to register the callback for
* `callback` :
    the function to call
";

%feature("docstring") helics::ValueFederateManager::registerCallback "

register a callback function to call when the specified subscription is updated

Parameters
----------
* `ids` :
    the set of ids to register the callback for
* `callback` :
    the function to call
";

%feature("docstring") helics::ValueFederateManager::disconnect "

disconnect from the coreObject
";

%feature("docstring") helics::ValueFederateManager::getPublicationCount "

get a count of the number publications registered
";

%feature("docstring") helics::ValueFederateManager::getSubscriptionCount "

get a count of the number subscriptions registered
";

// File: classhelics_1_1ValueSetter.xml


%feature("docstring") helics::ValueSetter "
";

// File: classhelics_1_1ValueStats.xml


%feature("docstring") helics::ValueStats "

helper class for displaying statistics

C++ includes: recorder.h
";

// File: classhelics_1_1VectorSubscription.xml


%feature("docstring") helics::VectorSubscription "

class to handle a Vector Subscription

templateparam
-------------
* `X` :
    the class of the value associated with the vector subscription

C++ includes: Subscriptions.hpp
";

%feature("docstring") helics::VectorSubscription::VectorSubscription "
";

%feature("docstring") helics::VectorSubscription::VectorSubscription "

constructor to build a subscription object

Parameters
----------
* `required` :
    a flag indicating that the subscription is required to have a matching
    publication
* `valueFed` :
    the ValueFederate to use
* `name` :
    the name of the subscription
* `units` :
    the units associated with a Federate
";

%feature("docstring") helics::VectorSubscription::VectorSubscription "

constructor to build a subscription object

Parameters
----------
* `valueFed` :
    the ValueFederate to use
* `name` :
    the name of the subscription
* `units` :
    the units associated with a Federate
";

%feature("docstring") helics::VectorSubscription::VectorSubscription "

move constructor
";

%feature("docstring") helics::VectorSubscription::getVals "

get the most recent value

Returns
-------
the value
";

%feature("docstring") helics::VectorSubscription::registerCallback "

register a callback for the update

the callback is called in the just before the time request function returns

Parameters
----------
* `callback` :
    a function with signature void(X val, Time time) val is the new value and
    time is the time the value was updated
";

// File: classhelics_1_1VectorSubscription2d.xml


%feature("docstring") helics::VectorSubscription2d "

class to handle a Vector Subscription

templateparam
-------------
* `X` :
    the class of the value associated with the vector subscription

C++ includes: Subscriptions.hpp
";

%feature("docstring") helics::VectorSubscription2d::VectorSubscription2d "
";

%feature("docstring") helics::VectorSubscription2d::VectorSubscription2d "

constructor to build a subscription object

Parameters
----------
* `required` :
    a flag indicating that the subscription is required to have a matching
    publication
* `valueFed` :
    the ValueFederate to use
* `name` :
    the name of the subscription
* `units` :
    the units associated with a Federate
";

%feature("docstring") helics::VectorSubscription2d::VectorSubscription2d "

constructor to build a subscription object

Parameters
----------
* `valueFed` :
    the ValueFederate to use
* `name` :
    the name of the subscription
* `units` :
    the units associated with a Federate
";

%feature("docstring") helics::VectorSubscription2d::VectorSubscription2d "

move constructor
";

%feature("docstring") helics::VectorSubscription2d::getVals "

get the most recent value

Returns
-------
the value
";

%feature("docstring") helics::VectorSubscription2d::at "

get the value in the given variable

Parameters
----------
* `out` :
    the location to store the value
";

%feature("docstring") helics::VectorSubscription2d::registerCallback "

register a callback for the update

the callback is called in the just before the time request function returns

Parameters
----------
* `callback` :
    a function with signature void(X val, Time time) val is the new value and
    time is the time the value was updated
";

// File: classhelics_1_1waitingResponse.xml


%feature("docstring") helics::waitingResponse "

helper class to manage REQ sockets that are awaiting a response

C++ includes: ZmqRequestSets.h
";

// File: structzmq__event__t.xml


%feature("docstring") zmq_event_t "
";

// File: classhelics_1_1ZmqBroker.xml


%feature("docstring") helics::ZmqBroker "
";

%feature("docstring") helics::ZmqBroker::ZmqBroker "

default constructor
";

%feature("docstring") helics::ZmqBroker::ZmqBroker "
";

%feature("docstring") helics::ZmqBroker::initializeFromArgs "

initialize from command line arguments
";

%feature("docstring") helics::ZmqBroker::~ZmqBroker "

destructor
";

%feature("docstring") helics::ZmqBroker::getAddress "

get the connection address for the broker
";

%feature("docstring") helics::ZmqBroker::displayHelp "
";

// File: classhelics_1_1ZmqComms.xml


%feature("docstring") helics::ZmqComms "

implementation for the communication interface that uses ZMQ messages to
communicate

C++ includes: ZmqComms.h
";

%feature("docstring") helics::ZmqComms::ZmqComms "

default constructor
";

%feature("docstring") helics::ZmqComms::ZmqComms "
";

%feature("docstring") helics::ZmqComms::ZmqComms "
";

%feature("docstring") helics::ZmqComms::~ZmqComms "

destructor
";

%feature("docstring") helics::ZmqComms::setBrokerPort "

set the port numbers for the local ports
";

%feature("docstring") helics::ZmqComms::setPortNumber "
";

%feature("docstring") helics::ZmqComms::setAutomaticPortStartPort "
";

%feature("docstring") helics::ZmqComms::getPort "

get the port number of the comms object to push message to
";

%feature("docstring") helics::ZmqComms::getAddress "
";

%feature("docstring") helics::ZmqComms::getPushAddress "
";

// File: classzmqContextManager.xml


%feature("docstring") zmqContextManager "

class defining a singleton context manager for all zmq usage in gridDyn

C++ includes: zmqContextManager.h
";

%feature("docstring") zmqContextManager::getContextPointer "
";

%feature("docstring") zmqContextManager::getContext "
";

%feature("docstring") zmqContextManager::closeContext "
";

%feature("docstring") zmqContextManager::setContextToLeakOnDelete "

tell the context to free the pointer and leak the memory on delete

You may ask why, well in windows systems when operating in a DLL if this context
is closed after certain other operations that happen when the DLL is unlinked
bad things can happen, and since in nearly all cases this happens at Shutdown
leaking really doesn't matter that much

Returns
-------
true if the context was found and the flag set, false otherwise
";

%feature("docstring") zmqContextManager::~zmqContextManager "
";

%feature("docstring") zmqContextManager::getName "
";

%feature("docstring") zmqContextManager::getBaseContext "
";

// File: classhelics_1_1ZmqCore.xml


%feature("docstring") helics::ZmqCore "

implementation for the core that uses zmq messages to communicate

C++ includes: ZmqCore.h
";

%feature("docstring") helics::ZmqCore::ZmqCore "

default constructor
";

%feature("docstring") helics::ZmqCore::ZmqCore "

construct from with a core name
";

%feature("docstring") helics::ZmqCore::~ZmqCore "

destructor
";

%feature("docstring") helics::ZmqCore::initializeFromArgs "

initialize the core manager with command line arguments

Parameters
----------
* `argc` :
    the number of arguments
* `argv` :
    char pointers to the arguments
";

%feature("docstring") helics::ZmqCore::getAddress "

get a string representing the connection info to send data to this object
";

// File: classzmqProxyHub.xml


%feature("docstring") zmqProxyHub "

class building and managing a zmq proxy

the proxy runs in its own thread managed by the proxy class

C++ includes: zmqProxyHub.h
";

%feature("docstring") zmqProxyHub::getProxy "
";

%feature("docstring") zmqProxyHub::~zmqProxyHub "
";

%feature("docstring") zmqProxyHub::startProxy "
";

%feature("docstring") zmqProxyHub::stopProxy "
";

%feature("docstring") zmqProxyHub::modifyIncomingConnection "
";

%feature("docstring") zmqProxyHub::modifyOutgoingConnection "
";

%feature("docstring") zmqProxyHub::getName "
";

%feature("docstring") zmqProxyHub::getIncomingConnection "
";

%feature("docstring") zmqProxyHub::getOutgoingConnection "
";

%feature("docstring") zmqProxyHub::isRunning "
";

// File: classzmqReactor.xml


%feature("docstring") zmqReactor "

class that manages receive sockets and triggers callbacks  the class starts up a
thread that listens for

C++ includes: zmqReactor.h
";

%feature("docstring") zmqReactor::getReactorInstance "
";

%feature("docstring") zmqReactor::~zmqReactor "
";

%feature("docstring") zmqReactor::addSocket "
";

%feature("docstring") zmqReactor::modifySocket "
";

%feature("docstring") zmqReactor::closeSocket "

asyncrhonous call to close a specific socket

Parameters
----------
* `socketName` :
    the name of the socket to close
";

%feature("docstring") zmqReactor::addSocketBlocking "
";

%feature("docstring") zmqReactor::modifySocketBlocking "
";

%feature("docstring") zmqReactor::closeSocketBlocking "
";

%feature("docstring") zmqReactor::getName "
";

%feature("docstring") zmqReactor::terminateReactor "
";

// File: classhelics_1_1ZmqRequestSets.xml


%feature("docstring") helics::ZmqRequestSets "

class for dealing with the priority message paths from a ZMQ comm object

it manages a set of routes to different priority queues and handles the
responses THIS CLASS IS NOT THREAD SAFE- ZMQ sockets cannot be transferred
between threads without special care so it would be VERY problematic to use this
where multiple threads will interact with it, thus no reason to make it thread
safe

C++ includes: ZmqRequestSets.h
";

%feature("docstring") helics::ZmqRequestSets::ZmqRequestSets "

constructor
";

%feature("docstring") helics::ZmqRequestSets::addRoutes "

add a route to the request set
";

%feature("docstring") helics::ZmqRequestSets::transmit "

transmit a command to a specific route number
";

%feature("docstring") helics::ZmqRequestSets::waiting "

check if the request set is waiting on any on responses
";

%feature("docstring") helics::ZmqRequestSets::checkForMessages "

check for messages with a 0 second timeout

Returns
-------
the number of message waiting to be received
";

%feature("docstring") helics::ZmqRequestSets::checkForMessages "

check for messages with an explicit timeout

Returns
-------
the number of message waiting to be received
";

%feature("docstring") helics::ZmqRequestSets::hasMessages "

check if there are any waiting message without scanning the sockets
";

%feature("docstring") helics::ZmqRequestSets::getMessage "

get any messages that have been received
";

%feature("docstring") helics::ZmqRequestSets::close "

close all the sockets
";

// File: classzmqSocketDescriptor.xml


%feature("docstring") zmqSocketDescriptor "

data class describing a socket and some operations on it

C++ includes: zmqSocketDescriptor.h
";

%feature("docstring") zmqSocketDescriptor::zmqSocketDescriptor "
";

%feature("docstring") zmqSocketDescriptor::zmqSocketDescriptor "
";

%feature("docstring") zmqSocketDescriptor::addOperation "
";

%feature("docstring") zmqSocketDescriptor::makeSocket "
";

%feature("docstring") zmqSocketDescriptor::makeSocketPtr "
";

%feature("docstring") zmqSocketDescriptor::modifySocket "
";

// File: namespaceboost.xml

// File: namespaceboost_1_1asio.xml

// File: namespaceboost_1_1program__options.xml

// File: namespaceboost_1_1system.xml

// File: namespacehelics.xml

%feature("docstring") helics::action_message_def::cleanupHelicsLibrary "

function to do some housekeeping work

this runs some cleanup routines and tries to close out any residual thread that
haven't been shutdown yet
";

%feature("docstring") helics::action_message_def::LoadFederateInfo "

generate a FederateInfo object from a json file
";

%feature("docstring") helics::action_message_def::randDouble "
";

%feature("docstring") helics::action_message_def::addOperations "
";

%feature("docstring") helics::action_message_def::make_destination_filter "

create a destination filter

Parameters
----------
* `type` :
    the type of filter to create
* `fed` :
    the federate to create the filter through
* `target` :
    the target endpoint all message with the specified target as a destination
    will route through the filter
* `name` :
    the name of the filter (optional)

Returns
-------
a unique pointer to a destination Filter object, note destroying the object does
not deactivate the filter
";

%feature("docstring") helics::action_message_def::make_destination_filter "

create a destination filter

Parameters
----------
* `type` :
    the type of filter to create
* `cr` :
    the core to create the federate through
* `target` :
    the target endpoint all message with the specified target as a destination
    will route through the filter
* `name` :
    the name of the filter (optional)

Returns
-------
a unique pointer to a destination Filter object, note destroying the object does
not deactivate the filter
";

%feature("docstring") helics::action_message_def::make_source_filter "

create a source filter

Parameters
----------
* `type` :
    the type of filter to create
* `cr` :
    the core to create the filter through
* `target` :
    the target endpoint all message coming from the specified source will route
    through the filter
* `name` :
    the name of the filter (optional)

Returns
-------
a unique pointer to a source Filter object, note destroying the object does not
deactivate the filter
";

%feature("docstring") helics::action_message_def::make_source_filter "

create a source filter

Parameters
----------
* `type` :
    the type of filter to create
* `fed` :
    the federate to create the filter through
* `target` :
    the target endpoint all message coming from the specified source will route
    through the filter
* `name` :
    the name of the filter (optional)

Returns
-------
a unique pointer to a source Filter object, note destroying the object does not
deactivate the filter
";

%feature("docstring") helics::action_message_def::changeDetected "
";

%feature("docstring") helics::action_message_def::changeDetected "
";

%feature("docstring") helics::action_message_def::changeDetected "
";

%feature("docstring") helics::action_message_def::changeDetected "
";

%feature("docstring") helics::action_message_def::changeDetected "
";

%feature("docstring") helics::action_message_def::changeDetected "
";

%feature("docstring") helics::action_message_def::changeDetected "
";

%feature("docstring") helics::action_message_def::valueExtract "
";

%feature("docstring") helics::action_message_def::valueExtract "
";

%feature("docstring") helics::action_message_def::valueExtract "
";

%feature("docstring") helics::action_message_def::valueExtract "
";

%feature("docstring") helics::action_message_def::valueExtract "
";

%feature("docstring") helics::action_message_def::valueExtract "
";

%feature("docstring") helics::action_message_def::valueExtract "
";

%feature("docstring") helics::action_message_def::valueExtract "
";

%feature("docstring") helics::action_message_def::valueExtract "

for numeric types
";

%feature("docstring") helics::action_message_def::valueExtract "

assume it is some numeric type (int or double)
";

%feature("docstring") helics::action_message_def::doubleString "
";

%feature("docstring") helics::action_message_def::intString "
";

%feature("docstring") helics::action_message_def::stringString "
";

%feature("docstring") helics::action_message_def::complexString "
";

%feature("docstring") helics::action_message_def::doubleVecString "
";

%feature("docstring") helics::action_message_def::complexVecString "
";

%feature("docstring") helics::action_message_def::typeNameStringRef "

sometime we just need a ref to a string for the basic types
";

%feature("docstring") helics::action_message_def::helicsComplexString "
";

%feature("docstring") helics::action_message_def::helicsComplexString "
";

%feature("docstring") helics::action_message_def::getTypeFromString "

convert a string to a type
";

%feature("docstring") helics::action_message_def::creg "
";

%feature("docstring") helics::action_message_def::helicsGetComplex "

convert a string to a complex number
";

%feature("docstring") helics::action_message_def::helicsVectorString "
";

%feature("docstring") helics::action_message_def::helicsVectorString "
";

%feature("docstring") helics::action_message_def::helicsComplexVectorString "
";

%feature("docstring") helics::action_message_def::helicsGetVector "

convert a string to a vector
";

%feature("docstring") helics::action_message_def::helicsGetVector "
";

%feature("docstring") helics::action_message_def::helicsGetComplexVector "

convert a string to a complex vector
";

%feature("docstring") helics::action_message_def::helicsGetComplexVector "
";

%feature("docstring") helics::action_message_def::readSize "
";

%feature("docstring") helics::action_message_def::typeConvert "
";

%feature("docstring") helics::action_message_def::typeConvert "
";

%feature("docstring") helics::action_message_def::typeConvert "
";

%feature("docstring") helics::action_message_def::typeConvert "
";

%feature("docstring") helics::action_message_def::typeConvert "
";

%feature("docstring") helics::action_message_def::typeConvert "
";

%feature("docstring") helics::action_message_def::typeConvert "
";

%feature("docstring") helics::action_message_def::typeConvert "
";

%feature("docstring") helics::action_message_def::typeConvert "
";

%feature("docstring") helics::action_message_def::typeNameString "

template class for generating a known name of a type
";

%feature("docstring") helics::action_message_def::typeNameString< std::vector< std::string > > "
";

%feature("docstring") helics::action_message_def::typeNameString< std::vector< double > > "
";

%feature("docstring") helics::action_message_def::typeNameString< double > "

for float
";

%feature("docstring") helics::action_message_def::typeNameString< float > "

for float
";

%feature("docstring") helics::action_message_def::typeNameString< char > "

for character
";

%feature("docstring") helics::action_message_def::typeNameString< unsigned char > "

for unsigned character
";

%feature("docstring") helics::action_message_def::typeNameString< std::int32_t > "

for integer
";

%feature("docstring") helics::action_message_def::typeNameString< std::uint32_t > "

for unsigned integer
";

%feature("docstring") helics::action_message_def::typeNameString< int64_t > "

for 64 bit unsigned integer
";

%feature("docstring") helics::action_message_def::typeNameString< std::uint64_t > "

for 64 bit unsigned integer
";

%feature("docstring") helics::action_message_def::typeNameString< std::string > "
";

%feature("docstring") helics::action_message_def::helicsType "

template class for generating a known name of a type
";

%feature("docstring") helics::action_message_def::helicsType< int64_t > "
";

%feature("docstring") helics::action_message_def::helicsType< std::string > "
";

%feature("docstring") helics::action_message_def::helicsType< double > "
";

%feature("docstring") helics::action_message_def::helicsType< std::vector< double > > "
";

%feature("docstring") helics::action_message_def::isConvertableType "
";

%feature("docstring") helics::action_message_def::isConvertableType "
";

%feature("docstring") helics::action_message_def::isConvertableType< float > "
";

%feature("docstring") helics::action_message_def::isConvertableType< long double > "
";

%feature("docstring") helics::action_message_def::isConvertableType< int > "
";

%feature("docstring") helics::action_message_def::isConvertableType< short > "
";

%feature("docstring") helics::action_message_def::isConvertableType< unsigned int > "
";

%feature("docstring") helics::action_message_def::isConvertableType< char > "
";

%feature("docstring") helics::action_message_def::isConvertableType< uint64_t > "
";

%feature("docstring") helics::action_message_def::invalidValue "

generate an invalid value for the various types
";

%feature("docstring") helics::action_message_def::invalidValue< double > "
";

%feature("docstring") helics::action_message_def::invalidValue< uint64_t > "
";

%feature("docstring") helics::action_message_def::typeNameString< std::vector< data_block > > "
";

%feature("docstring") helics::action_message_def::make_publication "
";

%feature("docstring") helics::action_message_def::make_publication "
";

%feature("docstring") helics::action_message_def::save "
";

%feature("docstring") helics::action_message_def::save "
";

%feature("docstring") helics::action_message_def::load "
";

%feature("docstring") helics::action_message_def::load "
";

%feature("docstring") helics::action_message_def::getMinSize "
";

%feature("docstring") helics::action_message_def::getMinSize "
";

%feature("docstring") helics::action_message_def::getMinSize "
";

%feature("docstring") helics::action_message_def::publish "

publish directly from the publication key name

this is a convenience function to publish directly from the publication key

Parameters
----------
* `fed` :
    a shared_ptr to a value federate
* `pubKey` :
    the name of the publication

templateparam
-------------
* `pargs` :
    any combination of arguments that go into the other publish commands
";

%feature("docstring") helics::action_message_def::publish "

publish directly from the publication key name

this is a convenience function to publish directly from the publication key this
function should not be used as the primary means of publications as it does
involve an additional map find operation vs the member publish calls

Parameters
----------
* `fed` :
    a reference to a valueFederate
* `pubKey` :
    the name of the publication

templateparam
-------------
* `pargs` :
    any combination of arguments that go into the other publish commands
";

%feature("docstring") helics::action_message_def::getValue "

get a value directly from the subscription key name

this is a convenience function to get a value directly from the subscription key
name this function should not be used as the primary means of retrieving value
as it does involve an additional map find operation vs the member getValue calls

Parameters
----------
* `fed` :
    a shared pointer to a valueFederate
* `key` :
    the name of the publication
";

%feature("docstring") helics::action_message_def::getValue "

get a value directly from the subscription key name

this is a convenience function to get a value directly from the subscription key
name this function should not be used as the primary means of retrieving value
as it does involve an additional map find operation vs the member getValue calls

Parameters
----------
* `fed` :
    a shared pointer to a valueFederate
* `key` :
    the name of the publication
* `obj` :
    the obj to store the retrieved value
";

%feature("docstring") helics::action_message_def::getValue "

get a value directly from the subscription key name

this is a convenience function to get a value directly from the subscription key
name this function should not be used as the primary means of retrieving value
as it does involve an additional map find operation vs the member getValue calls

Parameters
----------
* `fed` :
    a reference to a valueFederate
* `key` :
    the name of the publication
";

%feature("docstring") helics::action_message_def::getValue "

get a value directly from the subscription key name

this is a convenience function to get a value directly from the subscription key
name this function should not be used as the primary means of retrieving value
as it does involve an additional map find operation vs the member getValue calls

Parameters
----------
* `fed` :
    a reference to a valueFederate
* `key` :
    the name of the publication
* `obj` :
    the obj to store the retrieved value
";

%feature("docstring") helics::action_message_def::getTypeSize "
";

%feature("docstring") helics::action_message_def::isBlockSizeValid "

function to check if the size is valid for the given type
";

%feature("docstring") helics::action_message_def::vComp "
";

%feature("docstring") helics::action_message_def::mComp "
";

%feature("docstring") helics::action_message_def::argumentParser "
";

%feature("docstring") helics::action_message_def::createMessage "

create a new message object that copies all the information from the
ActionMessage into newly allocated memory for the message
";

%feature("docstring") helics::action_message_def::createMessage "

create a new message object that moves all the information from the
ActionMessage into newly allocated memory for the message
";

%feature("docstring") helics::action_message_def::actionMessageType "

return the name of the action

Parameters
----------
* `action` :
    The action to get the name for

Returns
-------
a pointer to string with the name
";

%feature("docstring") helics::action_message_def::prettyPrintString "

generate a human readable string with information about a command

Parameters
----------
* `command` :
    the command to generate the string for

Returns
-------
a string representing information about the command
";

%feature("docstring") helics::action_message_def::isProtocolCommand "

check if a command is a protocol command
";

%feature("docstring") helics::action_message_def::isPriorityCommand "

check if a command is a priority command
";

%feature("docstring") helics::action_message_def::isTimingMessage "
";

%feature("docstring") helics::action_message_def::isDependencyMessage "
";

%feature("docstring") helics::action_message_def::isDisconnectCommand "

check if a command is a disconnect command
";

%feature("docstring") helics::action_message_def::isValidCommand "

check if a command is a priority command
";

%feature("docstring") helics::action_message_def::hasInfo "

check if the action has an info structure associated with it
";

%feature("docstring") helics::action_message_def::timerTickHandler "
";

%feature("docstring") helics::action_message_def::makeBroker "
";

%feature("docstring") helics::action_message_def::unknownString "
";

%feature("docstring") helics::action_message_def::matchingTypes "
";

%feature("docstring") helics::action_message_def::isValidIndex "

helper template to check whether an index is actually valid for a particular
vector
";

%feature("docstring") helics::action_message_def::helicsTypeString "

generate a string based on the core type
";

%feature("docstring") helics::action_message_def::coreTypeFromString "

generate a core type value from a std::string

Parameters
----------
* `a` :
    string describing the desired core type

Returns
-------
a value of the helics_core_type enumeration

Exceptions
----------
* `invalid_argument` :
    if the string is not recognized
";

%feature("docstring") helics::action_message_def::isCoreTypeAvailable "

Returns true if core/broker type specified is available in current compilation.
";

%feature("docstring") helics::action_message_def::makeGlobalHandleIdentifier "
";

%feature("docstring") helics::action_message_def::makeCore "
";

%feature("docstring") helics::action_message_def::helicsVersionString "

Returns
-------
a string containing version information
";

%feature("docstring") helics::action_message_def::helicsVersionMajor "

get the Major version number
";

%feature("docstring") helics::action_message_def::helicsVersionMinor "

get the Minor version number
";

%feature("docstring") helics::action_message_def::helicsVersionPatch "

get the patch number
";

%feature("docstring") helics::action_message_def::stringTranslateToCppName "

translate a string to a C++ qualified name for variable naming purposes
";

%feature("docstring") helics::action_message_def::makePortAddress "

generate a string with a full address based on an interface string and port
number

, how things get merged depend on what interface is used some use port number
some do not

Parameters
----------
* `interface` :
    a string with an interface description i.e 127.0.0.1
* `portNumber` :
    the number of the port to use

Returns
-------
a string with the merged address
";

%feature("docstring") helics::action_message_def::extractInterfaceandPort "

extract a port number and interface string from an address number

, if there is no port number it default to -1 this is true if none was listed or
the interface doesn't use port numbers

Parameters
----------
* `address` :
    a string with an network location description i.e 127.0.0.1:34

Returns
-------
a pair with a string and int with the interface name and port number
";

%feature("docstring") helics::action_message_def::extractInterfaceandPortString "

extract a port number string and interface string from an address number

, if there is no port number it default to empty string this is true if none was
listed or the interface doesn't use port numbers

Parameters
----------
* `address` :
    a string with an network location description i.e 127.0.0.1:34

Returns
-------
a pair with 2 strings with the interface name and port number
";

%feature("docstring") helics::action_message_def::getLocalExternalAddressV4 "

get the external ipv4 address of the current computer
";

%feature("docstring") helics::action_message_def::getLocalExternalAddressV4 "

get the external ipv4 Ethernet address of the current computer that best matches
the listed server
";

%feature("docstring") helics::action_message_def::getHelicsVersionString "

get a string with the helics version info
";

// File: namespacehelics_1_1action__message__def.xml

// File: namespacehelics_1_1BrokerFactory.xml

%feature("docstring") helics::BrokerFactory::create "

Creates a Broker object of the specified type.

Invokes initialize() on the instantiated Core object.
";

%feature("docstring") helics::BrokerFactory::create "
";

%feature("docstring") helics::BrokerFactory::create "
";

%feature("docstring") helics::BrokerFactory::create "
";

%feature("docstring") helics::BrokerFactory::available "
";

%feature("docstring") helics::BrokerFactory::findBroker "

locate a coreBroker by name

Parameters
----------
* `name` :
    the name of the broker

Returns
-------
a shared_ptr to the Broker
";

%feature("docstring") helics::BrokerFactory::registerBroker "

register a coreBroker so it can be found by others

also cleans up any leftover brokers that were previously unregistered this can
be controlled by calling cleanUpBrokers earlier if desired

Parameters
----------
* `broker` :
    a pointer to a Broker object that should be able to be found globally

Returns
-------
true if the registration was successful false otherwise
";

%feature("docstring") helics::BrokerFactory::cleanUpBrokers "

clean up unused brokers

when brokers are unregistered they get put in a holding area that gets cleaned
up when a new broker is registered or when the clean up function is called this
prevents some odd threading issues

Returns
-------
the number of brokers still operating
";

%feature("docstring") helics::BrokerFactory::cleanUpBrokers "

clean up unused brokers

when brokers are unregistered they get put in a holding area that gets cleaned
up when a new broker is registered or when the clean up function is called this
prevents some odd threading issues

Parameters
----------
* `delay` :
    the number of milliseconds to wait to ensure stuff is cleaned up

Returns
-------
the number of brokers still operating
";

%feature("docstring") helics::BrokerFactory::copyBrokerIdentifier "

make a copy of the broker pointer to allow access to the new name
";

%feature("docstring") helics::BrokerFactory::unregisterBroker "

remove a broker from the registry

Parameters
----------
* `name` :
    the name of the broker to unregister
";

%feature("docstring") helics::BrokerFactory::displayHelp "

display the help listing for a particular core_type
";

// File: namespacehelics_1_1CoreFactory.xml

%feature("docstring") helics::CoreFactory::create "

Creates a Core API object of the specified type.

Invokes initialize() on the instantiated Core object.
";

%feature("docstring") helics::CoreFactory::create "
";

%feature("docstring") helics::CoreFactory::create "
";

%feature("docstring") helics::CoreFactory::create "
";

%feature("docstring") helics::CoreFactory::FindOrCreate "

tries to find a named core if it fails it creates a new one
";

%feature("docstring") helics::CoreFactory::FindOrCreate "

tries to find a named core if it fails it creates a new one
";

%feature("docstring") helics::CoreFactory::findCore "

locate a registered Core by name

Parameters
----------
* `name` :
    the name of the core to find

Returns
-------
a shared_ptr to the testCore
";

%feature("docstring") helics::CoreFactory::isJoinableCoreOfType "
";

%feature("docstring") helics::CoreFactory::findJoinableCoreOfType "

try to find a joinable core of a specific type
";

%feature("docstring") helics::CoreFactory::registerCore "

register a testCore so it can be found by others

also cleans up any leftover bCoresrokers that were previously unregistered this
can be controlled by calling cleanUpBrokers earlier if desired

Parameters
----------
* `core` :
    a pointer to a testCore object that should be found globally

Returns
-------
true if the registration was successful false otherwise
";

%feature("docstring") helics::CoreFactory::cleanUpCores "

clean up unused cores

when Cores are unregistered they get put in a holding area that gets cleaned up
when a new Core is registered or when the clean up function is called this
prevents some odd threading issues

Returns
-------
the number of cores still operating
";

%feature("docstring") helics::CoreFactory::cleanUpCores "

clean up unused cores

when Cores are unregistered they get put in a holding area that gets cleaned up
when a new Core is registered or when the clean up function is called this
prevents some odd threading issues

Parameters
----------
* `delay` :
    the delay time in milliseconds to wait for the cores to finish before
    destroying

Returns
-------
the number of cores still operating
";

%feature("docstring") helics::CoreFactory::copyCoreIdentifier "

make a copy of the broker pointer to allow access to the new name
";

%feature("docstring") helics::CoreFactory::unregisterCore "

remove a Core from the registry

Parameters
----------
* `name` :
    the name of the Core to unregister
";

// File: namespacestd.xml

%feature("docstring") std::swap "
";

%feature("docstring") std::swap "
";

%feature("docstring") std::swap "
";

// File: namespacestd_1_1string__literals.xml

// File: namespacestringOps.xml

%feature("docstring") stringOps::splitline "
";

%feature("docstring") stringOps::splitline "
";

%feature("docstring") stringOps::splitline "
";

%feature("docstring") stringOps::splitline "
";

%feature("docstring") stringOps::splitlineQuotes "
";

%feature("docstring") stringOps::splitlineBracket "
";

%feature("docstring") stringOps::trimString "
";

%feature("docstring") stringOps::trim "
";

%feature("docstring") stringOps::trim "
";

%feature("docstring") stringOps::digits "
";

%feature("docstring") stringOps::trailingStringInt "
";

%feature("docstring") stringOps::trailingStringInt "
";

%feature("docstring") stringOps::quoteChars "
";

%feature("docstring") stringOps::removeQuotes "
";

%feature("docstring") stringOps::removeBrackets "
";

%feature("docstring") stringOps::getTailString "
";

%feature("docstring") stringOps::getTailString "
";

%feature("docstring") stringOps::findCloseStringMatch "
";

%feature("docstring") stringOps::removeChars "
";

%feature("docstring") stringOps::removeChar "
";

%feature("docstring") stringOps::characterReplace "
";

%feature("docstring") stringOps::xmlCharacterCodeReplace "
";

// File: namespaceutilities.xml

%feature("docstring") utilities::is_base64 "
";

%feature("docstring") utilities::base64_encode "

encode a binary sequence to a string
";

%feature("docstring") utilities::base64_decode "

decode a string to a vector of unsigned chars
";

%feature("docstring") utilities::base64_decode "

decode a string to the specified memory location
";

%feature("docstring") utilities::base64_decode_to_string "

decode a string to a string
";

%feature("docstring") utilities::base64_decode_type "

decode a string to a typed vector
";

%feature("docstring") utilities::numericMapper "

map that translates all characters that could be in numbers to true all others
to false
";

%feature("docstring") utilities::numericStartMapper "

map that translates all characters that could start a number to true all others
to false
";

%feature("docstring") utilities::numericEndMapper "

map that translates all characters that could end a number to true all others to
false
";

%feature("docstring") utilities::base64Mapper "

map that translates all base 64 characters to the appropriate numerical value
";

%feature("docstring") utilities::digitMapper "

map that translates numerical characters to the appropriate numerical value
";

%feature("docstring") utilities::hexMapper "

map that translates all hexadecimal characters to the appropriate numerical
value
";

%feature("docstring") utilities::pairMapper "

map that all containing characters that come in pairs to the appropriate match
'{' to '}'
";

// File: namespacezmq.xml

%feature("docstring") zmq::poll "
";

%feature("docstring") zmq::poll "
";

%feature("docstring") zmq::proxy "
";

%feature("docstring") zmq::proxy_steerable "
";

%feature("docstring") zmq::version "
";

// File: ActionMessage_8cpp.xml

// File: ActionMessage_8hpp.xml

// File: ActionMessageDefintions_8hpp.xml

// File: api-data_8h.xml

// File: api__objects_8h.xml

%feature("docstring") helics::getFed "
";

%feature("docstring") helics::getValueFed "
";

%feature("docstring") helics::getMessageFed "
";

%feature("docstring") helics::getCore "
";

%feature("docstring") helics::getFedSharedPtr "
";

%feature("docstring") helics::getValueFedSharedPtr "
";

%feature("docstring") helics::getMessageFedSharedPtr "
";

%feature("docstring") helics::getCoreSharedPtr "
";

%feature("docstring") helics::getMasterHolder "
";

%feature("docstring") helics::clearAllObjects "
";

// File: appMain_8cpp.xml

%feature("docstring") showHelp "
";

%feature("docstring") main "
";

// File: argParser_8cpp.xml

// File: argParser_8h.xml

// File: AsioServiceManager_8cpp.xml

%feature("docstring") serviceRunLoop "
";

// File: AsioServiceManager_8h.xml

%feature("docstring") serviceRunLoop "
";

// File: AsyncFedCallInfo_8hpp.xml

// File: barrier_8hpp.xml

// File: base64_8cpp.xml

// File: base64_8h.xml

// File: BasicHandleInfo_8hpp.xml

// File: blocking__queue_8h.xml

// File: BlockingPriorityQueue_8hpp.xml

// File: BlockingQueue_8hpp.xml

// File: core_2Broker_8hpp.xml

// File: cpp98_2Broker_8hpp.xml

// File: BrokerBase_8cpp.xml

%feature("docstring") helics::gen_id "
";

%feature("docstring") helics::argumentParser "
";

// File: BrokerBase_8hpp.xml

// File: BrokerFactory_8cpp.xml

// File: BrokerFactory_8hpp.xml

// File: charMapper_8cpp.xml

// File: charMapper_8h.xml

// File: chelics_8h.xml

// File: CombinationFederate_8cpp.xml

// File: application__api_2CombinationFederate_8hpp.xml

// File: cpp98_2CombinationFederate_8hpp.xml

// File: CommonCore_8cpp.xml

// File: CommonCore_8hpp.xml

// File: CommsBroker_8cpp.xml

// File: CommsBroker_8hpp.xml

// File: CommsBroker__impl_8hpp.xml

// File: CommsInterface_8cpp.xml

// File: CommsInterface_8hpp.xml

// File: core-data_8cpp.xml

// File: core-data_8hpp.xml

// File: core-exceptions_8hpp.xml

// File: core-types_8cpp.xml

// File: core-types_8hpp.xml

// File: Core_8hpp.xml

// File: CoreBroker_8cpp.xml

// File: CoreBroker_8hpp.xml

// File: CoreFactory_8cpp.xml

// File: CoreFactory_8hpp.xml

// File: CoreFederateInfo_8hpp.xml

// File: delayedDestructor_8hpp.xml

// File: DelayedObjects_8hpp.xml

// File: DualMappedVector_8hpp.xml

// File: echo_8cpp.xml

%feature("docstring") helics::echoArgumentParser "
";

// File: echo_8h.xml

// File: empty_8cpp.xml

// File: EndpointInfo_8cpp.xml

// File: EndpointInfo_8hpp.xml

// File: Endpoints_8cpp.xml

// File: Endpoints_8hpp.xml

// File: Federate_8cpp.xml

// File: application__api_2Federate_8hpp.xml

// File: cpp98_2Federate_8hpp.xml

// File: FederateExport_8cpp.xml

%feature("docstring") getFed "
";

%feature("docstring") getValueFed "
";

%feature("docstring") getMessageFed "
";

%feature("docstring") getFedSharedPtr "
";

%feature("docstring") getValueFedSharedPtr "
";

%feature("docstring") getMessageFedSharedPtr "
";

%feature("docstring") getMasterHolder "
";

%feature("docstring") clearAllObjects "
";

%feature("docstring") helicsCreateValueFederate "

create a value federate from a federate info object

helics_federate objects can be used in all functions that take a helics_federate
or helics_federate object as an argument

Parameters
----------
* `fi` :
    the federate info object that contains details on the federate

Returns
-------
an opaque value federate object
";

%feature("docstring") helicsCreateValueFederateFromJson "

create a value federate from a JSON file or JSON string

helics_federate objects can be used in all functions that take a helics_federate
or helics_federate object as an argument

Parameters
----------
* `JSON` :
    a JSON file or a JSON string that contains setup and configuration
    information

Returns
-------
an opaque value federate object
";

%feature("docstring") helicsCreateMessageFederate "

create a message federate from a federate info object

helics_message_federate objects can be used in all functions that take a
helics_message_federate or helics_federate object as an argument

Parameters
----------
* `fi` :
    the federate info object that contains details on the federate

Returns
-------
an opaque message federate object
";

%feature("docstring") helicsCreateMessageFederateFromJson "

create a message federate from a JSON file or JSON string

helics_message_federate objects can be used in all functions that take a
helics_message_federate or helics_federate object as an argument

Parameters
----------
* `JSON` :
    a JSON file or a JSON string that contains setup and configuration
    information

Returns
-------
an opaque message federate object
";

%feature("docstring") helicsCreateCombinationFederate "

create a combination federate from a federate info object

combination federates are both value federates and message federates, objects
can be used in all functions that take a helics_federate,
helics_message_federate or helics_federate object as an argument

Parameters
----------
* `fi` :
    the federate info object that contains details on the federate

Returns
-------
an opaque value federate object nullptr if the object creation failed
";

%feature("docstring") helicsCreateCombinationFederateFromJson "

create a combination federate from a JSON file or JSON string

combination federates are both value federates and message federates, objects
can be used in all functions that take a helics_federate,
helics_message_federate or helics_federate object as an argument

Parameters
----------
* `JSON` :
    a JSON file or a JSON string that contains setup and configuration
    information

Returns
-------
an opaque combination federate object
";

%feature("docstring") helicsFederateGetCoreObject "

get the core object associated with a federate

Parameters
----------
* `fed` :
    a federate object

Returns
-------
a core object, nullptr if invalid
";

%feature("docstring") helicsFederateFinalize "

finalize the federate this halts all communication in the federate and
disconnects it from the core
";

%feature("docstring") helicsFederateEnterInitializationMode "

enter the initialization state of a federate

the initialization state allows initial values to be set and received if the
iteration is requested on entry to the execution state This is a blocking call
and will block until the core allows it to proceed
";

%feature("docstring") helicsFederateEnterInitializationModeAsync "

non blocking alternative to  the function
helicsFederateEnterInitializationModeFinalize must be called to finish the
operation
";

%feature("docstring") helicsFederateIsAsyncOperationCompleted "

check if the current Asynchronous operation has completed

Parameters
----------
* `fed` :
    the federate to operate on

Returns
-------
0 if not completed, 1 if completed
";

%feature("docstring") helicsFederateEnterInitializationModeComplete "

finalize the entry to initialize mode that was initiated with
";

%feature("docstring") helicsFederateEnterExecutionMode "

request that the federate enter the Execution mode

this call is blocking until granted entry by the core object for an asynchronous
alternative call /ref helicsFederateEnterExecutionModeAsync

Parameters
----------
* `fed` :
    a federate to change modes

Returns
-------
a helics_status enumeration helics_error if something went wrong
helicsInvalidReference if fed is invalid
";

%feature("docstring") getIterationRequest "
";

%feature("docstring") getIterationStatus "
";

%feature("docstring") helicsFederateEnterExecutionModeIterative "
";

%feature("docstring") helicsFederateEnterExecutionModeAsync "

request that the federate enter the Execution mode

this call is non-blocking and will return immediately call /ref
helicsFederateEnterExecutionModeComplete to finish the call sequence /ref
";

%feature("docstring") helicsFederateEnterExecutionModeIterativeAsync "
";

%feature("docstring") helicsFederateEnterExecutionModeComplete "

complete the call to /ref EnterExecutionModeAsync

Parameters
----------
* `fed` :
    the federate object to complete the call
";

%feature("docstring") helicsFederateEnterExecutionModeIterativeComplete "
";

%feature("docstring") helicsFederateRequestTime "

request the next time for federate execution

Parameters
----------
* `fed` :
    the federate to make the request of
* `requestTime` :
    the next requested time
* `timeOut` :
    the time granted to the federate

Returns
-------
a helics_status if the return value is equal to helics_ok the timeOut will
contain the new granted time, otherwise timeOut is invalid
";

%feature("docstring") helicsFederateRequestTimeIterative "

request an iterative time

this call allows for finer grain control of the iterative process then /ref
helicsFederateRequestTime it takes a time and and iteration request and return a
time and iteration status

Parameters
----------
* `fed` :
    the federate to make the request of
* `requestTime` :
    the next desired time
* `iterate` :
    the requested iteration mode
* `timeOut` :
    the granted time
* `outIterate` :
    the iteration specification of the result

Returns
-------
a helics_status object with a return code of the result
";

%feature("docstring") helicsFederateRequestTimeAsync "
";

%feature("docstring") helicsFederateRequestTimeIterativeAsync "
";

%feature("docstring") helicsFederateRequestTimeComplete "
";

%feature("docstring") helicsFederateGetState "

get the current state of a federate

Parameters
----------
* `fed` :
    the fed to query
* `state` :
    the resulting state if helics_status return helics_ok
";

%feature("docstring") helicsFederateGetName "

get the name of the federate

Parameters
----------
* `fed` :
    the federate object to query
* `str` :
    memory buffer to store the result
* `maxlen` :
    the maximum size of the buffer

Returns
-------
helics_status object indicating success or error
";

%feature("docstring") helicsFederateSetTimeDelta "

set the minimum time delta for the federate

Parameters
----------
* `tdelta` :
    the minimum time delta to return from a time request function
";

%feature("docstring") helicsFederateSetOutputDelay "

set the look ahead time

the look ahead is the propagation time for messages/event to propagate from the
Federate the federate

Parameters
----------
* `lookAhead` :
    the look ahead time
";

%feature("docstring") helicsFederateSetInputDelay "

set the impact Window time

the impact window is the time window around the time request in which other
federates cannot affect the federate

Parameters
----------
* `lookAhead` :
    the look ahead time
";

%feature("docstring") helicsFederateSetPeriod "

set the period and offset of the federate

the federate will on grant time on N*period+offset interval

Parameters
----------
* `period` :
    the length of time between each subsequent grants
* `offset` :
    the shift of the period from 0 offset must be < period
";

%feature("docstring") helicsFederateSetFlag "

set a flag for the federate

Parameters
----------
* `fed` :
    the federate to alter a flag for
* `flag` :
    the flag to change
* `flagValue` :
    the new value of the flag 0 for false !=0 for true
";

%feature("docstring") helicsFederateSetLoggingLevel "

set the logging level for the federate @ details debug and trace only do
anything if they were enabled in the compilation

Parameters
----------
* `loggingLevel` :
    (-1: none, 0: error_only, 1: warnings, 2: normal, 3: debug, 4: trace)
";

%feature("docstring") helicsFederateGetCurrentTime "

get the current time of the federate

Parameters
----------
* `fed` :
    the federate object to query
* `time` :
    storage location for the time variable

Returns
-------
helics_status object indicating success or error
";

%feature("docstring") helicsFederateRequestTimeIterativeComplete "
";

// File: FederateInfo_8cpp.xml

%feature("docstring") helics::argumentParser "
";

// File: FederateState_8cpp.xml

// File: FederateState_8hpp.xml

// File: FilterFunctions_8hpp.xml

// File: FilterInfo_8cpp.xml

// File: FilterInfo_8hpp.xml

// File: FilterOperations_8cpp.xml

// File: FilterOperations_8hpp.xml

// File: Filters_8cpp.xml

// File: Filters_8hpp.xml

// File: flag-definitions_8h.xml

// File: generic__string__ops_8hpp.xml

%feature("docstring") generalized_string_split "

this file defines some operations that can be performed on string like objects
";

%feature("docstring") getChunkEnd "
";

%feature("docstring") generalized_section_splitting "
";

// File: helics-broker_8cpp.xml

%feature("docstring") argumentParser "
";

%feature("docstring") main "
";

// File: helics-time_8hpp.xml

// File: helics_8h.xml

%feature("docstring") helicsGetVersion "
";

%feature("docstring") helicsIsCoreTypeAvailable "

Returns true if core/broker type specified is available in current compilation.
";

%feature("docstring") helicsCreateCore "

create a core object

Parameters
----------
* `type` :
    the type of the core to create
* `name` :
    the name of the core , may be a nullptr or empty string to have a name
    automatically assigned
* `initString` :
    an initialization string to send to the core-the format is similar to
    command line arguments typical options include a broker address
    --broker=\"XSSAF\" or the number of federates or the address

Returns
-------
a helics_core object
";

%feature("docstring") helicsCreateCoreFromArgs "

create a core object by passing command line arguments

Parameters
----------
* `type` :
    the type of the core to create
* `name` :
    the name of the core , may be a nullptr or empty string to have a name
    automatically assigned
* `argc` :
    the number of arguments
* `argv` :
    the string values from a command line

Returns
-------
a helics_core object
";

%feature("docstring") helicsCreateBroker "

create a broker object

Parameters
----------
* `type` :
    the type of the broker to create
* `name` :
    the name of the broker , may be a nullptr or empty string to have a name
    automatically assigned
* `initString` :
    an initialization string to send to the core-the format is similar to
    command line arguments typical options include a broker address
    --broker=\"XSSAF\" if this is a subbroker or the number of federates or the
    address

Returns
-------
a helics_core object
";

%feature("docstring") helicsCreateBrokerFromArgs "

create a core object by passing command line arguments

Parameters
----------
* `type` :
    the type of the core to create
* `name` :
    the name of the core , may be a nullptr or empty string to have a name
    automatically assigned
* `argc` :
    the number of arguments
* `argv` :
    the string values from a command line

Returns
-------
a helics_core object
";

%feature("docstring") helicsBrokerIsConnected "

check if a broker is connected a connected broker implies is attached to cores
or cores could reach out to communicate return 0 if not connected , something
else if it is connected
";

%feature("docstring") helicsCoreIsConnected "

check if a core is connected a connected core implies is attached to federate or
federates could be attached to it return 0 if not connected , something else if
it is connected
";

%feature("docstring") helicsBrokerGetIdentifier "

get an identifier for the broker

Parameters
----------
* `broker` :
    the broker to query
* `identifier` :
    storage space to place the identifier string
* `maxlen` :
    the maximum space available in identifier

Returns
-------
a helics_status enumeration indicating any error condition
";

%feature("docstring") helicsCoreGetIdentifier "

get an identifier for the core

Parameters
----------
* `core` :
    the core to query
* `identifier` :
    storage space to place the identifier string
* `maxlen` :
    the maximum space available in identifier

Returns
-------
a helics_status enumeration indicating any error condition
";

%feature("docstring") helicsBrokerGetAddress "

get the network address associated with a broker

Parameters
----------
* `broker` :
    the broker to query
* `identifier` :
    storage space to place the identifier string
* `maxlen` :
    the maximum space available in identifier

Returns
-------
a helics_status enumeration indicating any error condition
";

%feature("docstring") helicsCoreDisconnect "

get an identifier for the core

Parameters
----------
* `core` :
    the core to query
* `identifier` :
    storage space to place the identifier string
* `maxlen` :
    the maximum space available in identifier

Returns
-------
a helics_status enumeration indicating any error condition
";

%feature("docstring") helicsBrokerDisconnect "

get the network address associated with a broker

Parameters
----------
* `broker` :
    the broker to query
* `identifier` :
    storage space to place the identifier string
* `maxlen` :
    the maximum space available in identifier

Returns
-------
a helics_status enumeration indicating any error condition
";

%feature("docstring") helicsCoreFree "

release the memory associated with a core
";

%feature("docstring") helicsBrokerFree "

release the memory associated with a broker
";

%feature("docstring") helicsCreateValueFederate "

create a value federate from a federate info object

helics_federate objects can be used in all functions that take a helics_federate
or helics_federate object as an argument

Parameters
----------
* `fi` :
    the federate info object that contains details on the federate

Returns
-------
an opaque value federate object
";

%feature("docstring") helicsCreateValueFederateFromJson "

create a value federate from a JSON file or JSON string

helics_federate objects can be used in all functions that take a helics_federate
or helics_federate object as an argument

Parameters
----------
* `JSON` :
    a JSON file or a JSON string that contains setup and configuration
    information

Returns
-------
an opaque value federate object
";

%feature("docstring") helicsCreateMessageFederate "

create a message federate from a federate info object

helics_message_federate objects can be used in all functions that take a
helics_message_federate or helics_federate object as an argument

Parameters
----------
* `fi` :
    the federate info object that contains details on the federate

Returns
-------
an opaque message federate object
";

%feature("docstring") helicsCreateMessageFederateFromJson "

create a message federate from a JSON file or JSON string

helics_message_federate objects can be used in all functions that take a
helics_message_federate or helics_federate object as an argument

Parameters
----------
* `JSON` :
    a JSON file or a JSON string that contains setup and configuration
    information

Returns
-------
an opaque message federate object
";

%feature("docstring") helicsCreateCombinationFederate "

create a combination federate from a federate info object

combination federates are both value federates and message federates, objects
can be used in all functions that take a helics_federate,
helics_message_federate or helics_federate object as an argument

Parameters
----------
* `fi` :
    the federate info object that contains details on the federate

Returns
-------
an opaque value federate object nullptr if the object creation failed
";

%feature("docstring") helicsCreateCombinationFederateFromJson "

create a combination federate from a JSON file or JSON string

combination federates are both value federates and message federates, objects
can be used in all functions that take a helics_federate,
helics_message_federate or helics_federate object as an argument

Parameters
----------
* `JSON` :
    a JSON file or a JSON string that contains setup and configuration
    information

Returns
-------
an opaque combination federate object
";

%feature("docstring") helicsFederateInfoCreate "

create a federate info object for specifying federate information when
constructing a federate

Returns
-------
a helics_federate_info_t object which is a reference to the created object
";

%feature("docstring") helicsFederateInfoLoadFromArgs "

load a federate info from command line arguments

Parameters
----------
* `fi` :
    a federateInfo object
* `argc` :
    the number of command line arguments
* `argv` :
    an array of strings from the command line

Returns
-------
a helics_status enumeration indicating success or any potential errors
";

%feature("docstring") helicsFederateInfoFree "

delete the memory associated with a federate info object
";

%feature("docstring") helicsFederateInfoSetFederateName "

set the federate name in the Federate Info structure

Parameters
----------
* `fi` :
    the federate info object to alter
* `name` :
    the new identifier for the federate

Returns
-------
a helics_status enumeration helics_ok on success
";

%feature("docstring") helicsFederateInfoSetCoreName "

set the name of the core to link to for a federate

Parameters
----------
* `fi` :
    the federate info object to alter
* `corename` :
    the identifier for a core to link to

Returns
-------
a helics_status enumeration helics_ok on success helicsInvalidReference if fi is
not a valid reference
";

%feature("docstring") helicsFederateInfoSetCoreInitString "

set the initialization string for the core usually in the form of command line
arguments

Parameters
----------
* `fi` :
    the federate info object to alter
* `coreInit` :
    a string with the core initialization strings

Returns
-------
a helics_status enumeration helics_ok on success helicsInvalidReference if fi is
not a valid reference
";

%feature("docstring") helicsFederateInfoSetCoreTypeFromString "

set the core type from a string

Parameters
----------
* `fi` :
    the federate info object to alter
* `coretype` :
    a string naming a core type

Returns
-------
a helics_status enumeration helics_ok on success helicsInvalidReference if fi is
not a valid reference helics_discard if the string is not recognized
";

%feature("docstring") helicsFederateInfoSetCoreType "
";

%feature("docstring") helicsFederateInfoSetFlag "
";

%feature("docstring") helicsFederateInfoSetOutputDelay "
";

%feature("docstring") helicsFederateInfoSetTimeDelta "
";

%feature("docstring") helicsFederateInfoSetInputDelay "
";

%feature("docstring") helicsFederateInfoSetTimeOffset "
";

%feature("docstring") helicsFederateInfoSetPeriod "
";

%feature("docstring") helicsFederateInfoSetMaxIterations "
";

%feature("docstring") helicsFederateInfoSetLoggingLevel "
";

%feature("docstring") helicsFederateFinalize "

finalize the federate this halts all communication in the federate and
disconnects it from the core
";

%feature("docstring") helicsFederateFree "

release the memory associated withe a federate
";

%feature("docstring") helicsCloseLibrary "

call when done using the helics library, this function will ensure the threads
are closed properly if possible this should be the last call before exiting,
";

%feature("docstring") helicsFederateEnterInitializationMode "

enter the initialization state of a federate

the initialization state allows initial values to be set and received if the
iteration is requested on entry to the execution state This is a blocking call
and will block until the core allows it to proceed
";

%feature("docstring") helicsFederateEnterInitializationModeAsync "

non blocking alternative to  the function
helicsFederateEnterInitializationModeFinalize must be called to finish the
operation
";

%feature("docstring") helicsFederateIsAsyncOperationCompleted "

check if the current Asynchronous operation has completed

Parameters
----------
* `fed` :
    the federate to operate on

Returns
-------
0 if not completed, 1 if completed
";

%feature("docstring") helicsFederateEnterInitializationModeComplete "

finalize the entry to initialize mode that was initiated with
";

%feature("docstring") helicsFederateEnterExecutionMode "

request that the federate enter the Execution mode

this call is blocking until granted entry by the core object for an asynchronous
alternative call /ref helicsFederateEnterExecutionModeAsync

Parameters
----------
* `fed` :
    a federate to change modes

Returns
-------
a helics_status enumeration helics_error if something went wrong
helicsInvalidReference if fed is invalid
";

%feature("docstring") helicsFederateEnterExecutionModeAsync "

request that the federate enter the Execution mode

this call is non-blocking and will return immediately call /ref
helicsFederateEnterExecutionModeComplete to finish the call sequence /ref
";

%feature("docstring") helicsFederateEnterExecutionModeComplete "

complete the call to /ref EnterExecutionModeAsync

Parameters
----------
* `fed` :
    the federate object to complete the call
";

%feature("docstring") helicsFederateEnterExecutionModeIterative "
";

%feature("docstring") helicsFederateEnterExecutionModeIterativeAsync "
";

%feature("docstring") helicsFederateEnterExecutionModeIterativeComplete "
";

%feature("docstring") helicsFederateGetState "

get the current state of a federate

Parameters
----------
* `fed` :
    the fed to query
* `state` :
    the resulting state if helics_status return helics_ok
";

%feature("docstring") helicsFederateGetCoreObject "

get the core object associated with a federate

Parameters
----------
* `fed` :
    a federate object

Returns
-------
a core object, nullptr if invalid
";

%feature("docstring") helicsFederateRequestTime "

request the next time for federate execution

Parameters
----------
* `fed` :
    the federate to make the request of
* `requestTime` :
    the next requested time
* `timeOut` :
    the time granted to the federate

Returns
-------
a helics_status if the return value is equal to helics_ok the timeOut will
contain the new granted time, otherwise timeOut is invalid
";

%feature("docstring") helicsFederateRequestTimeIterative "

request an iterative time

this call allows for finer grain control of the iterative process then /ref
helicsFederateRequestTime it takes a time and and iteration request and return a
time and iteration status

Parameters
----------
* `fed` :
    the federate to make the request of
* `requestTime` :
    the next desired time
* `iterate` :
    the requested iteration mode
* `timeOut` :
    the granted time
* `outIterate` :
    the iteration specification of the result

Returns
-------
a helics_status object with a return code of the result
";

%feature("docstring") helicsFederateRequestTimeAsync "
";

%feature("docstring") helicsFederateRequestTimeComplete "
";

%feature("docstring") helicsFederateRequestTimeIterativeAsync "
";

%feature("docstring") helicsFederateRequestTimeIterativeComplete "
";

%feature("docstring") helicsFederateGetName "

get the name of the federate

Parameters
----------
* `fed` :
    the federate object to query
* `str` :
    memory buffer to store the result
* `maxlen` :
    the maximum size of the buffer

Returns
-------
helics_status object indicating success or error
";

%feature("docstring") helicsFederateSetTimeDelta "

set the minimum time delta for the federate

Parameters
----------
* `tdelta` :
    the minimum time delta to return from a time request function
";

%feature("docstring") helicsFederateSetOutputDelay "

set the look ahead time

the look ahead is the propagation time for messages/event to propagate from the
Federate the federate

Parameters
----------
* `lookAhead` :
    the look ahead time
";

%feature("docstring") helicsFederateSetInputDelay "

set the impact Window time

the impact window is the time window around the time request in which other
federates cannot affect the federate

Parameters
----------
* `lookAhead` :
    the look ahead time
";

%feature("docstring") helicsFederateSetPeriod "

set the period and offset of the federate

the federate will on grant time on N*period+offset interval

Parameters
----------
* `period` :
    the length of time between each subsequent grants
* `offset` :
    the shift of the period from 0 offset must be < period
";

%feature("docstring") helicsFederateSetFlag "

set a flag for the federate

Parameters
----------
* `fed` :
    the federate to alter a flag for
* `flag` :
    the flag to change
* `flagValue` :
    the new value of the flag 0 for false !=0 for true
";

%feature("docstring") helicsFederateSetLoggingLevel "

set the logging level for the federate @ details debug and trace only do
anything if they were enabled in the compilation

Parameters
----------
* `loggingLevel` :
    (-1: none, 0: error_only, 1: warnings, 2: normal, 3: debug, 4: trace)
";

%feature("docstring") helicsFederateGetCurrentTime "

get the current time of the federate

Parameters
----------
* `fed` :
    the federate object to query
* `time` :
    storage location for the time variable

Returns
-------
helics_status object indicating success or error
";

%feature("docstring") helicsCreateQuery "

create a query object

a query object consists of a target and query string
";

%feature("docstring") helicsQueryExecute "

Execute a query

the call will block until the query finishes which may require communication or
other delays

Parameters
----------
* `query` :
    the query object to use in the query
* `fed` :
    a federate to send the query through

Returns
-------
a pointer to a string. the string will remain valid until the query is freed or
executed again the return will be nullptr if fed or query is an invalid object
";

%feature("docstring") helicsQueryExecuteAsync "

Execute a query in a non-blocking call

Parameters
----------
* `query` :
    the query object to use in the query
* `fed` :
    a federate to send the query through

Returns
-------
a helics status enumeration with the result of the query specification
";

%feature("docstring") helicsQueryExecuteComplete "

complete the return from a query called with /ref helicsExecuteQueryAsync

the function will block until the query completes /ref isQueryComplete can be
called to determine if a query has completed or not

Parameters
----------
* `query` :
    the query object to

Returns
-------
a pointer to a string. the string will remain valid until the query is freed or
executed again the return will be nullptr if query is an invalid object
";

%feature("docstring") helicsQueryIsCompleted "

check if an asynchronously executed query has completed

Returns
-------
will return helics_true if an async query has complete or a regular query call
was made with a result and false if an async query has not completed or is
invalid
";

%feature("docstring") helicsQueryFree "

free the memory associated with a query object
";

// File: cpp98_2helics_8hpp.xml

// File: helics_8hpp.xml

// File: helicsExport_8cpp.xml

%feature("docstring") versionStr "
";

%feature("docstring") helicsGetVersion "
";

%feature("docstring") helicsIsCoreTypeAvailable "

Returns true if core/broker type specified is available in current compilation.
";

%feature("docstring") helicsFederateInfoCreate "

create a federate info object for specifying federate information when
constructing a federate

Returns
-------
a helics_federate_info_t object which is a reference to the created object
";

%feature("docstring") helicsFederateInfoFree "

delete the memory associated with a federate info object
";

%feature("docstring") helicsFederateInfoLoadFromArgs "

load a federate info from command line arguments

Parameters
----------
* `fi` :
    a federateInfo object
* `argc` :
    the number of command line arguments
* `argv` :
    an array of strings from the command line

Returns
-------
a helics_status enumeration indicating success or any potential errors
";

%feature("docstring") helicsFederateInfoSetFederateName "

set the federate name in the Federate Info structure

Parameters
----------
* `fi` :
    the federate info object to alter
* `name` :
    the new identifier for the federate

Returns
-------
a helics_status enumeration helics_ok on success
";

%feature("docstring") helicsFederateInfoSetCoreName "

set the name of the core to link to for a federate

Parameters
----------
* `fi` :
    the federate info object to alter
* `corename` :
    the identifier for a core to link to

Returns
-------
a helics_status enumeration helics_ok on success helicsInvalidReference if fi is
not a valid reference
";

%feature("docstring") helicsFederateInfoSetCoreInitString "

set the initialization string for the core usually in the form of command line
arguments

Parameters
----------
* `fi` :
    the federate info object to alter
* `coreInit` :
    a string with the core initialization strings

Returns
-------
a helics_status enumeration helics_ok on success helicsInvalidReference if fi is
not a valid reference
";

%feature("docstring") helicsFederateInfoSetCoreType "
";

%feature("docstring") helicsFederateInfoSetCoreTypeFromString "

set the core type from a string

Parameters
----------
* `fi` :
    the federate info object to alter
* `coretype` :
    a string naming a core type

Returns
-------
a helics_status enumeration helics_ok on success helicsInvalidReference if fi is
not a valid reference helics_discard if the string is not recognized
";

%feature("docstring") helicsFederateInfoSetFlag "
";

%feature("docstring") helicsFederateInfoSetOutputDelay "
";

%feature("docstring") helicsFederateInfoSetTimeDelta "
";

%feature("docstring") helicsFederateInfoSetInputDelay "
";

%feature("docstring") helicsFederateInfoSetTimeOffset "
";

%feature("docstring") helicsFederateInfoSetPeriod "
";

%feature("docstring") helicsFederateInfoSetLoggingLevel "
";

%feature("docstring") helicsFederateInfoSetMaxIterations "
";

%feature("docstring") getCore "
";

%feature("docstring") getCoreSharedPtr "
";

%feature("docstring") getBroker "
";

%feature("docstring") getBrokerSharedPtr "
";

%feature("docstring") helicsCreateCore "

create a core object

Parameters
----------
* `type` :
    the type of the core to create
* `name` :
    the name of the core , may be a nullptr or empty string to have a name
    automatically assigned
* `initString` :
    an initialization string to send to the core-the format is similar to
    command line arguments typical options include a broker address
    --broker=\"XSSAF\" or the number of federates or the address

Returns
-------
a helics_core object
";

%feature("docstring") helicsCreateCoreFromArgs "

create a core object by passing command line arguments

Parameters
----------
* `type` :
    the type of the core to create
* `name` :
    the name of the core , may be a nullptr or empty string to have a name
    automatically assigned
* `argc` :
    the number of arguments
* `argv` :
    the string values from a command line

Returns
-------
a helics_core object
";

%feature("docstring") helicsCreateBroker "

create a broker object

Parameters
----------
* `type` :
    the type of the broker to create
* `name` :
    the name of the broker , may be a nullptr or empty string to have a name
    automatically assigned
* `initString` :
    an initialization string to send to the core-the format is similar to
    command line arguments typical options include a broker address
    --broker=\"XSSAF\" if this is a subbroker or the number of federates or the
    address

Returns
-------
a helics_core object
";

%feature("docstring") helicsCreateBrokerFromArgs "

create a core object by passing command line arguments

Parameters
----------
* `type` :
    the type of the core to create
* `name` :
    the name of the core , may be a nullptr or empty string to have a name
    automatically assigned
* `argc` :
    the number of arguments
* `argv` :
    the string values from a command line

Returns
-------
a helics_core object
";

%feature("docstring") helicsBrokerIsConnected "

check if a broker is connected a connected broker implies is attached to cores
or cores could reach out to communicate return 0 if not connected , something
else if it is connected
";

%feature("docstring") helicsCoreIsConnected "

check if a core is connected a connected core implies is attached to federate or
federates could be attached to it return 0 if not connected , something else if
it is connected
";

%feature("docstring") helicsBrokerGetIdentifier "

get an identifier for the broker

Parameters
----------
* `broker` :
    the broker to query
* `identifier` :
    storage space to place the identifier string
* `maxlen` :
    the maximum space available in identifier

Returns
-------
a helics_status enumeration indicating any error condition
";

%feature("docstring") helicsCoreGetIdentifier "

get an identifier for the core

Parameters
----------
* `core` :
    the core to query
* `identifier` :
    storage space to place the identifier string
* `maxlen` :
    the maximum space available in identifier

Returns
-------
a helics_status enumeration indicating any error condition
";

%feature("docstring") helicsBrokerGetAddress "

get the network address associated with a broker

Parameters
----------
* `broker` :
    the broker to query
* `identifier` :
    storage space to place the identifier string
* `maxlen` :
    the maximum space available in identifier

Returns
-------
a helics_status enumeration indicating any error condition
";

%feature("docstring") helicsCoreDisconnect "

get an identifier for the core

Parameters
----------
* `core` :
    the core to query
* `identifier` :
    storage space to place the identifier string
* `maxlen` :
    the maximum space available in identifier

Returns
-------
a helics_status enumeration indicating any error condition
";

%feature("docstring") helicsBrokerDisconnect "

get the network address associated with a broker

Parameters
----------
* `broker` :
    the broker to query
* `identifier` :
    storage space to place the identifier string
* `maxlen` :
    the maximum space available in identifier

Returns
-------
a helics_status enumeration indicating any error condition
";

%feature("docstring") helicsCoreFree "

release the memory associated with a core
";

%feature("docstring") helicsBrokerFree "

release the memory associated with a broker
";

%feature("docstring") helicsFederateFree "

release the memory associated withe a federate
";

%feature("docstring") helicsCloseLibrary "

call when done using the helics library, this function will ensure the threads
are closed properly if possible this should be the last call before exiting,
";

%feature("docstring") helicsCreateQuery "

create a query object

a query object consists of a target and query string
";

%feature("docstring") helicsQueryExecute "

Execute a query

the call will block until the query finishes which may require communication or
other delays

Parameters
----------
* `query` :
    the query object to use in the query
* `fed` :
    a federate to send the query through

Returns
-------
a pointer to a string. the string will remain valid until the query is freed or
executed again the return will be nullptr if fed or query is an invalid object
";

%feature("docstring") helicsQueryExecuteAsync "

Execute a query in a non-blocking call

Parameters
----------
* `query` :
    the query object to use in the query
* `fed` :
    a federate to send the query through

Returns
-------
a helics status enumeration with the result of the query specification
";

%feature("docstring") helicsQueryExecuteComplete "

complete the return from a query called with /ref helicsExecuteQueryAsync

the function will block until the query completes /ref isQueryComplete can be
called to determine if a query has completed or not

Parameters
----------
* `query` :
    the query object to

Returns
-------
a pointer to a string. the string will remain valid until the query is freed or
executed again the return will be nullptr if query is an invalid object
";

%feature("docstring") helicsQueryIsCompleted "

check if an asynchronously executed query has completed

Returns
-------
will return helics_true if an async query has complete or a regular query call
was made with a result and false if an async query has not completed or is
invalid
";

%feature("docstring") helicsQueryFree "

free the memory associated with a query object
";

// File: helicsPrimaryTypes_8cpp.xml

// File: HelicsPrimaryTypes_8hpp.xml

// File: helicsTypes_8cpp.xml

// File: helicsTypes_8hpp.xml

// File: helicsVersion_8cpp.xml

// File: helicsVersion_8hpp.xml

// File: IpcBroker_8cpp.xml

// File: IpcBroker_8h.xml

// File: IpcComms_8cpp.xml

// File: IpcComms_8h.xml

// File: IpcCore_8cpp.xml

// File: IpcCore_8h.xml

// File: IpcQueueHelper_8cpp.xml

// File: IpcQueueHelper_8h.xml

// File: logger_8cpp.xml

// File: logger_8h.xml

// File: loggingHelper_8hpp.xml

// File: MappedPointerVector_8hpp.xml

// File: MappedVector_8hpp.xml

// File: Message_8cpp.xml

// File: Message_8hpp.xml

// File: MessageFederate_8cpp.xml

// File: MessageFederate_8h.xml

%feature("docstring") helicsFederateRegisterEndpoint "
";

%feature("docstring") helicsFederateRegisterGlobalEndpoint "
";

%feature("docstring") helicsEndpointSetDefaultDestination "
";

%feature("docstring") helicsEndpointSendMessageRaw "
";

%feature("docstring") helicsEndpointSendEventRaw "
";

%feature("docstring") helicsEndpointSendMessage "
";

%feature("docstring") helicsEndpointSubscribe "

subscribe an endpoint to a publication

Parameters
----------
* `endpoint` :
    the endpoint to use
* `key` :
    the name of the publication
* `type` :
    the type of the publication that is expected (nullptr or \"\" for DON'T
    CARE)
";

%feature("docstring") helicsFederateHasMessage "

check if the federate has any outstanding messages
";

%feature("docstring") helicsEndpointHasMessage "
";

%feature("docstring") helicsFederateReceiveCount "

Returns the number of pending receives for the specified destination endpoint.
";

%feature("docstring") helicsEndpointReceiveCount "

Returns the number of pending receives for all endpoints of particular federate.
";

%feature("docstring") helicsEndpointGetMessage "

receive a packet from a particular endpoint

Parameters
----------
* `endpoint` :
    the identifier for the endpoint

Returns
-------
a message object
";

%feature("docstring") helicsFederateGetMessage "

receive a communication message for any endpoint in the federate

the return order will be in order of endpoint creation then order of arrival all
messages for the first endpoint, then all for the second, and so on

Returns
-------
a unique_ptr to a Message object containing the message data
";

%feature("docstring") helicsEndpointGetType "

get the type specified for an endpoint

Parameters
----------
* `endpoint` :
    the endpoint object in question
* `str` :
    the location where the string is stored
* `maxlen` :
    the maximum string length that can be stored in str

Returns
-------
a status variable
";

%feature("docstring") helicsEndpointGetName "

get the name of an endpoint

Parameters
----------
* `endpoint` :
    the endpoint object in question
* `str` :
    the location where the string is stored
* `maxlen` :
    the maximum string length that can be stored in str

Returns
-------
a status variable
";

// File: application__api_2MessageFederate_8hpp.xml

// File: cpp98_2MessageFederate_8hpp.xml

// File: MessageFederateExport_8cpp.xml

%feature("docstring") addEndpoint "
";

%feature("docstring") helicsFederateRegisterEndpoint "
";

%feature("docstring") helicsFederateRegisterGlobalEndpoint "
";

%feature("docstring") helicsEndpointSetDefaultDestination "
";

%feature("docstring") helicsEndpointSendMessageRaw "
";

%feature("docstring") helicsEndpointSendEventRaw "
";

%feature("docstring") helicsEndpointSendMessage "
";

%feature("docstring") helicsEndpointSubscribe "

subscribe an endpoint to a publication

Parameters
----------
* `endpoint` :
    the endpoint to use
* `key` :
    the name of the publication
* `type` :
    the type of the publication that is expected (nullptr or \"\" for DON'T
    CARE)
";

%feature("docstring") helicsFederateHasMessage "

check if the federate has any outstanding messages
";

%feature("docstring") helicsEndpointHasMessage "
";

%feature("docstring") helicsFederateReceiveCount "

Returns the number of pending receives for the specified destination endpoint.
";

%feature("docstring") helicsEndpointReceiveCount "

Returns the number of pending receives for all endpoints of particular federate.
";

%feature("docstring") emptyMessage "
";

%feature("docstring") helicsEndpointGetMessage "

receive a packet from a particular endpoint

Parameters
----------
* `endpoint` :
    the identifier for the endpoint

Returns
-------
a message object
";

%feature("docstring") helicsFederateGetMessage "

receive a communication message for any endpoint in the federate

the return order will be in order of endpoint creation then order of arrival all
messages for the first endpoint, then all for the second, and so on

Returns
-------
a unique_ptr to a Message object containing the message data
";

%feature("docstring") helicsEndpointGetType "

get the type specified for an endpoint

Parameters
----------
* `endpoint` :
    the endpoint object in question
* `str` :
    the location where the string is stored
* `maxlen` :
    the maximum string length that can be stored in str

Returns
-------
a status variable
";

%feature("docstring") helicsEndpointGetName "

get the name of an endpoint

Parameters
----------
* `endpoint` :
    the endpoint object in question
* `str` :
    the location where the string is stored
* `maxlen` :
    the maximum string length that can be stored in str

Returns
-------
a status variable
";

// File: MessageFederateManager_8cpp.xml

// File: MessageFederateManager_8hpp.xml

// File: MessageFilters_8h.xml

%feature("docstring") helicsFederateRegisterSourceFilter "

create a source Filter on the specified federate

filters can be created through a federate or a core , linking through a federate
allows a few extra features of name matching to function on the federate
interface but otherwise equivalent behavior

Parameters
----------
* `fed` :
    the fed to register through
* `name` :
    the name of the filter (can be nullptr)
* `inputType` :
    the input type of the filter, used for ordering (can be nullptr)
* `outputType` :
    the output type of the filter, used for ordering (can be nullptr)

Returns
-------
a helics_source_filter object
";

%feature("docstring") helicsFederateRegisterDestinationFilter "
";

%feature("docstring") helicsFederateRegisterCloningFilter "
";

%feature("docstring") helicsCoreRegisterSourceFilter "
";

%feature("docstring") helicsCoreRegisterDestinationFilter "
";

%feature("docstring") helicsCoreRegisterCloningFilter "
";

%feature("docstring") helicsFilterGetTarget "

get the target of the filter
";

%feature("docstring") helicsFilterGetName "

get the name of the filter
";

%feature("docstring") helicsFilterSet "
";

%feature("docstring") setString "
";

%feature("docstring") helicsFilterAddDestinationTarget "
";

%feature("docstring") helicsFilterAddSourceTarget "
";

%feature("docstring") helicsFilterAddDeliveryEndpoint "
";

%feature("docstring") helicsFilterRemoveDestinationTarget "
";

%feature("docstring") helicsFilterRemoveSourceTarget "
";

%feature("docstring") helicsFilterRemoveDeliveryEndpoint "
";

// File: MessageFiltersExport_8cpp.xml

%feature("docstring") federateAddFilter "
";

%feature("docstring") coreAddFilter "
";

%feature("docstring") helicsFederateRegisterSourceFilter "

create a source Filter on the specified federate

filters can be created through a federate or a core , linking through a federate
allows a few extra features of name matching to function on the federate
interface but otherwise equivalent behavior

Parameters
----------
* `fed` :
    the fed to register through
* `name` :
    the name of the filter (can be nullptr)
* `inputType` :
    the input type of the filter, used for ordering (can be nullptr)
* `outputType` :
    the output type of the filter, used for ordering (can be nullptr)

Returns
-------
a helics_source_filter object
";

%feature("docstring") helicsFederateRegisterDestinationFilter "
";

%feature("docstring") helicsCoreRegisterSourceFilter "
";

%feature("docstring") helicsCoreRegisterDestinationFilter "
";

%feature("docstring") helicsFederateRegisterCloningFilter "
";

%feature("docstring") helicsCoreRegisterCloningFilter "
";

%feature("docstring") getFilter "
";

%feature("docstring") getCloningFilter "
";

%feature("docstring") helicsFilterGetTarget "

get the target of the filter
";

%feature("docstring") helicsFilterGetName "

get the name of the filter
";

%feature("docstring") helicsFilterSet "
";

%feature("docstring") setString "
";

%feature("docstring") helicsFilterAddDestinationTarget "
";

%feature("docstring") helicsFilterAddSourceTarget "
";

%feature("docstring") helicsFilterAddDeliveryEndpoint "
";

%feature("docstring") helicsFilterRemoveDestinationTarget "
";

%feature("docstring") helicsFilterRemoveSourceTarget "
";

%feature("docstring") helicsFilterRemoveDeliveryEndpoint "
";

// File: MessageOperators_8cpp.xml

// File: MessageOperators_8hpp.xml

// File: MpiBroker_8cpp.xml

// File: MpiBroker_8h.xml

// File: MpiComms_8cpp.xml

// File: MpiComms_8h.xml

// File: MpiCore_8cpp.xml

// File: MpiCore_8h.xml

// File: NetworkBrokerData_8cpp.xml

// File: NetworkBrokerData_8hpp.xml

// File: player_8cpp.xml

%feature("docstring") helics::playerArgumentParser "
";

// File: player_8h.xml

// File: playerMain_8cpp.xml

%feature("docstring") main "
";

// File: PrecHelper_8cpp.xml

%feature("docstring") getType "
";

%feature("docstring") typeCharacter "
";

// File: PrecHelper_8h.xml

%feature("docstring") helics::getType "
";

%feature("docstring") helics::typeCharacter "
";

// File: PublicationInfo_8cpp.xml

// File: PublicationInfo_8hpp.xml

// File: Publications_8cpp.xml

// File: Publications_8hpp.xml

// File: queryFunctions_8cpp.xml

%feature("docstring") vectorizeQueryResult "

function takes a query result and vectorizes it if the query is a vector result,
if not the results go into the first element of the vector
";

%feature("docstring") vectorizeQueryResult "

function takes a query result and vectorizes it if the query is a vector result,
if not the results go into the first element of the vector
";

%feature("docstring") vectorizeAndSortQueryResult "

function takes a query result, vectorizes and sorts it if the query is a vector
result, if not the results go into the first element of the vector
";

%feature("docstring") vectorizeAndSortQueryResult "

function takes a query result, vectorizes and sorts it if the query is a vector
result, if not the results go into the first element of the vector
";

%feature("docstring") waitForInit "

helper function to wait for a particular federate has requested initialization
mode

this is useful for querying information and being reasonably certain the
federate is done adding to its interface

Parameters
----------
* `fed` :
    a pointer to the federate
* `fedName` :
    the name of the federate we are querying

Returns
-------
true if the federate is now trying to enter initialization false if the timeout
was reached
";

%feature("docstring") waitForFed "

helper function to wait for a particular federate to be created

this is useful if some reason we need to make sure a federate is created before
proceeding

Parameters
----------
* `fed` :
    a pointer to the federate
* `fedName` :
    the name of the federate we are querying
* `timeout` :
    the amount of time in ms to wait before returning false

Returns
-------
true if the federate exists, false if the timeout occurred
";

// File: queryFunctions_8hpp.xml

%feature("docstring") helics::vectorizeQueryResult "

function takes a query result and vectorizes it if the query is a vector result,
if not the results go into the first element of the vector
";

%feature("docstring") helics::vectorizeQueryResult "

function takes a query result and vectorizes it if the query is a vector result,
if not the results go into the first element of the vector
";

%feature("docstring") helics::vectorizeAndSortQueryResult "

function takes a query result, vectorizes and sorts it if the query is a vector
result, if not the results go into the first element of the vector
";

%feature("docstring") helics::vectorizeAndSortQueryResult "

function takes a query result, vectorizes and sorts it if the query is a vector
result, if not the results go into the first element of the vector
";

%feature("docstring") helics::waitForInit "

helper function to wait for a particular federate has requested initialization
mode

this is useful for querying information and being reasonably certain the
federate is done adding to its interface

Parameters
----------
* `fed` :
    a pointer to the federate
* `fedName` :
    the name of the federate we are querying

Returns
-------
true if the federate is now trying to enter initialization false if the timeout
was reached
";

%feature("docstring") helics::waitForFed "

helper function to wait for a particular federate to be created

this is useful if some reason we need to make sure a federate is created before
proceeding

Parameters
----------
* `fed` :
    a pointer to the federate
* `fedName` :
    the name of the federate we are querying
* `timeout` :
    the amount of time in ms to wait before returning false

Returns
-------
true if the federate exists, false if the timeout occurred
";

// File: recorder_8cpp.xml

%feature("docstring") helics::recorderArgumentParser "
";

// File: recorder_8h.xml

// File: recorderMain_8cpp.xml

%feature("docstring") main "
";

// File: searchableObjectHolder_8hpp.xml

// File: simpleQueue_8hpp.xml

// File: source_8cpp.xml

%feature("docstring") helics::sourceArgumentParser "
";

// File: source_8h.xml

// File: stringOps_8cpp.xml

%feature("docstring") stringOps::convertToLowerCase "

convert a string to lower case as a new string

Parameters
----------
* `input` :
    the string to convert

Returns
-------
the string with all upper case converted to lower case
";

%feature("docstring") stringOps::convertToUpperCase "

convert a string to upper case as a new string

Parameters
----------
* `input` :
    the string to convert

Returns
-------
the string with all lower case letters converted to upper case
";

%feature("docstring") stringOps::makeLowerCase "

make a string lower case

Parameters
----------
* `input` :
    the string to convert
";

%feature("docstring") stringOps::makeUpperCase "

make a string upper case

Parameters
----------
* `input` :
    the string to convert
";

%feature("docstring") stringOps::while "
";

// File: stringOps_8h.xml

%feature("docstring") convertToLowerCase "

convert a string to lower case as a new string

Parameters
----------
* `input` :
    the string to convert

Returns
-------
the string with all upper case converted to lower case
";

%feature("docstring") convertToUpperCase "

convert a string to upper case as a new string

Parameters
----------
* `input` :
    the string to convert

Returns
-------
the string with all lower case letters converted to upper case
";

%feature("docstring") makeLowerCase "

make a string lower case

Parameters
----------
* `input` :
    the string to convert
";

%feature("docstring") makeUpperCase "

make a string upper case

Parameters
----------
* `input` :
    the string to convert
";

// File: stringToCmdLine_8cpp.xml

// File: stringToCmdLine_8h.xml

// File: SubscriptionInfo_8cpp.xml

// File: SubscriptionInfo_8hpp.xml

// File: Subscriptions_8cpp.xml

// File: Subscriptions_8hpp.xml

// File: TcpBroker_8cpp.xml

// File: TcpBroker_8h.xml

// File: TcpComms_8cpp.xml

// File: TcpComms_8h.xml

// File: TcpCore_8cpp.xml

// File: TcpCore_8h.xml

// File: TcpHelperClasses_8cpp.xml

// File: TcpHelperClasses_8h.xml

// File: TestBroker_8cpp.xml

// File: TestBroker_8h.xml

// File: TestCore_8cpp.xml

// File: TestCore_8h.xml

// File: TimeCoordinator_8cpp.xml

// File: TimeCoordinator_8hpp.xml

// File: TimeDependencies_8cpp.xml

// File: TimeDependencies_8hpp.xml

// File: timeRepresentation_8hpp.xml

%feature("docstring") pow2 "

generate powers to two as a constexpr

Parameters
----------
* `exponent` :
    the power of 2 desired
";

// File: UdpBroker_8cpp.xml

// File: UdpBroker_8h.xml

// File: UdpComms_8cpp.xml

// File: UdpComms_8h.xml

// File: UdpCore_8cpp.xml

// File: UdpCore_8h.xml

// File: ValueConverter_8cpp.xml

// File: ValueConverter_8hpp.xml

// File: ValueConverter__impl_8hpp.xml

// File: ValueFederate_8cpp.xml

// File: ValueFederate_8h.xml

%feature("docstring") helicsFederateRegisterSubscription "

create a subscription

the subscription becomes part of the federate and is destroyed when the federate
is freed so there are no separate free functions for subscriptions and
publications

Parameters
----------
* `fed` :
    the federate object in which to create a subscription must have been create
    with helicsCreateValueFederate or helicsCreateCombinationFederate
* `key` :
    the identifier matching a publication to get a subscription for
* `type` :
    a string describing the expected type of the publication may be NULL
* `units` :
    a string listing the units of the subscription maybe NULL

Returns
-------
an object containing the subscription
";

%feature("docstring") helicsFederateRegisterTypeSubscription "

create a subscription of a specific known type

the subscription becomes part of the federate and is destroyed when the federate
is freed so there are no separate free functions for subscriptions and
publications

Parameters
----------
* `fed` :
    the federate object in which to create a subscription
* `key` :
    the identifier matching a publication to get a subscription for
* `type` :
    a known type identifier HELICS_STRING_TYPE, HELICS_INT_TYPE,
    HELICS_DOUBLE_TYPE, HELICS_COMPLEX_TYPE, HELICS_VECTOR_TYPE, HELICS_RAW_TYPE
* `units` :
    a string listing the units of the subscription maybe NULL

Returns
-------
an object containing the subscription
";

%feature("docstring") helicsFederateRegisterOptionalSubscription "

create a subscription that is specifically stated to be optional

the subscription becomes part of the federate and is destroyed when the federate
is freed so there are no separate free functions for subscriptions and
publications

optional implies that there may or may not be matching publication elsewhere in
the federation

Parameters
----------
* `fed` :
    the federate object in which to create a subscription
* `key` :
    the identifier matching a publication to get a subscription for
* `type` :
    a string describing the expected type of the publication may be NULL
* `units` :
    a string listing the units of the subscription maybe NULL

Returns
-------
an object containing the subscription
";

%feature("docstring") helicsFederateRegisterOptionalTypeSubscription "

create a subscription of a specific known type that is specifically stated to be
optional

the subscription becomes part of the federate and is destroyed when the federate
is freed so there are no separate free functions for subscriptions and
publications optional implies that there may or may not be matching publication
elsewhere in the federation

Parameters
----------
* `fed` :
    the federate object in which to create a subscription
* `key` :
    the identifier matching a publication to get a subscription for
* `type` :
    a known type identifier HELICS_STRING_TYPE, HELICS_INT_TYPE,
    HELICS_DOUBLE_TYPE, HELICS_COMPLEX_TYPE, HELICS_VECTOR_TYPE, HELICS_RAW_TYPE
* `units` :
    a string listing the units of the subscription maybe NULL

Returns
-------
an object containing the subscription
";

%feature("docstring") helicsFederateRegisterPublication "
";

%feature("docstring") helicsFederateRegisterTypePublication "
";

%feature("docstring") helicsFederateRegisterGlobalPublication "
";

%feature("docstring") helicsFederateRegisterGlobalTypePublication "
";

%feature("docstring") helicsPublicationPublish "
";

%feature("docstring") helicsPublicationPublishString "
";

%feature("docstring") helicsPublicationPublishInteger "
";

%feature("docstring") helicsPublicationPublishDouble "
";

%feature("docstring") helicsPublicationPublishComplex "
";

%feature("docstring") helicsPublicationPublishVector "
";

%feature("docstring") helicsSubscriptionGetValueSize "
";

%feature("docstring") helicsSubscriptionGetValue "
";

%feature("docstring") helicsSubscriptionGetString "
";

%feature("docstring") helicsSubscriptionGetInteger "
";

%feature("docstring") helicsSubscriptionGetDouble "
";

%feature("docstring") helicsSubscriptionGetComplex "
";

%feature("docstring") helicsSubscriptionGetVectorSize "
";

%feature("docstring") helicsSubscriptionGetVector "

get a vector from a subscription

Parameters
----------
* `sub` :
    the subscription to get the result for
* `data` :
    the location to store the data
* `maxlen` :
    the maximum size of the vector
* `actualSize` :
    pointer to variable to store the actual size
";

%feature("docstring") helicsSubscriptionSetDefault "
";

%feature("docstring") helicsSubscriptionSetDefaultString "
";

%feature("docstring") helicsSubscriptionSetDefaultInteger "
";

%feature("docstring") helicsSubscriptionSetDefaultDouble "
";

%feature("docstring") helicsSubscriptionSetDefaultComplex "
";

%feature("docstring") helicsSubscriptionSetDefaultVector "
";

%feature("docstring") helicsSubscriptionGetType "
";

%feature("docstring") helicsPublicationGetType "
";

%feature("docstring") helicsSubscriptionGetKey "
";

%feature("docstring") helicsPublicationGetKey "
";

%feature("docstring") helicsSubscriptionGetUnits "
";

%feature("docstring") helicsPublicationGetUnits "
";

%feature("docstring") helicsSubscriptionIsUpdated "
";

%feature("docstring") helicsSubscriptionLastUpdateTime "
";

// File: application__api_2ValueFederate_8hpp.xml

// File: cpp98_2ValueFederate_8hpp.xml

// File: ValueFederateExport_8cpp.xml

%feature("docstring") addSubscription "
";

%feature("docstring") addPublication "
";

%feature("docstring") helicsFederateRegisterSubscription "

create a subscription

the subscription becomes part of the federate and is destroyed when the federate
is freed so there are no separate free functions for subscriptions and
publications

Parameters
----------
* `fed` :
    the federate object in which to create a subscription must have been create
    with helicsCreateValueFederate or helicsCreateCombinationFederate
* `key` :
    the identifier matching a publication to get a subscription for
* `type` :
    a string describing the expected type of the publication may be NULL
* `units` :
    a string listing the units of the subscription maybe NULL

Returns
-------
an object containing the subscription
";

%feature("docstring") helicsFederateRegisterTypeSubscription "

create a subscription of a specific known type

the subscription becomes part of the federate and is destroyed when the federate
is freed so there are no separate free functions for subscriptions and
publications

Parameters
----------
* `fed` :
    the federate object in which to create a subscription
* `key` :
    the identifier matching a publication to get a subscription for
* `type` :
    a known type identifier HELICS_STRING_TYPE, HELICS_INT_TYPE,
    HELICS_DOUBLE_TYPE, HELICS_COMPLEX_TYPE, HELICS_VECTOR_TYPE, HELICS_RAW_TYPE
* `units` :
    a string listing the units of the subscription maybe NULL

Returns
-------
an object containing the subscription
";

%feature("docstring") helicsFederateRegisterOptionalSubscription "

create a subscription that is specifically stated to be optional

the subscription becomes part of the federate and is destroyed when the federate
is freed so there are no separate free functions for subscriptions and
publications

optional implies that there may or may not be matching publication elsewhere in
the federation

Parameters
----------
* `fed` :
    the federate object in which to create a subscription
* `key` :
    the identifier matching a publication to get a subscription for
* `type` :
    a string describing the expected type of the publication may be NULL
* `units` :
    a string listing the units of the subscription maybe NULL

Returns
-------
an object containing the subscription
";

%feature("docstring") helicsFederateRegisterOptionalTypeSubscription "

create a subscription of a specific known type that is specifically stated to be
optional

the subscription becomes part of the federate and is destroyed when the federate
is freed so there are no separate free functions for subscriptions and
publications optional implies that there may or may not be matching publication
elsewhere in the federation

Parameters
----------
* `fed` :
    the federate object in which to create a subscription
* `key` :
    the identifier matching a publication to get a subscription for
* `type` :
    a known type identifier HELICS_STRING_TYPE, HELICS_INT_TYPE,
    HELICS_DOUBLE_TYPE, HELICS_COMPLEX_TYPE, HELICS_VECTOR_TYPE, HELICS_RAW_TYPE
* `units` :
    a string listing the units of the subscription maybe NULL

Returns
-------
an object containing the subscription
";

%feature("docstring") helicsFederateRegisterPublication "
";

%feature("docstring") helicsFederateRegisterTypePublication "
";

%feature("docstring") helicsFederateRegisterGlobalPublication "
";

%feature("docstring") helicsFederateRegisterGlobalTypePublication "
";

%feature("docstring") helicsPublicationPublish "
";

%feature("docstring") helicsPublicationPublishString "
";

%feature("docstring") helicsPublicationPublishInteger "
";

%feature("docstring") helicsPublicationPublishDouble "
";

%feature("docstring") helicsPublicationPublishComplex "
";

%feature("docstring") helicsPublicationPublishVector "
";

%feature("docstring") helicsSubscriptionGetValueSize "
";

%feature("docstring") helicsSubscriptionGetValue "
";

%feature("docstring") helicsSubscriptionGetString "
";

%feature("docstring") helicsSubscriptionGetInteger "
";

%feature("docstring") helicsSubscriptionGetDouble "
";

%feature("docstring") helicsSubscriptionGetComplex "
";

%feature("docstring") helicsSubscriptionGetVectorSize "
";

%feature("docstring") helicsSubscriptionGetVector "

get a vector from a subscription

Parameters
----------
* `sub` :
    the subscription to get the result for
* `data` :
    the location to store the data
* `maxlen` :
    the maximum size of the vector
* `actualSize` :
    pointer to variable to store the actual size
";

%feature("docstring") helicsSubscriptionSetDefault "
";

%feature("docstring") helicsSubscriptionSetDefaultString "
";

%feature("docstring") helicsSubscriptionSetDefaultInteger "
";

%feature("docstring") helicsSubscriptionSetDefaultDouble "
";

%feature("docstring") helicsSubscriptionSetDefaultComplex "
";

%feature("docstring") helicsSubscriptionSetDefaultVector "
";

%feature("docstring") helicsSubscriptionGetType "
";

%feature("docstring") helicsPublicationGetType "
";

%feature("docstring") helicsSubscriptionGetKey "
";

%feature("docstring") helicsPublicationGetKey "
";

%feature("docstring") helicsSubscriptionGetUnits "
";

%feature("docstring") helicsPublicationGetUnits "
";

%feature("docstring") helicsSubscriptionIsUpdated "
";

%feature("docstring") helicsSubscriptionLastUpdateTime "
";

// File: ValueFederateManager_8cpp.xml

// File: ValueFederateManager_8hpp.xml

// File: zmq_8hpp.xml

// File: zmq__addon_8hpp.xml

// File: ZmqBroker_8cpp.xml

// File: ZmqBroker_8h.xml

// File: ZmqComms_8cpp.xml

// File: ZmqComms_8h.xml

// File: zmqContextManager_8cpp.xml

// File: zmqContextManager_8h.xml

// File: ZmqCore_8cpp.xml

// File: ZmqCore_8h.xml

// File: zmqHelper_8cpp.xml

%feature("docstring") socketTypeFromString "
";

// File: zmqHelper_8h.xml

%feature("docstring") zmq::socketTypeFromString "
";

// File: zmqProxyHub_8cpp.xml

// File: zmqProxyHub_8h.xml

// File: zmqReactor_8cpp.xml

%feature("docstring") zero "
";

%feature("docstring") findSocketByName "
";

// File: zmqReactor_8h.xml

// File: ZmqRequestSets_8cpp.xml

// File: ZmqRequestSets_8h.xml

// File: zmqSocketDescriptor_8cpp.xml

// File: zmqSocketDescriptor_8h.xml

// File: dir_44f6db0dd09c06503695d457c83a8f0f.xml

// File: dir_226623cd513001daa9dd00ed2764382e.xml

// File: dir_af73d593f760128229b3cb30c91fee15.xml

// File: dir_236bf7b4805770569f73c76a3b2639e0.xml

// File: dir_86ccd03f1cd6cad8c64fe0983bba7a8d.xml

// File: dir_b677e63cdf8b0f113c4961f215522b93.xml

// File: dir_1c0454af7f4a9fcd3c5d1acfee8465af.xml

// File: dir_bf898dd3f93638ea94ff42eeb4e9e6cd.xml

// File: dir_472641d7347bb612ce810b6207d86332.xml

// File: dir_64565678bad6be0a8986d5992b62e1ce.xml

// File: dir_4b245f4d320fe2979531602877ae17a1.xml

// File: dir_68267d1309a1af8e8297ef4c3efbcdba.xml

// File: dir_6314cd4f663b3b7d40cdbf46e30ee06a.xml

// File: dir_e02ec16ed846792fdb6c984706cd2eec.xml

// File: dir_e768cf1bfc5ecbd454099b3b8991689d.xml

// File: setSeparator-example.xml

