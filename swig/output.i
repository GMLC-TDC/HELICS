
// File: index.xml

// File: classhelics_1_1ActionMessage.xml


%feature("docstring") helics::ActionMessage "
";

%feature("docstring") helics::ActionMessage::ActionMessage "
`ActionMessage() noexcept`  

default constructor  
";

%feature("docstring") helics::ActionMessage::ActionMessage "
`ActionMessage(action_message_def::action_t startingAction)`  

construct from an action type  

this is an implicit constructor  
";

%feature("docstring") helics::ActionMessage::ActionMessage "
`ActionMessage(action_message_def::action_t startingAction, int32_t sourceId,
    int32_t destId)`  

construct from action, source and destination id's  
";

%feature("docstring") helics::ActionMessage::ActionMessage "
`ActionMessage(ActionMessage &&act) noexcept`  

move constructor  
";

%feature("docstring") helics::ActionMessage::ActionMessage "
`ActionMessage(std::unique_ptr< Message > message)`  

build an action message from a message  
";

%feature("docstring") helics::ActionMessage::ActionMessage "
`ActionMessage(const std::string &bytes)`  

construct from a string  
";

%feature("docstring") helics::ActionMessage::ActionMessage "
`ActionMessage(const std::vector< char > &bytes)`  

construct from a data vector  
";

%feature("docstring") helics::ActionMessage::ActionMessage "
`ActionMessage(const char *data, size_t size)`  

construct from a data pointer and size  
";

%feature("docstring") helics::ActionMessage::ActionMessage "
`ActionMessage(const ActionMessage &act)`  

copy constructor  
";

%feature("docstring") helics::ActionMessage::~ActionMessage "
`~ActionMessage()`  

destructor  
";

%feature("docstring") helics::ActionMessage::action "
`action() const noexcept -> action_message_def::action_t`  

get the action of the message  
";

%feature("docstring") helics::ActionMessage::setAction "
`setAction(action_message_def::action_t newAction)`  

set the action  
";

%feature("docstring") helics::ActionMessage::info "
`info() -> AdditionalInfo &`  

get a reference to the additional info structure  
";

%feature("docstring") helics::ActionMessage::info "
`info() const -> const AdditionalInfo &`  

get a const ref to the info structure  
";

%feature("docstring") helics::ActionMessage::moveInfo "
`moveInfo(std::unique_ptr< Message > message)`  

move a message data into the actionMessage  

take ownership of the message and move the contents out then destroy the message
shell  

Parameters
----------
* `message` :  
    the message to move.  
";

%feature("docstring") helics::ActionMessage::save "
`save(Archive &ar) const`  
";

%feature("docstring") helics::ActionMessage::load "
`load(Archive &ar)`  
";

%feature("docstring") helics::ActionMessage::toByteArray "
`toByteArray(char *data, size_t buffer_size) const -> int`  

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
`to_string(std::string &data) const`  

convert to a string using a reference  
";

%feature("docstring") helics::ActionMessage::to_string "
`to_string() const -> std::string`  

convert to a byte string  
";

%feature("docstring") helics::ActionMessage::packetize "
`packetize() const -> std::string`  

packettize the message with a simple header and tail sequence  
";

%feature("docstring") helics::ActionMessage::to_vector "
`to_vector(std::vector< char > &data) const`  

covert to a byte vector using a reference  
";

%feature("docstring") helics::ActionMessage::to_vector "
`to_vector() const -> std::vector< char >`  

convert a command to a byte vector  
";

%feature("docstring") helics::ActionMessage::fromByteArray "
`fromByteArray(const char *data, size_t buffer_size)`  

generate a command from a raw data stream  
";

%feature("docstring") helics::ActionMessage::depacketize "
`depacketize(const char *data, size_t buffer_size) -> size_t`  

load a command from a packetized stream /ref packetize  

Returns
-------
the number of bytes used  
";

%feature("docstring") helics::ActionMessage::from_string "
`from_string(const std::string &data)`  

read a command from a string  
";

%feature("docstring") helics::ActionMessage::from_vector "
`from_vector(const std::vector< char > &data)`  

read a command from a char vector  
";

// File: classhelics_1_1ActionMessage_1_1AdditionalInfo.xml


%feature("docstring") helics::ActionMessage::AdditionalInfo "
";

%feature("docstring") helics::ActionMessage::AdditionalInfo::AdditionalInfo "
`AdditionalInfo() noexcept`  

constructor  
";

%feature("docstring") helics::ActionMessage::AdditionalInfo::AdditionalInfo "
`AdditionalInfo(const AdditionalInfo &ai)`  

copy constructor  
";

%feature("docstring") helics::ActionMessage::AdditionalInfo::AdditionalInfo "
`AdditionalInfo(AdditionalInfo &&ai) noexcept`  

move constructor  
";

%feature("docstring") helics::ActionMessage::AdditionalInfo::save "
`save(Archive &ar) const`  
";

%feature("docstring") helics::ActionMessage::AdditionalInfo::load "
`load(Archive &ar)`  
";

// File: classhelics_1_1ArgDescriptor.xml


%feature("docstring") helics::ArgDescriptor "

class to contain a descriptor for a command line argument  

C++ includes: argParser.h
";

%feature("docstring") helics::ArgDescriptor::ArgDescriptor "
`ArgDescriptor()=default`  
";

%feature("docstring") helics::ArgDescriptor::ArgDescriptor "
`ArgDescriptor(std::string flag, std::string type, std::string desc)`  
";

// File: classAsioServiceManager.xml


%feature("docstring") AsioServiceManager "

class defining a (potential) singleton Asio Io_service manager for all
boost::asio usage  

C++ includes: AsioServiceManager.h
";

%feature("docstring") AsioServiceManager::getServicePointer "
`getServicePointer(const std::string &serviceName=\"\") -> std::shared_ptr<
    AsioServiceManager >`  

return a pointer to a service manager  

the function will search for an existing service manager for the name if it
doesn't find one it will create a new one  

Parameters
----------
* `serviceName` :  
    the name of the service to find or create  
";

%feature("docstring") AsioServiceManager::getExistingServicePointer "
`getExistingServicePointer(const std::string &serviceName=\"\") ->
    std::shared_ptr< AsioServiceManager >`  

return a pointer to a service manager  

the function will search for an existing service manager for the name if it
doesn't find one it will return nullptr  

Parameters
----------
* `serviceName` :  
    the name of the service to find  
";

%feature("docstring") AsioServiceManager::getService "
`getService(const std::string &serviceName=\"\") -> boost::asio::io_service &`  

get the boost io_service associated with the service manager  
";

%feature("docstring") AsioServiceManager::getExistingService "
`getExistingService(const std::string &serviceName=\"\") ->
    boost::asio::io_service &`  

get the boost io_service associated with the service manager but only if the
service exists if it doesn't this will throw and invalid_argument exception  
";

%feature("docstring") AsioServiceManager::closeService "
`closeService(const std::string &serviceName=\"\")`  
";

%feature("docstring") AsioServiceManager::setServiceToLeakOnDelete "
`setServiceToLeakOnDelete(const std::string &serviceName=\"\")`  

tell the service to free the pointer and leak the memory on delete  

You may ask why, well in windows systems when operating in a DLL if this context
is closed after certain other operations that happen when the DLL is unlinked
bad things can happen, and since in nearly all cases this happens at Shutdown
leaking really doesn't matter that much and if you don't the service could
terminate before some other parts of the program which cause all sorts of odd
errors and issues  
";

%feature("docstring") AsioServiceManager::runServiceLoop "
`runServiceLoop(const std::string &serviceName=\"\")`  

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
`haltServiceLoop(const std::string &serviceName=\"\")`  

halt the service loop thread if the counter==0  

decrements the loop request counter and if it is 0 then will halt the service
loop  

Parameters
----------
* `in` :  
    the name of the service  
";

%feature("docstring") AsioServiceManager::~AsioServiceManager "
`~AsioServiceManager()`  
";

%feature("docstring") AsioServiceManager::getName "
`getName() const -> const std::string &`  

get the name of the current service manager  
";

%feature("docstring") AsioServiceManager::getBaseService "
`getBaseService() const -> boost::asio::io_service &`  

get the underlying boost::io_service reference  
";

%feature("docstring") AsioServiceManager::serviceRunLoop "
`serviceRunLoop(std::shared_ptr< AsioServiceManager > ptr) -> friend void`  
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
`Barrier(std::size_t iCount)`  
";

%feature("docstring") helics::Barrier::Wait "
`Wait()`  
";

// File: classhelics_1_1BasicBrokerInfo.xml


%feature("docstring") helics::BasicBrokerInfo "

class defining the common information about a broker federate  

C++ includes: CoreBroker.hpp
";

%feature("docstring") helics::BasicBrokerInfo::BasicBrokerInfo "
`BasicBrokerInfo(const std::string &brokerName)`  
";

// File: classhelics_1_1BasicFedInfo.xml


%feature("docstring") helics::BasicFedInfo "

class defining the common information for a federate  

C++ includes: CoreBroker.hpp
";

%feature("docstring") helics::BasicFedInfo::BasicFedInfo "
`BasicFedInfo(const std::string &fedname)`  
";

// File: classhelics_1_1BasicHandleInfo.xml


%feature("docstring") helics::BasicHandleInfo "

class defining and capturing basic information about a handle  

C++ includes: BasicHandleInfo.hpp
";

%feature("docstring") helics::BasicHandleInfo::BasicHandleInfo "
`BasicHandleInfo() noexcept`  

default constructor  
";

%feature("docstring") helics::BasicHandleInfo::BasicHandleInfo "
`BasicHandleInfo(Core::handle_id_t id_, Core::federate_id_t fed_id_,
    BasicHandleType what_, const std::string &key_, const std::string &type_,
    const std::string &units_)`  

construct from the data  
";

%feature("docstring") helics::BasicHandleInfo::BasicHandleInfo "
`BasicHandleInfo(Core::handle_id_t id_, Core::federate_id_t fed_id_,
    BasicHandleType what_, const std::string &key_, const std::string &target_,
    const std::string &type_in_, const std::string &type_out_)`  

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
`BlockingPriorityQueue()=default`  

default constructor  
";

%feature("docstring") BlockingPriorityQueue::BlockingPriorityQueue "
`BlockingPriorityQueue(size_t capacity)`  

constructor with the capacity numbers  

there are two internal vectors that alternate so the actual reserve is 2x the
capacity numbers in two different vectors  

Parameters
----------
* `capacity` :  
    the initial reserve capacity for the arrays  
";

%feature("docstring") BlockingPriorityQueue::BlockingPriorityQueue "
`BlockingPriorityQueue(BlockingPriorityQueue &&bq) noexcept`  

enable the move constructor not the copy constructor  
";

%feature("docstring") BlockingPriorityQueue::BlockingPriorityQueue "
`BlockingPriorityQueue(const BlockingPriorityQueue &)=delete`  

DISABLE_COPY_AND_ASSIGN  
";

%feature("docstring") BlockingPriorityQueue::clear "
`clear()`  

clear the queue  
";

%feature("docstring") BlockingPriorityQueue::~BlockingPriorityQueue "
`~BlockingPriorityQueue()`  
";

%feature("docstring") BlockingPriorityQueue::reserve "
`reserve(size_t capacity)`  

set the capacity of the queue actually double the requested the size will be
reserved due to the use of two vectors internally  

Parameters
----------
* `capacity` :  
    the capacity to reserve  
";

%feature("docstring") BlockingPriorityQueue::push "
`push(Z &&val)`  

push an element onto the queue val the value to push on the queue  
";

%feature("docstring") BlockingPriorityQueue::pushPriority "
`pushPriority(Z &&val)`  

push an element onto the queue val the value to push on the queue  
";

%feature("docstring") BlockingPriorityQueue::emplace "
`emplace(Args &&... args)`  

construct on object in place on the queue  
";

%feature("docstring") BlockingPriorityQueue::emplacePriority "
`emplacePriority(Args &&... args)`  

emplace an element onto the priority queue val the value to push on the queue  
";

%feature("docstring") BlockingPriorityQueue::try_peek "
`try_peek() const -> stx::optional< T >`  

try to peek at an object without popping it from the stack  

only available for copy assignable objects  

Returns
-------
an optional object with an object of type T if available  
";

%feature("docstring") BlockingPriorityQueue::try_pop "
`try_pop() -> stx::optional< T >`  

try to pop an object from the queue  

Returns
-------
an optional containing the value if successful the optional will be empty if
there is no element in the queue  
";

%feature("docstring") BlockingPriorityQueue::pop "
`pop() -> T`  

blocking call to wait on an object from the stack  
";

%feature("docstring") BlockingPriorityQueue::pop "
`pop(Functor callOnWaitFunction) -> T`  

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
`empty() const -> bool`  

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
`BlockingQueue()=default`  

default constructor  
";

%feature("docstring") BlockingQueue::BlockingQueue "
`BlockingQueue(size_t capacity)`  

constructor with the capacity numbers  

there are two internal vectors that alternate so the actual reserve is 2x the
capacity numbers in two different vectors  

Parameters
----------
* `capacity` :  
    the initial reserve capacity for the arrays  
";

%feature("docstring") BlockingQueue::BlockingQueue "
`BlockingQueue(BlockingQueue &&bq) noexcept`  

enable the move constructor not the copy constructor  
";

%feature("docstring") BlockingQueue::BlockingQueue "
`BlockingQueue(const BlockingQueue &)=delete`  

DISABLE_COPY_AND_ASSIGN  
";

%feature("docstring") BlockingQueue::~BlockingQueue "
`~BlockingQueue()`  
";

%feature("docstring") BlockingQueue::clear "
`clear()`  

clear the queue  
";

%feature("docstring") BlockingQueue::reserve "
`reserve(size_t capacity)`  

set the capacity of the queue actually double the requested the size will be
reserved due to the use of two vectors internally  

Parameters
----------
* `capacity` :  
    the capacity to reserve  
";

%feature("docstring") BlockingQueue::push "
`push(Z &&val)`  

push an element onto the queue val the value to push on the queue  
";

%feature("docstring") BlockingQueue::emplace "
`emplace(Args &&... args)`  

construct on object in place on the queue  
";

%feature("docstring") BlockingQueue::try_peek "
`try_peek() const -> stx::optional< T >`  

try to peek at an object without popping it from the stack  

only available for copy assignable objects  

Returns
-------
an optional object with an object of type T if available  
";

%feature("docstring") BlockingQueue::try_pop "
`try_pop() -> stx::optional< T >`  

try to pop an object from the queue  

Returns
-------
an optional containing the value if successful the optional will be empty if
there is no element in the queue  
";

%feature("docstring") BlockingQueue::pop "
`pop() -> T`  

blocking call to wait on an object from the stack  
";

%feature("docstring") BlockingQueue::pop "
`pop(Functor callOnWaitFunction) -> T`  

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
`empty() const -> bool`  

check whether there are any elements in the queue because this is meant for
multi-threaded applications this may or may not have any meaning depending on
the number of consumers  
";

%feature("docstring") BlockingQueue::size "
`size() const -> size_t`  

get the current size of the queue  

this may or may not have much meaning depending on the number of consumers  
";

// File: classhelics_1_1BlockingQueue__old.xml


%feature("docstring") helics::BlockingQueue_old "

a queue that blocks while waiting for an input  

C++ includes: blocking_queue.h
";

%feature("docstring") helics::BlockingQueue_old::BlockingQueue_old "
`BlockingQueue_old()=default`  

default constructor  
";

%feature("docstring") helics::BlockingQueue_old::BlockingQueue_old "
`BlockingQueue_old(const BlockingQueue_old &)=delete`  

DISABLE_COPY_AND_ASSIGN  
";

%feature("docstring") helics::BlockingQueue_old::push "
`push(const T &t)`  

push an object onto the queue  
";

%feature("docstring") helics::BlockingQueue_old::emplace "
`emplace(Args &&... args)`  

construct on object in place on the queue  
";

%feature("docstring") helics::BlockingQueue_old::try_pop "
`try_pop() -> stx::optional< T >`  

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
`pop(const std::string &log_on_wait=\"\") -> T`  
";

%feature("docstring") helics::BlockingQueue_old::try_peek "
`try_peek() const -> stx::optional< T >`  

try to peek at an object without popping it from the stack  

Returns
-------
an optional object with an objec of type T if available  
";

%feature("docstring") helics::BlockingQueue_old::size "
`size() const -> size_t`  

get the current size of the queue  
";

// File: classhelics_1_1Broker.xml


%feature("docstring") helics::Broker "

virtual class defining a public interface to a broker  

C++ includes: Broker.hpp
";

%feature("docstring") helics::Broker::Broker "
`Broker()=default`  

default constructor  

Parameters
----------
* `setAsRootBroker` :  
    set to true to indicate this object is a root broker  
";

%feature("docstring") helics::Broker::Broker "
`Broker()`  
";

%feature("docstring") helics::Broker::Broker "
`Broker(std::string type, std::string name, std::string initString)`  
";

%feature("docstring") helics::Broker::Broker "
`Broker(std::string type, std::string name, int argc, const char **argv)`  
";

%feature("docstring") helics::Broker::~Broker "
`~Broker()=default`  

destructor  
";

%feature("docstring") helics::Broker::~Broker "
`~Broker()`  
";

%feature("docstring") helics::Broker::connect "
`connect()=0 -> bool`  

connect the core to its broker  

should be done after initialization has complete  
";

%feature("docstring") helics::Broker::disconnect "
`disconnect()=0`  

disconnect the broker from any other brokers and communications  
";

%feature("docstring") helics::Broker::isConnected "
`isConnected() const =0 -> bool`  

check if the broker is connected  
";

%feature("docstring") helics::Broker::isConnected "
`isConnected() -> bool`  
";

%feature("docstring") helics::Broker::setAsRoot "
`setAsRoot()=0`  

set the broker to be a root broker  

only valid before the initialization function is called  
";

%feature("docstring") helics::Broker::isRoot "
`isRoot() const =0 -> bool`  

return true if the broker is a root broker  
";

%feature("docstring") helics::Broker::isOpenToNewFederates "
`isOpenToNewFederates() const =0 -> bool`  

check if the broker is ready to accept new federates or cores  
";

%feature("docstring") helics::Broker::initialize "
`initialize(const std::string &initializationString)=0`  

start up the broker with an initialization string containing commands and
parameters  
";

%feature("docstring") helics::Broker::initializeFromArgs "
`initializeFromArgs(int argc, const char *const *argv)=0`  

initialize from command line arguments  
";

%feature("docstring") helics::Broker::getIdentifier "
`getIdentifier() const =0 -> const std::string &`  

get the local identification for the broker  
";

%feature("docstring") helics::Broker::getAddress "
`getAddress() const =0 -> std::string`  

get the connection address for the broker  
";

// File: classhelics_1_1BrokerBase.xml


%feature("docstring") helics::BrokerBase "

base class for broker like objects  

C++ includes: BrokerBase.hpp
";

%feature("docstring") helics::BrokerBase::displayHelp "
`displayHelp()`  
";

%feature("docstring") helics::BrokerBase::BrokerBase "
`BrokerBase() noexcept`  
";

%feature("docstring") helics::BrokerBase::BrokerBase "
`BrokerBase(const std::string &broker_name)`  
";

%feature("docstring") helics::BrokerBase::~BrokerBase "
`~BrokerBase()`  
";

%feature("docstring") helics::BrokerBase::initializeFromCmdArgs "
`initializeFromCmdArgs(int argc, const char *const *argv)`  

initialize the core manager with command line arguments  

Parameters
----------
* `argc` :  
    the number of arguments  
* `argv` :  
    char pointers to the arguments  
";

%feature("docstring") helics::BrokerBase::addActionMessage "
`addActionMessage(const ActionMessage &m)`  

add an action Message to the process queue  
";

%feature("docstring") helics::BrokerBase::addActionMessage "
`addActionMessage(ActionMessage &&m)`  

move a action Message into the commandQueue  
";

%feature("docstring") helics::BrokerBase::setLoggerFunction "
`setLoggerFunction(std::function< void(int, const std::string &, const
    std::string &)> logFunction)`  

set the logging callback function  

Parameters
----------
* `logFunction` :  
    a function with a signature of void(int level,  const std::string &source,
    const std::string &message) the function takes a level indicating the
    logging level string with the source name and a string with the message  
";

%feature("docstring") helics::BrokerBase::processDisconnect "
`processDisconnect(bool skipUnregister=false)=0`  

process a disconnect signal  
";

%feature("docstring") helics::BrokerBase::isRunning "
`isRunning() const -> bool`  

check if the main processing loop of a broker is running  
";

%feature("docstring") helics::BrokerBase::setLogLevel "
`setLogLevel(int32_t level)`  

set the logging level  
";

%feature("docstring") helics::BrokerBase::setLogLevels "
`setLogLevels(int32_t consoleLevel, int32_t fileLevel)`  

set the logging levels  

Parameters
----------
* `consoleLevel` :  
    the logging level for the console display  
* `fileLevel` :  
    the logging level for the log file  
";

%feature("docstring") helics::BrokerBase::joinAllThreads "
`joinAllThreads()`  

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
`changeOnDestroy(std::atomic< X > &var, X finalValue)`  
";

%feature("docstring") helics::changeOnDestroy::~changeOnDestroy "
`~changeOnDestroy()`  
";

// File: classutilities_1_1charMapper.xml


%feature("docstring") utilities::charMapper "

small helper class to map characters to values  

C++ includes: charMapper.h
";

%feature("docstring") utilities::charMapper::charMapper "
`charMapper(V defVal=V(0))`  

default constructor  
";

%feature("docstring") utilities::charMapper::addKey "
`addKey(unsigned char x, V val)`  

update a the value returned from a key query  

this is purposely distinct from the [] operator to make it an error to try to
assign something that way  
";

%feature("docstring") utilities::charMapper::at "
`at(unsigned char x) const -> V`  

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
`CloneFilterOperation(Core *core)`  

this operation needs a pointer to a core to operate  
";

%feature("docstring") helics::CloneFilterOperation::~CloneFilterOperation "
`~CloneFilterOperation()`  
";

%feature("docstring") helics::CloneFilterOperation::set "
`set(const std::string &property, double val) override`  

set a property on a filter  

Parameters
----------
* `property` :  
    the name of the property of the filter to change  
* `val` :  
    the numerical value of the property  
";

%feature("docstring") helics::CloneFilterOperation::setString "
`setString(const std::string &property, const std::string &val) override`  

set a string property on a filter  

Parameters
----------
* `property` :  
    the name of the property of the filter to change  
* `val` :  
    the numerical value of the property  
";

%feature("docstring") helics::CloneFilterOperation::getOperator "
`getOperator() override -> std::shared_ptr< FilterOperator >`  
";

// File: classhelics_1_1CloneOperator.xml


%feature("docstring") helics::CloneOperator "

class defining an message operator that either passes the message or not  

the evaluation function used should return true if the message should be allowed
through false if it should be dropped  

C++ includes: MessageOperators.hpp
";

%feature("docstring") helics::CloneOperator::CloneOperator "
`CloneOperator()=default`  

default constructor  
";

%feature("docstring") helics::CloneOperator::CloneOperator "
`CloneOperator(std::function< void(const Message *)> userCloneFunction)`  

set the function to modify the data of the message in the constructor  
";

%feature("docstring") helics::CloneOperator::setCloneFunction "
`setCloneFunction(std::function< void(const Message *)> userCloneFunction)`  

set the function to modify the data of the message  
";

// File: classhelics_1_1CloningFilter.xml


%feature("docstring") helics::CloningFilter "

class used to clone message for delivery to other endpoints  

C++ includes: Filters.hpp
";

%feature("docstring") helics::CloningFilter::CloningFilter "
`CloningFilter(Core *cr)`  

construct from a core object  
";

%feature("docstring") helics::CloningFilter::CloningFilter "
`CloningFilter(Federate *fed)`  

construct from a Federate  
";

%feature("docstring") helics::CloningFilter::addSourceTarget "
`addSourceTarget(const std::string &sourceName)`  

add a sourceEndpoint to the list of endpoint to clone  
";

%feature("docstring") helics::CloningFilter::addDestinationTarget "
`addDestinationTarget(const std::string &destinationName)`  

add a destination endpoint to the list of endpoints to clone  
";

%feature("docstring") helics::CloningFilter::addDeliveryEndpoint "
`addDeliveryEndpoint(const std::string &endpoint)`  

add a delivery address this is the name of an endpoint to deliver the message to  
";

%feature("docstring") helics::CloningFilter::removeSourceTarget "
`removeSourceTarget(const std::string &sourceName)`  

remove a sourceEndpoint to the list of endpoint to clone  
";

%feature("docstring") helics::CloningFilter::removeDestinationTarget "
`removeDestinationTarget(const std::string &destinationName)`  

remove a destination endpoint to the list of endpoints to clone  
";

%feature("docstring") helics::CloningFilter::removeDeliveryEndpoint "
`removeDeliveryEndpoint(const std::string &endpoint)`  

remove a delivery address this is the name of an endpoint to deliver the message
to  
";

%feature("docstring") helics::CloningFilter::setString "
`setString(const std::string &property, const std::string &val) override`  

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
`CombinationFederate()`  

default constructor  
";

%feature("docstring") helics::CombinationFederate::CombinationFederate "
`CombinationFederate(const FederateInfo &fi)`  

constructor taking a federate information structure and using the default core  

Parameters
----------
* `fi` :  
    a federate information structure  
";

%feature("docstring") helics::CombinationFederate::CombinationFederate "
`CombinationFederate(std::shared_ptr< Core > core, const FederateInfo &fi)`  

constructor taking a federate information structure and using the given core  

Parameters
----------
* `core` :  
    a pointer to core object which the federate can join  
* `fi` :  
    a federate information structure  
";

%feature("docstring") helics::CombinationFederate::CombinationFederate "
`CombinationFederate(const std::string &jsonString)`  

constructor taking a file with the required information  

Parameters
----------
* `file` :  
    a file defining the federate information  
";

%feature("docstring") helics::CombinationFederate::CombinationFederate "
`CombinationFederate(CombinationFederate &&fed) noexcept`  

move construction  
";

%feature("docstring") helics::CombinationFederate::CombinationFederate "
`CombinationFederate(FederateInfo &fi)`  
";

%feature("docstring") helics::CombinationFederate::CombinationFederate "
`CombinationFederate(const std::string &jsonString)`  
";

%feature("docstring") helics::CombinationFederate::~CombinationFederate "
`~CombinationFederate()`  

destructor  
";

%feature("docstring") helics::CombinationFederate::disconnect "
`disconnect() override`  

disconnect a simulation from the core (will also call finalize before
disconnecting if necessary)  
";

%feature("docstring") helics::CombinationFederate::registerInterfaces "
`registerInterfaces(const std::string &jsonString) override`  

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
`CommonCore() noexcept`  

default constructor  
";

%feature("docstring") helics::CommonCore::CommonCore "
`CommonCore(bool arg) noexcept`  

function mainly to match some other object constructors does the same thing as
the default constructor  
";

%feature("docstring") helics::CommonCore::CommonCore "
`CommonCore(const std::string &core_name)`  

construct from a core name  
";

%feature("docstring") helics::CommonCore::~CommonCore "
`~CommonCore()`  

virtual destructor  
";

%feature("docstring") helics::CommonCore::initialize "
`initialize(const std::string &initializationString) override final`  

Simulator control. Initialize the core.  

Should be invoked a single time to initialize the co-simulation core.  
";

%feature("docstring") helics::CommonCore::initializeFromArgs "
`initializeFromArgs(int argc, const char *const *argv) override`  

initialize the core manager with command line arguments  

Parameters
----------
* `argc` :  
    the number of arguments  
* `argv` :  
    char pointers to the arguments  
";

%feature("docstring") helics::CommonCore::isInitialized "
`isInitialized() const override final -> bool`  

Returns true if the core has been initialized.  
";

%feature("docstring") helics::CommonCore::isOpenToNewFederates "
`isOpenToNewFederates() const override final -> bool`  

check if the core is ready to accept new federates  
";

%feature("docstring") helics::CommonCore::error "
`error(federate_id_t federateID, int errorCode=-1) override final`  

Federate has encountered an unrecoverable error.  
";

%feature("docstring") helics::CommonCore::finalize "
`finalize(federate_id_t federateID) override final`  

Federate has completed.  

Should be invoked a single time to complete the simulation.  
";

%feature("docstring") helics::CommonCore::enterInitializingState "
`enterInitializingState(federate_id_t federateID) override final`  

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
`enterExecutingState(federate_id_t federateID, helics_iteration_request
    iterate=NO_ITERATION) override final -> iteration_result`  

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
`registerFederate(const std::string &name, const CoreFederateInfo &info)
    override final -> federate_id_t`  

Register a federate.  

The returned FederateId is local to invoking process, FederateId's should not be
used as a global identifier.  

May only be invoked in initialize state otherwise throws an error  
";

%feature("docstring") helics::CommonCore::getFederateName "
`getFederateName(federate_id_t federateID) const override final -> const
    std::string &`  

Returns the federate name.  
";

%feature("docstring") helics::CommonCore::getFederateId "
`getFederateId(const std::string &name) override final -> federate_id_t`  

Returns the federate Id.  
";

%feature("docstring") helics::CommonCore::getFederationSize "
`getFederationSize() override final -> int32_t`  

Returns the global number of federates that are registered only return
accurately after the initialization state has been entered  
";

%feature("docstring") helics::CommonCore::timeRequest "
`timeRequest(federate_id_t federateID, Time next) override final -> Time`  

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
`requestTimeIterative(federate_id_t federateID, Time next,
    helics_iteration_request iterate) override final -> iteration_time`  

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
`getCurrentTime(federate_id_t federateID) const override final -> Time`  

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
`getCurrentReiteration(federate_id_t federateID) const override final ->
    uint64_t`  

Returns the current reiteration count for the specified federate.  
";

%feature("docstring") helics::CommonCore::setMaximumIterations "
`setMaximumIterations(federate_id_t federateID, int32_t iterations) override
    final`  

Set the maximum number of iterations allowed.  

The minimum value set in any federate is used.  

Default value is the maximum allowed value for uint64_t.  

May only be invoked in the initialize state.  
";

%feature("docstring") helics::CommonCore::setTimeDelta "
`setTimeDelta(federate_id_t federateID, Time time) override final`  

Set the minimum time resolution for the specified federate.  

The value is used to constrain when the timeRequest methods return to values
that are multiples of the specified delta. This is useful for federates that are
time-stepped and making sub-time-step updates is not meaningful.  

Parameters
----------
* `time` :  
";

%feature("docstring") helics::CommonCore::setOutputDelay "
`setOutputDelay(federate_id_t federateID, Time outputDelayTime) override final`  

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
`setPeriod(federate_id_t federateID, Time timePeriod) override final`  

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
`setTimeOffset(federate_id_t federateID, Time timeOffset) override final`  

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
`setInputDelay(federate_id_t federateID, Time impactTime) override final`  

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
`setLoggingLevel(federate_id_t federateID, int loggingLevel) override final`  

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
`setFlag(federate_id_t federateID, int flag, bool flagValue=true) override
    final`  

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
`registerSubscription(federate_id_t federateID, const std::string &key, const
    std::string &type, const std::string &units, handle_check_mode check_mode)
    override final -> handle_id_t`  
";

%feature("docstring") helics::CommonCore::getSubscription "
`getSubscription(federate_id_t federateID, const std::string &key) const
    override final -> handle_id_t`  

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
`registerPublication(federate_id_t federateID, const std::string &key, const
    std::string &type, const std::string &units) override final -> handle_id_t`  

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
`getPublication(federate_id_t federateID, const std::string &key) const override
    final -> handle_id_t`  

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
`getHandleName(handle_id_t handle) const override final -> const std::string &`  

Returns the name or identifier for a specified handle  
";

%feature("docstring") helics::CommonCore::getTarget "
`getTarget(handle_id_t handle) const override final -> const std::string &`  

Returns the target of a specified handle  

for publications and subscriptions this is the key for filters this is the
target and for endpoints this will return an empty string  
";

%feature("docstring") helics::CommonCore::getUnits "
`getUnits(handle_id_t handle) const override final -> const std::string &`  

Returns units for specified handle.  
";

%feature("docstring") helics::CommonCore::getType "
`getType(handle_id_t handle) const override final -> const std::string &`  

Returns type for specified handle.  

for endpoints, publications, and filters, this is the input type for
subscriptions this is the type of the publication(if available)  

Parameters
----------
* `handle` :  
    the handle from the publication, subscription, endpoint or filter  
";

%feature("docstring") helics::CommonCore::getOutputType "
`getOutputType(handle_id_t handle) const override final -> const std::string &`  

Returns output type for specified handle.  

for filters this is the outputType, for Subscriptions this is the expected type
for endpoints and publications this is the same as getType();  

Parameters
----------
* `handle` :  
    the handle from the publication, subscription, endpoint or filter  
";

%feature("docstring") helics::CommonCore::setValue "
`setValue(handle_id_t handle, const char *data, uint64_t len) override final`  

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
`getValue(handle_id_t handle) override final -> std::shared_ptr< const
    data_block >`  

Return the data for the specified handle.  
";

%feature("docstring") helics::CommonCore::getValueUpdates "
`getValueUpdates(federate_id_t federateID) override final -> const std::vector<
    handle_id_t > &`  

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
`registerEndpoint(federate_id_t federateID, const std::string &name, const
    std::string &type) override final -> handle_id_t`  

Message interface. Designed for point-to-point communication patterns. Register
an endpoint.  

May only be invoked in the Initialization state.  
";

%feature("docstring") helics::CommonCore::getEndpoint "
`getEndpoint(federate_id_t federateID, const std::string &name) const override
    final -> handle_id_t`  

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
`registerSourceFilter(const std::string &filterName, const std::string &source,
    const std::string &type_in, const std::string &type_out) override final ->
    handle_id_t`  

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
`registerDestinationFilter(const std::string &filterName, const std::string
    &dest, const std::string &type_in, const std::string &type_out) override
    final -> handle_id_t`  

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
`getSourceFilter(const std::string &name) const override final -> handle_id_t`  

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
`getDestinationFilter(const std::string &name) const override final ->
    handle_id_t`  

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
`addDependency(federate_id_t federateID, const std::string &federateName)
    override final`  

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
`registerFrequentCommunicationsPair(const std::string &source, const std::string
    &dest) override final`  

Register known frequently communicating source/destination end points.  

May be used for error checking for compatible types and possible optimization by
pre-registering the intent for these endpoints to communicate.  
";

%feature("docstring") helics::CommonCore::send "
`send(handle_id_t sourceHandle, const std::string &destination, const char
    *data, uint64_t length) override final`  

Send data from source to destination.  

Time is implicitly defined as the end of the current time advancement window
(value returned by last call to nextTime().  

This send version was designed to enable communication of data between federates
with the possible introduction of source and destination filters to represent
properties of a communication network. This enables simulations to be run
with/without a communications model present.  
";

%feature("docstring") helics::CommonCore::sendEvent "
`sendEvent(Time time, handle_id_t sourceHandle, const std::string &destination,
    const char *data, uint64_t length) override final`  

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
`sendMessage(handle_id_t sourceHandle, std::unique_ptr< Message > message)
    override final`  

Send for filters.  

Continues sending the message to the next filter or to final destination.  
";

%feature("docstring") helics::CommonCore::receiveCount "
`receiveCount(handle_id_t destination) override final -> uint64_t`  

Returns the number of pending receives for the specified destination endpoint or
filter.  
";

%feature("docstring") helics::CommonCore::receive "
`receive(handle_id_t destination) override final -> std::unique_ptr< Message >`  

Returns the next buffered message the specified destination endpoint or filter.  

this is a non-blocking call and will return a nullptr if no message are
available  
";

%feature("docstring") helics::CommonCore::receiveAny "
`receiveAny(federate_id_t federateID, handle_id_t &endpoint_id) override final
    -> std::unique_ptr< Message >`  

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
`receiveCountAny(federate_id_t federateID) override final -> uint64_t`  

Returns number of messages for all destinations.  
";

%feature("docstring") helics::CommonCore::logMessage "
`logMessage(federate_id_t federateID, int logLevel, const std::string
    &logMessage) override final`  

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
`setFilterOperator(handle_id_t filter, std::shared_ptr< FilterOperator >
    callback) override final`  

set the filter callback operator  

Parameters
----------
* `filter` :  
    the handle of the filter  
* `operator` :  
    pointer to the operator class executing the filter  
";

%feature("docstring") helics::CommonCore::setIdentifier "
`setIdentifier(const std::string &name)`  

set the local identification for the core  
";

%feature("docstring") helics::CommonCore::getIdentifier "
`getIdentifier() const override final -> const std::string &`  

get the local identifier for the core  
";

%feature("docstring") helics::CommonCore::getFederateNameNoThrow "
`getFederateNameNoThrow(federate_id_t federateID) const noexcept -> const
    std::string &`  
";

%feature("docstring") helics::CommonCore::setLoggingCallback "
`setLoggingCallback(federate_id_t federateID, std::function< void(int, const
    std::string &, const std::string &)> logFunction) override final`  

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
`query(const std::string &target, const std::string &queryStr) override ->
    std::string`  

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
`setQueryCallback(federate_id_t federateID, std::function< std::string(const
    std::string &)> queryFunction) override`  

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
`getAddress() const =0 -> std::string`  

get a string representing the connection info to send data to this object  
";

%feature("docstring") helics::CommonCore::connect "
`connect() override final -> bool`  

connect the core to a broker if needed  

Returns
-------
true if the connection was successful  
";

%feature("docstring") helics::CommonCore::isConnected "
`isConnected() const override final -> bool`  

check if the core is connected properly  
";

%feature("docstring") helics::CommonCore::disconnect "
`disconnect() override final`  

disconnect the core from its broker  
";

%feature("docstring") helics::CommonCore::unregister "
`unregister()`  

unregister the core from any process find functions  
";

%feature("docstring") helics::CommonCore::processDisconnect "
`processDisconnect(bool skipUnregister=false) override final`  

process a disconnect signal  
";

// File: classhelics_1_1CommsBroker.xml


%feature("docstring") helics::CommsBroker "

helper class defining some common functionality for brokers and cores that use
different communication methods  

C++ includes: CommsBroker.hpp
";

%feature("docstring") helics::CommsBroker::CommsBroker "
`CommsBroker() noexcept`  

default constructor  
";

%feature("docstring") helics::CommsBroker::CommsBroker "
`CommsBroker(bool arg) noexcept`  

create from a single argument  
";

%feature("docstring") helics::CommsBroker::CommsBroker "
`CommsBroker(const std::string &obj_name)`  

create from an object name  
";

%feature("docstring") helics::CommsBroker::~CommsBroker "
`~CommsBroker()`  

destructor  
";

%feature("docstring") helics::CommsBroker::transmit "
`transmit(int route_id, const ActionMessage &cmd) override`  
";

%feature("docstring") helics::CommsBroker::addRoute "
`addRoute(int route_id, const std::string &routeInfo) override`  
";

// File: classhelics_1_1CommsInterface.xml


%feature("docstring") helics::CommsInterface "

implementation of a generic communications interface  

C++ includes: CommsInterface.hpp
";

%feature("docstring") helics::CommsInterface::CommsInterface "
`CommsInterface()=default`  

default constructor  
";

%feature("docstring") helics::CommsInterface::CommsInterface "
`CommsInterface(const std::string &localTarget, const std::string
    &brokerTarget)`  

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
`CommsInterface(const NetworkBrokerData &netInfo)`  

construct from a NetworkBrokerData structure  
";

%feature("docstring") helics::CommsInterface::~CommsInterface "
`~CommsInterface()`  

destructor  
";

%feature("docstring") helics::CommsInterface::transmit "
`transmit(int route_id, const ActionMessage &cmd)`  

transmit a message along a particular route  
";

%feature("docstring") helics::CommsInterface::addRoute "
`addRoute(int route_id, const std::string &routeInfo)`  

add a new route assigned to the appropriate id  
";

%feature("docstring") helics::CommsInterface::connect "
`connect() -> bool`  

connect the commsInterface  

Returns
-------
true if the connection was successful false otherwise  
";

%feature("docstring") helics::CommsInterface::disconnect "
`disconnect()`  

disconnected the comms interface  
";

%feature("docstring") helics::CommsInterface::reconnect "
`reconnect() -> bool`  

try reconnected from a mismatched or disconnection  
";

%feature("docstring") helics::CommsInterface::setName "
`setName(const std::string &name)`  

set the name of the communicator  
";

%feature("docstring") helics::CommsInterface::setCallback "
`setCallback(std::function< void(ActionMessage &&)> callback)`  

set the callback for processing the messages  
";

%feature("docstring") helics::CommsInterface::setMessageSize "
`setMessageSize(int maxMessageSize, int maxMessageCount)`  

set the max message size and max Queue size  
";

%feature("docstring") helics::CommsInterface::isConnected "
`isConnected() const -> bool`  

check if the commInterface is connected  
";

%feature("docstring") helics::CommsInterface::setTimeout "
`setTimeout(int timeout)`  

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
`conditionalChangeOnDestroy(std::atomic< X > &var, X finalValue, X expValue)`  
";

%feature("docstring") helics::conditionalChangeOnDestroy::~conditionalChangeOnDestroy "
`~conditionalChangeOnDestroy()`  
";

// File: classhelics_1_1ConnectionFailure.xml


%feature("docstring") helics::ConnectionFailure "

exception indicating that the registration of an object has failed  

C++ includes: core-exceptions.hpp
";

%feature("docstring") helics::ConnectionFailure::ConnectionFailure "
`ConnectionFailure(const std::string &message=\"failed to connect\")`  
";

// File: classzmq_1_1context__t.xml


%feature("docstring") zmq::context_t "
";

%feature("docstring") zmq::context_t::context_t "
`context_t()`  
";

%feature("docstring") zmq::context_t::context_t "
`context_t(int io_threads_, int max_sockets_=ZMQ_MAX_SOCKETS_DFLT)`  
";

%feature("docstring") zmq::context_t::~context_t "
`~context_t() ZMQ_NOTHROW`  
";

%feature("docstring") zmq::context_t::close "
`close() ZMQ_NOTHROW`  
";

// File: classhelics_1_1Core.xml


%feature("docstring") helics::Core "

the class defining the core interface through an abstract class  

C++ includes: Core.hpp
";

%feature("docstring") helics::Core::Core "
`Core()=default`  

default constructor  
";

%feature("docstring") helics::Core::~Core "
`~Core()=default`  

virtual destructor  
";

%feature("docstring") helics::Core::initialize "
`initialize(const std::string &initializationString)=0`  

Simulator control. Initialize the core.  

Should be invoked a single time to initialize the co-simulation core.  
";

%feature("docstring") helics::Core::initializeFromArgs "
`initializeFromArgs(int argc, const char *const *argv)=0`  

Initialize the core from command line arguments.  

Should be invoked a single time to initialize the co-simulation core.  
";

%feature("docstring") helics::Core::isInitialized "
`isInitialized() const =0 -> bool`  

Returns true if the core has been initialized.  
";

%feature("docstring") helics::Core::connect "
`connect()=0 -> bool`  

connect the core to a broker if needed  

Returns
-------
true if the connection was successful  
";

%feature("docstring") helics::Core::isConnected "
`isConnected() const =0 -> bool`  

check if the core is connected properly  
";

%feature("docstring") helics::Core::disconnect "
`disconnect()=0`  

disconnect the core from its broker  
";

%feature("docstring") helics::Core::isOpenToNewFederates "
`isOpenToNewFederates() const =0 -> bool`  

check if the core is ready to accept new federates  
";

%feature("docstring") helics::Core::getIdentifier "
`getIdentifier() const =0 -> const std::string &`  

get and identifier string for the core  
";

%feature("docstring") helics::Core::error "
`error(federate_id_t federateID, int errorCode=-1)=0`  

Federate has encountered an unrecoverable error.  
";

%feature("docstring") helics::Core::finalize "
`finalize(federate_id_t federateID)=0`  

Federate has completed.  

Should be invoked a single time to complete the simulation.  
";

%feature("docstring") helics::Core::enterInitializingState "
`enterInitializingState(federate_id_t federateID)=0`  

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
`enterExecutingState(federate_id_t federateID, helics_iteration_request
    iterate=NO_ITERATION)=0 -> iteration_result`  

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
`registerFederate(const std::string &name, const CoreFederateInfo &info)=0 ->
    federate_id_t`  

Register a federate.  

The returned FederateId is local to invoking process, FederateId's should not be
used as a global identifier.  

May only be invoked in initialize state otherwise throws an error  
";

%feature("docstring") helics::Core::getFederateName "
`getFederateName(federate_id_t federateID) const =0 -> const std::string &`  

Returns the federate name.  
";

%feature("docstring") helics::Core::getFederateId "
`getFederateId(const std::string &name)=0 -> federate_id_t`  

Returns the federate Id.  
";

%feature("docstring") helics::Core::getFederationSize "
`getFederationSize()=0 -> int32_t`  

Returns the global number of federates that are registered only return
accurately after the initialization state has been entered  
";

%feature("docstring") helics::Core::timeRequest "
`timeRequest(federate_id_t federateID, Time next)=0 -> Time`  

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
`requestTimeIterative(federate_id_t federateID, Time next,
    helics_iteration_request iterate)=0 -> iteration_time`  

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
`getCurrentReiteration(federate_id_t federateID) const =0 -> uint64_t`  

Returns the current reiteration count for the specified federate.  
";

%feature("docstring") helics::Core::getCurrentTime "
`getCurrentTime(federate_id_t federateID) const =0 -> Time`  

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
`setMaximumIterations(federate_id_t federateID, int32_t iterations)=0`  

Set the maximum number of iterations allowed.  

The minimum value set in any federate is used.  

Default value is the maximum allowed value for uint64_t.  

May only be invoked in the initialize state.  
";

%feature("docstring") helics::Core::setTimeDelta "
`setTimeDelta(federate_id_t federateID, Time time)=0`  

Set the minimum time resolution for the specified federate.  

The value is used to constrain when the timeRequest methods return to values
that are multiples of the specified delta. This is useful for federates that are
time-stepped and making sub-time-step updates is not meaningful.  

Parameters
----------
* `time` :  
";

%feature("docstring") helics::Core::setOutputDelay "
`setOutputDelay(federate_id_t federateID, Time timeoutputDelay)=0`  

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
`setPeriod(federate_id_t federateID, Time timePeriod)=0`  

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
`setTimeOffset(federate_id_t federateID, Time timeOffset)=0`  

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
`setInputDelay(federate_id_t federateID, Time timeImpact)=0`  

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
`setLoggingLevel(federate_id_t federateID, int loggingLevel)=0`  

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
`setFlag(federate_id_t federateID, int flag, bool flagValue=true)=0`  

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
`registerSubscription(federate_id_t federateID, const std::string &key, const
    std::string &(type), const std::string &units, handle_check_mode
    check_mode)=0 -> handle_id_t`  

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
`getSubscription(federate_id_t federateID, const std::string &key) const =0 ->
    handle_id_t`  

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
`registerPublication(federate_id_t federateID, const std::string &key, const
    std::string &type, const std::string &units)=0 -> handle_id_t`  

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
`getPublication(federate_id_t federateID, const std::string &key) const =0 ->
    handle_id_t`  

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
`getHandleName(handle_id_t handle) const =0 -> const std::string &`  

Returns the name or identifier for a specified handle  
";

%feature("docstring") helics::Core::getTarget "
`getTarget(handle_id_t handle) const =0 -> const std::string &`  

Returns the target of a specified handle  

for publications and subscriptions this is the key for filters this is the
target and for endpoints this will return an empty string  
";

%feature("docstring") helics::Core::getUnits "
`getUnits(handle_id_t handle) const =0 -> const std::string &`  

Returns units for specified handle.  
";

%feature("docstring") helics::Core::getType "
`getType(handle_id_t handle) const =0 -> const std::string &`  

Returns type for specified handle.  

for endpoints, publications, and filters, this is the input type for
subscriptions this is the type of the publication(if available)  

Parameters
----------
* `handle` :  
    the handle from the publication, subscription, endpoint or filter  
";

%feature("docstring") helics::Core::getOutputType "
`getOutputType(handle_id_t handle) const =0 -> const std::string &`  

Returns output type for specified handle.  

for filters this is the outputType, for Subscriptions this is the expected type
for endpoints and publications this is the same as getType();  

Parameters
----------
* `handle` :  
    the handle from the publication, subscription, endpoint or filter  
";

%feature("docstring") helics::Core::setValue "
`setValue(handle_id_t handle, const char *data, uint64_t len)=0`  

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
`getValue(handle_id_t handle)=0 -> std::shared_ptr< const data_block >`  

Return the data for the specified handle.  
";

%feature("docstring") helics::Core::getValueUpdates "
`getValueUpdates(federate_id_t federateID)=0 -> const std::vector< handle_id_t >
    &`  

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
`registerEndpoint(federate_id_t federateID, const std::string &name, const
    std::string &type)=0 -> handle_id_t`  

Message interface. Designed for point-to-point communication patterns. Register
an endpoint.  

May only be invoked in the Initialization state.  
";

%feature("docstring") helics::Core::getEndpoint "
`getEndpoint(federate_id_t federateID, const std::string &name) const =0 ->
    handle_id_t`  

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
`registerSourceFilter(const std::string &filterName, const std::string &source,
    const std::string &type_in, const std::string &type_out)=0 -> handle_id_t`  

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
`registerDestinationFilter(const std::string &filterName, const std::string
    &dest, const std::string &type_in, const std::string &type_out)=0 ->
    handle_id_t`  

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
`getSourceFilter(const std::string &name) const =0 -> handle_id_t`  

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
`getDestinationFilter(const std::string &name) const =0 -> handle_id_t`  

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
`addDependency(federate_id_t federateID, const std::string &federateName)=0`  

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
`registerFrequentCommunicationsPair(const std::string &source, const std::string
    &dest)=0`  

Register known frequently communicating source/destination end points.  

May be used for error checking for compatible types and possible optimization by
pre-registering the intent for these endpoints to communicate.  
";

%feature("docstring") helics::Core::send "
`send(handle_id_t sourceHandle, const std::string &destination, const char
    *data, uint64_t length)=0`  

Send data from source to destination.  

Time is implicitly defined as the end of the current time advancement window
(value returned by last call to nextTime().  

This send version was designed to enable communication of data between federates
with the possible introduction of source and destination filters to represent
properties of a communication network. This enables simulations to be run
with/without a communications model present.  
";

%feature("docstring") helics::Core::sendEvent "
`sendEvent(Time time, handle_id_t sourceHandle, const std::string &destination,
    const char *data, uint64_t length)=0`  

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
`sendMessage(handle_id_t sourceHandle, std::unique_ptr< Message > message)=0`  

Send for filters.  

Continues sending the message to the next filter or to final destination.  
";

%feature("docstring") helics::Core::receiveCount "
`receiveCount(handle_id_t destination)=0 -> uint64_t`  

Returns the number of pending receives for the specified destination endpoint or
filter.  
";

%feature("docstring") helics::Core::receive "
`receive(handle_id_t destination)=0 -> std::unique_ptr< Message >`  

Returns the next buffered message the specified destination endpoint or filter.  

this is a non-blocking call and will return a nullptr if no message are
available  
";

%feature("docstring") helics::Core::receiveAny "
`receiveAny(federate_id_t federateID, handle_id_t &enpoint_id)=0 ->
    std::unique_ptr< Message >`  

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
`receiveCountAny(federate_id_t federateID)=0 -> uint64_t`  

Returns number of messages for all destinations.  
";

%feature("docstring") helics::Core::logMessage "
`logMessage(federate_id_t federateID, int logLevel, const std::string
    &logMessage)=0`  

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
`setFilterOperator(handle_id_t filter, std::shared_ptr< FilterOperator >
    callback)=0`  

set the filter callback operator  

Parameters
----------
* `filter` :  
    the handle of the filter  
* `operator` :  
    pointer to the operator class executing the filter  
";

%feature("docstring") helics::Core::setLoggingCallback "
`setLoggingCallback(federate_id_t federateID, std::function< void(int, const
    std::string &, const std::string &)> logFunction)=0`  

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
`query(const std::string &target, const std::string &queryStr)=0 -> std::string`  

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
`setQueryCallback(federate_id_t federateID, std::function< std::string(const
    std::string &)> queryFunction)=0`  

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
`connect() override final -> bool`  

connect the core to its broker  

should be done after initialization has complete  
";

%feature("docstring") helics::CoreBroker::disconnect "
`disconnect() override final`  

disconnect the broker from any other brokers and communications  
";

%feature("docstring") helics::CoreBroker::unregister "
`unregister()`  

unregister the broker from the factory find methods  
";

%feature("docstring") helics::CoreBroker::processDisconnect "
`processDisconnect(bool skipUnregister=false) override final`  

disconnect the broker from any other brokers and communications if the flag is
set it should not do the unregister step of the disconnection, if this is set it
is presumed the unregistration has already happened or it will be taken care of
manually  
";

%feature("docstring") helics::CoreBroker::isConnected "
`isConnected() const override final -> bool`  

check if the broker is connected  
";

%feature("docstring") helics::CoreBroker::setAsRoot "
`setAsRoot() override final`  

set the broker to be a root broker  

only valid before the initialization function is called  
";

%feature("docstring") helics::CoreBroker::isRoot "
`isRoot() const override final -> bool`  

return true if the broker is a root broker  
";

%feature("docstring") helics::CoreBroker::isOpenToNewFederates "
`isOpenToNewFederates() const override -> bool`  

check if the broker is ready to accept new federates or cores  
";

%feature("docstring") helics::CoreBroker::CoreBroker "
`CoreBroker(bool setAsRootBroker=false) noexcept`  

default constructor  

Parameters
----------
* `setAsRootBroker` :  
    set to true to indicate this object is a root broker  
";

%feature("docstring") helics::CoreBroker::CoreBroker "
`CoreBroker(const std::string &broker_name)`  

constructor to set the name of the broker  
";

%feature("docstring") helics::CoreBroker::~CoreBroker "
`~CoreBroker()`  

destructor  
";

%feature("docstring") helics::CoreBroker::initialize "
`initialize(const std::string &initializationString) override final`  

start up the broker with an initialization string containing commands and
parameters  
";

%feature("docstring") helics::CoreBroker::initializeFromArgs "
`initializeFromArgs(int argc, const char *const *argv) override`  

initialize from command line arguments  
";

%feature("docstring") helics::CoreBroker::allInitReady "
`allInitReady() const -> bool`  

check if all the local federates are ready to be initialized  

Returns
-------
true if everyone is ready, false otherwise  
";

%feature("docstring") helics::CoreBroker::allDisconnected "
`allDisconnected() const -> bool`  
";

%feature("docstring") helics::CoreBroker::setIdentifier "
`setIdentifier(const std::string &name)`  

set the local identification string for the broker  
";

%feature("docstring") helics::CoreBroker::getIdentifier "
`getIdentifier() const override final -> const std::string &`  

get the local identification for the broker  
";

%feature("docstring") helics::CoreBroker::displayHelp "
`displayHelp()`  

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
`CoreObject()=default`  
";

%feature("docstring") helics::CoreObject::~CoreObject "
`~CoreObject()`  
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
`maxVal() noexcept -> constexpr baseType`  
";

%feature("docstring") count_time::minVal "
`minVal() noexcept -> constexpr baseType`  
";

%feature("docstring") count_time::zeroVal "
`zeroVal() noexcept -> constexpr baseType`  
";

%feature("docstring") count_time::epsilon "
`epsilon() noexcept -> constexpr baseType`  
";

%feature("docstring") count_time::convert "
`convert(double t) noexcept -> constexpr baseType`  
";

%feature("docstring") count_time::toDouble "
`toDouble(baseType val) noexcept -> double`  
";

%feature("docstring") count_time::toCount "
`toCount(baseType val, timeUnits units) noexcept -> std::int64_t`  
";

%feature("docstring") count_time::fromCount "
`fromCount(std::int64_t val, timeUnits units) noexcept -> baseType`  
";

%feature("docstring") count_time::seconds "
`seconds(baseType val) noexcept -> std::int64_t`  
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
`data_block() noexcept`  

default constructor  
";

%feature("docstring") helics::data_block::data_block "
`data_block(size_t blockSize)`  

size allocation constructor  
";

%feature("docstring") helics::data_block::data_block "
`data_block(size_t blockSize, char init)`  

size and data  
";

%feature("docstring") helics::data_block::data_block "
`data_block(const data_block &db)=default`  

copy constructor  
";

%feature("docstring") helics::data_block::data_block "
`data_block(data_block &&db) noexcept`  

move constructor  
";

%feature("docstring") helics::data_block::data_block "
`data_block(const char *s)`  

construct from char *  
";

%feature("docstring") helics::data_block::data_block "
`data_block(const std::string &str)`  

construct from string  
";

%feature("docstring") helics::data_block::data_block "
`data_block(std::string &&str) noexcept`  

move from string  
";

%feature("docstring") helics::data_block::data_block "
`data_block(const char *s, size_t len)`  

char * and length  
";

%feature("docstring") helics::data_block::data_block "
`data_block(const std::vector< char > &vdata)`  

construct from a vector object  
";

%feature("docstring") helics::data_block::data_block "
`data_block(const std::vector< X > &vdata)`  

construct from an arbitrary vector  
";

%feature("docstring") helics::data_block::assign "
`assign(const char *s, size_t len) -> data_block &`  

assignment from string and length  
";

%feature("docstring") helics::data_block::swap "
`swap(data_block &db2) noexcept`  

swap function  
";

%feature("docstring") helics::data_block::append "
`append(const char *s, size_t len)`  

append the existing data with a additional data  
";

%feature("docstring") helics::data_block::append "
`append(const std::string &str)`  

append the existing data with a string  
";

%feature("docstring") helics::data_block::data "
`data() -> char *`  

return a pointer to the data  
";

%feature("docstring") helics::data_block::data "
`data() const -> const char *`  

if the object is const return a const pointer  
";

%feature("docstring") helics::data_block::empty "
`empty() const noexcept -> bool`  

check if the block is empty  
";

%feature("docstring") helics::data_block::size "
`size() const -> size_t`  

get the size of the data block  
";

%feature("docstring") helics::data_block::resize "
`resize(size_t newSize)`  

resize the data storage  
";

%feature("docstring") helics::data_block::resize "
`resize(size_t newSize, char T)`  

resize the data storage  
";

%feature("docstring") helics::data_block::reserve "
`reserve(size_t space)`  

reserve space in a data_block  
";

%feature("docstring") helics::data_block::to_string "
`to_string() const -> const std::string &`  

get a string reference  
";

%feature("docstring") helics::data_block::begin "
`begin() -> auto`  

non const iterator  
";

%feature("docstring") helics::data_block::end "
`end() -> auto`  

non const iterator end  
";

%feature("docstring") helics::data_block::cbegin "
`cbegin() const -> auto`  

const iterator  
";

%feature("docstring") helics::data_block::cend "
`cend() const -> auto`  

const iterator end  
";

%feature("docstring") helics::data_block::push_back "
`push_back(char newchar)`  

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
`data_view() noexcept`  

default constructor  
";

%feature("docstring") helics::data_view::data_view "
`data_view(std::shared_ptr< const data_block > dt)`  

construct from a shared_ptr to a data_block  
";

%feature("docstring") helics::data_view::data_view "
`data_view(const data_block &dt) noexcept`  

construct from a regular data_block  
";

%feature("docstring") helics::data_view::data_view "
`data_view(const data_view &dt) noexcept=default`  

copy constructor  
";

%feature("docstring") helics::data_view::data_view "
`data_view(data_view &&dv) noexcept`  

move constructor  
";

%feature("docstring") helics::data_view::data_view "
`data_view(const char *dt) noexcept`  

construct from a string  
";

%feature("docstring") helics::data_view::data_view "
`data_view(const char *dt, size_t len) noexcept`  

construct from a char Pointer and length  
";

%feature("docstring") helics::data_view::data_view "
`data_view(const std::string &str) noexcept`  

construct from a string  
";

%feature("docstring") helics::data_view::data_view "
`data_view(std::string &&str)`  

construct from a rValue to a string  
";

%feature("docstring") helics::data_view::data_view "
`data_view(const std::vector< char > &dvec) noexcept`  

construct from a char vector  
";

%feature("docstring") helics::data_view::data_view "
`data_view(const stx::string_view &sview) noexcept`  

construct from a string_view  
";

%feature("docstring") helics::data_view::to_data_block "
`to_data_block() const -> data_block`  

create a new datablock from the data  
";

%feature("docstring") helics::data_view::swap "
`swap(data_view &dv2) noexcept`  

swap function  
";

%feature("docstring") helics::data_view::data "
`data() const noexcept -> const char *`  

get the data block  
";

%feature("docstring") helics::data_view::size "
`size() const noexcept -> size_t`  

get the length  
";

%feature("docstring") helics::data_view::empty "
`empty() const noexcept -> bool`  

check if the view is empty  
";

%feature("docstring") helics::data_view::string "
`string() const -> std::string`  

return a string of the data  

this actually does a copy to a new string  
";

%feature("docstring") helics::data_view::begin "
`begin() -> auto`  

begin iterator  
";

%feature("docstring") helics::data_view::end "
`end() -> auto`  

end iterator  
";

%feature("docstring") helics::data_view::cbegin "
`cbegin() const -> auto`  

begin const iterator  
";

%feature("docstring") helics::data_view::cend "
`cend() const -> auto`  

end const iterator  
";

// File: classDelayedDestructor.xml


%feature("docstring") DelayedDestructor "

helper class to destroy objects at a late time when it is convenient and there
are no more possibilities of threading issues  

C++ includes: delayedDestructor.hpp
";

%feature("docstring") DelayedDestructor::DelayedDestructor "
`DelayedDestructor()=default`  
";

%feature("docstring") DelayedDestructor::DelayedDestructor "
`DelayedDestructor(std::function< void(std::shared_ptr< X > &ptr)> callFirst)`  
";

%feature("docstring") DelayedDestructor::DelayedDestructor "
`DelayedDestructor(DelayedDestructor &&) noexcept=delete`  
";

%feature("docstring") DelayedDestructor::~DelayedDestructor "
`~DelayedDestructor()`  
";

%feature("docstring") DelayedDestructor::destroyObjects "
`destroyObjects() -> size_t`  
";

%feature("docstring") DelayedDestructor::destroyObjects "
`destroyObjects(int delay) -> size_t`  
";

%feature("docstring") DelayedDestructor::addObjectsToBeDestroyed "
`addObjectsToBeDestroyed(std::shared_ptr< X > &&obj)`  
";

%feature("docstring") DelayedDestructor::addObjectsToBeDestroyed "
`addObjectsToBeDestroyed(std::shared_ptr< X > &obj)`  
";

// File: classDelayedObjects.xml


%feature("docstring") DelayedObjects "

class holding a map of delayed object  

C++ includes: DelayedObjects.hpp
";

%feature("docstring") DelayedObjects::DelayedObjects "
`DelayedObjects()=default`  
";

%feature("docstring") DelayedObjects::DelayedObjects "
`DelayedObjects(const DelayedObjects &)=delete`  
";

%feature("docstring") DelayedObjects::DelayedObjects "
`DelayedObjects(DelayedObjects &&)=delete`  
";

%feature("docstring") DelayedObjects::~DelayedObjects "
`~DelayedObjects()`  
";

%feature("docstring") DelayedObjects::setDelayedValue "
`setDelayedValue(int index, const X &val)`  
";

%feature("docstring") DelayedObjects::setDelayedValue "
`setDelayedValue(const std::string &name, const X &val)`  
";

%feature("docstring") DelayedObjects::getFuture "
`getFuture(int index) -> std::future< X >`  
";

%feature("docstring") DelayedObjects::getFuture "
`getFuture(const std::string &name) -> std::future< X >`  
";

%feature("docstring") DelayedObjects::finishedWithValue "
`finishedWithValue(int index)`  
";

%feature("docstring") DelayedObjects::finishedWithValue "
`finishedWithValue(const std::string &name)`  
";

// File: classhelics_1_1DelayFilterOperation.xml


%feature("docstring") helics::DelayFilterOperation "

filter for delaying a message in time  

C++ includes: FilterOperations.hpp
";

%feature("docstring") helics::DelayFilterOperation::DelayFilterOperation "
`DelayFilterOperation(Time delayTime=timeZero)`  
";

%feature("docstring") helics::DelayFilterOperation::set "
`set(const std::string &property, double val) override`  

set a property on a filter  

Parameters
----------
* `property` :  
    the name of the property of the filter to change  
* `val` :  
    the numerical value of the property  
";

%feature("docstring") helics::DelayFilterOperation::getOperator "
`getOperator() override -> std::shared_ptr< FilterOperator >`  
";

// File: classhelics_1_1DependencyInfo.xml


%feature("docstring") helics::DependencyInfo "

data class containing information about inter-federate dependencies  

C++ includes: TimeDependencies.hpp
";

%feature("docstring") helics::DependencyInfo::DependencyInfo "
`DependencyInfo()=default`  

default constructor  
";

%feature("docstring") helics::DependencyInfo::DependencyInfo "
`DependencyInfo(Core::federate_id_t id)`  

construct from a federate id  
";

%feature("docstring") helics::DependencyInfo::ProcessMessage "
`ProcessMessage(const ActionMessage &m) -> bool`  
";

// File: classhelics_1_1DestinationFilter.xml


%feature("docstring") helics::DestinationFilter "

class wrapping a destination filter  

C++ includes: Filters.hpp
";

%feature("docstring") helics::DestinationFilter::DestinationFilter "
`DestinationFilter(Federate *fed, const std::string &target, const std::string
    &name=EMPTY_STRING, const std::string &input_type=EMPTY_STRING, const
    std::string &output_type=EMPTY_STRING)`  

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
`DestinationFilter(Core *cr, const std::string &target, const std::string
    &name=EMPTY_STRING, const std::string &input_type=EMPTY_STRING, const
    std::string &output_type=EMPTY_STRING)`  

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
`~DestinationFilter()=default`  
";

// File: classdouble__time.xml


%feature("docstring") double_time "

class representing time as a floating point value  

C++ includes: timeRepresentation.hpp
";

%feature("docstring") double_time::convert "
`convert(double t) noexcept -> constexpr baseType`  
";

%feature("docstring") double_time::toDouble "
`toDouble(baseType val) noexcept -> constexpr double`  
";

%feature("docstring") double_time::maxVal "
`maxVal() noexcept -> constexpr baseType`  
";

%feature("docstring") double_time::minVal "
`minVal() noexcept -> constexpr baseType`  
";

%feature("docstring") double_time::zeroVal "
`zeroVal() noexcept -> constexpr baseType`  
";

%feature("docstring") double_time::epsilon "
`epsilon() noexcept -> constexpr baseType`  
";

%feature("docstring") double_time::toCount "
`toCount(baseType val, timeUnits units) noexcept -> std::int64_t`  
";

%feature("docstring") double_time::fromCount "
`fromCount(std::int64_t val, timeUnits units) noexcept -> baseType`  
";

%feature("docstring") double_time::seconds "
`seconds(baseType val) noexcept -> constexpr std::int64_t`  
";

// File: classDualMappedVector.xml


%feature("docstring") DualMappedVector "
";

%feature("docstring") DualMappedVector::insert "
`insert(const searchType1 &searchValue1, const searchType2 &searchValue2, Us
    &&... data) -> auto &`  
";

%feature("docstring") DualMappedVector::find "
`find(const searchType1 &searchValue) const -> auto`  
";

%feature("docstring") DualMappedVector::find "
`find(const searchType2 &searchValue) const -> auto`  
";

%feature("docstring") DualMappedVector::find "
`find(const searchType1 &searchValue) -> auto`  
";

%feature("docstring") DualMappedVector::find "
`find(const searchType2 &searchValue) -> auto`  
";

%feature("docstring") DualMappedVector::begin "
`begin() -> auto`  
";

%feature("docstring") DualMappedVector::end "
`end() -> auto`  
";

%feature("docstring") DualMappedVector::cbegin "
`cbegin() const -> auto`  
";

%feature("docstring") DualMappedVector::cend "
`cend() const -> auto`  
";

%feature("docstring") DualMappedVector::size "
`size() const -> auto`  
";

%feature("docstring") DualMappedVector::clear "
`clear()`  
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
`Echo()=default`  

default constructor  
";

%feature("docstring") helics::Echo::Echo "
`Echo(int argc, char *argv[])`  

construct from command line arguments  

Parameters
----------
* `argc` :  
    the number of arguments  
* `argv` :  
    the strings in the input  
";

%feature("docstring") helics::Echo::Echo "
`Echo(const FederateInfo &fi)`  

construct from a federate info object  

Parameters
----------
* `fi` :  
    a pointer info object containing information on the desired federate
    configuration  
";

%feature("docstring") helics::Echo::Echo "
`Echo(std::shared_ptr< Core > core, const FederateInfo &fi)`  

constructor taking a federate information structure and using the given core  

Parameters
----------
* `core` :  
    a pointer to core object which the federate can join  
* `fi` :  
    a federate information structure  
";

%feature("docstring") helics::Echo::Echo "
`Echo(const std::string &jsonString)`  

constructor taking a file with the required information  

Parameters
----------
* `jsonString` :  
    file or json string defining the federate information and other
    configuration  
";

%feature("docstring") helics::Echo::Echo "
`Echo(Echo &&other_echo)=default`  

move construction  
";

%feature("docstring") helics::Echo::Echo "
`Echo(const Echo &other_echo)=delete`  

don't allow the copy constructor  
";

%feature("docstring") helics::Echo::~Echo "
`~Echo()`  
";

%feature("docstring") helics::Echo::loadFile "
`loadFile(const std::string &filename)`  

load a file containing publication information  

Parameters
----------
* `filename` :  
    the file containing the configuration and Player data accepted format are
    json, xml, and a Player format which is tab delimited or comma delimited  
";

%feature("docstring") helics::Echo::initialize "
`initialize()`  

initialize the Player federate  

generate all the publications and organize the points, the final publication
count will be available after this time and the Player will enter the
initialization mode, which means it will not be possible to add more
publications calling run will automatically do this if necessary  
";

%feature("docstring") helics::Echo::run "
`run()`  
";

%feature("docstring") helics::Echo::run "
`run(Time stopTime_input)`  

run the Echo federate until the specified time  

Parameters
----------
* `stopTime_input` :  
    the desired stop time  
";

%feature("docstring") helics::Echo::addEndpoint "
`addEndpoint(const std::string &endpointName, const std::string
    &endpointType=\"\")`  

add an endpoint to the Player  

Parameters
----------
* `endpointName` :  
    the name of the endpoint  
* `endpointType` :  
    the named type of the endpoint  
";

%feature("docstring") helics::Echo::echoCount "
`echoCount() const -> auto`  

get the number of points loaded  
";

%feature("docstring") helics::Echo::endpointCount "
`endpointCount() const -> auto`  

get the number of endpoints  
";

// File: classhelics_1_1Endpoint.xml


%feature("docstring") helics::Endpoint "

class to manage an endpoint  

C++ includes: Endpoints.hpp
";

%feature("docstring") helics::Endpoint::Endpoint "
`Endpoint(MessageFederate *mFed, const std::string &name, const std::string
    &type=\"\")`  

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
`Endpoint(interface_visibility locality, MessageFederate *mFed, const
    std::string &name, const std::string &type=\"\")`  

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
`Endpoint(MessageFederate *mFed, int endpointIndex)`  
";

%feature("docstring") helics::Endpoint::send "
`send(const std::string &dest, const char *data, size_t data_size) const`  

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
`send(const std::string &dest, const char *data, size_t data_size, Time
    sendTime) const`  

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
`send(const char *data, size_t data_size, Time sendTime) const`  

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
`send(const std::string &dest, data_view data) const`  

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
`send(const std::string &dest, data_view data, Time sendTime) const`  

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
`send(const char *data, size_t data_size) const`  

send a data block and length to the target destination  

Parameters
----------
* `data` :  
    pointer to data location  
* `data_size` :  
    the length of the data  
";

%feature("docstring") helics::Endpoint::send "
`send(data_view data) const`  

send a data_view to the target destination  

a data view can convert from many different formats so this function should be
catching many of the common use cases  

Parameters
----------
* `data` :  
    the information to send  
";

%feature("docstring") helics::Endpoint::send "
`send(data_view data, Time sendTime) const`  

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
`send(Message &mess) const`  

send a message object  

this is to send a pre-built message  

Parameters
----------
* `mess` :  
    a reference to an actual message object  
";

%feature("docstring") helics::Endpoint::subscribe "
`subscribe(const std::string &key, const std::string &type)`  

subscribe the endpoint to a particular publication  
";

%feature("docstring") helics::Endpoint::getMessage "
`getMessage() const -> auto`  

get an available message if there is no message the returned object is empty  
";

%feature("docstring") helics::Endpoint::hasMessage "
`hasMessage() const -> bool`  

check if there is a message available  
";

%feature("docstring") helics::Endpoint::receiveCount "
`receiveCount() const -> auto`  

check if there is a message available  
";

%feature("docstring") helics::Endpoint::setCallback "
`setCallback(std::function< void(endpoint_id_t, Time)> callback)`  

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
`setTargetDestination(const std::string &target)`  

set a target destination for unspecified messages  
";

%feature("docstring") helics::Endpoint::getName "
`getName() const -> std::string`  

get the name of the endpoint  
";

%feature("docstring") helics::Endpoint::getType "
`getType() const -> std::string`  

get the specified type of the endpoint  
";

%feature("docstring") helics::Endpoint::getID "
`getID() const -> endpoint_id_t`  

get the actual endpoint id for the fed  
";

// File: structhelics_1_1endpoint__info.xml


%feature("docstring") helics::endpoint_info "

structure containing information about an endpoint  

C++ includes: MessageFederateManager.hpp
";

%feature("docstring") helics::endpoint_info::endpoint_info "
`endpoint_info(std::string n_name, std::string n_type)`  
";

// File: classhelics_1_1EndpointInfo.xml


%feature("docstring") helics::EndpointInfo "

data class containing the information about an endpoint  

C++ includes: EndpointInfo.hpp
";

%feature("docstring") helics::EndpointInfo::EndpointInfo "
`EndpointInfo(Core::handle_id_t id_, Core::federate_id_t fed_id_, const
    std::string &key_, const std::string &type_)`  

constructor from all data  
";

%feature("docstring") helics::EndpointInfo::getMessage "
`getMessage(Time maxTime) -> std::unique_ptr< Message >`  

get the next message up to the specified time  
";

%feature("docstring") helics::EndpointInfo::queueSize "
`queueSize(Time maxTime) const -> int32_t`  

get the number of messages in the queue up to the specified time  
";

%feature("docstring") helics::EndpointInfo::addMessage "
`addMessage(std::unique_ptr< Message > message)`  

add a message to the queue  
";

%feature("docstring") helics::EndpointInfo::firstMessageTime "
`firstMessageTime() const -> Time`  

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
`error_t()`  
";

%feature("docstring") zmq::error_t::what "
`what() const -> const char *`  
";

%feature("docstring") zmq::error_t::num "
`num() const -> int`  
";

// File: classhelics_1_1Federate.xml


%feature("docstring") helics::Federate "

base class for a federate in the application API  

C++ includes: Federate.hpp
";

%feature("docstring") helics::Federate::Federate "
`Federate(const FederateInfo &fi)`  

constructor taking a federate information structure  

Parameters
----------
* `fi` :  
    a federate information structure  

make sure the core is connected  
";

%feature("docstring") helics::Federate::Federate "
`Federate(std::shared_ptr< Core > core, const FederateInfo &fi)`  

constructor taking a core and a federate information structure  

Parameters
----------
* `fi` :  
    a federate information structure  

make sure the core is connected  
";

%feature("docstring") helics::Federate::Federate "
`Federate(const std::string &jsonString)`  

constructor taking a file with the required information  

Parameters
----------
* `jsonString` :  
    can be either a JSON file or a string containing JSON code  
";

%feature("docstring") helics::Federate::Federate "
`Federate() noexcept`  
";

%feature("docstring") helics::Federate::Federate "
`Federate(Federate &&fed) noexcept`  
";

%feature("docstring") helics::Federate::Federate "
`Federate(const Federate &fed)=delete`  
";

%feature("docstring") helics::Federate::Federate "
`Federate()`  
";

%feature("docstring") helics::Federate::~Federate "
`~Federate()`  

virtual destructor function  
";

%feature("docstring") helics::Federate::~Federate "
`~Federate()`  
";

%feature("docstring") helics::Federate::enterInitializationState "
`enterInitializationState()`  

enter the initialization mode after all interfaces have been defined  

the call will block until all federates have entered initialization mode  
";

%feature("docstring") helics::Federate::enterInitializationState "
`enterInitializationState()`  
";

%feature("docstring") helics::Federate::enterInitializationStateAsync "
`enterInitializationStateAsync()`  

enter the initialization mode after all interfaces have been defined  

the call will not block  
";

%feature("docstring") helics::Federate::enterInitializationStateAsync "
`enterInitializationStateAsync()`  
";

%feature("docstring") helics::Federate::isAsyncOperationCompleted "
`isAsyncOperationCompleted() const -> bool`  

called after one of the async calls and will indicate true if an async operation
has completed  

only call from the same thread as the one that called the initial async call and
will return false if called when no aysnc operation is in flight  
";

%feature("docstring") helics::Federate::isAsyncOperationCompleted "
`isAsyncOperationCompleted() const -> bool`  
";

%feature("docstring") helics::Federate::enterInitializationStateComplete "
`enterInitializationStateComplete()`  

second part of the async process for entering initializationState call after a
call to enterInitializationStateAsync if call any other time it will throw an
InvalidFunctionCall exception  
";

%feature("docstring") helics::Federate::enterInitializationStateComplete "
`enterInitializationStateComplete()`  
";

%feature("docstring") helics::Federate::enterExecutionState "
`enterExecutionState(helics_iteration_request
    iterate=helics_iteration_request::no_iterations) -> iteration_result`  

enter the normal execution mode  

call will block until all federates have entered this mode  
";

%feature("docstring") helics::Federate::enterExecutionState "
`enterExecutionState(helics_iteration_request iterate=no_iteration) ->
    helics_iteration_status`  
";

%feature("docstring") helics::Federate::enterExecutionStateAsync "
`enterExecutionStateAsync(helics_iteration_request
    iterate=helics_iteration_request::no_iterations)`  

enter the normal execution mode  

call will block until all federates have entered this mode  
";

%feature("docstring") helics::Federate::enterExecutionStateAsync "
`enterExecutionStateAsync(helics_iteration_request iterate=no_iteration)`  
";

%feature("docstring") helics::Federate::enterExecutionStateComplete "
`enterExecutionStateComplete() -> iteration_result`  

complete the async call for entering Execution state  

call will not block but will return quickly. The
enterInitializationStateFinalize must be called before doing other operations  
";

%feature("docstring") helics::Federate::enterExecutionStateComplete "
`enterExecutionStateComplete() -> helics_iteration_status`  
";

%feature("docstring") helics::Federate::finalize "
`finalize()`  

terminate the simulation  

call is normally non-blocking, but may block if called in the midst of an
asynchronous call sequence, no core calling commands may be called after
completion of this function  
";

%feature("docstring") helics::Federate::finalize "
`finalize()`  
";

%feature("docstring") helics::Federate::disconnect "
`disconnect()`  

disconnect a simulation from the core (will also call finalize before
disconnecting if necessary)  
";

%feature("docstring") helics::Federate::error "
`error(int errorcode)`  

specify the simulator had an unrecoverable error  
";

%feature("docstring") helics::Federate::error "
`error(int errorcode, const std::string &message)`  

specify the simulator had an error with error code and message  
";

%feature("docstring") helics::Federate::setSeparator "
`setSeparator(char separator)`  
";

%feature("docstring") helics::Federate::requestTime "
`requestTime(Time nextInternalTimeStep) -> Time`  

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
`requestTime(helics_time_t time) -> helics_time_t`  
";

%feature("docstring") helics::Federate::requestTimeIterative "
`requestTimeIterative(Time nextInternalTimeStep, helics_iteration_request
    iterate) -> iteration_time`  

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
`requestTimeIterative(helics_time_t time, helics_iteration_request iterate) ->
    helics_iteration_time`  
";

%feature("docstring") helics::Federate::requestTimeAsync "
`requestTimeAsync(Time nextInternalTimeStep)`  

request a time advancement  

Parameters
----------
* `the` :  
    next requested time step  
";

%feature("docstring") helics::Federate::requestTimeAsync "
`requestTimeAsync(helics_time_t time)`  
";

%feature("docstring") helics::Federate::requestTimeIterativeAsync "
`requestTimeIterativeAsync(Time nextInternalTimeStep, helics_iteration_request
    iterate)`  

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
`requestTimeIterativeAsync(helics_time_t time, helics_iteration_request
    iterate)`  
";

%feature("docstring") helics::Federate::requestTimeComplete "
`requestTimeComplete() -> Time`  

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
`requestTimeComplete() -> helics_time_t`  
";

%feature("docstring") helics::Federate::requestTimeIterativeComplete "
`requestTimeIterativeComplete() -> iteration_time`  

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
`requestTimeIterativeComplete() -> helics_iteration_time`  
";

%feature("docstring") helics::Federate::setTimeDelta "
`setTimeDelta(Time tdelta)`  

set the minimum time delta for the federate  

Parameters
----------
* `tdelta` :  
    the minimum time delta to return from a time request function  
";

%feature("docstring") helics::Federate::setOutputDelay "
`setOutputDelay(Time outputDelay)`  

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
`setInputDelay(Time inputDelay)`  

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
`setPeriod(Time period, Time offset=timeZero)`  

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
`setFlag(int flag, bool flagValue=true)`  

set a flag for the federate  

Parameters
----------
* `flag` :  
    an index into the flag /ref flag-definitions.h  
* `flagvalue` :  
    the value of the flag defaults to true  
";

%feature("docstring") helics::Federate::setLoggingLevel "
`setLoggingLevel(int loggingLevel)`  

set the logging level for the federate @ details debug and trace only do
anything if they were enabled in the compilation  

Parameters
----------
* `loggingLevel` :  
    (-1: none, 0: error_only, 1: warnings, 2: normal, 3: debug, 4: trace)  
";

%feature("docstring") helics::Federate::query "
`query(const std::string &target, const std::string &queryStr) -> std::string`  

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
`query(const std::string &queryStr) -> std::string`  

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
`query(const std::string &target, const std::string &queryStr) -> std::string`  

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
`queryAsync(const std::string &target, const std::string &queryStr) ->
    query_id_t`  

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
`queryAsync(const std::string &queryStr) -> query_id_t`  

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
`queryComplete(query_id_t queryIndex) -> std::string`  

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
`isQueryCompleted(query_id_t queryIndex) const -> bool`  

check if an asynchronous query call has been completed  

Returns
-------
true if the results are ready for  
";

%feature("docstring") helics::Federate::registerSourceFilter "
`registerSourceFilter(const std::string &filterName, const std::string
    &sourceEndpoint, const std::string &inputType=\"\", const std::string
    &outputType=\"\") -> filter_id_t`  

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
`registerSourceFilter(const std::string &sourceEndpoint) -> filter_id_t`  

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
`registerDestinationFilter(const std::string &filterName, const std::string
    &destEndpoint, const std::string &inputType=\"\", const std::string
    &outputType=\"\") -> filter_id_t`  

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
`registerDestinationFilter(const std::string &destEndpoint) -> filter_id_t`  

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
`getFilterName(filter_id_t id) const -> std::string`  

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
`getFilterEndpoint(filter_id_t id) const -> std::string`  

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
`getFilterInputType(filter_id_t id) const -> std::string`  

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
`getFilterOutputType(filter_id_t id) const -> std::string`  

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
`getFilterId(const std::string &filterName) const -> filter_id_t`  

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
`getSourceFilterId(const std::string &filterName) const -> filter_id_t`  

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
`getDestFilterId(const std::string &filterName) const -> filter_id_t`  

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
`setFilterOperator(filter_id_t filter, std::shared_ptr< FilterOperator > op)`  

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
`setFilterOperator(const std::vector< filter_id_t > &filters, std::shared_ptr<
    FilterOperator > op)`  

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
`registerInterfaces(const std::string &jsonString)`  

register a set of interfaces defined in a file  

call is only valid in startup mode  

Parameters
----------
* `jsonString` :  
    the location of the file or json String to load to generate the interfaces  
";

%feature("docstring") helics::Federate::getID "
`getID() const noexcept -> unsigned int`  

get the underlying federateID for the core  
";

%feature("docstring") helics::Federate::getCurrentState "
`getCurrentState() const -> op_states`  

get the current state of the federate  
";

%feature("docstring") helics::Federate::getCurrentTime "
`getCurrentTime() const -> Time`  

get the current Time  

the most recent granted time of the federate  
";

%feature("docstring") helics::Federate::getName "
`getName() const -> const std::string &`  

get the federate name  
";

%feature("docstring") helics::Federate::getCorePointer "
`getCorePointer() -> std::shared_ptr< Core >`  

get a pointer to the core object used by the federate  
";

// File: classhelics_1_1FederateInfo.xml


%feature("docstring") helics::FederateInfo "

data class defining federate properties and information  

C++ includes: Federate.hpp
";

%feature("docstring") helics::FederateInfo::FederateInfo "
`FederateInfo()=default`  

default constructor  
";

%feature("docstring") helics::FederateInfo::FederateInfo "
`FederateInfo(std::string fedname)`  

construct from the federate name  
";

%feature("docstring") helics::FederateInfo::FederateInfo "
`FederateInfo(std::string fedname, core_type cType)`  

construct from the name and type  
";

%feature("docstring") helics::FederateInfo::FederateInfo "
`FederateInfo(int argc, const char *const *argv)`  

load a federateInfo object from command line arguments  

Parameters
----------
* `argc` :  
    the number of arguments  
* `argv` :  
    an array of char * pointers to the arguments  
";

%feature("docstring") helics::FederateInfo::FederateInfo "
`FederateInfo()`  
";

%feature("docstring") helics::FederateInfo::FederateInfo "
`FederateInfo(std::string fedname)`  
";

%feature("docstring") helics::FederateInfo::FederateInfo "
`FederateInfo(std::string fedname, std::string coretype)`  
";

%feature("docstring") helics::FederateInfo::loadInfoFromArgs "
`loadInfoFromArgs(int argc, const char *const *argv)`  

load a federateInfo object from command line arguments  

Parameters
----------
* `argc` :  
    the number of arguments  
* `argv` :  
    an array of char * pointers to the arguments  
";

%feature("docstring") helics::FederateInfo::~FederateInfo "
`~FederateInfo()`  
";

%feature("docstring") helics::FederateInfo::setFederateName "
`setFederateName(std::string name)`  
";

%feature("docstring") helics::FederateInfo::setCoreName "
`setCoreName(std::string corename)`  
";

%feature("docstring") helics::FederateInfo::setCoreInitString "
`setCoreInitString(std::string coreInit)`  
";

%feature("docstring") helics::FederateInfo::setCoreTypeFromString "
`setCoreTypeFromString(std::string coretype)`  
";

%feature("docstring") helics::FederateInfo::setCoreType "
`setCoreType(int coretype)`  
";

%feature("docstring") helics::FederateInfo::setFlag "
`setFlag(int flag, int value)`  
";

%feature("docstring") helics::FederateInfo::setOutputDelay "
`setOutputDelay(helics_time_t outputDelay)`  
";

%feature("docstring") helics::FederateInfo::setTimeDelta "
`setTimeDelta(helics_time_t timeDelta)`  
";

%feature("docstring") helics::FederateInfo::setInputDelay "
`setInputDelay(helics_time_t inputDelay)`  
";

%feature("docstring") helics::FederateInfo::setTimeOffset "
`setTimeOffset(helics_time_t timeOffset)`  
";

%feature("docstring") helics::FederateInfo::setPeriod "
`setPeriod(helics_time_t period)`  
";

%feature("docstring") helics::FederateInfo::setMaxIterations "
`setMaxIterations(int max_iterations)`  
";

%feature("docstring") helics::FederateInfo::setLoggingLevel "
`setLoggingLevel(int logLevel)`  
";

%feature("docstring") helics::FederateInfo::getInfo "
`getInfo() -> helics_federate_info_t`  
";

// File: classhelics_1_1FederateState.xml


%feature("docstring") helics::FederateState "

class managing the information about a single federate  

C++ includes: FederateState.hpp
";

%feature("docstring") helics::FederateState::FederateState "
`FederateState(const std::string &name_, const CoreFederateInfo &info_)`  

constructor from name and information structure  
";

%feature("docstring") helics::FederateState::~FederateState "
`~FederateState()`  

destructor  
";

%feature("docstring") helics::FederateState::reset "
`reset()`  

reset the federate to created state  
";

%feature("docstring") helics::FederateState::reInit "
`reInit()`  

reset the federate to the initializing state  
";

%feature("docstring") helics::FederateState::getIdentifier "
`getIdentifier() const -> const std::string &`  

get the name of the federate  
";

%feature("docstring") helics::FederateState::getState "
`getState() const -> helics_federate_state_type`  
";

%feature("docstring") helics::FederateState::getSubscription "
`getSubscription(const std::string &subName) const -> SubscriptionInfo *`  
";

%feature("docstring") helics::FederateState::getSubscription "
`getSubscription(Core::handle_id_t handle_) const -> SubscriptionInfo *`  
";

%feature("docstring") helics::FederateState::getPublication "
`getPublication(const std::string &pubName) const -> PublicationInfo *`  
";

%feature("docstring") helics::FederateState::getPublication "
`getPublication(Core::handle_id_t handle_) const -> PublicationInfo *`  
";

%feature("docstring") helics::FederateState::getEndpoint "
`getEndpoint(const std::string &endpointName) const -> EndpointInfo *`  
";

%feature("docstring") helics::FederateState::getEndpoint "
`getEndpoint(Core::handle_id_t handle_) const -> EndpointInfo *`  
";

%feature("docstring") helics::FederateState::createSubscription "
`createSubscription(Core::handle_id_t handle, const std::string &key, const
    std::string &type, const std::string &units, handle_check_mode check_mode)`  
";

%feature("docstring") helics::FederateState::createPublication "
`createPublication(Core::handle_id_t handle, const std::string &key, const
    std::string &type, const std::string &units)`  
";

%feature("docstring") helics::FederateState::createEndpoint "
`createEndpoint(Core::handle_id_t handle, const std::string &key, const
    std::string &type)`  
";

%feature("docstring") helics::FederateState::getQueueSize "
`getQueueSize(Core::handle_id_t id) const -> uint64_t`  

get the size of a message queue for a specific endpoint or filter handle  
";

%feature("docstring") helics::FederateState::getQueueSize "
`getQueueSize() const -> uint64_t`  

get the sum of all message queue sizes i.e. the total number of messages
available in all endpoints  
";

%feature("docstring") helics::FederateState::getCurrentIteration "
`getCurrentIteration() const -> int32_t`  

get the current iteration counter for an iterative call  

this will work properly even when a federate is processing  
";

%feature("docstring") helics::FederateState::receive "
`receive(Core::handle_id_t id) -> std::unique_ptr< Message >`  

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
`receiveAny(Core::handle_id_t &id) -> std::unique_ptr< Message >`  

get any message ready for reception  

Parameters
----------
* `id` :  
    the endpoint related to the message  
";

%feature("docstring") helics::FederateState::setParent "
`setParent(CommonCore *coreObject)`  

set the CommonCore object that is managing this Federate  
";

%feature("docstring") helics::FederateState::getInfo "
`getInfo() const -> CoreFederateInfo`  

specify the core object that manages this federate get the info structure for
the federate  
";

%feature("docstring") helics::FederateState::updateFederateInfo "
`updateFederateInfo(const ActionMessage &cmd)`  

update the info structure  

public call so it also calls the federate lock before calling private update
function the action Message should be CMD_FED_CONFIGURE  
";

%feature("docstring") helics::FederateState::grantedTime "
`grantedTime() const -> Time`  

get the granted time of a federate  
";

%feature("docstring") helics::FederateState::getEvents "
`getEvents() const -> const std::vector< Core::handle_id_t > &`  

get a reference to the handles of subscriptions with value updates  

< if we are processing this vector is in an undefined state  
";

%feature("docstring") helics::FederateState::getDependents "
`getDependents() const -> const std::vector< Core::federate_id_t > &`  

get a reference to the global ids of dependent federates  
";

%feature("docstring") helics::FederateState::setCoreObject "
`setCoreObject(CommonCore *parent)`  
";

%feature("docstring") helics::FederateState::waitSetup "
`waitSetup() -> iteration_result`  

process until the federate has verified its membership and assigned a global id
number  
";

%feature("docstring") helics::FederateState::enterInitState "
`enterInitState() -> iteration_result`  

process until the init state has been entered or there is a failure  
";

%feature("docstring") helics::FederateState::enterExecutingState "
`enterExecutingState(helics_iteration_request iterate) -> iteration_result`  

function to call when entering execution state  

Parameters
----------
* `converged` :  
    indicator of whether the fed should iterate if need be or not returns either
    converged or nonconverged depending on whether an iteration is needed  
";

%feature("docstring") helics::FederateState::requestTime "
`requestTime(Time nextTime, helics_iteration_request iterate) -> iteration_time`  

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
`genericUnspecifiedQueueProcess() -> iteration_result`  

function to process the queue in a generic fashion used to just process messages
with no specific end in mind  
";

%feature("docstring") helics::FederateState::addAction "
`addAction(const ActionMessage &action)`  

add an action message to the queue  
";

%feature("docstring") helics::FederateState::addAction "
`addAction(ActionMessage &&action)`  

move a message to the queue  
";

%feature("docstring") helics::FederateState::logMessage "
`logMessage(int level, const std::string &logMessageSource, const std::string
    &message) const`  

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
`setLogger(std::function< void(int, const std::string &, const std::string &)>
    logFunction)`  

set the logging function  

function must have signature void(int level, const std::string &sourceName,
const std::string &message)  
";

%feature("docstring") helics::FederateState::setQueryCallback "
`setQueryCallback(std::function< std::string(const std::string &)>
    queryCallbackFunction)`  

set the query callback function  

function must have signature std::string(const std::string &query)  
";

%feature("docstring") helics::FederateState::processQuery "
`processQuery(const std::string &query) const -> std::string`  

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
`checkSetValue(Core::handle_id_t pub_id, const char *data, uint64_t len) const
    -> bool`  

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
`FedObject()=default`  
";

%feature("docstring") helics::FedObject::~FedObject "
`~FedObject()`  
";

// File: classhelics_1_1Filter.xml


%feature("docstring") helics::Filter "

class for managing a particular filter  

C++ includes: Filters.hpp
";

%feature("docstring") helics::Filter::Filter "
`Filter()=default`  

default constructor  
";

%feature("docstring") helics::Filter::Filter "
`Filter(Federate *fed)`  

construct through a federate  
";

%feature("docstring") helics::Filter::Filter "
`Filter(Core *cr)`  

construct through a core object  
";

%feature("docstring") helics::Filter::~Filter "
`~Filter()=default`  

virtual destructor  
";

%feature("docstring") helics::Filter::setOperator "
`setOperator(std::shared_ptr< FilterOperator > mo)`  

set a message operator to process the message  
";

%feature("docstring") helics::Filter::getID "
`getID() const -> filter_id_t`  
";

%feature("docstring") helics::Filter::getCoreHandle "
`getCoreHandle() const -> Core::handle_id_t`  
";

%feature("docstring") helics::Filter::getTarget "
`getTarget() const -> const std::string &`  

get the target of the filter  
";

%feature("docstring") helics::Filter::getName "
`getName() const -> const std::string &`  

get the name for the filter  
";

%feature("docstring") helics::Filter::getInputType "
`getInputType() const -> const std::string &`  

get the specified input type of the filter  
";

%feature("docstring") helics::Filter::getOutputType "
`getOutputType() const -> const std::string &`  

get the specified output type of the filter  
";

%feature("docstring") helics::Filter::set "
`set(const std::string &property, double val)`  

set a property on a filter  

Parameters
----------
* `property` :  
    the name of the property of the filter to change  
* `val` :  
    the numerical value of the property  
";

%feature("docstring") helics::Filter::setString "
`setString(const std::string &property, const std::string &val)`  

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
`FilterInfo(Core::federate_id_t fed_id_, Core::handle_id_t handle_, const
    std::string &key_, const std::string &target_, const std::string &type_in_,
    const std::string &type_out_, bool destFilter_)`  

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
`FilterOperations()=default`  
";

%feature("docstring") helics::FilterOperations::FilterOperations "
`FilterOperations(const FilterOperations &fo)=delete`  
";

%feature("docstring") helics::FilterOperations::FilterOperations "
`FilterOperations(FilterOperations &&fo)=delete`  
";

%feature("docstring") helics::FilterOperations::~FilterOperations "
`~FilterOperations()=default`  
";

%feature("docstring") helics::FilterOperations::set "
`set(const std::string &property, double val)`  

set a property on a filter  

Parameters
----------
* `property` :  
    the name of the property of the filter to change  
* `val` :  
    the numerical value of the property  
";

%feature("docstring") helics::FilterOperations::setString "
`setString(const std::string &property, const std::string &val)`  

set a string property on a filter  

Parameters
----------
* `property` :  
    the name of the property of the filter to change  
* `val` :  
    the numerical value of the property  
";

%feature("docstring") helics::FilterOperations::getOperator "
`getOperator()=0 -> std::shared_ptr< FilterOperator >`  
";

// File: classhelics_1_1FilterOperator.xml


%feature("docstring") helics::FilterOperator "

FilterOperator abstract class  

FilterOperators will transform a message in some way in a direct fashion  

C++ includes: core-data.hpp
";

%feature("docstring") helics::FilterOperator::FilterOperator "
`FilterOperator()=default`  

default constructor  
";

%feature("docstring") helics::FilterOperator::~FilterOperator "
`~FilterOperator()=default`  

virtual destructor  
";

%feature("docstring") helics::FilterOperator::process "
`process(std::unique_ptr< Message > message)=0 -> std::unique_ptr< Message >`  

filter the message either modify the message or generate a new one  
";

// File: classhelics_1_1FunctionExecutionFailure.xml


%feature("docstring") helics::FunctionExecutionFailure "

exception class indicating that a function has failed for some reason  

C++ includes: core-exceptions.hpp
";

%feature("docstring") helics::FunctionExecutionFailure::FunctionExecutionFailure "
`FunctionExecutionFailure(const std::string &message=\"HELICS execution
    failure\")`  
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
`HelicsException()=default`  
";

%feature("docstring") helics::HelicsException::HelicsException "
`HelicsException(const std::string &message)`  
";

%feature("docstring") helics::HelicsException::what "
`what() const noexcept override -> const char *`  
";

// File: classhelics_1_1HelicsTerminated.xml


%feature("docstring") helics::HelicsTerminated "

severe exception indicating HELICS has terminated  

C++ includes: core-exceptions.hpp
";

%feature("docstring") helics::HelicsTerminated::HelicsTerminated "
`HelicsTerminated(const std::string &message=\"HELICS termination\")`  
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
`identifier_id_t() noexcept -> constexpr`  

default constructor  
";

%feature("docstring") helics::identifier_id_t::identifier_id_t "
`identifier_id_t(BaseType val) noexcept -> constexpr`  

value based constructor  
";

%feature("docstring") helics::identifier_id_t::identifier_id_t "
`identifier_id_t(const identifier_id_t &id) noexcept -> constexpr`  

copy constructor  
";

%feature("docstring") helics::identifier_id_t::value "
`value() const noexcept -> BaseType`  

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
`maxVal() noexcept -> constexpr baseType`  
";

%feature("docstring") integer_time::minVal "
`minVal() noexcept -> constexpr baseType`  
";

%feature("docstring") integer_time::zeroVal "
`zeroVal() noexcept -> constexpr baseType`  
";

%feature("docstring") integer_time::epsilon "
`epsilon() noexcept -> constexpr baseType`  
";

%feature("docstring") integer_time::convert "
`convert(double t) noexcept -> constexpr baseType`  

convert to a base type representation  
";

%feature("docstring") integer_time::toDouble "
`toDouble(baseType val) noexcept -> double`  

convert the value to a double representation in seconds  
";

%feature("docstring") integer_time::toCount "
`toCount(baseType val, timeUnits units) noexcept -> std::int64_t`  

convert the val to a count of the specified time units  

really kind of awkward to do with this time representation so I just convert to
a double first  
";

%feature("docstring") integer_time::fromCount "
`fromCount(std::uint64_t count, timeUnits units) noexcept -> baseType`  
";

%feature("docstring") integer_time::seconds "
`seconds(baseType val) noexcept -> std::int64_t`  

convert to an integer count in seconds  
";

// File: classhelics_1_1InvalidFunctionCall.xml


%feature("docstring") helics::InvalidFunctionCall "

exception thrown when a function call was made at an inappropriate time  

defining an exception class for invalid function calls  

C++ includes: core-exceptions.hpp
";

%feature("docstring") helics::InvalidFunctionCall::InvalidFunctionCall "
`InvalidFunctionCall(const std::string &message=\"invalid function call\")`  
";

%feature("docstring") helics::InvalidFunctionCall::InvalidFunctionCall "
`InvalidFunctionCall(const char *s)`  
";

// File: classhelics_1_1InvalidIdentifier.xml


%feature("docstring") helics::InvalidIdentifier "

exception for an invalid identification Handle  

C++ includes: core-exceptions.hpp
";

%feature("docstring") helics::InvalidIdentifier::InvalidIdentifier "
`InvalidIdentifier(const std::string &message=\"invalid identifier\")`  
";

// File: classhelics_1_1InvalidParameter.xml


%feature("docstring") helics::InvalidParameter "

exception when one or more of the parameters in the function call were invalid  

C++ includes: core-exceptions.hpp
";

%feature("docstring") helics::InvalidParameter::InvalidParameter "
`InvalidParameter(const std::string &message=\"invalid parameter\")`  
";

// File: classhelics_1_1InvalidParameterValue.xml


%feature("docstring") helics::InvalidParameterValue "

defining an exception class for invalid parameter values  

C++ includes: Federate.hpp
";

%feature("docstring") helics::InvalidParameterValue::InvalidParameterValue "
`InvalidParameterValue(const char *s)`  
";

// File: classhelics_1_1InvalidStateTransition.xml


%feature("docstring") helics::InvalidStateTransition "

defining an exception class for state transition errors  

C++ includes: Federate.hpp
";

%feature("docstring") helics::InvalidStateTransition::InvalidStateTransition "
`InvalidStateTransition(const char *s)`  
";

// File: classhelics_1_1IpcBroker.xml


%feature("docstring") helics::IpcBroker "
";

%feature("docstring") helics::IpcBroker::IpcBroker "
`IpcBroker(bool rootBroker=false) noexcept`  

default constructor  
";

%feature("docstring") helics::IpcBroker::IpcBroker "
`IpcBroker(const std::string &broker_name)`  
";

%feature("docstring") helics::IpcBroker::initializeFromArgs "
`initializeFromArgs(int argc, const char *const *argv) override`  

initialize from command line arguments  
";

%feature("docstring") helics::IpcBroker::~IpcBroker "
`~IpcBroker()`  

destructor  
";

%feature("docstring") helics::IpcBroker::getAddress "
`getAddress() const override -> std::string`  

get the connection address for the broker  
";

%feature("docstring") helics::IpcBroker::displayHelp "
`displayHelp(bool local_only=false)`  
";

// File: classhelics_1_1IpcComms.xml


%feature("docstring") helics::IpcComms "

implementation for the core that uses boost interprocess messages to communicate  

C++ includes: IpcComms.h
";

%feature("docstring") helics::IpcComms::IpcComms "
`IpcComms()=default`  

default constructor  
";

%feature("docstring") helics::IpcComms::IpcComms "
`IpcComms(const std::string &brokerTarget, const std::string &localTarget)`  
";

%feature("docstring") helics::IpcComms::~IpcComms "
`~IpcComms()`  

destructor  
";

// File: classhelics_1_1IpcCore.xml


%feature("docstring") helics::IpcCore "

implementation for the core that uses Interprocess messages to communicate  

C++ includes: IpcCore.h
";

%feature("docstring") helics::IpcCore::IpcCore "
`IpcCore() noexcept`  

default constructor  
";

%feature("docstring") helics::IpcCore::IpcCore "
`IpcCore(const std::string &core_name)`  
";

%feature("docstring") helics::IpcCore::~IpcCore "
`~IpcCore()`  

destructor  
";

%feature("docstring") helics::IpcCore::initializeFromArgs "
`initializeFromArgs(int argc, const char *const *argv) override`  

initialize the core manager with command line arguments  

Parameters
----------
* `argc` :  
    the number of arguments  
* `argv` :  
    char pointers to the arguments  
";

%feature("docstring") helics::IpcCore::getAddress "
`getAddress() const override -> std::string`  

get a string representing the connection info to send data to this object  
";

// File: structhelics_1_1is__iterable.xml


%feature("docstring") helics::is_iterable "

template trait for figuring out if something is an iterable container  

C++ includes: ValueConverter_impl.hpp
";

// File: structhelics_1_1is__iterable_3_01T_00_01typename_01std_1_1enable__if__t_3_01std_1_1is__same_3_012393cea268135d6212d4810890e5a85a.xml


%feature("docstring") helics::is_iterable< T, typename std::enable_if_t< std::is_same< decltype(std::begin(T()) !=std::end(T()), void(), void(*std::begin(T())), std::true_type{}), std::true_type >::value > > "
";

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
`iteration_time()=default`  

default constructor  
";

%feature("docstring") helics::iteration_time::iteration_time "
`iteration_time(Time t, iteration_result iterate) noexcept -> constexpr`  

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
`Logger()`  

default constructor  
";

%feature("docstring") helics::Logger::Logger "
`Logger(std::shared_ptr< LoggingCore > core)`  

construct and link to the specified logging Core  
";

%feature("docstring") helics::Logger::~Logger "
`~Logger()`  

destructor  
";

%feature("docstring") helics::Logger::openFile "
`openFile(const std::string &file)`  

open a file to write the log messages  

Parameters
----------
* `file` :  
    the name of the file to write messages to  
";

%feature("docstring") helics::Logger::startLogging "
`startLogging(int cLevel, int fLevel)`  

function to start the logging thread  

Parameters
----------
* `cLevel` :  
    the console print level  
* `fLevel` :  
    the file print level messages coming in below these levels will be printed  
";

%feature("docstring") helics::Logger::startLogging "
`startLogging()`  

overload of  

See also: startLogging with unspecified logging levels  
";

%feature("docstring") helics::Logger::haltLogging "
`haltLogging()`  

stop logging for a time messages received while halted are ignored  
";

%feature("docstring") helics::Logger::log "
`log(int level, std::string logMessage)`  

log a message at a particular level  

Parameters
----------
* `level` :  
    the level of the message  
* `logMessage` :  
    the actual message to log  
";

%feature("docstring") helics::Logger::log "
`log(std::string logMessage)`  

message to log without regard for levels*  

Parameters
----------
* `logMessage` :  
    the message to log  
";

%feature("docstring") helics::Logger::flush "
`flush()`  

flush the log queue  
";

%feature("docstring") helics::Logger::isRunning "
`isRunning() const -> bool`  

check if the Logger is running  
";

%feature("docstring") helics::Logger::changeLevels "
`changeLevels(int cLevel, int fLevel)`  

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
`getLoggerManager(const std::string &loggerName=\"\") -> std::shared_ptr<
    LoggerManager >`  

get a pointer to a logging manager so it cannot go out of scope  
";

%feature("docstring") helics::LoggerManager::getLoggerCore "
`getLoggerCore(const std::string &loggerName=\"\") -> std::shared_ptr<
    LoggingCore >`  

get a pointer to a logging core  
";

%feature("docstring") helics::LoggerManager::closeLogger "
`closeLogger(const std::string &loggerName=\"\")`  

close the named Logger  

prevents the Logger from being retrieved through this class but does not
necessarily destroy the Logger  
";

%feature("docstring") helics::LoggerManager::logMessage "
`logMessage(const std::string &message)`  

sends a message to the default Logger  
";

%feature("docstring") helics::LoggerManager::~LoggerManager "
`~LoggerManager()`  
";

%feature("docstring") helics::LoggerManager::getName "
`getName() const -> const std::string &`  
";

// File: classhelics_1_1LoggerNoThread.xml


%feature("docstring") helics::LoggerNoThread "

logging class that handle the logs immediately with no threading or
synchronization  

C++ includes: logger.h
";

%feature("docstring") helics::LoggerNoThread::LoggerNoThread "
`LoggerNoThread()`  

default constructor  
";

%feature("docstring") helics::LoggerNoThread::LoggerNoThread "
`LoggerNoThread(const std::shared_ptr< LoggingCore > &core)`  

this does nothing with the argument since it is not threaded here to match the
API of Logger  
";

%feature("docstring") helics::LoggerNoThread::openFile "
`openFile(const std::string &file)`  

open a file to write the log messages  

Parameters
----------
* `file` :  
    the name of the file to write messages to  
";

%feature("docstring") helics::LoggerNoThread::startLogging "
`startLogging(int cLevel, int fLevel)`  

function to start the logging thread  

Parameters
----------
* `cLevel` :  
    the console print level  
* `fLevel` :  
    the file print level messages coming in below these levels will be printed  
";

%feature("docstring") helics::LoggerNoThread::startLogging "
`startLogging()`  

overload of ::startLogging with unspecified logging levels  
";

%feature("docstring") helics::LoggerNoThread::log "
`log(int level, const std::string &logMessage)`  

log a message at a particular level  

Parameters
----------
* `level` :  
    the level of the message  
* `logMessage` :  
    the actual message to log  
";

%feature("docstring") helics::LoggerNoThread::log "
`log(const std::string &logMessage)`  

message to log without regard for levels*  

Parameters
----------
* `logMessage` :  
    the message to log  
";

%feature("docstring") helics::LoggerNoThread::isRunning "
`isRunning() const -> bool`  

check if the logging thread is running  
";

%feature("docstring") helics::LoggerNoThread::flush "
`flush()`  

flush the log queue  
";

%feature("docstring") helics::LoggerNoThread::changeLevels "
`changeLevels(int cLevel, int fLevel)`  

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
`LoggingCore()`  

default constructor  
";

%feature("docstring") helics::LoggingCore::~LoggingCore "
`~LoggingCore()`  

destructor  
";

%feature("docstring") helics::LoggingCore::addMessage "
`addMessage(const std::string &message)`  

add a message for the LoggingCore or just general console print  
";

%feature("docstring") helics::LoggingCore::addMessage "
`addMessage(std::string &&message)`  

move a message for the LoggingCore or just general console print  
";

%feature("docstring") helics::LoggingCore::addMessage "
`addMessage(int index, const std::string &message)`  

add a message for a specific Logger  

Parameters
----------
* `index` :  
    the index of the function callback to use  
* `message` :  
    the message to send  
";

%feature("docstring") helics::LoggingCore::addMessage "
`addMessage(int index, std::string &&message)`  

add a message for a specific Logger  

Parameters
----------
* `index` :  
    the index of the function callback to use  
* `message` :  
    the message to send  
";

%feature("docstring") helics::LoggingCore::addFileProcessor "
`addFileProcessor(std::function< void(std::string &&message)> newFunction) ->
    int`  

add a file processing callback (not just files)  

Parameters
----------
* `newFunction` :  
    the callback to call on receipt of a message  
";

%feature("docstring") helics::LoggingCore::haltOperations "
`haltOperations(int)`  

remove a function callback  
";

%feature("docstring") helics::LoggingCore::updateProcessingFunction "
`updateProcessingFunction(int index, std::function< void(std::string &&message)>
    newFunction)`  

update a callback for a particular instance  
";

// File: classMappedPointerVector.xml


%feature("docstring") MappedPointerVector "
";

%feature("docstring") MappedPointerVector::insert "
`insert(const searchType &searchValue, Us &&... data) -> auto &`  
";

%feature("docstring") MappedPointerVector::find "
`find(const searchType &searchValue) -> VType *`  
";

%feature("docstring") MappedPointerVector::begin "
`begin() -> auto`  
";

%feature("docstring") MappedPointerVector::end "
`end() -> auto`  
";

%feature("docstring") MappedPointerVector::cbegin "
`cbegin() const -> auto`  
";

%feature("docstring") MappedPointerVector::cend "
`cend() const -> auto`  
";

%feature("docstring") MappedPointerVector::size "
`size() const -> auto`  
";

%feature("docstring") MappedPointerVector::clear "
`clear()`  
";

// File: classMappedVector.xml


%feature("docstring") MappedVector "
";

%feature("docstring") MappedVector::insert "
`insert(const searchType &searchValue, Us &&... data) -> auto &`  
";

%feature("docstring") MappedVector::find "
`find(const searchType &searchValue) -> auto`  
";

%feature("docstring") MappedVector::find "
`find(const searchType &searchValue) const -> auto`  
";

%feature("docstring") MappedVector::begin "
`begin() -> auto`  
";

%feature("docstring") MappedVector::end "
`end() -> auto`  
";

%feature("docstring") MappedVector::cbegin "
`cbegin() const -> auto`  
";

%feature("docstring") MappedVector::cend "
`cend() const -> auto`  
";

%feature("docstring") MappedVector::size "
`size() const -> auto`  
";

%feature("docstring") MappedVector::clear "
`clear()`  
";

// File: classMasterObjectHolder.xml


%feature("docstring") MasterObjectHolder "

class for containing all the objects associated with a federation  

C++ includes: api_objects.h
";

%feature("docstring") MasterObjectHolder::MasterObjectHolder "
`MasterObjectHolder() noexcept`  
";

%feature("docstring") MasterObjectHolder::~MasterObjectHolder "
`~MasterObjectHolder()`  
";

%feature("docstring") MasterObjectHolder::addBroker "
`addBroker(helics::BrokerObject *broker) -> int`  
";

%feature("docstring") MasterObjectHolder::addCore "
`addCore(helics::CoreObject *core) -> int`  
";

%feature("docstring") MasterObjectHolder::addFed "
`addFed(helics::FedObject *fed) -> int`  
";

%feature("docstring") MasterObjectHolder::clearBroker "
`clearBroker(int index)`  
";

%feature("docstring") MasterObjectHolder::clearCore "
`clearCore(int index)`  
";

%feature("docstring") MasterObjectHolder::clearFed "
`clearFed(int index)`  
";

%feature("docstring") MasterObjectHolder::deleteAll "
`deleteAll()`  
";

// File: classhelics_1_1Message.xml


%feature("docstring") helics::Message "

class containing a message structure  

C++ includes: core-data.hpp
";

%feature("docstring") helics::Message::Message "
`Message() noexcept`  

default constructor  
";

%feature("docstring") helics::Message::Message "
`Message(Message &&m) noexcept`  

move constructor  
";

%feature("docstring") helics::Message::Message "
`Message(const Message &m)=default`  

copy constructor  
";

%feature("docstring") helics::Message::swap "
`swap(Message &m2) noexcept`  

swap operation for the Message  
";

%feature("docstring") helics::Message::isValid "
`isValid() const noexcept -> bool`  

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
`message_t()`  
";

%feature("docstring") zmq::message_t::message_t "
`message_t(size_t size_)`  
";

%feature("docstring") zmq::message_t::message_t "
`message_t(I first, I last)`  
";

%feature("docstring") zmq::message_t::message_t "
`message_t(const void *data_, size_t size_)`  
";

%feature("docstring") zmq::message_t::message_t "
`message_t(void *data_, size_t size_, free_fn *ffn_, void *hint_=NULL)`  
";

%feature("docstring") zmq::message_t::~message_t "
`~message_t() ZMQ_NOTHROW`  
";

%feature("docstring") zmq::message_t::rebuild "
`rebuild()`  
";

%feature("docstring") zmq::message_t::rebuild "
`rebuild(size_t size_)`  
";

%feature("docstring") zmq::message_t::rebuild "
`rebuild(const void *data_, size_t size_)`  
";

%feature("docstring") zmq::message_t::rebuild "
`rebuild(void *data_, size_t size_, free_fn *ffn_, void *hint_=NULL)`  
";

%feature("docstring") zmq::message_t::move "
`move(message_t const *msg_)`  
";

%feature("docstring") zmq::message_t::copy "
`copy(message_t const *msg_)`  
";

%feature("docstring") zmq::message_t::more "
`more() const ZMQ_NOTHROW -> bool`  
";

%feature("docstring") zmq::message_t::data "
`data() ZMQ_NOTHROW -> void *`  
";

%feature("docstring") zmq::message_t::data "
`data() const ZMQ_NOTHROW -> const void *`  
";

%feature("docstring") zmq::message_t::data "
`data() ZMQ_NOTHROW -> T *`  
";

%feature("docstring") zmq::message_t::data "
`data() const ZMQ_NOTHROW -> T const  *`  
";

%feature("docstring") zmq::message_t::size "
`size() const ZMQ_NOTHROW -> size_t`  
";

%feature("docstring") zmq::message_t::equal "
`equal(const message_t *other) const ZMQ_NOTHROW -> bool`  
";

%feature("docstring") zmq::message_t::gets "
`gets(const char *property_) -> const char *`  
";

// File: classhelics_1_1MessageConditionalOperator.xml


%feature("docstring") helics::MessageConditionalOperator "

class defining an message operator that either passes the message or not  

the evaluation function used should return true if the message should be allowed
through false if it should be dropped  

C++ includes: MessageOperators.hpp
";

%feature("docstring") helics::MessageConditionalOperator::MessageConditionalOperator "
`MessageConditionalOperator()=default`  

default constructor  
";

%feature("docstring") helics::MessageConditionalOperator::MessageConditionalOperator "
`MessageConditionalOperator(std::function< bool(const Message *)>
    userConditionalFunction)`  

set the function to modify the data of the message in the constructor  
";

%feature("docstring") helics::MessageConditionalOperator::setConditionFunction "
`setConditionFunction(std::function< bool(const Message *)>
    userConditionalFunction)`  

set the function to modify the data of the message  
";

// File: classhelics_1_1MessageDataOperator.xml


%feature("docstring") helics::MessageDataOperator "

class defining an message operator that operates purely on the data aspect of a
message  

C++ includes: MessageOperators.hpp
";

%feature("docstring") helics::MessageDataOperator::MessageDataOperator "
`MessageDataOperator()=default`  

default constructor  
";

%feature("docstring") helics::MessageDataOperator::MessageDataOperator "
`MessageDataOperator(std::function< data_view(data_view)> userDataFunction)`  

set the function to modify the data of the message in the constructor  
";

%feature("docstring") helics::MessageDataOperator::setDataFunction "
`setDataFunction(std::function< data_view(data_view)> userDataFunction)`  

set the function to modify the data of the message  
";

// File: classhelics_1_1MessageDestOperator.xml


%feature("docstring") helics::MessageDestOperator "

class defining an message operator that operates purely on the destination
aspect of a message  

C++ includes: MessageOperators.hpp
";

%feature("docstring") helics::MessageDestOperator::MessageDestOperator "
`MessageDestOperator()=default`  

default constructor  
";

%feature("docstring") helics::MessageDestOperator::MessageDestOperator "
`MessageDestOperator(std::function< std::string(const std::string &)>
    userDestFunction)`  

set the function to modify the time of the message in the constructor  
";

%feature("docstring") helics::MessageDestOperator::setDestFunction "
`setDestFunction(std::function< std::string(const std::string &)>
    userDestFunction)`  

set the function to modify the time of the message  
";

// File: classhelics_1_1MessageFederate.xml


%feature("docstring") helics::MessageFederate "

class defining the block communication based interface  

C++ includes: MessageFederate.hpp
";

%feature("docstring") helics::MessageFederate::MessageFederate "
`MessageFederate(const FederateInfo &fi)`  

constructor taking a federate information structure and using the default core  

Parameters
----------
* `fi` :  
    a federate information structure  
";

%feature("docstring") helics::MessageFederate::MessageFederate "
`MessageFederate(std::shared_ptr< Core > core, const FederateInfo &fi)`  

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
`MessageFederate(const std::string &jsonString)`  

constructor taking a string with the required information  

Parameters
----------
* `jsonString` :  
    can be either a json file or a string containing json code  
";

%feature("docstring") helics::MessageFederate::MessageFederate "
`MessageFederate(MessageFederate &&mFed) noexcept`  

move constructor  
";

%feature("docstring") helics::MessageFederate::MessageFederate "
`MessageFederate()`  

default constructor  
";

%feature("docstring") helics::MessageFederate::MessageFederate "
`MessageFederate(bool res)`  

special constructor should only be used by child classes in constructor due to
virtual inheritance  
";

%feature("docstring") helics::MessageFederate::MessageFederate "
`MessageFederate(FederateInfo &fi)`  
";

%feature("docstring") helics::MessageFederate::MessageFederate "
`MessageFederate(const std::string &jsonString)`  
";

%feature("docstring") helics::MessageFederate::MessageFederate "
`MessageFederate()`  
";

%feature("docstring") helics::MessageFederate::~MessageFederate "
`~MessageFederate()`  

destructor  
";

%feature("docstring") helics::MessageFederate::~MessageFederate "
`~MessageFederate()`  
";

%feature("docstring") helics::MessageFederate::registerEndpoint "
`registerEndpoint(const std::string &name, const std::string &type=\"\") ->
    endpoint_id_t`  

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
`registerEndpoint(const std::string &name, const std::string &type=\"\") ->
    helics_endpoint`  

Methods for registering endpoints  
";

%feature("docstring") helics::MessageFederate::registerGlobalEndpoint "
`registerGlobalEndpoint(const std::string &name, const std::string &type=\"\")
    -> endpoint_id_t`  

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
`registerGlobalEndpoint(const std::string &name, const std::string &type=\"\")
    -> helics_endpoint`  
";

%feature("docstring") helics::MessageFederate::registerInterfaces "
`registerInterfaces(const std::string &jsonString) override`  

register a set of interfaces defined in a file  

call is only valid in startup mode  

Parameters
----------
* `jsonString` :  
    the location of the file or json String to load to generate the interfaces  
";

%feature("docstring") helics::MessageFederate::registerKnownCommunicationPath "
`registerKnownCommunicationPath(endpoint_id_t localEndpoint, const std::string
    &remoteEndpoint)`  

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
`subscribe(endpoint_id_t endpoint, const std::string &name, const std::string
    &type)`  

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
`subscribe(helics_endpoint ep, const std::string &name, const std::string
    &type)`  

Subscribe to an endpoint  
";

%feature("docstring") helics::MessageFederate::hasMessage "
`hasMessage() const -> bool`  

check if the federate has any outstanding messages  
";

%feature("docstring") helics::MessageFederate::hasMessage "
`hasMessage(endpoint_id_t id) const -> bool`  
";

%feature("docstring") helics::MessageFederate::hasMessage "
`hasMessage() const -> bool`  

Checks if federate has any messages  
";

%feature("docstring") helics::MessageFederate::hasMessage "
`hasMessage(helics_endpoint ep) const -> bool`  
";

%feature("docstring") helics::MessageFederate::receiveCount "
`receiveCount(endpoint_id_t id) const -> uint64_t`  

Returns the number of pending receives for the specified destination endpoint.  
";

%feature("docstring") helics::MessageFederate::receiveCount "
`receiveCount() const -> uint64_t`  

Returns the number of pending receives for all endpoints.  

Returns the number of pending receives for the specified destination endpoint.  

this function is not preferred in multithreaded contexts due to the required
locking prefer to just use getMessage until it returns an invalid Message.  
";

%feature("docstring") helics::MessageFederate::receiveCount "
`receiveCount(helics_endpoint ep) const -> uint64_t`  

Returns the number of pending receives for endpoint  
";

%feature("docstring") helics::MessageFederate::receiveCount "
`receiveCount() const -> uint64_t`  

Returns the number of pending receives for all endpoints.  
";

%feature("docstring") helics::MessageFederate::getMessage "
`getMessage(endpoint_id_t endpoint) -> std::unique_ptr< Message >`  

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
`getMessage() -> std::unique_ptr< Message >`  

receive a communication message for any endpoint in the federate  

the return order will be in order of endpoint creation then order of arrival all
messages for the first endpoint, then all for the second, and so on  

Returns
-------
a unique_ptr to a Message object containing the message data  
";

%feature("docstring") helics::MessageFederate::getMessage "
`getMessage(helics_endpoint ep) -> message_t`  

Get a packet from an endpoint  
";

%feature("docstring") helics::MessageFederate::getMessage "
`getMessage() -> message_t`  

Get a packet for any endpoints in the federate  
";

%feature("docstring") helics::MessageFederate::sendMessage "
`sendMessage(endpoint_id_t source, const std::string &dest, const char *data,
    size_t len)`  

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
`sendMessage(endpoint_id_t source, const std::string &dest, const data_view
    &message)`  

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
`sendMessage(endpoint_id_t source, const std::string &dest, const char *data,
    size_t len, Time sendTime)`  

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
`sendMessage(endpoint_id_t source, const std::string &dest, const data_view
    &message, Time sendTime)`  

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
`sendMessage(endpoint_id_t source, std::unique_ptr< Message > message)`  

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
`sendMessage(endpoint_id_t source, const Message &message)`  

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
`sendMessage(helics_endpoint source, const std::string &dest, const char *data,
    size_t len)`  

Methods for sending a message  
";

%feature("docstring") helics::MessageFederate::sendMessage "
`sendMessage(helics_endpoint source, const std::string &dest, const char *data,
    size_t len, helics_time_t time)`  
";

%feature("docstring") helics::MessageFederate::sendMessage "
`sendMessage(helics_endpoint source, message_t &message)`  
";

%feature("docstring") helics::MessageFederate::getEndpointName "
`getEndpointName(endpoint_id_t id) const -> std::string`  

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
`getEndpointName(helics_endpoint ep) const -> std::string`  
";

%feature("docstring") helics::MessageFederate::getEndpointId "
`getEndpointId(const std::string &name) const -> endpoint_id_t`  

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
`getEndpointType(endpoint_id_t ep) -> std::string`  

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
`getEndpointType(helics_endpoint ep) -> std::string`  
";

%feature("docstring") helics::MessageFederate::registerEndpointCallback "
`registerEndpointCallback(std::function< void(endpoint_id_t, Time)> callback)`  

register a callback for all endpoints  

Parameters
----------
* `callback` :  
    the function to execute upon receipt of a message for any endpoint  
";

%feature("docstring") helics::MessageFederate::registerEndpointCallback "
`registerEndpointCallback(endpoint_id_t ep, std::function< void(endpoint_id_t,
    Time)> callback)`  

register a callback for a specific endpoint  

Parameters
----------
* `ep` :  
    the endpoint to associate with the specified callback  
* `callback` :  
    the function to execute upon receipt of a message for the given endpoint  
";

%feature("docstring") helics::MessageFederate::registerEndpointCallback "
`registerEndpointCallback(const std::vector< endpoint_id_t > &ep, std::function<
    void(endpoint_id_t, Time)> callback)`  

register a callback for a set of specific endpoint  

Parameters
----------
* `ep` :  
    a vector of endpoints to associate with the specified callback  
* `callback` :  
    the function to execute upon receipt of a message for the given endpoint  
";

%feature("docstring") helics::MessageFederate::disconnect "
`disconnect() override`  

disconnect a simulation from the core (will also call finalize before
disconnecting if necessary)  
";

%feature("docstring") helics::MessageFederate::getEndpointCount "
`getEndpointCount() const -> int`  

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
`MessageFederateManager(std::shared_ptr< Core > coreOb, Core::federate_id_t id)`  

construct from a pointer to a core and a specified federate id  
";

%feature("docstring") helics::MessageFederateManager::~MessageFederateManager "
`~MessageFederateManager()`  
";

%feature("docstring") helics::MessageFederateManager::registerEndpoint "
`registerEndpoint(const std::string &name, const std::string &type) ->
    endpoint_id_t`  

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
`registerKnownCommunicationPath(endpoint_id_t localEndpoint, const std::string
    &remoteEndpoint)`  

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
`subscribe(endpoint_id_t endpoint, const std::string &name, const std::string
    &type)`  

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
`hasMessage() const -> bool`  

check if the federate has any outstanding messages  
";

%feature("docstring") helics::MessageFederateManager::hasMessage "
`hasMessage(endpoint_id_t id) const -> bool`  
";

%feature("docstring") helics::MessageFederateManager::receiveCount "
`receiveCount(endpoint_id_t id) const -> uint64_t`  

Returns the number of pending receives for the specified destination endpoint.  
";

%feature("docstring") helics::MessageFederateManager::receiveCount "
`receiveCount() const -> uint64_t`  

Returns the number of pending receives for the specified destination endpoint.  

Returns the number of pending receives for the specified destination endpoint.  

this function is not preferred in multi-threaded contexts due to the required
locking prefer to just use getMessage until it returns an invalid Message.  
";

%feature("docstring") helics::MessageFederateManager::getMessage "
`getMessage(endpoint_id_t endpoint) -> std::unique_ptr< Message >`  

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
`getMessage() -> std::unique_ptr< Message >`  
";

%feature("docstring") helics::MessageFederateManager::sendMessage "
`sendMessage(endpoint_id_t source, const std::string &dest, data_view message)`  
";

%feature("docstring") helics::MessageFederateManager::sendMessage "
`sendMessage(endpoint_id_t source, const std::string &dest, data_view message,
    Time sendTime)`  
";

%feature("docstring") helics::MessageFederateManager::sendMessage "
`sendMessage(endpoint_id_t source, std::unique_ptr< Message > message)`  
";

%feature("docstring") helics::MessageFederateManager::updateTime "
`updateTime(Time newTime, Time oldTime)`  

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
`startupToInitializeStateTransition()`  

transition from Startup To the Initialize State  
";

%feature("docstring") helics::MessageFederateManager::initializeToExecuteStateTransition "
`initializeToExecuteStateTransition()`  

transition from initialize to execution State  
";

%feature("docstring") helics::MessageFederateManager::getEndpointName "
`getEndpointName(endpoint_id_t id) const -> std::string`  

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
`getEndpointId(const std::string &name) const -> endpoint_id_t`  

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
`getEndpointType(endpoint_id_t id) const -> std::string`  

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
`registerCallback(std::function< void(endpoint_id_t, Time)> callback)`  

register a callback function to call when any endpoint receives a message  

there can only be one generic callback  

Parameters
----------
* `callback` :  
    the function to call  
";

%feature("docstring") helics::MessageFederateManager::registerCallback "
`registerCallback(endpoint_id_t id, std::function< void(endpoint_id_t, Time)>
    callback)`  

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
`registerCallback(const std::vector< endpoint_id_t > &ids, std::function<
    void(endpoint_id_t, Time)> callback)`  

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
`disconnect()`  

disconnect from the coreObject  
";

%feature("docstring") helics::MessageFederateManager::getEndpointCount "
`getEndpointCount() const -> int`  

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
`MessageTimeOperator()=default`  

default constructor  
";

%feature("docstring") helics::MessageTimeOperator::MessageTimeOperator "
`MessageTimeOperator(std::function< Time(Time)> userTimeFunction)`  

set the function to modify the time of the message in the constructor  
";

%feature("docstring") helics::MessageTimeOperator::setTimeFunction "
`setTimeFunction(std::function< Time(Time)> userTimeFunction)`  

set the function to modify the time of the message  
";

// File: classzmq_1_1monitor__t.xml


%feature("docstring") zmq::monitor_t "
";

%feature("docstring") zmq::monitor_t::monitor_t "
`monitor_t()`  
";

%feature("docstring") zmq::monitor_t::~monitor_t "
`~monitor_t()`  
";

%feature("docstring") zmq::monitor_t::monitor "
`monitor(socket_t &socket, std::string const &addr, int events=ZMQ_EVENT_ALL)`  
";

%feature("docstring") zmq::monitor_t::monitor "
`monitor(socket_t &socket, const char *addr_, int events=ZMQ_EVENT_ALL)`  
";

%feature("docstring") zmq::monitor_t::init "
`init(socket_t &socket, std::string const &addr, int events=ZMQ_EVENT_ALL)`  
";

%feature("docstring") zmq::monitor_t::init "
`init(socket_t &socket, const char *addr_, int events=ZMQ_EVENT_ALL)`  
";

%feature("docstring") zmq::monitor_t::check_event "
`check_event(int timeout=0) -> bool`  
";

%feature("docstring") zmq::monitor_t::on_monitor_started "
`on_monitor_started()`  
";

%feature("docstring") zmq::monitor_t::on_event_connected "
`on_event_connected(const zmq_event_t &event_, const char *addr_)`  
";

%feature("docstring") zmq::monitor_t::on_event_connect_delayed "
`on_event_connect_delayed(const zmq_event_t &event_, const char *addr_)`  
";

%feature("docstring") zmq::monitor_t::on_event_connect_retried "
`on_event_connect_retried(const zmq_event_t &event_, const char *addr_)`  
";

%feature("docstring") zmq::monitor_t::on_event_listening "
`on_event_listening(const zmq_event_t &event_, const char *addr_)`  
";

%feature("docstring") zmq::monitor_t::on_event_bind_failed "
`on_event_bind_failed(const zmq_event_t &event_, const char *addr_)`  
";

%feature("docstring") zmq::monitor_t::on_event_accepted "
`on_event_accepted(const zmq_event_t &event_, const char *addr_)`  
";

%feature("docstring") zmq::monitor_t::on_event_accept_failed "
`on_event_accept_failed(const zmq_event_t &event_, const char *addr_)`  
";

%feature("docstring") zmq::monitor_t::on_event_closed "
`on_event_closed(const zmq_event_t &event_, const char *addr_)`  
";

%feature("docstring") zmq::monitor_t::on_event_close_failed "
`on_event_close_failed(const zmq_event_t &event_, const char *addr_)`  
";

%feature("docstring") zmq::monitor_t::on_event_disconnected "
`on_event_disconnected(const zmq_event_t &event_, const char *addr_)`  
";

%feature("docstring") zmq::monitor_t::on_event_handshake_failed_no_detail "
`on_event_handshake_failed_no_detail(const zmq_event_t &event_, const char
    *addr_)`  
";

%feature("docstring") zmq::monitor_t::on_event_handshake_failed_protocol "
`on_event_handshake_failed_protocol(const zmq_event_t &event_, const char
    *addr_)`  
";

%feature("docstring") zmq::monitor_t::on_event_handshake_failed_auth "
`on_event_handshake_failed_auth(const zmq_event_t &event_, const char *addr_)`  
";

%feature("docstring") zmq::monitor_t::on_event_handshake_succeeded "
`on_event_handshake_succeeded(const zmq_event_t &event_, const char *addr_)`  
";

%feature("docstring") zmq::monitor_t::on_event_unknown "
`on_event_unknown(const zmq_event_t &event_, const char *addr_)`  
";

// File: classhelics_1_1MpiBroker.xml


%feature("docstring") helics::MpiBroker "
";

%feature("docstring") helics::MpiBroker::MpiBroker "
`MpiBroker(bool isRoot_=false) noexcept`  

default constructor  
";

%feature("docstring") helics::MpiBroker::MpiBroker "
`MpiBroker(const std::string &broker_name)`  
";

%feature("docstring") helics::MpiBroker::InitializeFromArgs "
`InitializeFromArgs(int argc, const char *const *argv) override`  
";

%feature("docstring") helics::MpiBroker::~MpiBroker "
`~MpiBroker()`  

destructor  
";

%feature("docstring") helics::MpiBroker::getAddress "
`getAddress() const override -> std::string`  

get the connection address for the broker  
";

// File: classhelics_1_1MpiComms.xml


%feature("docstring") helics::MpiComms "

implementation for the core that uses zmq messages to communicate  

C++ includes: MpiComms.h
";

%feature("docstring") helics::MpiComms::MpiComms "
`MpiComms()=default`  

default constructor  
";

%feature("docstring") helics::MpiComms::MpiComms "
`MpiComms(const std::string &brokerTarget, const std::string &localTarget)`  
";

%feature("docstring") helics::MpiComms::~MpiComms "
`~MpiComms()`  

destructor  
";

// File: classhelics_1_1MpiCore.xml


%feature("docstring") helics::MpiCore "

implementation for the core that uses zmq messages to communicate  

C++ includes: MpiCore.h
";

%feature("docstring") helics::MpiCore::MpiCore "
`MpiCore() noexcept`  

default constructor  
";

%feature("docstring") helics::MpiCore::MpiCore "
`MpiCore(const std::string &core_name)`  
";

%feature("docstring") helics::MpiCore::~MpiCore "
`~MpiCore()`  

destructor  
";

%feature("docstring") helics::MpiCore::InitializeFromArgs "
`InitializeFromArgs(int argc, const char *const *argv) override`  
";

%feature("docstring") helics::MpiCore::getAddress "
`getAddress() const override -> std::string`  

get a string representing the connection info to send data to this object  
";

// File: classhelics_1_1NetworkBrokerData.xml


%feature("docstring") helics::NetworkBrokerData "

helper class designed to contain the common elements between networking brokers
and cores  

C++ includes: NetworkBrokerData.hpp
";

%feature("docstring") helics::NetworkBrokerData::NetworkBrokerData "
`NetworkBrokerData()=default`  
";

%feature("docstring") helics::NetworkBrokerData::NetworkBrokerData "
`NetworkBrokerData(interface_type type)`  

constructor from the allowed type  
";

%feature("docstring") helics::NetworkBrokerData::initializeFromArgs "
`initializeFromArgs(int argc, const char *const *argv, const std::string
    &localAddress)`  
";

%feature("docstring") helics::NetworkBrokerData::setInterfaceType "
`setInterfaceType(interface_type type)`  
";

%feature("docstring") helics::NetworkBrokerData::displayHelp "
`displayHelp()`  
";

// File: classhelics_1_1NullFilterOperator.xml


%feature("docstring") helics::NullFilterOperator "

special filter operator defining no operation the original message is simply
returned  

C++ includes: core-data.hpp
";

%feature("docstring") helics::NullFilterOperator::NullFilterOperator "
`NullFilterOperator()=default`  

default constructor  
";

%feature("docstring") helics::NullFilterOperator::process "
`process(std::unique_ptr< Message > message) override -> std::unique_ptr<
    Message >`  

filter the message either modify the message or generate a new one  
";

// File: classhelics_1_1ownedQueue.xml


%feature("docstring") helics::ownedQueue "

class implementing a queue owned by a particular object  

C++ includes: IpcQueueHelper.h
";

%feature("docstring") helics::ownedQueue::ownedQueue "
`ownedQueue()=default`  
";

%feature("docstring") helics::ownedQueue::~ownedQueue "
`~ownedQueue()`  
";

%feature("docstring") helics::ownedQueue::connect "
`connect(const std::string &connection, int maxMessages, int maxSize) -> bool`  
";

%feature("docstring") helics::ownedQueue::changeState "
`changeState(queue_state_t newState)`  
";

%feature("docstring") helics::ownedQueue::getMessage "
`getMessage(int timeout) -> stx::optional< ActionMessage >`  
";

%feature("docstring") helics::ownedQueue::getMessage "
`getMessage() -> ActionMessage`  
";

%feature("docstring") helics::ownedQueue::getError "
`getError() const -> const std::string &`  
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
`Player()=default`  

default constructor  
";

%feature("docstring") helics::Player::Player "
`Player(int argc, char *argv[])`  

construct from command line arguments  

Parameters
----------
* `argc` :  
    the number of arguments  
* `argv` :  
    the strings in the input  
";

%feature("docstring") helics::Player::Player "
`Player(const FederateInfo &fi)`  

construct from a federate info object  

Parameters
----------
* `fi` :  
    a pointer info object containing information on the desired federate
    configuration  
";

%feature("docstring") helics::Player::Player "
`Player(std::shared_ptr< Core > core, const FederateInfo &fi)`  

constructor taking a federate information structure and using the given core  

Parameters
----------
* `core` :  
    a pointer to core object which the federate can join  
* `fi` :  
    a federate information structure  
";

%feature("docstring") helics::Player::Player "
`Player(const std::string &jsonString)`  

constructor taking a file with the required information  

Parameters
----------
* `jsonString` :  
    file or JSON string defining the federate information and other
    configuration  
";

%feature("docstring") helics::Player::Player "
`Player(Player &&other_player)=default`  

move construction  
";

%feature("docstring") helics::Player::Player "
`Player(const Player &other_player)=delete`  

don't allow the copy constructor  
";

%feature("docstring") helics::Player::~Player "
`~Player()`  
";

%feature("docstring") helics::Player::loadFile "
`loadFile(const std::string &filename)`  

load a file containing publication information  

Parameters
----------
* `filename` :  
    the file containing the configuration and Player data accepted format are
    JSON, xml, and a Player format which is tab delimited or comma delimited  
";

%feature("docstring") helics::Player::initialize "
`initialize()`  

initialize the Player federate  

generate all the publications and organize the points, the final publication
count will be available after this time and the Player will enter the
initialization mode, which means it will not be possible to add more
publications calling run will automatically do this if necessary  
";

%feature("docstring") helics::Player::run "
`run()`  
";

%feature("docstring") helics::Player::run "
`run(Time stopTime_input)`  

run the Player until the specified time  

Parameters
----------
* `stopTime_input` :  
    the desired stop time  
";

%feature("docstring") helics::Player::addPublication "
`addPublication(const std::string &key, helics_type_t type, const std::string
    &units=\"\")`  

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
`addPublication(const std::string &key, const std::string &units=\"\") ->
    std::enable_if_t< helicsType< valType >) !=helics_type_t::helicsInvalid >`  

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
`addEndpoint(const std::string &endpointName, const std::string
    &endpointType=\"\")`  

add an endpoint to the Player  

Parameters
----------
* `endpointName` :  
    the name of the endpoint  
* `endpointType` :  
    the named type of the endpoint  
";

%feature("docstring") helics::Player::addPoint "
`addPoint(Time pubTime, const std::string &key, const valType &val)`  

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
`addMessage(Time sendTime, const std::string &src, const std::string &dest,
    const std::string &payload)`  

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
`addMessage(Time sendTime, Time actionTime, const std::string &src, const
    std::string &dest, const std::string &payload)`  

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
`pointCount() const -> auto`  

get the number of points loaded  
";

%feature("docstring") helics::Player::messageCount "
`messageCount() const -> auto`  

get the number of messages loaded  
";

%feature("docstring") helics::Player::publicationCount "
`publicationCount() const -> auto`  

get the number of publications  
";

%feature("docstring") helics::Player::endpointCount "
`endpointCount() const -> auto`  

get the number of endpoints  
";

%feature("docstring") helics::Player::finalize "
`finalize()`  

finalize the Player federate  
";

%feature("docstring") helics::Player::isActive "
`isActive() const -> bool`  

check if the Player is ready to run  
";

// File: classhelics_1_1Publication.xml


%feature("docstring") helics::Publication "
";

%feature("docstring") helics::Publication::Publication "
`Publication() noexcept`  
";

%feature("docstring") helics::Publication::Publication "
`Publication(ValueFederate *valueFed, const std::string &key, helics_type_t
    type, std::string units=\"\")`  

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
`Publication(interface_visibility locality, ValueFederate *valueFed, std::string
    key, helics_type_t type, std::string units=\"\")`  

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
`Publication(ValueFederate *valueFed, int pubIndex)`  

generate a publication object from a preexisting publication  

Parameters
----------
* `valueFed` :  
    a pointer to the appropriate value Federate  
* `pubIndex` :  
    the index of the subscription  
";

%feature("docstring") helics::Publication::publish "
`publish(double val) const`  

send a value for publication  

Parameters
----------
* `val` :  
    the value to publish  
";

%feature("docstring") helics::Publication::publish "
`publish(int64_t val) const`  
";

%feature("docstring") helics::Publication::publish "
`publish(const char *val) const`  
";

%feature("docstring") helics::Publication::publish "
`publish(const std::string &val) const`  
";

%feature("docstring") helics::Publication::publish "
`publish(const std::vector< double > &val) const`  
";

%feature("docstring") helics::Publication::publish "
`publish(const std::vector< std::complex< double >> &val) const`  
";

%feature("docstring") helics::Publication::publish "
`publish(const double *vals, int size) const`  
";

%feature("docstring") helics::Publication::publish "
`publish(std::complex< double > val) const`  
";

%feature("docstring") helics::Publication::publish "
`publish(const defV &val) const`  
";

%feature("docstring") helics::Publication::publish "
`publish(const X &val, const std::string &) const`  

secondary publish function to allow unit conversion before publication  

Parameters
----------
* `val` :  
    the value to publish  
* `units` :  
    the units association with the publication  
";

%feature("docstring") helics::Publication::setMinimumChange "
`setMinimumChange(double deltaV)`  
";

%feature("docstring") helics::Publication::enableChangeDetection "
`enableChangeDetection(bool enabled=true)`  
";

// File: structhelics_1_1publication__info.xml


%feature("docstring") helics::publication_info "

structure used to contain information about a publication  

C++ includes: ValueFederateManager.hpp
";

%feature("docstring") helics::publication_info::publication_info "
`publication_info(const std::string &n_name, const std::string &n_type, const
    std::string &n_units)`  
";

// File: classhelics_1_1PublicationBase.xml


%feature("docstring") helics::PublicationBase "
";

%feature("docstring") helics::PublicationBase::PublicationBase "
`PublicationBase()=default`  
";

%feature("docstring") helics::PublicationBase::PublicationBase "
`PublicationBase(ValueFederate *valueFed, const std::string &key, const
    std::string &type, const std::string &units=\"\")`  
";

%feature("docstring") helics::PublicationBase::PublicationBase "
`PublicationBase(interface_visibility locality, ValueFederate *valueFed, const
    std::string &key, const std::string &type, const std::string &units=\"\")`  
";

%feature("docstring") helics::PublicationBase::PublicationBase "
`PublicationBase(ValueFederate *valueFed, int pubIndex)`  

generate a publication object from an existing publication in a federate  useful
for creating publication objects from publications generated by a configuration
script  
";

%feature("docstring") helics::PublicationBase::~PublicationBase "
`~PublicationBase()=default`  

default destructor  
";

%feature("docstring") helics::PublicationBase::getID "
`getID() const -> publication_id_t`  
";

%feature("docstring") helics::PublicationBase::getKey "
`getKey() const -> std::string`  

get the key for the subscription  
";

%feature("docstring") helics::PublicationBase::getName "
`getName() const -> const std::string &`  

get the key for the subscription  
";

%feature("docstring") helics::PublicationBase::getType "
`getType() const -> const std::string &`  

get the key for the subscription  
";

%feature("docstring") helics::PublicationBase::getUnits "
`getUnits() const -> const std::string &`  
";

// File: classhelics_1_1PublicationInfo.xml


%feature("docstring") helics::PublicationInfo "

data class containing the information about a publication  

C++ includes: PublicationInfo.hpp
";

%feature("docstring") helics::PublicationInfo::PublicationInfo "
`PublicationInfo(Core::handle_id_t id_, Core::federate_id_t fed_id_, const
    std::string &key_, const std::string &type_, const std::string &units_)`  

constructor from the basic information  
";

%feature("docstring") helics::PublicationInfo::CheckSetValue "
`CheckSetValue(const char *checkData, uint64_t len) -> bool`  

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
`PublicationOnChange()=default`  
";

%feature("docstring") helics::PublicationOnChange::PublicationOnChange "
`PublicationOnChange(ValueFederate *valueFed, const std::string &name, const X
    &minChange, const std::string &units=\"\")`  

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
`publish(const X &val) const override`  

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
`PublicationT()=default`  
";

%feature("docstring") helics::PublicationT::PublicationT "
`PublicationT(ValueFederate *valueFed, const std::string &name, const
    std::string &units=\"\")`  

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
`PublicationT(interface_visibility locality, ValueFederate *valueFed, const
    std::string &name, const std::string &units=\"\")`  

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
`publish(const X &val) const`  

send a value for publication  

Parameters
----------
* `val` :  
    the value to publish  
";

%feature("docstring") helics::PublicationT::publish "
`publish(const X &val, const std::string &) const`  

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
`RandomDelayFilterOperation()`  
";

%feature("docstring") helics::RandomDelayFilterOperation::~RandomDelayFilterOperation "
`~RandomDelayFilterOperation()`  
";

%feature("docstring") helics::RandomDelayFilterOperation::set "
`set(const std::string &property, double val) override`  

set a property on a filter  

Parameters
----------
* `property` :  
    the name of the property of the filter to change  
* `val` :  
    the numerical value of the property  
";

%feature("docstring") helics::RandomDelayFilterOperation::setString "
`setString(const std::string &property, const std::string &val) override`  

set a string property on a filter  

Parameters
----------
* `property` :  
    the name of the property of the filter to change  
* `val` :  
    the numerical value of the property  
";

%feature("docstring") helics::RandomDelayFilterOperation::getOperator "
`getOperator() override -> std::shared_ptr< FilterOperator >`  
";

// File: classhelics_1_1randomDelayGenerator.xml


%feature("docstring") helics::randomDelayGenerator "

class wrapping the distribution generation functions and parameters  
";

%feature("docstring") helics::randomDelayGenerator::generate "
`generate() -> double`  
";

// File: classhelics_1_1RandomDropFilterOperation.xml


%feature("docstring") helics::RandomDropFilterOperation "

filter for randomly dropping a packet  

C++ includes: FilterOperations.hpp
";

%feature("docstring") helics::RandomDropFilterOperation::RandomDropFilterOperation "
`RandomDropFilterOperation()`  
";

%feature("docstring") helics::RandomDropFilterOperation::~RandomDropFilterOperation "
`~RandomDropFilterOperation()`  
";

%feature("docstring") helics::RandomDropFilterOperation::set "
`set(const std::string &property, double val) override`  

set a property on a filter  

Parameters
----------
* `property` :  
    the name of the property of the filter to change  
* `val` :  
    the numerical value of the property  
";

%feature("docstring") helics::RandomDropFilterOperation::setString "
`setString(const std::string &property, const std::string &val) override`  

set a string property on a filter  

Parameters
----------
* `property` :  
    the name of the property of the filter to change  
* `val` :  
    the numerical value of the property  
";

%feature("docstring") helics::RandomDropFilterOperation::getOperator "
`getOperator() override -> std::shared_ptr< FilterOperator >`  
";

// File: classhelics_1_1Recorder.xml


%feature("docstring") helics::Recorder "

class designed to capture data points from a set of subscriptions or endpoints  

C++ includes: recorder.h
";

%feature("docstring") helics::Recorder::Recorder "
`Recorder(FederateInfo &fi)`  

construct from a FederateInfo structure  
";

%feature("docstring") helics::Recorder::Recorder "
`Recorder(int argc, char *argv[])`  

construct from command line arguments  
";

%feature("docstring") helics::Recorder::Recorder "
`Recorder(std::shared_ptr< Core > core, const FederateInfo &fi)`  

constructor taking a federate information structure and using the given core  

Parameters
----------
* `core` :  
    a pointer to core object which the federate can join  
* `fi` :  
    a federate information structure  
";

%feature("docstring") helics::Recorder::Recorder "
`Recorder(const std::string &jsonString)`  

constructor taking a file with the required information  

Parameters
----------
* `file` :  
    a file defining the federate information  
";

%feature("docstring") helics::Recorder::Recorder "
`Recorder(Recorder &&other_player)=default`  

move construction  
";

%feature("docstring") helics::Recorder::Recorder "
`Recorder(const Recorder &other_player)=delete`  

don't allow the copy constructor  
";

%feature("docstring") helics::Recorder::~Recorder "
`~Recorder()`  

destructor  
";

%feature("docstring") helics::Recorder::loadFile "
`loadFile(const std::string &filename) -> int`  

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
`run()`  
";

%feature("docstring") helics::Recorder::run "
`run(Time stopTime)`  

run the Player until the specified time  
";

%feature("docstring") helics::Recorder::addSubscription "
`addSubscription(const std::string &key)`  

add a subscription to capture  

add a subscription to record  
";

%feature("docstring") helics::Recorder::addEndpoint "
`addEndpoint(const std::string &endpoint)`  

add an endpoint  
";

%feature("docstring") helics::Recorder::addSourceEndpointClone "
`addSourceEndpointClone(const std::string &sourceEndpoint)`  

copy all messages that come from a specified endpoint  
";

%feature("docstring") helics::Recorder::addDestEndpointClone "
`addDestEndpointClone(const std::string &destEndpoint)`  

copy all messages that are going to a specific endpoint  
";

%feature("docstring") helics::Recorder::addCapture "
`addCapture(const std::string &captureDesc)`  

add a capture interface  

Parameters
----------
* `captureDesc` :  
    describes a federate to capture all the interfaces for  
";

%feature("docstring") helics::Recorder::saveFile "
`saveFile(const std::string &filename)`  

save the data to a file  
";

%feature("docstring") helics::Recorder::pointCount "
`pointCount() const -> auto`  

get the number of captured points  
";

%feature("docstring") helics::Recorder::messageCount "
`messageCount() const -> auto`  

get the number of captured messages  
";

%feature("docstring") helics::Recorder::getValue "
`getValue(int index) const -> std::pair< std::string, std::string >`  

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
`getMessage(int index) const -> std::unique_ptr< Message >`  

get a message  

makes a copy of a message and returns it in a unique_ptr  

Parameters
----------
* `index` :  
    the number of the message to retrieve  
";

%feature("docstring") helics::Recorder::finalize "
`finalize()`  

finalize the federate  
";

%feature("docstring") helics::Recorder::isActive "
`isActive() const -> bool`  

check if the Recorder is ready to run  
";

// File: classhelics_1_1RegistrationFailure.xml


%feature("docstring") helics::RegistrationFailure "

exception indicating that the registration of an object has failed  

C++ includes: core-exceptions.hpp
";

%feature("docstring") helics::RegistrationFailure::RegistrationFailure "
`RegistrationFailure(const std::string &message=\"registration failure\")`  
";

// File: classhelics_1_1RerouteFilterOperation.xml


%feature("docstring") helics::RerouteFilterOperation "

filter for rerouting a packet to a particular endpoint  

C++ includes: FilterOperations.hpp
";

%feature("docstring") helics::RerouteFilterOperation::RerouteFilterOperation "
`RerouteFilterOperation()`  
";

%feature("docstring") helics::RerouteFilterOperation::~RerouteFilterOperation "
`~RerouteFilterOperation()`  
";

%feature("docstring") helics::RerouteFilterOperation::set "
`set(const std::string &property, double val) override`  

set a property on a filter  

Parameters
----------
* `property` :  
    the name of the property of the filter to change  
* `val` :  
    the numerical value of the property  
";

%feature("docstring") helics::RerouteFilterOperation::setString "
`setString(const std::string &property, const std::string &val) override`  

set a string property on a filter  

Parameters
----------
* `property` :  
    the name of the property of the filter to change  
* `val` :  
    the numerical value of the property  
";

%feature("docstring") helics::RerouteFilterOperation::getOperator "
`getOperator() override -> std::shared_ptr< FilterOperator >`  
";

// File: classSearchableObjectHolder.xml


%feature("docstring") SearchableObjectHolder "

helper class to destroy objects at a late time when it is convenient and there
are no more possibilities of threading issues  

C++ includes: searchableObjectHolder.hpp
";

%feature("docstring") SearchableObjectHolder::SearchableObjectHolder "
`SearchableObjectHolder()=default`  
";

%feature("docstring") SearchableObjectHolder::SearchableObjectHolder "
`SearchableObjectHolder(SearchableObjectHolder &&) noexcept=delete`  
";

%feature("docstring") SearchableObjectHolder::~SearchableObjectHolder "
`~SearchableObjectHolder()`  
";

%feature("docstring") SearchableObjectHolder::addObject "
`addObject(const std::string &name, std::shared_ptr< X > &obj) -> bool`  
";

%feature("docstring") SearchableObjectHolder::removeObject "
`removeObject(const std::string &name) -> bool`  
";

%feature("docstring") SearchableObjectHolder::removeObject "
`removeObject(std::function< bool(const std::shared_ptr< X > &)> operand) ->
    bool`  
";

%feature("docstring") SearchableObjectHolder::copyObject "
`copyObject(const std::string &copyFromName, const std::string &copyToName) ->
    bool`  
";

%feature("docstring") SearchableObjectHolder::findObject "
`findObject(const std::string &name) -> std::shared_ptr< X >`  
";

%feature("docstring") SearchableObjectHolder::findObject "
`findObject(std::function< bool(const std::shared_ptr< X > &)> operand) ->
    std::shared_ptr< X >`  
";

// File: classhelics_1_1sendToQueue.xml


%feature("docstring") helics::sendToQueue "

class implementing interactions with a queue to transmit data  

C++ includes: IpcQueueHelper.h
";

%feature("docstring") helics::sendToQueue::sendToQueue "
`sendToQueue()=default`  
";

%feature("docstring") helics::sendToQueue::connect "
`connect(const std::string &connection, bool initOnly, int retries) -> bool`  
";

%feature("docstring") helics::sendToQueue::sendMessage "
`sendMessage(const ActionMessage &cmd, int priority)`  
";

%feature("docstring") helics::sendToQueue::getError "
`getError() const -> const std::string &`  
";

// File: classhelics_1_1shared__queue__state.xml


%feature("docstring") helics::shared_queue_state "

class defining a shared queue state meaning interaction with a queue the object
is not the owner of  

C++ includes: IpcQueueHelper.h
";

%feature("docstring") helics::shared_queue_state::getState "
`getState() const -> queue_state_t`  
";

%feature("docstring") helics::shared_queue_state::setState "
`setState(queue_state_t newState) -> bool`  
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
`SimpleQueue()=default`  

default constructor  
";

%feature("docstring") SimpleQueue::SimpleQueue "
`SimpleQueue(size_t capacity)`  

constructor with a reservation size  

Parameters
----------
* `capacity` :  
    the initial storage capacity of the queue  
";

%feature("docstring") SimpleQueue::SimpleQueue "
`SimpleQueue(SimpleQueue &&sq) noexcept`  

enable the move constructor not the copy constructor  
";

%feature("docstring") SimpleQueue::SimpleQueue "
`SimpleQueue(const SimpleQueue &)=delete`  

DISABLE_COPY_AND_ASSIGN  
";

%feature("docstring") SimpleQueue::~SimpleQueue "
`~SimpleQueue()`  
";

%feature("docstring") SimpleQueue::empty "
`empty() const -> bool`  

check whether there are any elements in the queue because this is meant for
multi threaded applications this may or may not have any meaning depending on
the number of consumers  
";

%feature("docstring") SimpleQueue::size "
`size() const -> size_t`  

get the current size of the queue  
";

%feature("docstring") SimpleQueue::clear "
`clear()`  

clear the queue  
";

%feature("docstring") SimpleQueue::reserve "
`reserve(size_t capacity)`  

set the capacity of the queue actually double the requested the size will be
reserved due to the use of two vectors internally  

Parameters
----------
* `capacity` :  
    the capacity to reserve  
";

%feature("docstring") SimpleQueue::push "
`push(Z &&val)`  

push an element onto the queue val the value to push on the queue  
";

%feature("docstring") SimpleQueue::emplace "
`emplace(Args &&... args)`  

push an element onto the queue val the value to push on the queue  
";

%feature("docstring") SimpleQueue::pop "
`pop() -> stx::optional< X >`  

extract the first element from the queue  

Returns
-------
an empty optional if there is no element otherwise the optional will contain a
value  
";

%feature("docstring") SimpleQueue::peek "
`peek() const -> stx::optional< X >`  

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
`socket_t(context_t &context_, int type_)`  
";

%feature("docstring") zmq::socket_t::~socket_t "
`~socket_t() ZMQ_NOTHROW`  
";

%feature("docstring") zmq::socket_t::close "
`close() ZMQ_NOTHROW`  
";

%feature("docstring") zmq::socket_t::setsockopt "
`setsockopt(int option_, const std::string &optval)`  
";

%feature("docstring") zmq::socket_t::setsockopt "
`setsockopt(int option_, T const &optval)`  
";

%feature("docstring") zmq::socket_t::setsockopt "
`setsockopt(int option_, const void *optval_, size_t optvallen_)`  
";

%feature("docstring") zmq::socket_t::getsockopt "
`getsockopt(int option_, void *optval_, size_t *optvallen_) const`  
";

%feature("docstring") zmq::socket_t::getsockopt "
`getsockopt(int option_) const -> T`  
";

%feature("docstring") zmq::socket_t::getsockopt "
`getsockopt(int option_) const -> std::string`  
";

%feature("docstring") zmq::socket_t::bind "
`bind(std::string const &addr)`  
";

%feature("docstring") zmq::socket_t::bind "
`bind(const char *addr_)`  
";

%feature("docstring") zmq::socket_t::unbind "
`unbind(std::string const &addr)`  
";

%feature("docstring") zmq::socket_t::unbind "
`unbind(const char *addr_)`  
";

%feature("docstring") zmq::socket_t::connect "
`connect(std::string const &addr)`  
";

%feature("docstring") zmq::socket_t::connect "
`connect(const char *addr_)`  
";

%feature("docstring") zmq::socket_t::disconnect "
`disconnect(std::string const &addr)`  
";

%feature("docstring") zmq::socket_t::disconnect "
`disconnect(const char *addr_)`  
";

%feature("docstring") zmq::socket_t::connected "
`connected() const ZMQ_NOTHROW -> bool`  
";

%feature("docstring") zmq::socket_t::send "
`send(std::string const &msg, int flags_=0) -> size_t`  
";

%feature("docstring") zmq::socket_t::send "
`send(const void *buf_, size_t len_, int flags_=0) -> size_t`  
";

%feature("docstring") zmq::socket_t::send "
`send(message_t &msg_, int flags_=0) -> bool`  
";

%feature("docstring") zmq::socket_t::send "
`send(I first, I last, int flags_=0) -> bool`  
";

%feature("docstring") zmq::socket_t::recv "
`recv(void *buf_, size_t len_, int flags_=0) -> size_t`  
";

%feature("docstring") zmq::socket_t::recv "
`recv(message_t *msg_, int flags_=0) -> bool`  
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
`Source()=default`  

default constructor  
";

%feature("docstring") helics::Source::Source "
`Source(int argc, char *argv[])`  

construct from command line arguments  

Parameters
----------
* `argc` :  
    the number of arguments  
* `argv` :  
    the strings in the input  
";

%feature("docstring") helics::Source::Source "
`Source(const FederateInfo &fi)`  

construct from a federate info object  

Parameters
----------
* `fi` :  
    a pointer info object containing information on the desired federate
    configuration  
";

%feature("docstring") helics::Source::Source "
`Source(std::shared_ptr< Core > core, const FederateInfo &fi)`  

constructor taking a federate information structure and using the given core  

Parameters
----------
* `core` :  
    a pointer to core object which the federate can join  
* `fi` :  
    a federate information structure  
";

%feature("docstring") helics::Source::Source "
`Source(const std::string &jsonString)`  

constructor taking a file with the required information  

Parameters
----------
* `jsonString` :  
    file or json string defining the federate information and other
    configuration  
";

%feature("docstring") helics::Source::Source "
`Source(Source &&other_source)=default`  

move construction  
";

%feature("docstring") helics::Source::Source "
`Source(const Source &other_source)=delete`  

don't allow the copy constructor  
";

%feature("docstring") helics::Source::~Source "
`~Source()`  
";

%feature("docstring") helics::Source::loadFile "
`loadFile(const std::string &filename)`  

load a file containing publication information  

Parameters
----------
* `filename` :  
    the file containing the configuration and source data accepted format are
    json, xml, and a source format which is tab delimited or comma delimited  
";

%feature("docstring") helics::Source::initialize "
`initialize()`  

initialize the source federate  

generate all the publications and organize the points, the final publication
count will be available after this time and the source will enter the
initialization mode, which means it will not be possible to add more
publications calling run will automatically do this if necessary  
";

%feature("docstring") helics::Source::run "
`run()`  
";

%feature("docstring") helics::Source::run "
`run(Time stopTime_input)`  

run the source until the specified time  

Parameters
----------
* `stopTime_input` :  
    the desired stop time  
";

%feature("docstring") helics::Source::addSource "
`addSource(const std::string &key, helics_type_t type, const std::string
    &units=\"\")`  

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
`SourceFilter(Federate *fed, const std::string &target, const std::string
    &name=EMPTY_STRING, const std::string &input_type=EMPTY_STRING, const
    std::string &output_type=EMPTY_STRING)`  

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
`SourceFilter(Core *cr, const std::string &target, const std::string
    &name=EMPTY_STRING, const std::string &input_type=EMPTY_STRING, const
    std::string &output_type=EMPTY_STRING)`  

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
`~SourceFilter()=default`  
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
`StringToCmdLine(const std::string &cmdString)`  

construct from a string  
";

%feature("docstring") StringToCmdLine::load "
`load(const std::string &cmdString)`  

load a string  

Parameters
----------
* `cmdString` :  
    a single string containing command line arguments  
";

%feature("docstring") StringToCmdLine::getArgCount "
`getArgCount() const -> int`  

get the number of separate arguments corresponding to argc  
";

%feature("docstring") StringToCmdLine::getArgV "
`getArgV() -> auto`  

get the argument values corresponding to char *argv[]  
";

// File: classhelics_1_1Subscription.xml


%feature("docstring") helics::Subscription "

primary subscription object class  

can convert between the helics primary base class types  

C++ includes: Subscriptions.hpp
";

%feature("docstring") helics::Subscription::Subscription "
`Subscription()=default`  
";

%feature("docstring") helics::Subscription::Subscription "
`Subscription(ValueFederate *valueFed, const std::string &key, const std::string
    &units=\"\")`  
";

%feature("docstring") helics::Subscription::Subscription "
`Subscription(bool required, ValueFederate *valueFed, const std::string &key,
    const std::string &units=\"\")`  
";

%feature("docstring") helics::Subscription::Subscription "
`Subscription(ValueFederate *valueFed, const std::string &key, helics_type_t
    defType, const std::string &units=\"\")`  
";

%feature("docstring") helics::Subscription::Subscription "
`Subscription(bool required, ValueFederate *valueFed, const std::string &key,
    helics_type_t defType, const std::string &units=\"\")`  
";

%feature("docstring") helics::Subscription::Subscription "
`Subscription(ValueFederate *valueFed, int subIndex)`  

generate a subscription object from a preexisting subscription  

Parameters
----------
* `valueFed` :  
    a pointer to the appropriate value Federate  
* `subIndex` :  
    the index of the subscription  
";

%feature("docstring") helics::Subscription::isUpdated "
`isUpdated() const override -> bool`  

check if the value has been updated  
";

%feature("docstring") helics::Subscription::getValue "
`getValue(X &out) -> std::enable_if_t< helicsType< X >)
    !=helics_type_t::helicsInvalid >`  

get the latest value for the subscription  

Parameters
----------
* `out` :  
    the location to store the value  
";

%feature("docstring") helics::Subscription::getValue "
`getValue() -> std::enable_if_t< helicsType< X >)
    !=helics_type_t::helicsInvalid, X >`  

get the most recent value  

Returns
-------
the value  
";

%feature("docstring") helics::Subscription::getValueAs "
`getValueAs() -> std::enable_if_t< isConvertableType< X >), X >`  

get the most recent value  

Returns
-------
the value  
";

%feature("docstring") helics::Subscription::getValueAs "
`getValueAs(X &out) -> std::enable_if_t< isConvertableType< X >)>`  

get the most recent calculation with the result as a convertible type  
";

%feature("docstring") helics::Subscription::registerCallback "
`registerCallback(std::function< void(const X &, Time)> callback) ->
    std::enable_if_t< helicsType< X >) !=helics_type_t::helicsInvalid, void >`  

register a callback for the update  

the callback is called in the just before the time request function returns  

Parameters
----------
* `callback` :  
    a function with signature void(X val, Time time) val is the new value and
    time is the time the value was updated  
";

%feature("docstring") helics::Subscription::registerCallback "
`registerCallback(std::function< void(Time)> callback)`  

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
`setDefault(const X &val) -> std::enable_if_t< helicsType< X >)
    !=helics_type_t::helicsInvalid, void >`  

set the default value to use before any update has been published  
";

%feature("docstring") helics::Subscription::setMinimumChange "
`setMinimumChange(double deltaV)`  

set the minimum delta for change detection  

Parameters
----------
* `detltaV` :  
    a double with the change in a value in order to register a different value  
";

%feature("docstring") helics::Subscription::enableChangeDetection "
`enableChangeDetection(bool enabled=true)`  

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
`subscription_info(const std::string &n_name, const std::string &n_type, const
    std::string &n_units)`  
";

// File: classhelics_1_1SubscriptionBase.xml


%feature("docstring") helics::SubscriptionBase "

base class for a subscription object  

C++ includes: Subscriptions.hpp
";

%feature("docstring") helics::SubscriptionBase::SubscriptionBase "
`SubscriptionBase()=default`  
";

%feature("docstring") helics::SubscriptionBase::SubscriptionBase "
`SubscriptionBase(ValueFederate *valueFed, const std::string &key, const
    std::string &type=\"def\", const std::string &units=\"\")`  
";

%feature("docstring") helics::SubscriptionBase::SubscriptionBase "
`SubscriptionBase(bool required, ValueFederate *valueFed, const std::string
    &key, const std::string &type=\"def\", const std::string &units=\"\")`  
";

%feature("docstring") helics::SubscriptionBase::SubscriptionBase "
`SubscriptionBase(ValueFederate *valueFed, int subIndex)`  
";

%feature("docstring") helics::SubscriptionBase::SubscriptionBase "
`SubscriptionBase(SubscriptionBase &&base)=default`  
";

%feature("docstring") helics::SubscriptionBase::~SubscriptionBase "
`~SubscriptionBase()=default`  
";

%feature("docstring") helics::SubscriptionBase::getLastUpdate "
`getLastUpdate() const -> Time`  

get the time of the last update  

Returns
-------
the time of the last update  
";

%feature("docstring") helics::SubscriptionBase::isUpdated "
`isUpdated() const -> bool`  

check if the value has subscription has been updated  
";

%feature("docstring") helics::SubscriptionBase::getID "
`getID() const -> subscription_id_t`  
";

%feature("docstring") helics::SubscriptionBase::registerCallback "
`registerCallback(std::function< void(Time)> callback)`  

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
`getKey() const -> const std::string &`  

get the key for the subscription  
";

%feature("docstring") helics::SubscriptionBase::getName "
`getName() const -> const std::string &`  

get the key for the subscription  
";

%feature("docstring") helics::SubscriptionBase::getType "
`getType() const -> std::string`  

get the key for the subscription  
";

%feature("docstring") helics::SubscriptionBase::getUnits "
`getUnits() const -> const std::string &`  

get the units associated with a subscription  
";

// File: classhelics_1_1SubscriptionInfo.xml


%feature("docstring") helics::SubscriptionInfo "

data class for managing information about a subscription  

C++ includes: SubscriptionInfo.hpp
";

%feature("docstring") helics::SubscriptionInfo::SubscriptionInfo "
`SubscriptionInfo(Core::handle_id_t id_, Core::federate_id_t fed_id_, const
    std::string &key_, const std::string &type_, const std::string &units_, bool
    required_=false)`  

constructor with all the information  
";

%feature("docstring") helics::SubscriptionInfo::getData "
`getData() -> std::shared_ptr< const data_block >`  

get the current data  
";

%feature("docstring") helics::SubscriptionInfo::addData "
`addData(Time valueTime, std::shared_ptr< const data_block > data)`  

add a data block into the queue  
";

%feature("docstring") helics::SubscriptionInfo::updateTime "
`updateTime(Time newTime) -> bool`  

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
`nextValueTime() const -> Time`  
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
`SubscriptionT()=default`  
";

%feature("docstring") helics::SubscriptionT::SubscriptionT "
`SubscriptionT(ValueFederate *valueFed, const std::string &name, const
    std::string &units=\"\")`  

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
`SubscriptionT(bool required, ValueFederate *valueFed, const std::string &name,
    const std::string &units=\"\")`  

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
`getValue() const -> X`  

get the most recent value  

Returns
-------
the value  
";

%feature("docstring") helics::SubscriptionT::getValue "
`getValue(X &out) const`  

store the value in the given variable  

Parameters
----------
* `out` :  
    the location to store the value  
";

%feature("docstring") helics::SubscriptionT::registerCallback "
`registerCallback(std::function< void(X, Time)> callback)`  

register a callback for the update  

the callback is called in the just before the time request function returns  

Parameters
----------
* `callback` :  
    a function with signature void(X val, Time time) val is the new value and
    time is the time the value was updated  
";

%feature("docstring") helics::SubscriptionT::registerCallback "
`registerCallback(std::function< void(Time)> callback)`  

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
`setDefault(const X &val)`  

set a default value  

Parameters
----------
* `val` :  
    the value to set as the default  
";

%feature("docstring") helics::SubscriptionT::setMinimumChange "
`setMinimumChange(double deltaV)`  

set a minimum change value  
";

%feature("docstring") helics::SubscriptionT::enableChangeDetection "
`enableChangeDetection(bool enabled=true)`  
";

// File: classtcp__connection.xml


%feature("docstring") tcp_connection "

tcp socket connection for connecting to a server  

C++ includes: TcpHelperClasses.h
";

%feature("docstring") tcp_connection::create "
`create(boost::asio::io_service &io_service, const std::string &connection,
    const std::string &port, size_t bufferSize=10192) -> pointer`  
";

%feature("docstring") tcp_connection::socket "
`socket() -> boost::asio::ip::tcp::socket &`  
";

%feature("docstring") tcp_connection::cancel "
`cancel()`  
";

%feature("docstring") tcp_connection::close "
`close()`  

close the socket connection  

Parameters
----------
*  :  
";

%feature("docstring") tcp_connection::send "
`send(const void *buffer, size_t dataLength)`  

send raw data  

Exceptions
----------
* `boost::system::system_error` :  
    on failure  
";

%feature("docstring") tcp_connection::send "
`send(const std::string &dataString)`  

send a string  

Exceptions
----------
* `boost::system::system_error` :  
    on failure  
";

%feature("docstring") tcp_connection::receive "
`receive(void *buffer, size_t maxDataSize) -> size_t`  

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
`send_async(const void *buffer, size_t dataLength, Process callback)`  

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
`async_receive(void *buffer, size_t dataLength, Process callback)`  

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
`async_receive(std::function< void(tcp_connection::pointer, const char *,
    size_t, const boost::system::error_code &error)> callback)`  

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
`isConnected() const -> bool`  

check if the socket has finished the connection process  
";

%feature("docstring") tcp_connection::waitUntilConnected "
`waitUntilConnected(int timeOut) -> int`  

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
`create(boost::asio::io_service &io_service, size_t bufferSize) -> pointer`  
";

%feature("docstring") tcp_rx_connection::socket "
`socket() -> boost::asio::ip::tcp::socket &`  
";

%feature("docstring") tcp_rx_connection::start "
`start()`  
";

%feature("docstring") tcp_rx_connection::stop "
`stop()`  
";

%feature("docstring") tcp_rx_connection::close "
`close()`  
";

%feature("docstring") tcp_rx_connection::setDataCall "
`setDataCall(std::function< size_t(tcp_rx_connection::pointer, const char *,
    size_t)> dataFunc)`  
";

%feature("docstring") tcp_rx_connection::setErrorCall "
`setErrorCall(std::function< bool(tcp_rx_connection::pointer, const
    boost::system::error_code &)> errorFunc)`  
";

%feature("docstring") tcp_rx_connection::send "
`send(const void *buffer, size_t dataLength)`  

send raw data  

Exceptions
----------
* `boost::system::system_error` :  
    on failure  
";

%feature("docstring") tcp_rx_connection::send "
`send(const std::string &dataString)`  

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
`tcp_server(boost::asio::io_service &io_service, int PortNum, int
    nominalBufferSize=10192)`  
";

%feature("docstring") tcp_server::stop "
`stop()`  
";

%feature("docstring") tcp_server::start "
`start()`  
";

%feature("docstring") tcp_server::close "
`close()`  
";

%feature("docstring") tcp_server::setDataCall "
`setDataCall(std::function< size_t(tcp_rx_connection::pointer, const char *,
    size_t)> dataFunc)`  
";

%feature("docstring") tcp_server::setErrorCall "
`setErrorCall(std::function< bool(tcp_rx_connection::pointer, const
    boost::system::error_code &)> errorFunc)`  
";

// File: classhelics_1_1TcpBroker.xml


%feature("docstring") helics::TcpBroker "
";

%feature("docstring") helics::TcpBroker::TcpBroker "
`TcpBroker(bool rootBroker=false) noexcept`  

default constructor  
";

%feature("docstring") helics::TcpBroker::TcpBroker "
`TcpBroker(const std::string &broker_name)`  
";

%feature("docstring") helics::TcpBroker::initializeFromArgs "
`initializeFromArgs(int argc, const char *const *argv) override`  

initialize from command line arguments  
";

%feature("docstring") helics::TcpBroker::~TcpBroker "
`~TcpBroker()`  

destructor  
";

%feature("docstring") helics::TcpBroker::getAddress "
`getAddress() const override -> std::string`  

get the connection address for the broker  
";

%feature("docstring") helics::TcpBroker::displayHelp "
`displayHelp(bool local_only=false)`  
";

// File: classhelics_1_1TcpComms.xml


%feature("docstring") helics::TcpComms "

implementation for the communication interface that uses TCP messages to
communicate  

C++ includes: TcpComms.h
";

%feature("docstring") helics::TcpComms::TcpComms "
`TcpComms()`  

default constructor  
";

%feature("docstring") helics::TcpComms::TcpComms "
`TcpComms(const std::string &brokerTarget, const std::string &localTarget)`  
";

%feature("docstring") helics::TcpComms::TcpComms "
`TcpComms(const NetworkBrokerData &netInfo)`  
";

%feature("docstring") helics::TcpComms::~TcpComms "
`~TcpComms()`  

destructor  
";

%feature("docstring") helics::TcpComms::setBrokerPort "
`setBrokerPort(int brokerPortNumber)`  

set the port numbers for the local ports  
";

%feature("docstring") helics::TcpComms::setPortNumber "
`setPortNumber(int localPortNumber)`  
";

%feature("docstring") helics::TcpComms::setAutomaticPortStartPort "
`setAutomaticPortStartPort(int startingPort)`  
";

%feature("docstring") helics::TcpComms::getPort "
`getPort() const -> int`  

get the port number of the comms object to push message to  
";

%feature("docstring") helics::TcpComms::getAddress "
`getAddress() const -> std::string`  
";

// File: classhelics_1_1TcpCore.xml


%feature("docstring") helics::TcpCore "

implementation for the core that uses tcp messages to communicate  

C++ includes: TcpCore.h
";

%feature("docstring") helics::TcpCore::TcpCore "
`TcpCore() noexcept`  

default constructor  
";

%feature("docstring") helics::TcpCore::TcpCore "
`TcpCore(const std::string &core_name)`  
";

%feature("docstring") helics::TcpCore::~TcpCore "
`~TcpCore()`  
";

%feature("docstring") helics::TcpCore::initializeFromArgs "
`initializeFromArgs(int argc, const char *const *argv) override`  

initialize the core manager with command line arguments  

Parameters
----------
* `argc` :  
    the number of arguments  
* `argv` :  
    char pointers to the arguments  
";

%feature("docstring") helics::TcpCore::getAddress "
`getAddress() const override -> std::string`  

get a string representing the connection info to send data to this object  
";

// File: classhelics_1_1TestBroker.xml


%feature("docstring") helics::TestBroker "

class implementing a basic broker that links to other brokers in process memory  

C++ includes: TestBroker.h
";

%feature("docstring") helics::TestBroker::TestBroker "
`TestBroker(bool isRoot_=false) noexcept`  

default constructor  
";

%feature("docstring") helics::TestBroker::TestBroker "
`TestBroker(const std::string &broker_name)`  
";

%feature("docstring") helics::TestBroker::TestBroker "
`TestBroker(std::shared_ptr< TestBroker > nbroker)`  

construct with a pointer to a broker  
";

%feature("docstring") helics::TestBroker::~TestBroker "
`~TestBroker()`  
";

%feature("docstring") helics::TestBroker::initializeFromArgs "
`initializeFromArgs(int argc, const char *const *argv) override`  

initialize from command line arguments  
";

%feature("docstring") helics::TestBroker::getAddress "
`getAddress() const override -> std::string`  

get the connection address for the broker  
";

%feature("docstring") helics::TestBroker::displayHelp "
`displayHelp(bool localOnly=false)`  

static method to display the help message  
";

// File: classhelics_1_1TestCore.xml


%feature("docstring") helics::TestCore "

an object implementing a local core object that can communicate in process  

C++ includes: TestCore.h
";

%feature("docstring") helics::TestCore::TestCore "
`TestCore()=default`  

default constructor  
";

%feature("docstring") helics::TestCore::TestCore "
`TestCore(const std::string &core_name)`  

construct from a core name  
";

%feature("docstring") helics::TestCore::TestCore "
`TestCore(std::shared_ptr< CoreBroker > nbroker)`  

construct with a pointer to a broker  
";

%feature("docstring") helics::TestCore::~TestCore "
`~TestCore()`  

destructor  
";

%feature("docstring") helics::TestCore::initializeFromArgs "
`initializeFromArgs(int argc, const char *const *argv) override`  

initialize the core manager with command line arguments  

Parameters
----------
* `argc` :  
    the number of arguments  
* `argv` :  
    char pointers to the arguments  
";

%feature("docstring") helics::TestCore::getAddress "
`getAddress() const override -> std::string`  

get a string representing the connection info to send data to this object  
";

// File: classhelics_1_1TimeCoordinator.xml


%feature("docstring") helics::TimeCoordinator "
";

%feature("docstring") helics::TimeCoordinator::TimeCoordinator "
`TimeCoordinator()=default`  
";

%feature("docstring") helics::TimeCoordinator::TimeCoordinator "
`TimeCoordinator(const CoreFederateInfo &info_)`  
";

%feature("docstring") helics::TimeCoordinator::getFedInfo "
`getFedInfo() -> CoreFederateInfo &`  
";

%feature("docstring") helics::TimeCoordinator::getFedInfo "
`getFedInfo() const -> const CoreFederateInfo &`  
";

%feature("docstring") helics::TimeCoordinator::setInfo "
`setInfo(const CoreFederateInfo &info_)`  
";

%feature("docstring") helics::TimeCoordinator::setMessageSender "
`setMessageSender(std::function< void(const ActionMessage &)>
    sendMessageFunction_)`  
";

%feature("docstring") helics::TimeCoordinator::getGrantedTime "
`getGrantedTime() const -> Time`  
";

%feature("docstring") helics::TimeCoordinator::getDependencies "
`getDependencies() const -> std::vector< Core::federate_id_t >`  
";

%feature("docstring") helics::TimeCoordinator::getDependents "
`getDependents() const -> const std::vector< Core::federate_id_t > &`  

get a reference to the dependents vector  
";

%feature("docstring") helics::TimeCoordinator::getCurrentIteration "
`getCurrentIteration() const -> int32_t`  

get the current iteration counter for an iterative call  

this will work properly even when a federate is processing  
";

%feature("docstring") helics::TimeCoordinator::updateTimeFactors "
`updateTimeFactors() -> bool`  

compute updates to time values  

Returns
-------
true if they have been modified  
";

%feature("docstring") helics::TimeCoordinator::updateValueTime "
`updateValueTime(Time valueUpdateTime)`  

update the time_value variable with a new value if needed  
";

%feature("docstring") helics::TimeCoordinator::updateMessageTime "
`updateMessageTime(Time messageUpdateTime)`  

update the time_message variable with a new value if needed  
";

%feature("docstring") helics::TimeCoordinator::getDependencyInfo "
`getDependencyInfo(Core::federate_id_t ofed) -> DependencyInfo *`  

take a global id and get a pointer to the dependencyInfo for the other fed will
be nullptr if it doesn't exist  
";

%feature("docstring") helics::TimeCoordinator::isDependency "
`isDependency(Core::federate_id_t ofed) const -> bool`  

check whether a federate is a dependency  
";

%feature("docstring") helics::TimeCoordinator::processTimeMessage "
`processTimeMessage(const ActionMessage &cmd) -> bool`  

process a message related to time  

Returns
-------
true if it did anything  
";

%feature("docstring") helics::TimeCoordinator::processConfigUpdateMessage "
`processConfigUpdateMessage(const ActionMessage &cmd, bool initMode=false)`  

process a message related to configuration  

Parameters
----------
* `cmd` :  
    the update command  
* `initMode` :  
    set to true to allow init only updates  
";

%feature("docstring") helics::TimeCoordinator::processDependencyUpdateMessage "
`processDependencyUpdateMessage(const ActionMessage &cmd)`  

process a dependency update message  
";

%feature("docstring") helics::TimeCoordinator::addDependency "
`addDependency(Core::federate_id_t fedID) -> bool`  

add a federate dependency  

Returns
-------
true if it was actually added, false if the federate was already present  
";

%feature("docstring") helics::TimeCoordinator::addDependent "
`addDependent(Core::federate_id_t fedID) -> bool`  

add a dependent federate  

Returns
-------
true if it was actually added, false if the federate was already present  
";

%feature("docstring") helics::TimeCoordinator::removeDependency "
`removeDependency(Core::federate_id_t fedID)`  
";

%feature("docstring") helics::TimeCoordinator::removeDependent "
`removeDependent(Core::federate_id_t fedID)`  
";

%feature("docstring") helics::TimeCoordinator::checkExecEntry "
`checkExecEntry() -> iteration_state`  

check if entry to the executing state can be granted  
";

%feature("docstring") helics::TimeCoordinator::timeRequest "
`timeRequest(Time nextTime, helics_iteration_request iterate, Time newValueTime,
    Time newMessageTime)`  
";

%feature("docstring") helics::TimeCoordinator::enteringExecMode "
`enteringExecMode(helics_iteration_request mode)`  
";

%feature("docstring") helics::TimeCoordinator::checkTimeGrant "
`checkTimeGrant() -> iteration_state`  

check if it is valid to grant a time  
";

%feature("docstring") helics::TimeCoordinator::printTimeStatus "
`printTimeStatus() const -> std::string`  

generate a string with the current time status  
";

// File: classhelics_1_1TimeDependencies.xml


%feature("docstring") helics::TimeDependencies "

class for managing a set of dependencies  

C++ includes: TimeDependencies.hpp
";

%feature("docstring") helics::TimeDependencies::TimeDependencies "
`TimeDependencies()=default`  

default constructor  
";

%feature("docstring") helics::TimeDependencies::isDependency "
`isDependency(Core::federate_id_t ofed) const -> bool`  

return true if the given federate is already a member  
";

%feature("docstring") helics::TimeDependencies::addDependency "
`addDependency(Core::federate_id_t id) -> bool`  

insert a dependency into the structure  

Returns
-------
true if the dependency was added, false if it existed already  
";

%feature("docstring") helics::TimeDependencies::removeDependency "
`removeDependency(Core::federate_id_t id)`  

remove dependency from consideration  
";

%feature("docstring") helics::TimeDependencies::updateTime "
`updateTime(const ActionMessage &m) -> bool`  

update the info about a dependency based on a message  
";

%feature("docstring") helics::TimeDependencies::begin "
`begin() -> auto`  

iterator to first dependency  
";

%feature("docstring") helics::TimeDependencies::begin "
`begin() const -> auto`  

const iterator to first dependency  
";

%feature("docstring") helics::TimeDependencies::end "
`end() -> auto`  

iterator to end point  
";

%feature("docstring") helics::TimeDependencies::end "
`end() const -> auto`  

const iterator to end point  
";

%feature("docstring") helics::TimeDependencies::cbegin "
`cbegin() const -> auto`  

const iterator to first dependency  
";

%feature("docstring") helics::TimeDependencies::cend "
`cend() const -> auto`  

const iterator to first dependency  
";

%feature("docstring") helics::TimeDependencies::getDependencyInfo "
`getDependencyInfo(Core::federate_id_t id) -> DependencyInfo *`  
";

%feature("docstring") helics::TimeDependencies::checkIfReadyForExecEntry "
`checkIfReadyForExecEntry(bool iterating) const -> bool`  
";

%feature("docstring") helics::TimeDependencies::checkIfReadyForTimeGrant "
`checkIfReadyForTimeGrant(bool iterating, Time desiredGrantTime) const -> bool`  
";

%feature("docstring") helics::TimeDependencies::resetIteratingExecRequests "
`resetIteratingExecRequests()`  
";

%feature("docstring") helics::TimeDependencies::resetIteratingTimeRequests "
`resetIteratingTimeRequests(Time requestTime)`  
";

// File: classTimeRepresentation.xml


%feature("docstring") TimeRepresentation "

prototype class for representing time  

time representation class that has as a template argument a class that can
define time as a number and has some required features  

C++ includes: timeRepresentation.hpp
";

%feature("docstring") TimeRepresentation::TimeRepresentation "
`TimeRepresentation() noexcept`  

default constructor  
";

%feature("docstring") TimeRepresentation::TimeRepresentation "
`TimeRepresentation(double t) noexcept -> constexpr`  

normal time constructor from a double representation of seconds  
";

%feature("docstring") TimeRepresentation::TimeRepresentation "
`TimeRepresentation(std::int64_t count, timeUnits units) noexcept -> constexpr`  
";

%feature("docstring") TimeRepresentation::TimeRepresentation "
`TimeRepresentation(const TimeRepresentation &x) noexcept=default -> constexpr`  
";

%feature("docstring") TimeRepresentation::seconds "
`seconds() const noexcept -> std::int64_t`  

generate the time in seconds  
";

%feature("docstring") TimeRepresentation::toCount "
`toCount(timeUnits units) const noexcept -> std::int64_t`  
";

%feature("docstring") TimeRepresentation::getBaseTimeCode "
`getBaseTimeCode() const noexcept -> baseType`  

get the underlying time code value  
";

%feature("docstring") TimeRepresentation::setBaseTimeCode "
`setBaseTimeCode(baseType timecodeval) noexcept`  

set the underlying base representation of a time directly  

this is not recommended for normal use  
";

%feature("docstring") TimeRepresentation::maxVal "
`maxVal() noexcept -> constexpr TimeRepresentation`  

generate a TimeRepresentation of the maximum representative value  
";

%feature("docstring") TimeRepresentation::minVal "
`minVal() noexcept -> constexpr TimeRepresentation`  

generate a TimeRepresentation of the minimum representative value  
";

%feature("docstring") TimeRepresentation::zeroVal "
`zeroVal() noexcept -> constexpr TimeRepresentation`  

generate a TimeRepresentation of 0  
";

%feature("docstring") TimeRepresentation::epsilon "
`epsilon() noexcept -> constexpr TimeRepresentation`  

generate a TimeRepresentation of 0  
";

// File: classhelics_1_1UdpBroker.xml


%feature("docstring") helics::UdpBroker "
";

%feature("docstring") helics::UdpBroker::UdpBroker "
`UdpBroker(bool rootBroker=false) noexcept`  

default constructor  
";

%feature("docstring") helics::UdpBroker::UdpBroker "
`UdpBroker(const std::string &broker_name)`  
";

%feature("docstring") helics::UdpBroker::initializeFromArgs "
`initializeFromArgs(int argc, const char *const *argv) override`  

initialize from command line arguments  
";

%feature("docstring") helics::UdpBroker::~UdpBroker "
`~UdpBroker()`  

destructor  
";

%feature("docstring") helics::UdpBroker::getAddress "
`getAddress() const override -> std::string`  

get the connection address for the broker  
";

%feature("docstring") helics::UdpBroker::displayHelp "
`displayHelp(bool local_only=false)`  
";

// File: classhelics_1_1UdpComms.xml


%feature("docstring") helics::UdpComms "

implementation for the communication interface that uses ZMQ messages to
communicate  

C++ includes: UdpComms.h
";

%feature("docstring") helics::UdpComms::UdpComms "
`UdpComms()`  

default constructor  
";

%feature("docstring") helics::UdpComms::UdpComms "
`UdpComms(const std::string &brokerTarget, const std::string &localTarget)`  
";

%feature("docstring") helics::UdpComms::UdpComms "
`UdpComms(const NetworkBrokerData &netInfo)`  
";

%feature("docstring") helics::UdpComms::~UdpComms "
`~UdpComms()`  

destructor  
";

%feature("docstring") helics::UdpComms::setBrokerPort "
`setBrokerPort(int brokerPortNumber)`  

set the port numbers for the local ports  
";

%feature("docstring") helics::UdpComms::setPortNumber "
`setPortNumber(int localPortNumber)`  
";

%feature("docstring") helics::UdpComms::setAutomaticPortStartPort "
`setAutomaticPortStartPort(int startingPort)`  
";

%feature("docstring") helics::UdpComms::getPort "
`getPort() const -> int`  

get the port number of the comms object to push message to  
";

%feature("docstring") helics::UdpComms::getAddress "
`getAddress() const -> std::string`  
";

// File: classhelics_1_1UdpCore.xml


%feature("docstring") helics::UdpCore "

implementation for the core that uses udp messages to communicate  

C++ includes: UdpCore.h
";

%feature("docstring") helics::UdpCore::UdpCore "
`UdpCore() noexcept`  

default constructor  
";

%feature("docstring") helics::UdpCore::UdpCore "
`UdpCore(const std::string &core_name)`  
";

%feature("docstring") helics::UdpCore::~UdpCore "
`~UdpCore()`  
";

%feature("docstring") helics::UdpCore::initializeFromArgs "
`initializeFromArgs(int argc, const char *const *argv) override`  

initialize the core manager with command line arguments  

Parameters
----------
* `argc` :  
    the number of arguments  
* `argv` :  
    char pointers to the arguments  
";

%feature("docstring") helics::UdpCore::getAddress "
`getAddress() const override -> std::string`  

get a string representing the connection info to send data to this object  
";

// File: classhelics_1_1ValueCapture.xml


%feature("docstring") helics::ValueCapture "

helper class for capturing data points  

C++ includes: recorder.h
";

%feature("docstring") helics::ValueCapture::ValueCapture "
`ValueCapture()=default`  
";

%feature("docstring") helics::ValueCapture::ValueCapture "
`ValueCapture(helics::Time t1, int id1, const std::string &val)`  
";

// File: classhelics_1_1ValueConverter.xml


%feature("docstring") helics::ValueConverter "

converter for a basic value  

C++ includes: ValueConverter.hpp
";

%feature("docstring") helics::ValueConverter::convert "
`convert(const X &val) -> data_block`  

convert the value to a block of data  

converter for a basic value  
";

%feature("docstring") helics::ValueConverter::convert "
`convert(const X &val, data_block &store)`  

convert the value and store to a specific block of data  
";

%feature("docstring") helics::ValueConverter::convert "
`convert(const X *vals, size_t size, data_block &store)`  

convert a raw vector of objects and store to a specific block  
";

%feature("docstring") helics::ValueConverter::convert "
`convert(const X *vals, size_t size) -> data_block`  

convert a raw vector of objects and store to a specific block  
";

%feature("docstring") helics::ValueConverter::interpret "
`interpret(const data_view &block) -> X`  

interpret a view of the data and convert back to a val  
";

%feature("docstring") helics::ValueConverter::interpret "
`interpret(const data_view &block, X &val)`  

interpret a view of the data block and store to the specified value  
";

%feature("docstring") helics::ValueConverter::type "
`type() -> std::string`  

get the type of the value  
";

// File: classhelics_1_1ValueConverter_3_01std_1_1string_01_4.xml


%feature("docstring") helics::ValueConverter< std::string > "

converter for a single string value  

C++ includes: ValueConverter.hpp
";

%feature("docstring") helics::ValueConverter< std::string >::convert "
`convert(std::string &&val) -> data_block`  
";

%feature("docstring") helics::ValueConverter< std::string >::convert "
`convert(const std::string &val) -> data_block`  
";

%feature("docstring") helics::ValueConverter< std::string >::convert "
`convert(const std::string &val, data_block &store)`  
";

%feature("docstring") helics::ValueConverter< std::string >::interpret "
`interpret(const data_view &block) -> std::string`  
";

%feature("docstring") helics::ValueConverter< std::string >::interpret "
`interpret(const data_view &block, std::string &val)`  
";

%feature("docstring") helics::ValueConverter< std::string >::type "
`type() -> std::string`  
";

// File: classhelics_1_1ValueFederate.xml


%feature("docstring") helics::ValueFederate "

class defining the value based interface  

C++ includes: ValueFederate.hpp
";

%feature("docstring") helics::ValueFederate::helics::FederateInfo "
`helics::FederateInfo -> friend class`  
";

%feature("docstring") helics::ValueFederate::ValueFederate "
`ValueFederate(const FederateInfo &fi)`  

constructor taking a federate information structure and using the default core  

Parameters
----------
* `fi` :  
    a federate information structure  

constructor taking a core engine and federate info structure  
";

%feature("docstring") helics::ValueFederate::ValueFederate "
`ValueFederate(std::shared_ptr< Core > core, const FederateInfo &fi)`  

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
`ValueFederate(const std::string &jsonString)`  

constructor taking a string with the required information  

Parameters
----------
* `jsonString` :  
    can be either a json file or a string containing json code  
";

%feature("docstring") helics::ValueFederate::ValueFederate "
`ValueFederate()`  

default constructor  
";

%feature("docstring") helics::ValueFederate::ValueFederate "
`ValueFederate(bool res)`  
";

%feature("docstring") helics::ValueFederate::ValueFederate "
`ValueFederate(const ValueFederate &fed)=delete`  
";

%feature("docstring") helics::ValueFederate::ValueFederate "
`ValueFederate(ValueFederate &&fed) noexcept`  
";

%feature("docstring") helics::ValueFederate::ValueFederate "
`ValueFederate(FederateInfo &fi)`  
";

%feature("docstring") helics::ValueFederate::ValueFederate "
`ValueFederate(const std::string &jsonString)`  
";

%feature("docstring") helics::ValueFederate::ValueFederate "
`ValueFederate()`  
";

%feature("docstring") helics::ValueFederate::~ValueFederate "
`~ValueFederate()`  

destructor  
";

%feature("docstring") helics::ValueFederate::~ValueFederate "
`~ValueFederate()`  
";

%feature("docstring") helics::ValueFederate::registerPublication "
`registerPublication(const std::string &key, const std::string &type, const
    std::string &units=std::string()) -> publication_id_t`  

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
`registerPublication(const std::string &key, const std::string
    &units=std::string()) -> publication_id_t`  

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
`registerPublication(const std::string &name, const std::string &type, const
    std::string &units=\"\") -> helics_publication`  

Methods to register publications  
";

%feature("docstring") helics::ValueFederate::registerGlobalPublication "
`registerGlobalPublication(const std::string &key, const std::string &type,
    const std::string &units=std::string()) -> publication_id_t`  

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
`registerGlobalPublication(const std::string &key, const std::string
    &units=std::string()) -> publication_id_t`  

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
`registerGlobalPublication(const std::string &name, const std::string &type,
    const std::string &units=\"\") -> helics_publication`  
";

%feature("docstring") helics::ValueFederate::registerPublicationIndexed "
`registerPublicationIndexed(const std::string &key, int index1, const
    std::string &units=std::string()) -> publication_id_t`  

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
`registerPublicationIndexed(const std::string &key, int index1, int index2,
    const std::string &units=std::string()) -> publication_id_t`  

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
`registerPublicationIndexed(const std::string &name, int index1, int type, const
    std::string &units=\"\") -> helics_publication`  
";

%feature("docstring") helics::ValueFederate::registerPublicationIndexed "
`registerPublicationIndexed(const std::string &name, int index1, int index2, int
    type, const std::string &units=\"\") -> helics_publication`  
";

%feature("docstring") helics::ValueFederate::registerRequiredSubscription "
`registerRequiredSubscription(const std::string &key, const std::string &type,
    const std::string &units=std::string()) -> subscription_id_t`  

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
`registerRequiredSubscription(const std::string &name, const std::string
    &units=std::string()) -> subscription_id_t`  

register a subscription  

call is only valid in startup mode  
";

%feature("docstring") helics::ValueFederate::registerRequiredSubscriptionIndexed "
`registerRequiredSubscriptionIndexed(const std::string &key, int index1, const
    std::string &units=std::string()) -> subscription_id_t`  

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
`registerRequiredSubscriptionIndexed(const std::string &key, int index1, int
    index2, const std::string &units=std::string()) -> subscription_id_t`  

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
`registerOptionalSubscription(const std::string &key, const std::string &type,
    const std::string &units=std::string()) -> subscription_id_t`  

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
`registerOptionalSubscription(const std::string &key, const std::string
    &units=std::string()) -> subscription_id_t`  

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
`registerOptionalSubscriptionIndexed(const std::string &key, int index1, const
    std::string &units=std::string()) -> subscription_id_t`  

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
`registerOptionalSubscriptionIndexed(const std::string &key, int index1, int
    index2, const std::string &units=std::string()) -> subscription_id_t`  

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
`addSubscriptionShortcut(subscription_id_t subid, const std::string
    &shortcutName)`  

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
`setDefaultValue(subscription_id_t id, data_view block)`  

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
`setDefaultValue(subscription_id_t id, const data_block &block)`  

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
`setDefaultValue(subscription_id_t id, const X &val)`  

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
`setDefaultValue(helics_subscription sub, const char *data, int len)`  

Methods to set default values for subscriptions  
";

%feature("docstring") helics::ValueFederate::setDefaultValue "
`setDefaultValue(helics_subscription sub, const std::string &str)`  
";

%feature("docstring") helics::ValueFederate::setDefaultValue "
`setDefaultValue(helics_subscription sub, int64_t val)`  
";

%feature("docstring") helics::ValueFederate::setDefaultValue "
`setDefaultValue(helics_subscription sub, double val)`  
";

%feature("docstring") helics::ValueFederate::setDefaultValue "
`setDefaultValue(helics_subscription sub, const std::complex< double > &cmplx)`  
";

%feature("docstring") helics::ValueFederate::setDefaultValue "
`setDefaultValue(helics_subscription sub, const std::vector< double > &data)`  
";

%feature("docstring") helics::ValueFederate::registerInterfaces "
`registerInterfaces(const std::string &jsonString) override`  

register a set of interfaces defined in a file  

call is only valid in startup mode  

Parameters
----------
* `jsonString` :  
    the location of the file to load to generate the interfaces  
";

%feature("docstring") helics::ValueFederate::getValueRaw "
`getValueRaw(subscription_id_t id) -> data_view`  

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
`getValue(subscription_id_t id, X &obj)`  

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
`getValue(subscription_id_t id) -> X`  

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
`getValue(helics_subscription sub, char *data, int maxlen) -> int`  

Methods to get subscription values  
";

%feature("docstring") helics::ValueFederate::publish "
`publish(publication_id_t id, data_view block)`  

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
`publish(publication_id_t id, const data_block &block)`  

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
`publish(publication_id_t id, const char *data)`  

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
`publish(publication_id_t id, const char *data, size_t data_size)`  

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
`publish(publication_id_t id, const X &value)`  

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
`publish(helics_publication pub, const char *data, int len)`  

Methods to publish values  
";

%feature("docstring") helics::ValueFederate::publish "
`publish(helics_publication pub, std::string str)`  
";

%feature("docstring") helics::ValueFederate::publish "
`publish(helics_publication pub, int64_t val)`  
";

%feature("docstring") helics::ValueFederate::publish "
`publish(helics_publication pub, double val)`  
";

%feature("docstring") helics::ValueFederate::publish "
`publish(helics_publication pub, std::complex< double > cmplx)`  
";

%feature("docstring") helics::ValueFederate::publish "
`publish(helics_publication pub, std::vector< double > data)`  
";

%feature("docstring") helics::ValueFederate::isUpdated "
`isUpdated(subscription_id_t sub_id) const -> bool`  

check if a given subscription has an update  

Returns
-------
true if the subscription id is valid and has an update  
";

%feature("docstring") helics::ValueFederate::isUpdated "
`isUpdated(helics_subscription sub) const -> bool`  

Check if a subscription is updated  
";

%feature("docstring") helics::ValueFederate::getLastUpdateTime "
`getLastUpdateTime(subscription_id_t sub_id) const -> Time`  

get the time of the last update  
";

%feature("docstring") helics::ValueFederate::getLastUpdateTime "
`getLastUpdateTime(helics_subscription sub) const -> helics_time_t`  

Get the last time a subscription was updated  
";

%feature("docstring") helics::ValueFederate::disconnect "
`disconnect() override`  

disconnect a simulation from the core (will also call finalize before
disconnecting if necessary)  
";

%feature("docstring") helics::ValueFederate::queryUpdates "
`queryUpdates() -> std::vector< subscription_id_t >`  

get a list of all the values that have been updated since the last call  

Returns
-------
a vector of subscription_ids with all the values that have not been retrieved
since updated  
";

%feature("docstring") helics::ValueFederate::queryUpdates "
`queryUpdates() -> std::vector< helics_subscription >`  

Get a list of all subscriptions with updates since the last call  
";

%feature("docstring") helics::ValueFederate::getSubscriptionKey "
`getSubscriptionKey(subscription_id_t) const -> std::string`  

get the key or the string identifier of a subscription from its id  

Returns
-------
empty string if an invalid id is passed  
";

%feature("docstring") helics::ValueFederate::getSubscriptionId "
`getSubscriptionId(const std::string &key) const -> subscription_id_t`  

get the id of a subscription  

Returns
-------
ivalid_subscription_id if name is not a recognized  
";

%feature("docstring") helics::ValueFederate::getSubscriptionId "
`getSubscriptionId(const std::string &key, int index1) const ->
    subscription_id_t`  

get the id of a subscription from a vector of subscriptions  

Returns
-------
ivalid_subscription_id if name is not a recognized  
";

%feature("docstring") helics::ValueFederate::getSubscriptionId "
`getSubscriptionId(const std::string &key, int index1, int index2) const ->
    subscription_id_t`  

get the id of a subscription from a 2-d vector of subscriptions  

Returns
-------
ivalid_subscription_id if name is not a recognized  
";

%feature("docstring") helics::ValueFederate::getPublicationKey "
`getPublicationKey(publication_id_t) const -> std::string`  

get the name of a publication from its id  

Returns
-------
empty string if an invalid id is passed  
";

%feature("docstring") helics::ValueFederate::getPublicationId "
`getPublicationId(const std::string &key) const -> publication_id_t`  

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
`getPublicationId(const std::string &key, int index1) const -> publication_id_t`  

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
`getPublicationId(const std::string &key, int index1, int index2) const ->
    publication_id_t`  

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
`getSubscriptionUnits(subscription_id_t id) const -> std::string`  

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
`getSubscriptionUnits(helics_subscription sub) const -> std::string`  
";

%feature("docstring") helics::ValueFederate::getPublicationUnits "
`getPublicationUnits(publication_id_t id) const -> std::string`  

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
`getPublicationUnits(helics_publication pub) const -> std::string`  
";

%feature("docstring") helics::ValueFederate::getSubscriptionType "
`getSubscriptionType(subscription_id_t id) const -> std::string`  

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
`getSubscriptionType(helics_subscription sub) const -> std::string`  
";

%feature("docstring") helics::ValueFederate::getPublicationType "
`getPublicationType(publication_id_t id) const -> std::string`  

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
`getPublicationType(subscription_id_t id) const -> std::string`  

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
`getPublicationType(helics_publication pub) const -> std::string`  
";

%feature("docstring") helics::ValueFederate::registerSubscriptionNotificationCallback "
`registerSubscriptionNotificationCallback(std::function< void(subscription_id_t,
    Time)> callback)`  

register a callback function to call when any subscribed value is updated  

there can only be one generic callback  

Parameters
----------
* `callback` :  
    the function to call signature void(subscription_id_t, Time)  
";

%feature("docstring") helics::ValueFederate::registerSubscriptionNotificationCallback "
`registerSubscriptionNotificationCallback(subscription_id_t id, std::function<
    void(subscription_id_t, Time)> callback)`  

register a callback function to call when the specified subscription is updated  

Parameters
----------
* `id` :  
    the id to register the callback for  
* `callback` :  
    the function to call  
";

%feature("docstring") helics::ValueFederate::registerSubscriptionNotificationCallback "
`registerSubscriptionNotificationCallback(const std::vector< subscription_id_t >
    &ids, std::function< void(subscription_id_t, Time)> callback)`  

register a callback function to call when the specified subscription is updated  

Parameters
----------
* `ids` :  
    the set of ids to register the callback for  
* `callback` :  
    the function to call  
";

%feature("docstring") helics::ValueFederate::getPublicationCount "
`getPublicationCount() const -> int`  

get a count of the number publications registered  
";

%feature("docstring") helics::ValueFederate::getSubscriptionCount "
`getSubscriptionCount() const -> int`  

get a count of the number subscriptions registered  
";

%feature("docstring") helics::ValueFederate::registerTypePublication "
`registerTypePublication(const std::string &name, int type, const std::string
    &units=\"\") -> helics_publication`  
";

%feature("docstring") helics::ValueFederate::registerGlobalTypePublication "
`registerGlobalTypePublication(const std::string &name, int type, const
    std::string &units=\"\") -> helics_publication`  
";

%feature("docstring") helics::ValueFederate::registerSubscription "
`registerSubscription(const std::string &name, const std::string &type, const
    std::string &units=\"\") -> helics_subscription`  

Methods to register subscriptions  
";

%feature("docstring") helics::ValueFederate::registerTypeSubscription "
`registerTypeSubscription(const std::string &name, int type, const std::string
    &units=\"\") -> helics_subscription`  
";

%feature("docstring") helics::ValueFederate::registerSubscriptionIndexed "
`registerSubscriptionIndexed(const std::string &name, int index1, int type,
    const std::string &units=\"\") -> helics_subscription`  
";

%feature("docstring") helics::ValueFederate::registerSubscriptionIndexed "
`registerSubscriptionIndexed(const std::string &name, int index1, int index2,
    int type, const std::string &units=\"\") -> helics_subscription`  
";

%feature("docstring") helics::ValueFederate::getString "
`getString(helics_subscription sub) -> std::string`  
";

%feature("docstring") helics::ValueFederate::getInteger "
`getInteger(helics_subscription sub) -> int64_t`  
";

%feature("docstring") helics::ValueFederate::getDouble "
`getDouble(helics_subscription sub) -> double`  
";

%feature("docstring") helics::ValueFederate::getComplex "
`getComplex(helics_subscription sub) -> std::complex< double >`  
";

%feature("docstring") helics::ValueFederate::getVector "
`getVector(helics_subscription sub, double *data, int maxlen) -> int`  
";

%feature("docstring") helics::ValueFederate::getSubscriptionName "
`getSubscriptionName(helics_subscription sub) const -> std::string`  
";

%feature("docstring") helics::ValueFederate::getPublicationName "
`getPublicationName(helics_publication pub) const -> std::string`  
";

// File: classhelics_1_1ValueFederateManager.xml


%feature("docstring") helics::ValueFederateManager "

class handling the implementation details of a value Federate  

C++ includes: ValueFederateManager.hpp
";

%feature("docstring") helics::ValueFederateManager::ValueFederateManager "
`ValueFederateManager(std::shared_ptr< Core > &coreOb, Core::federate_id_t id)`  
";

%feature("docstring") helics::ValueFederateManager::~ValueFederateManager "
`~ValueFederateManager()`  
";

%feature("docstring") helics::ValueFederateManager::registerPublication "
`registerPublication(const std::string &key, const std::string &type, const
    std::string &units) -> publication_id_t`  
";

%feature("docstring") helics::ValueFederateManager::registerRequiredSubscription "
`registerRequiredSubscription(const std::string &key, const std::string &type,
    const std::string &units) -> subscription_id_t`  

register a subscription  

call is only valid in startup mode  
";

%feature("docstring") helics::ValueFederateManager::registerOptionalSubscription "
`registerOptionalSubscription(const std::string &key, const std::string &type,
    const std::string &units) -> subscription_id_t`  

register a subscription  

call is only valid in startup mode  
";

%feature("docstring") helics::ValueFederateManager::addSubscriptionShortcut "
`addSubscriptionShortcut(subscription_id_t subid, const std::string
    &shortcutName)`  

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
`setDefaultValue(subscription_id_t id, data_view block)`  

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
`getValue(subscription_id_t id) -> data_view`  

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
`publish(publication_id_t id, data_view block)`  

publish a value  
";

%feature("docstring") helics::ValueFederateManager::queryUpdate "
`queryUpdate(subscription_id_t sub_id) const -> bool`  

check if a given subscription has and update  
";

%feature("docstring") helics::ValueFederateManager::queryLastUpdate "
`queryLastUpdate(subscription_id_t sub_id) const -> Time`  

get the time of the last update  
";

%feature("docstring") helics::ValueFederateManager::updateTime "
`updateTime(Time newTime, Time oldTime)`  

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
`startupToInitializeStateTransition()`  

transition from Startup To the Initialize State  
";

%feature("docstring") helics::ValueFederateManager::initializeToExecuteStateTransition "
`initializeToExecuteStateTransition()`  

transition from initialize to execution State  
";

%feature("docstring") helics::ValueFederateManager::queryUpdates "
`queryUpdates() -> std::vector< subscription_id_t >`  

get a list of all the values that have been updated since the last call  

Returns
-------
a vector of subscription_ids with all the values that have not been retrieved
since updated  
";

%feature("docstring") helics::ValueFederateManager::getSubscriptionKey "
`getSubscriptionKey(subscription_id_t id) const -> std::string`  

get the key of a subscription from its id  

Returns
-------
empty string if an invalid id is passed  
";

%feature("docstring") helics::ValueFederateManager::getSubscriptionId "
`getSubscriptionId(const std::string &name) const -> subscription_id_t`  

get the id of a subscription  

Returns
-------
ivalid_subscription_id if name is not a recognized  
";

%feature("docstring") helics::ValueFederateManager::getPublicationKey "
`getPublicationKey(publication_id_t id) const -> std::string`  

get the key of a publication from its id  

Returns
-------
empty string if an invalid id is passed  
";

%feature("docstring") helics::ValueFederateManager::getPublicationId "
`getPublicationId(const std::string &key) const -> publication_id_t`  

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
`getSubscriptionUnits(subscription_id_t id) const -> std::string`  

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
`getPublicationUnits(publication_id_t id) const -> std::string`  

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
`getSubscriptionType(subscription_id_t id) const -> std::string`  

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
`getPublicationType(publication_id_t id) const -> std::string`  

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
`getPublicationType(subscription_id_t id) const -> std::string`  

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
`registerCallback(std::function< void(subscription_id_t, Time)> callback)`  

register a callback function to call when any subscribed value is updated  

there can only be one generic callback  

Parameters
----------
* `callback` :  
    the function to call  
";

%feature("docstring") helics::ValueFederateManager::registerCallback "
`registerCallback(subscription_id_t id, std::function< void(subscription_id_t,
    Time)> callback)`  

register a callback function to call when the specified subscription is updated  

Parameters
----------
* `id` :  
    the id to register the callback for  
* `callback` :  
    the function to call  
";

%feature("docstring") helics::ValueFederateManager::registerCallback "
`registerCallback(const std::vector< subscription_id_t > &ids, std::function<
    void(subscription_id_t, Time)> callback)`  

register a callback function to call when the specified subscription is updated  

Parameters
----------
* `ids` :  
    the set of ids to register the callback for  
* `callback` :  
    the function to call  
";

%feature("docstring") helics::ValueFederateManager::disconnect "
`disconnect()`  

disconnect from the coreObject  
";

%feature("docstring") helics::ValueFederateManager::getPublicationCount "
`getPublicationCount() const -> int`  

get a count of the number publications registered  
";

%feature("docstring") helics::ValueFederateManager::getSubscriptionCount "
`getSubscriptionCount() const -> int`  

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
`VectorSubscription() noexcept`  
";

%feature("docstring") helics::VectorSubscription::VectorSubscription "
`VectorSubscription(bool required, ValueFederate *valueFed, std::string name,
    int startIndex, int count, const X &defValue, const std::string
    &units=\"\")`  

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
`VectorSubscription(ValueFederate *valueFed, std::string name, int startIndex,
    int count, const X &defValue, std::string units=\"\")`  

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
`VectorSubscription(VectorSubscription &&vs) noexcept`  

move constructor  
";

%feature("docstring") helics::VectorSubscription::getVals "
`getVals() const -> const std::vector< X > &`  

get the most recent value  

Returns
-------
the value  
";

%feature("docstring") helics::VectorSubscription::registerCallback "
`registerCallback(std::function< void(int, Time)> callback)`  

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
`VectorSubscription2d() noexcept`  
";

%feature("docstring") helics::VectorSubscription2d::VectorSubscription2d "
`VectorSubscription2d(bool required, ValueFederate *valueFed, std::string name,
    int startIndex_x, int count_x, int startIndex_y, int count_y, const X
    &defValue, const std::string &units=\"\")`  

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
`VectorSubscription2d(ValueFederate *valueFed, const std::string &name, int
    startIndex_x, int count_x, int startIndex_y, int count_y, const X &defValue,
    const std::string &units=\"\")`  

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
`VectorSubscription2d(VectorSubscription2d &&vs) noexcept`  

move constructor  
";

%feature("docstring") helics::VectorSubscription2d::getVals "
`getVals() const -> const std::vector< X > &`  

get the most recent value  

Returns
-------
the value  
";

%feature("docstring") helics::VectorSubscription2d::at "
`at(int index_x, int index_y) const -> const X &`  

get the value in the given variable  

Parameters
----------
* `out` :  
    the location to store the value  
";

%feature("docstring") helics::VectorSubscription2d::registerCallback "
`registerCallback(std::function< void(int, Time)> callback)`  

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
`ZmqBroker(bool rootBroker=false) noexcept`  

default constructor  
";

%feature("docstring") helics::ZmqBroker::ZmqBroker "
`ZmqBroker(const std::string &broker_name)`  
";

%feature("docstring") helics::ZmqBroker::initializeFromArgs "
`initializeFromArgs(int argc, const char *const *argv) override`  

initialize from command line arguments  
";

%feature("docstring") helics::ZmqBroker::~ZmqBroker "
`~ZmqBroker()`  

destructor  
";

%feature("docstring") helics::ZmqBroker::getAddress "
`getAddress() const override -> std::string`  

get the connection address for the broker  
";

%feature("docstring") helics::ZmqBroker::displayHelp "
`displayHelp(bool local_only=false)`  
";

// File: classhelics_1_1ZmqComms.xml


%feature("docstring") helics::ZmqComms "

implementation for the communication interface that uses ZMQ messages to
communicate  

C++ includes: ZmqComms.h
";

%feature("docstring") helics::ZmqComms::ZmqComms "
`ZmqComms()=default`  

default constructor  
";

%feature("docstring") helics::ZmqComms::ZmqComms "
`ZmqComms(const std::string &brokerTarget, const std::string &localTarget)`  
";

%feature("docstring") helics::ZmqComms::ZmqComms "
`ZmqComms(const NetworkBrokerData &netInfo)`  
";

%feature("docstring") helics::ZmqComms::~ZmqComms "
`~ZmqComms()`  

destructor  
";

%feature("docstring") helics::ZmqComms::setBrokerPort "
`setBrokerPort(int brokerPort)`  

set the port numbers for the local ports  
";

%feature("docstring") helics::ZmqComms::setPortNumber "
`setPortNumber(int portNumber)`  
";

%feature("docstring") helics::ZmqComms::setAutomaticPortStartPort "
`setAutomaticPortStartPort(int startingPort)`  
";

%feature("docstring") helics::ZmqComms::getPort "
`getPort() const -> int`  

get the port number of the comms object to push message to  
";

%feature("docstring") helics::ZmqComms::getAddress "
`getAddress() const -> std::string`  
";

%feature("docstring") helics::ZmqComms::getPushAddress "
`getPushAddress() const -> std::string`  
";

// File: classzmqContextManager.xml


%feature("docstring") zmqContextManager "

class defining a singleton context manager for all zmq usage in gridDyn  

C++ includes: zmqContextManager.h
";

%feature("docstring") zmqContextManager::getContextPointer "
`getContextPointer(const std::string &contextName=\"\") -> std::shared_ptr<
    zmqContextManager >`  
";

%feature("docstring") zmqContextManager::getContext "
`getContext(const std::string &contextName=\"\") -> zmq::context_t &`  
";

%feature("docstring") zmqContextManager::closeContext "
`closeContext(const std::string &contextName=\"\")`  
";

%feature("docstring") zmqContextManager::setContextToLeakOnDelete "
`setContextToLeakOnDelete(const std::string &contextName=\"\") -> bool`  

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
`~zmqContextManager()`  
";

%feature("docstring") zmqContextManager::getName "
`getName() const -> const std::string &`  
";

%feature("docstring") zmqContextManager::getBaseContext "
`getBaseContext() const -> zmq::context_t &`  
";

// File: classhelics_1_1ZmqCore.xml


%feature("docstring") helics::ZmqCore "

implementation for the core that uses zmq messages to communicate  

C++ includes: ZmqCore.h
";

%feature("docstring") helics::ZmqCore::ZmqCore "
`ZmqCore() noexcept`  

default constructor  
";

%feature("docstring") helics::ZmqCore::ZmqCore "
`ZmqCore(const std::string &core_name)`  

construct from with a core name  
";

%feature("docstring") helics::ZmqCore::~ZmqCore "
`~ZmqCore()`  

destructor  
";

%feature("docstring") helics::ZmqCore::initializeFromArgs "
`initializeFromArgs(int argc, const char *const *argv) override`  

initialize the core manager with command line arguments  

Parameters
----------
* `argc` :  
    the number of arguments  
* `argv` :  
    char pointers to the arguments  
";

%feature("docstring") helics::ZmqCore::getAddress "
`getAddress() const override -> std::string`  

get a string representing the connection info to send data to this object  
";

// File: classzmqProxyHub.xml


%feature("docstring") zmqProxyHub "

class building and managing a zmq proxy  

the proxy runs in its own thread managed by the proxy class  

C++ includes: zmqProxyHub.h
";

%feature("docstring") zmqProxyHub::getProxy "
`getProxy(const std::string &proxyName, const std::string &pairType=\"pubsub\",
    const std::string &contextName=\"\") -> std::shared_ptr< zmqProxyHub >`  
";

%feature("docstring") zmqProxyHub::~zmqProxyHub "
`~zmqProxyHub()`  
";

%feature("docstring") zmqProxyHub::startProxy "
`startProxy()`  
";

%feature("docstring") zmqProxyHub::stopProxy "
`stopProxy()`  
";

%feature("docstring") zmqProxyHub::modifyIncomingConnection "
`modifyIncomingConnection(socket_ops op, const std::string &connection)`  
";

%feature("docstring") zmqProxyHub::modifyOutgoingConnection "
`modifyOutgoingConnection(socket_ops op, const std::string &connection)`  
";

%feature("docstring") zmqProxyHub::getName "
`getName() const -> const std::string &`  
";

%feature("docstring") zmqProxyHub::getIncomingConnection "
`getIncomingConnection() const -> const std::string &`  
";

%feature("docstring") zmqProxyHub::getOutgoingConnection "
`getOutgoingConnection() const -> const std::string &`  
";

%feature("docstring") zmqProxyHub::isRunning "
`isRunning() const -> bool`  
";

// File: classzmqReactor.xml


%feature("docstring") zmqReactor "

class that manages receive sockets and triggers callbacks  the class starts up a
thread that listens for  

C++ includes: zmqReactor.h
";

%feature("docstring") zmqReactor::getReactorInstance "
`getReactorInstance(const std::string &reactorName, const std::string
    &contextName=\"\") -> std::shared_ptr< zmqReactor >`  
";

%feature("docstring") zmqReactor::~zmqReactor "
`~zmqReactor()`  
";

%feature("docstring") zmqReactor::addSocket "
`addSocket(const zmqSocketDescriptor &desc)`  
";

%feature("docstring") zmqReactor::modifySocket "
`modifySocket(const zmqSocketDescriptor &desc)`  
";

%feature("docstring") zmqReactor::closeSocket "
`closeSocket(const std::string &socketName)`  

asyncrhonous call to close a specific socket  

Parameters
----------
* `socketName` :  
    the name of the socket to close  
";

%feature("docstring") zmqReactor::addSocketBlocking "
`addSocketBlocking(const zmqSocketDescriptor &desc)`  
";

%feature("docstring") zmqReactor::modifySocketBlocking "
`modifySocketBlocking(const zmqSocketDescriptor &desc)`  
";

%feature("docstring") zmqReactor::closeSocketBlocking "
`closeSocketBlocking(const std::string &socketName)`  
";

%feature("docstring") zmqReactor::getName "
`getName() const -> const std::string &`  
";

%feature("docstring") zmqReactor::terminateReactor "
`terminateReactor()`  
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
`ZmqRequestSets()`  

constructor  
";

%feature("docstring") helics::ZmqRequestSets::addRoutes "
`addRoutes(int routeNumber, const std::string &routeInfo)`  

add a route to the request set  
";

%feature("docstring") helics::ZmqRequestSets::transmit "
`transmit(int routeNumber, const ActionMessage &command) -> bool`  

transmit a command to a specific route number  
";

%feature("docstring") helics::ZmqRequestSets::waiting "
`waiting() const -> bool`  

check if the request set is waiting on any on responses  
";

%feature("docstring") helics::ZmqRequestSets::checkForMessages "
`checkForMessages() -> int`  

check for messages with a 0 second timeout  

Returns
-------
the number of message waiting to be received  
";

%feature("docstring") helics::ZmqRequestSets::checkForMessages "
`checkForMessages(std::chrono::milliseconds timeout) -> int`  

check for messages with an explicit timeout  

Returns
-------
the number of message waiting to be received  
";

%feature("docstring") helics::ZmqRequestSets::hasMessages "
`hasMessages() const -> bool`  

check if there are any waiting message without scanning the sockets  
";

%feature("docstring") helics::ZmqRequestSets::getMessage "
`getMessage() -> stx::optional< ActionMessage >`  

get any messages that have been received  
";

%feature("docstring") helics::ZmqRequestSets::close "
`close()`  

close all the sockets  
";

// File: classzmqSocketDescriptor.xml


%feature("docstring") zmqSocketDescriptor "

data class describing a socket and some operations on it  

C++ includes: zmqSocketDescriptor.h
";

%feature("docstring") zmqSocketDescriptor::zmqSocketDescriptor "
`zmqSocketDescriptor(const std::string &socketName=\"\")`  
";

%feature("docstring") zmqSocketDescriptor::zmqSocketDescriptor "
`zmqSocketDescriptor(const std::string &socketName, zmq::socket_type stype)`  
";

%feature("docstring") zmqSocketDescriptor::addOperation "
`addOperation(socket_ops op, const std::string &desc)`  
";

%feature("docstring") zmqSocketDescriptor::makeSocket "
`makeSocket(zmq::context_t &ctx) const -> zmq::socket_t`  
";

%feature("docstring") zmqSocketDescriptor::makeSocketPtr "
`makeSocketPtr(zmq::context_t &ctx) const -> std::unique_ptr< zmq::socket_t >`  
";

%feature("docstring") zmqSocketDescriptor::modifySocket "
`modifySocket(zmq::socket_t &sock) const`  
";

// File: namespaceboost.xml

// File: namespaceboost_1_1asio.xml

// File: namespaceboost_1_1program__options.xml

// File: namespaceboost_1_1system.xml

// File: namespacehelics.xml

%feature("docstring") helics::action_message_def::cleanupHelicsLibrary "
`cleanupHelicsLibrary()`  

function to do some housekeeping work  

this runs some cleanup routines and tries to close out any residual thread that
haven't been shutdown yet  
";

%feature("docstring") helics::action_message_def::LoadFederateInfo "
`LoadFederateInfo(const std::string &jsonString) -> FederateInfo`  

generate a FederateInfo object from a json file  
";

%feature("docstring") helics::action_message_def::randDouble "
`randDouble(random_dists_t dist, double p1, double p2) -> double`  
";

%feature("docstring") helics::action_message_def::addOperations "
`addOperations(Filter *filt, defined_filter_types type, Core *cptr)`  
";

%feature("docstring") helics::action_message_def::make_destination_filter "
`make_destination_filter(defined_filter_types type, Federate *mFed, const
    std::string &target, const std::string &name) -> std::unique_ptr<
    DestinationFilter >`  

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
`make_destination_filter(defined_filter_types type, Core *cr, const std::string
    &target, const std::string &name) -> std::unique_ptr< DestinationFilter >`  

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
`make_source_filter(defined_filter_types type, Core *cr, const std::string
    &target, const std::string &name) -> std::unique_ptr< SourceFilter >`  

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
`make_source_filter(defined_filter_types type, Federate *fed, const std::string
    &target, const std::string &name) -> std::unique_ptr< SourceFilter >`  

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
`changeDetected(const defV &prevValue, const std::string &val, double) -> bool`  
";

%feature("docstring") helics::action_message_def::changeDetected "
`changeDetected(const defV &prevValue, const std::vector< double > &val, double
    deltaV) -> bool`  
";

%feature("docstring") helics::action_message_def::changeDetected "
`changeDetected(const defV &prevValue, const std::vector< std::complex< double
    >> &val, double deltaV) -> bool`  
";

%feature("docstring") helics::action_message_def::changeDetected "
`changeDetected(const defV &prevValue, const double *vals, size_t size, double
    deltaV) -> bool`  
";

%feature("docstring") helics::action_message_def::changeDetected "
`changeDetected(const defV &prevValue, const std::complex< double > &val, double
    deltaV) -> bool`  
";

%feature("docstring") helics::action_message_def::changeDetected "
`changeDetected(const defV &prevValue, double val, double deltaV) -> bool`  
";

%feature("docstring") helics::action_message_def::changeDetected "
`changeDetected(const defV &prevValue, int64_t val, double deltaV) -> bool`  
";

%feature("docstring") helics::action_message_def::valueExtract "
`valueExtract(const defV &dv, std::string &val)`  
";

%feature("docstring") helics::action_message_def::valueExtract "
`valueExtract(const defV &dv, std::complex< double > &val)`  
";

%feature("docstring") helics::action_message_def::valueExtract "
`valueExtract(const defV &dv, std::vector< double > &val)`  
";

%feature("docstring") helics::action_message_def::valueExtract "
`valueExtract(const defV &dv, std::vector< std::complex< double >> &val)`  
";

%feature("docstring") helics::action_message_def::valueExtract "
`valueExtract(const data_view &dv, helics_type_t baseType, std::string &val)`  
";

%feature("docstring") helics::action_message_def::valueExtract "
`valueExtract(const data_view &dv, helics_type_t baseType, std::vector< double >
    &val)`  
";

%feature("docstring") helics::action_message_def::valueExtract "
`valueExtract(const data_view &dv, helics_type_t baseType, std::vector<
    std::complex< double >> &val)`  
";

%feature("docstring") helics::action_message_def::valueExtract "
`valueExtract(const data_view &dv, helics_type_t baseType, std::complex< double
    > &val)`  
";

%feature("docstring") helics::action_message_def::valueExtract "
`valueExtract(const defV &dv, X &val) -> std::enable_if_t< std::is_arithmetic< X
    >::value >`  

for numeric types  
";

%feature("docstring") helics::action_message_def::valueExtract "
`valueExtract(const data_view &dv, helics_type_t baseType, X &val) ->
    std::enable_if_t< std::is_arithmetic< X >::value >`  

assume it is some numeric type (int or double)  
";

%feature("docstring") helics::action_message_def::doubleString "
`doubleString(\"double\") -> const std::string`  
";

%feature("docstring") helics::action_message_def::intString "
`intString(\"int64\") -> const std::string`  
";

%feature("docstring") helics::action_message_def::stringString "
`stringString(\"string\") -> const std::string`  
";

%feature("docstring") helics::action_message_def::complexString "
`complexString(\"complex\") -> const std::string`  
";

%feature("docstring") helics::action_message_def::doubleVecString "
`doubleVecString(\"double_vector\") -> const std::string`  
";

%feature("docstring") helics::action_message_def::complexVecString "
`complexVecString(\"complex_vector\") -> const std::string`  
";

%feature("docstring") helics::action_message_def::typeNameStringRef "
`typeNameStringRef(helics_type_t type) -> const std::string &`  

sometime we just need a ref to a string for the basic types  
";

%feature("docstring") helics::action_message_def::helicsComplexString "
`helicsComplexString(double real, double imag) -> std::string`  
";

%feature("docstring") helics::action_message_def::helicsComplexString "
`helicsComplexString(std::complex< double > val) -> std::string`  
";

%feature("docstring") helics::action_message_def::getTypeFromString "
`getTypeFromString(const std::string &typeName) -> helics_type_t`  

convert a string to a type  
";

%feature("docstring") helics::action_message_def::creg "
`creg(R\"(([+-]?(\\(\\+)?|\\+)([eE][+-]?\\)?)\\([+-]\\(\\(\\+)?|\\+)([eE][+-]?\\
    )?)[ji]*)\") -> const std::regex`  
";

%feature("docstring") helics::action_message_def::helicsGetComplex "
`helicsGetComplex(const std::string &val) -> std::complex< double >`  

convert a string to a complex number  
";

%feature("docstring") helics::action_message_def::helicsVectorString "
`helicsVectorString(const std::vector< double > &val) -> std::string`  
";

%feature("docstring") helics::action_message_def::helicsVectorString "
`helicsVectorString(const double *vals, size_t size) -> std::string`  
";

%feature("docstring") helics::action_message_def::helicsComplexVectorString "
`helicsComplexVectorString(const std::vector< std::complex< double >> &val) ->
    std::string`  
";

%feature("docstring") helics::action_message_def::helicsGetVector "
`helicsGetVector(const std::string &val) -> std::vector< double >`  

convert a string to a vector  
";

%feature("docstring") helics::action_message_def::helicsGetVector "
`helicsGetVector(const std::string &val, std::vector< double > &data)`  
";

%feature("docstring") helics::action_message_def::helicsGetComplexVector "
`helicsGetComplexVector(const std::string &val) -> std::vector< std::complex<
    double > >`  

convert a string to a complex vector  
";

%feature("docstring") helics::action_message_def::helicsGetComplexVector "
`helicsGetComplexVector(const std::string &val, std::vector< std::complex<
    double >> &data)`  
";

%feature("docstring") helics::action_message_def::readSize "
`readSize(const std::string &val) -> auto`  
";

%feature("docstring") helics::action_message_def::typeConvert "
`typeConvert(helics_type_t type, double val) -> data_block`  
";

%feature("docstring") helics::action_message_def::typeConvert "
`typeConvert(helics_type_t type, int64_t val) -> data_block`  
";

%feature("docstring") helics::action_message_def::typeConvert "
`typeConvert(helics_type_t type, const char *val) -> data_block`  
";

%feature("docstring") helics::action_message_def::typeConvert "
`typeConvert(helics_type_t type, const std::string &val) -> data_block`  
";

%feature("docstring") helics::action_message_def::typeConvert "
`typeConvert(helics_type_t type, const std::vector< double > &val) ->
    data_block`  
";

%feature("docstring") helics::action_message_def::typeConvert "
`typeConvert(helics_type_t type, const double *vals, size_t size) -> data_block`  
";

%feature("docstring") helics::action_message_def::typeConvert "
`typeConvert(helics_type_t type, const std::vector< std::complex< double >>
    &val) -> data_block`  
";

%feature("docstring") helics::action_message_def::typeConvert "
`typeConvert(helics_type_t type, const std::complex< double > &val) ->
    data_block`  
";

%feature("docstring") helics::action_message_def::typeConvert "
`typeConvert(helics_type_t type, const defV &val) -> data_block`  
";

%feature("docstring") helics::action_message_def::typeNameString "
`typeNameString() -> std::string`  

template class for generating a known name of a type  
";

%feature("docstring") helics::action_message_def::typeNameString< std::vector< std::string > > "
`typeNameString< std::vector< std::string > >() -> std::string`  
";

%feature("docstring") helics::action_message_def::typeNameString< std::vector< double > > "
`typeNameString< std::vector< double > >() -> std::string`  
";

%feature("docstring") helics::action_message_def::typeNameString< std::vector< std::complex< double > > > "
`typeNameString< std::vector< std::complex< double > > >() -> std::string`  
";

%feature("docstring") helics::action_message_def::typeNameString< double > "
`typeNameString< double >() -> std::string`  

for float  
";

%feature("docstring") helics::action_message_def::typeNameString< float > "
`typeNameString< float >() -> std::string`  

for float  
";

%feature("docstring") helics::action_message_def::typeNameString< char > "
`typeNameString< char >() -> std::string`  

for character  
";

%feature("docstring") helics::action_message_def::typeNameString< unsigned char > "
`typeNameString< unsigned char >() -> std::string`  

for unsigned character  
";

%feature("docstring") helics::action_message_def::typeNameString< std::int32_t > "
`typeNameString< std::int32_t >() -> std::string`  

for integer  
";

%feature("docstring") helics::action_message_def::typeNameString< std::uint32_t > "
`typeNameString< std::uint32_t >() -> std::string`  

for unsigned integer  
";

%feature("docstring") helics::action_message_def::typeNameString< int64_t > "
`typeNameString< int64_t >() -> std::string`  

for 64 bit unsigned integer  
";

%feature("docstring") helics::action_message_def::typeNameString< std::uint64_t > "
`typeNameString< std::uint64_t >() -> std::string`  

for 64 bit unsigned integer  
";

%feature("docstring") helics::action_message_def::typeNameString< std::complex< float > > "
`typeNameString< std::complex< float > >() -> std::string`  

for complex double  
";

%feature("docstring") helics::action_message_def::typeNameString< std::complex< double > > "
`typeNameString< std::complex< double > >() -> std::string`  

for complex double  
";

%feature("docstring") helics::action_message_def::typeNameString< std::string > "
`typeNameString< std::string >() -> std::string`  
";

%feature("docstring") helics::action_message_def::helicsType "
`helicsType() -> constexpr helics_type_t`  

template class for generating a known name of a type  
";

%feature("docstring") helics::action_message_def::helicsType< int64_t > "
`helicsType< int64_t >() -> constexpr helics_type_t`  
";

%feature("docstring") helics::action_message_def::helicsType< std::string > "
`helicsType< std::string >() -> constexpr helics_type_t`  
";

%feature("docstring") helics::action_message_def::helicsType< double > "
`helicsType< double >() -> constexpr helics_type_t`  
";

%feature("docstring") helics::action_message_def::helicsType< std::complex< double > > "
`helicsType< std::complex< double > >() -> constexpr helics_type_t`  
";

%feature("docstring") helics::action_message_def::helicsType< std::vector< double > > "
`helicsType< std::vector< double > >() -> constexpr helics_type_t`  
";

%feature("docstring") helics::action_message_def::helicsType< std::vector< std::complex< double > > > "
`helicsType< std::vector< std::complex< double > > >() -> constexpr
    helics_type_t`  
";

%feature("docstring") helics::action_message_def::isConvertableType "
`isConvertableType() -> constexpr bool`  
";

%feature("docstring") helics::action_message_def::isConvertableType "
`isConvertableType() -> constexpr std::enable_if< helicsType< X >)
    !=helics_type_t::helicsInvalid, bool >::type`  
";

%feature("docstring") helics::action_message_def::isConvertableType< float > "
`isConvertableType< float >() -> constexpr bool`  
";

%feature("docstring") helics::action_message_def::isConvertableType< long double > "
`isConvertableType< long double >() -> constexpr bool`  
";

%feature("docstring") helics::action_message_def::isConvertableType< int > "
`isConvertableType< int >() -> constexpr bool`  
";

%feature("docstring") helics::action_message_def::isConvertableType< short > "
`isConvertableType< short >() -> constexpr bool`  
";

%feature("docstring") helics::action_message_def::isConvertableType< unsigned int > "
`isConvertableType< unsigned int >() -> constexpr bool`  
";

%feature("docstring") helics::action_message_def::isConvertableType< char > "
`isConvertableType< char >() -> constexpr bool`  
";

%feature("docstring") helics::action_message_def::isConvertableType< uint64_t > "
`isConvertableType< uint64_t >() -> constexpr bool`  
";

%feature("docstring") helics::action_message_def::invalidValue "
`invalidValue() -> X`  

generate an invalid value for the various types  
";

%feature("docstring") helics::action_message_def::invalidValue< double > "
`invalidValue< double >() -> constexpr double`  
";

%feature("docstring") helics::action_message_def::invalidValue< uint64_t > "
`invalidValue< uint64_t >() -> constexpr uint64_t`  
";

%feature("docstring") helics::action_message_def::invalidValue< std::complex< double > > "
`invalidValue< std::complex< double > >() -> constexpr std::complex< double >`  
";

%feature("docstring") helics::action_message_def::typeNameString< std::vector< data_block > > "
`typeNameString< std::vector< data_block > >() -> std::string`  
";

%feature("docstring") helics::action_message_def::make_publication "
`make_publication(ValueFederate *valueFed, const std::string &name, const
    std::string &units=\"\") -> std::enable_if_t< helicsType< X >)
    !=helics_type_t::helicsInvalid, std::unique_ptr< Publication > >`  
";

%feature("docstring") helics::action_message_def::make_publication "
`make_publication(interface_visibility locality, ValueFederate *valueFed, const
    std::string &name, const std::string &units=\"\") -> std::enable_if_t<
    helicsType< X >) !=helics_type_t::helicsInvalid, std::unique_ptr<
    Publication > >`  
";

%feature("docstring") helics::action_message_def::save "
`save(Archive &ar, const data_block &db)`  
";

%feature("docstring") helics::action_message_def::save "
`save(Archive &ar, const data_view &db)`  
";

%feature("docstring") helics::action_message_def::load "
`load(Archive &ar, data_block &db)`  
";

%feature("docstring") helics::action_message_def::load "
`load(Archive &ar, data_view &db)`  
";

%feature("docstring") helics::action_message_def::getMinSize "
`getMinSize() -> constexpr std::enable_if_t<!is_iterable< X >::value
    &&!std::is_convertible< X, std::string >::value, size_t >`  
";

%feature("docstring") helics::action_message_def::getMinSize "
`getMinSize() -> constexpr std::enable_if_t< is_iterable< X >::value
    &&!std::is_convertible< X, std::string >::value, size_t >`  
";

%feature("docstring") helics::action_message_def::getMinSize "
`getMinSize() -> constexpr std::enable_if_t< std::is_convertible< X, std::string
    >::value, size_t >`  
";

%feature("docstring") helics::action_message_def::publish "
`publish(std::shared_ptr< ValueFederate > &fed, const std::string &pubKey, Us...
    pargs)`  

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
`publish(ValueFederate &fed, const std::string &pubKey, Us... pargs)`  

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
`getValue(std::shared_ptr< ValueFederate > &fed, const std::string &Key) -> X`  

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
`getValue(std::shared_ptr< ValueFederate > &fed, const std::string &Key, X
    &obj)`  

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
`getValue(ValueFederate &fed, const std::string &Key) -> X`  

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
`getValue(ValueFederate &fed, const std::string &Key, X &obj)`  

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
`getTypeSize(const std::string type) -> int`  
";

%feature("docstring") helics::action_message_def::isBlockSizeValid "
`isBlockSizeValid(int size, const publication_info &pubI) -> bool`  

function to check if the size is valid for the given type  
";

%feature("docstring") helics::action_message_def::vComp "
`vComp(const ValueSetter &v1, const ValueSetter &v2) -> bool`  
";

%feature("docstring") helics::action_message_def::mComp "
`mComp(const MessageHolder &m1, const MessageHolder &m2) -> bool`  
";

%feature("docstring") helics::action_message_def::argumentParser "
`argumentParser(int argc, const char *const *argv,
    boost::program_options::variables_map &vm_map, const ArgDescriptors
    &additionalArgs)`  
";

%feature("docstring") helics::action_message_def::createMessage "
`createMessage(const ActionMessage &cmd) -> std::unique_ptr< Message >`  

create a new message object that copies all the information from the
ActionMessage into newly allocated memory for the message  
";

%feature("docstring") helics::action_message_def::createMessage "
`createMessage(ActionMessage &&cmd) -> std::unique_ptr< Message >`  

create a new message object that moves all the information from the
ActionMessage into newly allocated memory for the message  
";

%feature("docstring") helics::action_message_def::actionMessageType "
`actionMessageType(action_message_def::action_t action) -> const char *`  

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
`prettyPrintString(const ActionMessage &command) -> std::string`  

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
`isProtocolCommand(const ActionMessage &command) noexcept -> bool`  

check if a command is a protocol command  
";

%feature("docstring") helics::action_message_def::isPriorityCommand "
`isPriorityCommand(const ActionMessage &command) noexcept -> bool`  

check if a command is a priority command  
";

%feature("docstring") helics::action_message_def::isTimingMessage "
`isTimingMessage(const ActionMessage &command) noexcept -> bool`  
";

%feature("docstring") helics::action_message_def::isDependencyMessage "
`isDependencyMessage(const ActionMessage &command) noexcept -> bool`  
";

%feature("docstring") helics::action_message_def::isDisconnectCommand "
`isDisconnectCommand(const ActionMessage &command) noexcept -> bool`  

check if a command is a disconnect command  
";

%feature("docstring") helics::action_message_def::isValidCommand "
`isValidCommand(const ActionMessage &command) noexcept -> bool`  

check if a command is a priority command  
";

%feature("docstring") helics::action_message_def::hasInfo "
`hasInfo(action_message_def::action_t action) -> bool`  

check if the action has an info structure associated with it  
";

%feature("docstring") helics::action_message_def::timerTickHandler "
`timerTickHandler(BrokerBase *bbase, activeProtector active, const
    boost::system::error_code &error)`  
";

%feature("docstring") helics::action_message_def::makeBroker "
`makeBroker(core_type type, const std::string &name) -> std::shared_ptr< Broker
    >`  
";

%feature("docstring") helics::action_message_def::unknownString "
`unknownString(\"#unknown\") -> const std::string`  
";

%feature("docstring") helics::action_message_def::matchingTypes "
`matchingTypes(const std::string &type1, const std::string &type2) -> bool`  
";

%feature("docstring") helics::action_message_def::isValidIndex "
`isValidIndex(sizeType testSize, const std::vector< dataType > &vec) -> bool`  

helper template to check whether an index is actually valid for a particular
vector  
";

%feature("docstring") helics::action_message_def::helicsTypeString "
`helicsTypeString(core_type type) -> std::string`  

generate a string based on the core type  
";

%feature("docstring") helics::action_message_def::coreTypeFromString "
`coreTypeFromString(std::string type) -> core_type`  

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
`isCoreTypeAvailable(core_type type) noexcept -> bool`  

Returns true if core/broker type specified is available in current compilation.  
";

%feature("docstring") helics::action_message_def::makeGlobalHandleIdentifier "
`makeGlobalHandleIdentifier(Core::federate_id_t fed_id, Core::handle_id_t
    handle) -> uint64_t`  
";

%feature("docstring") helics::action_message_def::makeCore "
`makeCore(core_type type, const std::string &name) -> std::shared_ptr< Core >`  
";

%feature("docstring") helics::action_message_def::helicsVersionString "
`helicsVersionString() -> std::string`  

Returns
-------
a string containing version information  
";

%feature("docstring") helics::action_message_def::helicsVersionMajor "
`helicsVersionMajor() -> int`  

get the Major version number  
";

%feature("docstring") helics::action_message_def::helicsVersionMinor "
`helicsVersionMinor() -> int`  

get the Minor version number  
";

%feature("docstring") helics::action_message_def::helicsVersionPatch "
`helicsVersionPatch() -> int`  

get the patch number  
";

%feature("docstring") helics::action_message_def::stringTranslateToCppName "
`stringTranslateToCppName(std::string in) -> std::string`  

translate a string to a C++ qualified name for variable naming purposes  
";

%feature("docstring") helics::action_message_def::makePortAddress "
`makePortAddress(const std::string &networkInterface, int portNumber) ->
    std::string`  

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
`extractInterfaceandPort(const std::string &address) -> std::pair< std::string,
    int >`  

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
`extractInterfaceandPortString(const std::string &address) -> std::pair<
    std::string, std::string >`  

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
`getLocalExternalAddressV4() -> std::string`  

get the external ipv4 address of the current computer  
";

%feature("docstring") helics::action_message_def::getLocalExternalAddressV4 "
`getLocalExternalAddressV4(const std::string &) -> std::string`  

get the external ipv4 Ethernet address of the current computer that best matches
the listed server  
";

%feature("docstring") helics::action_message_def::getHelicsVersionString "
`getHelicsVersionString() -> std::string`  

get a string with the helics version info  
";

// File: namespacehelics_1_1action__message__def.xml

// File: namespacehelics_1_1BrokerFactory.xml

%feature("docstring") helics::BrokerFactory::create "
`create(core_type type, const std::string &initializationString) ->
    std::shared_ptr< Broker >`  

Creates a Broker object of the specified type.  

Invokes initialize() on the instantiated Core object.  
";

%feature("docstring") helics::BrokerFactory::create "
`create(core_type type, const std::string &broker_name, const std::string
    &initializationString) -> std::shared_ptr< Broker >`  
";

%feature("docstring") helics::BrokerFactory::create "
`create(core_type type, int argc, const char *const *argv) -> std::shared_ptr<
    Broker >`  
";

%feature("docstring") helics::BrokerFactory::create "
`create(core_type type, const std::string &broker_name, int argc, const char
    *const *argv) -> std::shared_ptr< Broker >`  
";

%feature("docstring") helics::BrokerFactory::available "
`available(core_type type) -> bool`  
";

%feature("docstring") helics::BrokerFactory::findBroker "
`findBroker(const std::string &brokerName) -> std::shared_ptr< Broker >`  

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
`registerBroker(std::shared_ptr< Broker > broker) -> bool`  

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
`cleanUpBrokers() -> size_t`  

clean up unused brokers  

when brokers are unregistered they get put in a holding area that gets cleaned
up when a new broker is registered or when the clean up function is called this
prevents some odd threading issues  

Returns
-------
the number of brokers still operating  
";

%feature("docstring") helics::BrokerFactory::cleanUpBrokers "
`cleanUpBrokers(int delay) -> size_t`  

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
`copyBrokerIdentifier(const std::string &copyFromName, const std::string
    &copyToName)`  

make a copy of the broker pointer to allow access to the new name  
";

%feature("docstring") helics::BrokerFactory::unregisterBroker "
`unregisterBroker(const std::string &name)`  

remove a broker from the registry  

Parameters
----------
* `name` :  
    the name of the broker to unregister  
";

%feature("docstring") helics::BrokerFactory::displayHelp "
`displayHelp(core_type type)`  

display the help listing for a particular core_type  
";

// File: namespacehelics_1_1CoreFactory.xml

%feature("docstring") helics::CoreFactory::create "
`create(core_type type, const std::string &initializationString) ->
    std::shared_ptr< Core >`  

Creates a Core API object of the specified type.  

Invokes initialize() on the instantiated Core object.  
";

%feature("docstring") helics::CoreFactory::create "
`create(core_type type, const std::string &core_name, std::string
    &initializationString) -> std::shared_ptr< Core >`  
";

%feature("docstring") helics::CoreFactory::create "
`create(core_type type, int argc, const char *const *argv) -> std::shared_ptr<
    Core >`  
";

%feature("docstring") helics::CoreFactory::create "
`create(core_type type, const std::string &core_name, int argc, const char
    *const *argv) -> std::shared_ptr< Core >`  
";

%feature("docstring") helics::CoreFactory::FindOrCreate "
`FindOrCreate(core_type type, const std::string &core_name, const std::string
    &initializationString) -> std::shared_ptr< Core >`  

tries to find a named core if it fails it creates a new one  
";

%feature("docstring") helics::CoreFactory::FindOrCreate "
`FindOrCreate(core_type type, const std::string &core_name, int argc, const char
    *const *argv) -> std::shared_ptr< Core >`  

tries to find a named core if it fails it creates a new one  
";

%feature("docstring") helics::CoreFactory::findCore "
`findCore(const std::string &name) -> std::shared_ptr< Core >`  

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
`isJoinableCoreOfType(core_type type, const std::shared_ptr< CommonCore > &ptr)
    -> bool`  
";

%feature("docstring") helics::CoreFactory::findJoinableCoreOfType "
`findJoinableCoreOfType(core_type type) -> std::shared_ptr< Core >`  

try to find a joinable core of a specific type  
";

%feature("docstring") helics::CoreFactory::registerCore "
`registerCore(std::shared_ptr< Core > core) -> bool`  

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
`cleanUpCores() -> size_t`  

clean up unused cores  

when Cores are unregistered they get put in a holding area that gets cleaned up
when a new Core is registered or when the clean up function is called this
prevents some odd threading issues  

Returns
-------
the number of cores still operating  
";

%feature("docstring") helics::CoreFactory::cleanUpCores "
`cleanUpCores(int delay) -> size_t`  

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
`copyCoreIdentifier(const std::string &copyFromName, const std::string
    &copyToName)`  

make a copy of the broker pointer to allow access to the new name  
";

%feature("docstring") helics::CoreFactory::unregisterCore "
`unregisterCore(const std::string &name)`  

remove a Core from the registry  

Parameters
----------
* `name` :  
    the name of the Core to unregister  
";

// File: namespacestd.xml

%feature("docstring") std::swap "
`swap(helics::data_view &db1, helics::data_view &db2) noexcept`  
";

%feature("docstring") std::swap "
`swap(helics::data_block &db1, helics::data_block &db2) noexcept`  
";

%feature("docstring") std::swap "
`swap(helics::Message &m1, helics::Message &m2) noexcept`  
";

// File: namespacestd_1_1string__literals.xml

// File: namespacestringOps.xml

%feature("docstring") stringOps::splitline "
`splitline(const std::string &line, const std::string &delimiters,
    delimiter_compression compression) -> stringVector`  
";

%feature("docstring") stringOps::splitline "
`splitline(const std::string &line, char del) -> stringVector`  
";

%feature("docstring") stringOps::splitline "
`splitline(const std::string &line, stringVector &strVec, const std::string
    &delimiters, delimiter_compression compression)`  
";

%feature("docstring") stringOps::splitline "
`splitline(const std::string &line, stringVector &strVec, char del)`  
";

%feature("docstring") stringOps::splitlineQuotes "
`splitlineQuotes(const std::string &line, const std::string &delimiters, const
    std::string &quoteChars, delimiter_compression compression) -> stringVector`  
";

%feature("docstring") stringOps::splitlineBracket "
`splitlineBracket(const std::string &line, const std::string &delimiters, const
    std::string &bracketChars, delimiter_compression compression) ->
    stringVector`  
";

%feature("docstring") stringOps::trimString "
`trimString(std::string &input, const std::string &whitespace)`  
";

%feature("docstring") stringOps::trim "
`trim(const std::string &input, const std::string &whitespace) -> std::string`  
";

%feature("docstring") stringOps::trim "
`trim(stringVector &input, const std::string &whitespace)`  
";

%feature("docstring") stringOps::digits "
`digits(\"0123456789\") -> const std::string`  
";

%feature("docstring") stringOps::trailingStringInt "
`trailingStringInt(const std::string &input, std::string &output, int defNum) ->
    int`  
";

%feature("docstring") stringOps::trailingStringInt "
`trailingStringInt(const std::string &input, int defNum) -> int`  
";

%feature("docstring") stringOps::quoteChars "
`quoteChars(R\"raw(\"'`) raw\") -> const std::string`  
";

%feature("docstring") stringOps::removeQuotes "
`removeQuotes(const std::string &str) -> std::string`  
";

%feature("docstring") stringOps::removeBrackets "
`removeBrackets(const std::string &str) -> std::string`  
";

%feature("docstring") stringOps::getTailString "
`getTailString(const std::string &input, char sep) -> std::string`  
";

%feature("docstring") stringOps::getTailString "
`getTailString(const std::string &input, const std::string &sep) -> std::string`  
";

%feature("docstring") stringOps::findCloseStringMatch "
`findCloseStringMatch(const stringVector &testStrings, const stringVector
    &iStrings, string_match_type_t matchType) -> int`  
";

%feature("docstring") stringOps::removeChars "
`removeChars(const std::string &source, const std::string &remchars) ->
    std::string`  
";

%feature("docstring") stringOps::removeChar "
`removeChar(const std::string &source, char remchar) -> std::string`  
";

%feature("docstring") stringOps::characterReplace "
`characterReplace(const std::string &source, char key, std::string repStr) ->
    std::string`  
";

%feature("docstring") stringOps::xmlCharacterCodeReplace "
`xmlCharacterCodeReplace(std::string str) -> std::string`  
";

// File: namespaceutilities.xml

%feature("docstring") utilities::is_base64 "
`is_base64(unsigned char c) -> bool`  
";

%feature("docstring") utilities::base64_encode "
`base64_encode(unsigned char const *bytes_to_encode, int32_t in_len) ->
    std::string`  

encode a binary sequence to a string  
";

%feature("docstring") utilities::base64_decode "
`base64_decode(std::string const &encoded_string, size_t offset) -> std::vector<
    unsigned char >`  

decode a string to a vector of unsigned chars  
";

%feature("docstring") utilities::base64_decode "
`base64_decode(std::string const &encoded_string, void *data, size_t max_size)
    -> size_t`  

decode a string to the specified memory location  
";

%feature("docstring") utilities::base64_decode_to_string "
`base64_decode_to_string(std::string const &encoded_string, size_t offset) ->
    std::string`  

decode a string to a string  
";

%feature("docstring") utilities::base64_decode_type "
`base64_decode_type(std::string const &encoded_string) -> std::vector< vType >`  

decode a string to a typed vector  
";

%feature("docstring") utilities::numericMapper "
`numericMapper() -> charMapper< bool >`  

map that translates all characters that could be in numbers to true all others
to false  
";

%feature("docstring") utilities::numericStartMapper "
`numericStartMapper() -> charMapper< bool >`  

map that translates all characters that could start a number to true all others
to false  
";

%feature("docstring") utilities::numericEndMapper "
`numericEndMapper() -> charMapper< bool >`  

map that translates all characters that could end a number to true all others to
false  
";

%feature("docstring") utilities::base64Mapper "
`base64Mapper() -> charMapper< unsigned char >`  

map that translates all base 64 characters to the appropriate numerical value  
";

%feature("docstring") utilities::digitMapper "
`digitMapper() -> charMapper< unsigned char >`  

map that translates numerical characters to the appropriate numerical value  
";

%feature("docstring") utilities::hexMapper "
`hexMapper() -> charMapper< unsigned char >`  

map that translates all hexadecimal characters to the appropriate numerical
value  
";

%feature("docstring") utilities::pairMapper "
`pairMapper() -> charMapper< unsigned char >`  

map that all containing characters that come in pairs to the appropriate match
'{' to '}'  
";

// File: namespacezmq.xml

%feature("docstring") zmq::poll "
`poll(zmq_pollitem_t const *items_, size_t nitems_, long timeout_=-1) -> int`  
";

%feature("docstring") zmq::poll "
`poll(zmq_pollitem_t const *items, size_t nitems) -> int`  
";

%feature("docstring") zmq::proxy "
`proxy(void *frontend, void *backend, void *capture)`  
";

%feature("docstring") zmq::proxy_steerable "
`proxy_steerable(void *frontend, void *backend, void *capture, void *control)`  
";

%feature("docstring") zmq::version "
`version(int *major_, int *minor_, int *patch_)`  
";

// File: ActionMessage_8cpp.xml

// File: ActionMessage_8hpp.xml

// File: ActionMessageDefintions_8hpp.xml

// File: api-data_8h.xml

// File: api__objects_8h.xml

%feature("docstring") helics::getFed "
`getFed(helics_federate fed) -> helics::Federate *`  
";

%feature("docstring") helics::getValueFed "
`getValueFed(helics_federate fed) -> helics::ValueFederate *`  
";

%feature("docstring") helics::getMessageFed "
`getMessageFed(helics_federate fed) -> helics::MessageFederate *`  
";

%feature("docstring") helics::getCore "
`getCore(helics_core core) -> helics::Core *`  
";

%feature("docstring") helics::getFedSharedPtr "
`getFedSharedPtr(helics_federate fed) -> std::shared_ptr< helics::Federate >`  
";

%feature("docstring") helics::getValueFedSharedPtr "
`getValueFedSharedPtr(helics_federate fed) -> std::shared_ptr<
    helics::ValueFederate >`  
";

%feature("docstring") helics::getMessageFedSharedPtr "
`getMessageFedSharedPtr(helics_federate fed) -> std::shared_ptr<
    helics::MessageFederate >`  
";

%feature("docstring") helics::getCoreSharedPtr "
`getCoreSharedPtr(helics_core core) -> std::shared_ptr< helics::Core >`  
";

%feature("docstring") helics::getMasterHolder "
`getMasterHolder() -> MasterObjectHolder *`  
";

%feature("docstring") helics::clearAllObjects "
`clearAllObjects()`  
";

// File: appMain_8cpp.xml

%feature("docstring") showHelp "
`showHelp()`  
";

%feature("docstring") main "
`main(int argc, char *argv[]) -> int`  
";

// File: argParser_8cpp.xml

// File: argParser_8h.xml

// File: AsioServiceManager_8cpp.xml

%feature("docstring") serviceRunLoop "
`serviceRunLoop(std::shared_ptr< AsioServiceManager > ptr)`  
";

// File: AsioServiceManager_8h.xml

%feature("docstring") serviceRunLoop "
`serviceRunLoop(std::shared_ptr< AsioServiceManager > ptr)`  
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
`gen_id() -> std::string`  
";

%feature("docstring") helics::argumentParser "
`argumentParser(int argc, const char *const *argv,
    boost::program_options::variables_map &vm_map)`  
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
`echoArgumentParser(int argc, const char *const *argv, po::variables_map
    &vm_map)`  
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
`getFed(helics_federate fed) -> helics::Federate *`  
";

%feature("docstring") getValueFed "
`getValueFed(helics_federate fed) -> helics::ValueFederate *`  
";

%feature("docstring") getMessageFed "
`getMessageFed(helics_federate fed) -> helics::MessageFederate *`  
";

%feature("docstring") getFedSharedPtr "
`getFedSharedPtr(helics_federate fed) -> std::shared_ptr< helics::Federate >`  
";

%feature("docstring") getValueFedSharedPtr "
`getValueFedSharedPtr(helics_federate fed) -> std::shared_ptr<
    helics::ValueFederate >`  
";

%feature("docstring") getMessageFedSharedPtr "
`getMessageFedSharedPtr(helics_federate fed) -> std::shared_ptr<
    helics::MessageFederate >`  
";

%feature("docstring") getMasterHolder "
`getMasterHolder() -> MasterObjectHolder *`  
";

%feature("docstring") clearAllObjects "
`clearAllObjects()`  
";

%feature("docstring") helicsCreateValueFederate "
`helicsCreateValueFederate(const helics_federate_info_t fi) -> helics_federate`  

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
`helicsCreateValueFederateFromJson(const char *json) -> helics_federate`  

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
`helicsCreateMessageFederate(const helics_federate_info_t fi) ->
    helics_federate`  

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
`helicsCreateMessageFederateFromJson(const char *json) -> helics_federate`  

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
`helicsCreateCombinationFederate(const helics_federate_info_t fi) ->
    helics_federate`  

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
`helicsCreateCombinationFederateFromJson(const char *json) -> helics_federate`  

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
`helicsFederateGetCoreObject(helics_federate fed) -> helics_core`  

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
`helicsFederateFinalize(helics_federate fed) -> helics_status`  

finalize the federate this halts all communication in the federate and
disconnects it from the core  
";

%feature("docstring") helicsFederateEnterInitializationMode "
`helicsFederateEnterInitializationMode(helics_federate fed) -> helics_status`  

enter the initialization state of a federate  

the initialization state allows initial values to be set and received if the
iteration is requested on entry to the execution state This is a blocking call
and will block until the core allows it to proceed  
";

%feature("docstring") helicsFederateEnterInitializationModeAsync "
`helicsFederateEnterInitializationModeAsync(helics_federate fed) ->
    helics_status`  

non blocking alternative to  the function
helicsFederateEnterInitializationModeFinalize must be called to finish the
operation  
";

%feature("docstring") helicsFederateIsAsyncOperationCompleted "
`helicsFederateIsAsyncOperationCompleted(helics_federate fed) -> int`  

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
`helicsFederateEnterInitializationModeComplete(helics_federate fed) ->
    helics_status`  

finalize the entry to initialize mode that was initiated with  
";

%feature("docstring") helicsFederateEnterExecutionMode "
`helicsFederateEnterExecutionMode(helics_federate fed) -> helics_status`  

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
`getIterationRequest(helics_iteration_request iterate) ->
    helics::helics_iteration_request`  
";

%feature("docstring") getIterationStatus "
`getIterationStatus(helics::iteration_result iterationState) ->
    helics_iteration_status`  
";

%feature("docstring") helicsFederateEnterExecutionModeIterative "
`helicsFederateEnterExecutionModeIterative(helics_federate fed,
    helics_iteration_request iterate, helics_iteration_status *outIterate) ->
    helics_status`  
";

%feature("docstring") helicsFederateEnterExecutionModeAsync "
`helicsFederateEnterExecutionModeAsync(helics_federate fed) -> helics_status`  

request that the federate enter the Execution mode  

this call is non-blocking and will return immediately call /ref
helicsFederateEnterExecutionModeComplete to finish the call sequence /ref  
";

%feature("docstring") helicsFederateEnterExecutionModeIterativeAsync "
`helicsFederateEnterExecutionModeIterativeAsync(helics_federate fed,
    helics_iteration_request iterate) -> helics_status`  
";

%feature("docstring") helicsFederateEnterExecutionModeComplete "
`helicsFederateEnterExecutionModeComplete(helics_federate fed) -> helics_status`  

complete the call to /ref EnterExecutionModeAsync  

Parameters
----------
* `fed` :  
    the federate object to complete the call  
";

%feature("docstring") helicsFederateEnterExecutionModeIterativeComplete "
`helicsFederateEnterExecutionModeIterativeComplete(helics_federate fed,
    helics_iteration_status *outConverged) -> helics_status`  
";

%feature("docstring") helicsFederateRequestTime "
`helicsFederateRequestTime(helics_federate fed, helics_time_t requestTime,
    helics_time_t *timeOut) -> helics_status`  

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
`helicsFederateRequestTimeIterative(helics_federate fed, helics_time_t
    requestTime, helics_iteration_request iterate, helics_time_t *timeOut,
    helics_iteration_status *outIteration) -> helics_status`  

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
`helicsFederateRequestTimeAsync(helics_federate fed, helics_time_t requestTime)
    -> helics_status`  
";

%feature("docstring") helicsFederateRequestTimeIterativeAsync "
`helicsFederateRequestTimeIterativeAsync(helics_federate fed, helics_time_t
    requestTime, helics_iteration_request iterate) -> helics_status`  
";

%feature("docstring") helicsFederateRequestTimeComplete "
`helicsFederateRequestTimeComplete(helics_federate fed, helics_time_t *timeOut)
    -> helics_status`  
";

%feature("docstring") helicsFederateGetState "
`helicsFederateGetState(helics_federate fed, federate_state *state) ->
    helics_status`  

get the current state of a federate  

Parameters
----------
* `fed` :  
    the fed to query  
* `state` :  
    the resulting state if helics_status return helics_ok  
";

%feature("docstring") helicsFederateGetName "
`helicsFederateGetName(helics_federate fed, char *str, int maxlen) ->
    helics_status`  

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
`helicsFederateSetTimeDelta(helics_federate fed, helics_time_t time) ->
    helics_status`  

set the minimum time delta for the federate  

Parameters
----------
* `tdelta` :  
    the minimum time delta to return from a time request function  
";

%feature("docstring") helicsFederateSetOutputDelay "
`helicsFederateSetOutputDelay(helics_federate fed, helics_time_t outputDelay) ->
    helics_status`  

set the look ahead time  

the look ahead is the propagation time for messages/event to propagate from the
Federate the federate  

Parameters
----------
* `lookAhead` :  
    the look ahead time  
";

%feature("docstring") helicsFederateSetInputDelay "
`helicsFederateSetInputDelay(helics_federate fed, helics_time_t inputDelay) ->
    helics_status`  

set the impact Window time  

the impact window is the time window around the time request in which other
federates cannot affect the federate  

Parameters
----------
* `lookAhead` :  
    the look ahead time  
";

%feature("docstring") helicsFederateSetPeriod "
`helicsFederateSetPeriod(helics_federate fed, helics_time_t period,
    helics_time_t offset) -> helics_status`  

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
`helicsFederateSetFlag(helics_federate fed, int flag, helics_bool_t flagValue)
    -> helics_status`  

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
`helicsFederateSetLoggingLevel(helics_federate fed, int loggingLevel) ->
    helics_status`  

set the logging level for the federate @ details debug and trace only do
anything if they were enabled in the compilation  

Parameters
----------
* `loggingLevel` :  
    (-1: none, 0: error_only, 1: warnings, 2: normal, 3: debug, 4: trace)  
";

%feature("docstring") helicsFederateGetCurrentTime "
`helicsFederateGetCurrentTime(helics_federate fed, helics_time_t *time) ->
    helics_status`  

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
`helicsFederateRequestTimeIterativeComplete(helics_federate fed, helics_time_t
    *timeOut, helics_iteration_status *outIteration) -> helics_status`  
";

// File: FederateInfo_8cpp.xml

%feature("docstring") helics::argumentParser "
`argumentParser(int argc, const char *const *argv, po::variables_map &vm_map)`  
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
`generalized_string_split(const X &str, const X &delimiterCharacters, bool
    compress) -> std::vector< X >`  

this file defines some operations that can be performed on string like objects  
";

%feature("docstring") getChunkEnd "
`getChunkEnd(size_t start, const X &str, char ChunkStart, char ChunkEnd) ->
    size_t`  
";

%feature("docstring") generalized_section_splitting "
`generalized_section_splitting(const X &line, const X &delimiterCharacters,
    const X &sectionStartCharacters, const utilities::charMapper< unsigned char
    > &sectionMatch, bool compress) -> std::vector< X >`  
";

// File: helics-broker_8cpp.xml

%feature("docstring") argumentParser "
`argumentParser(int argc, const char *const *argv, po::variables_map &vm_map) ->
    int`  
";

%feature("docstring") main "
`main(int argc, char *argv[]) -> int`  
";

// File: helics-time_8hpp.xml

// File: helics_8h.xml

%feature("docstring") helicsGetVersion "
`helicsGetVersion() -> HELICS_Export const char *`  
";

%feature("docstring") helicsIsCoreTypeAvailable "
`helicsIsCoreTypeAvailable(const char *type) -> HELICS_Export helics_bool_t`  

Returns true if core/broker type specified is available in current compilation.  
";

%feature("docstring") helicsCreateCore "
`helicsCreateCore(const char *type, const char *name, const char *initString) ->
    HELICS_Export helics_core`  

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
`helicsCreateCoreFromArgs(const char *type, const char *name, int argc, const
    char *const *argv) -> HELICS_Export helics_core`  

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
`helicsCreateBroker(const char *type, const char *name, const char *initString)
    -> HELICS_Export helics_broker`  

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
`helicsCreateBrokerFromArgs(const char *type, const char *name, int argc, const
    char *const *argv) -> HELICS_Export helics_broker`  

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
`helicsBrokerIsConnected(helics_broker broker) -> HELICS_Export int`  

check if a broker is connected a connected broker implies is attached to cores
or cores could reach out to communicate return 0 if not connected , something
else if it is connected  
";

%feature("docstring") helicsCoreIsConnected "
`helicsCoreIsConnected(helics_core core) -> HELICS_Export int`  

check if a core is connected a connected core implies is attached to federate or
federates could be attached to it return 0 if not connected , something else if
it is connected  
";

%feature("docstring") helicsBrokerGetIdentifier "
`helicsBrokerGetIdentifier(helics_broker broker, char *identifier, int maxlen)
    -> HELICS_Export helics_status`  

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
`helicsCoreGetIdentifier(helics_core core, char *identifier, int maxlen) ->
    HELICS_Export helics_status`  

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
`helicsBrokerGetAddress(helics_broker broker, char *address, int maxlen) ->
    HELICS_Export helics_status`  

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
`helicsCoreDisconnect(helics_core core) -> HELICS_Export helics_status`  

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
`helicsBrokerDisconnect(helics_broker broker) -> HELICS_Export helics_status`  

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
`helicsCoreFree(helics_core core) -> HELICS_Export void`  

release the memory associated with a core  
";

%feature("docstring") helicsBrokerFree "
`helicsBrokerFree(helics_broker broker) -> HELICS_Export void`  

release the memory associated with a broker  
";

%feature("docstring") helicsCreateValueFederate "
`helicsCreateValueFederate(const helics_federate_info_t fi) -> HELICS_Export
    helics_federate`  

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
`helicsCreateValueFederateFromJson(const char *json) -> HELICS_Export
    helics_federate`  

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
`helicsCreateMessageFederate(const helics_federate_info_t fi) -> HELICS_Export
    helics_federate`  

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
`helicsCreateMessageFederateFromJson(const char *json) -> HELICS_Export
    helics_federate`  

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
`helicsCreateCombinationFederate(const helics_federate_info_t fi) ->
    HELICS_Export helics_federate`  

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
`helicsCreateCombinationFederateFromJson(const char *json) -> HELICS_Export
    helics_federate`  

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
`helicsFederateInfoCreate() -> HELICS_Export helics_federate_info_t`  

create a federate info object for specifying federate information when
constructing a federate  

Returns
-------
a helics_federate_info_t object which is a reference to the created object  
";

%feature("docstring") helicsFederateInfoLoadFromArgs "
`helicsFederateInfoLoadFromArgs(helics_federate_info_t fi, int argc, const char
    *const *argv) -> HELICS_Export helics_status`  

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
`helicsFederateInfoFree(helics_federate_info_t fi) -> HELICS_Export void`  

delete the memory associated with a federate info object  
";

%feature("docstring") helicsFederateInfoSetFederateName "
`helicsFederateInfoSetFederateName(helics_federate_info_t fi, const char *name)
    -> HELICS_Export helics_status`  

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
`helicsFederateInfoSetCoreName(helics_federate_info_t fi, const char *corename)
    -> HELICS_Export helics_status`  

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
`helicsFederateInfoSetCoreInitString(helics_federate_info_t fi, const char
    *coreInit) -> HELICS_Export helics_status`  

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
`helicsFederateInfoSetCoreTypeFromString(helics_federate_info_t fi, const char
    *coretype) -> HELICS_Export helics_status`  

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
`helicsFederateInfoSetCoreType(helics_federate_info_t fi, int coretype) ->
    HELICS_Export helics_status`  
";

%feature("docstring") helicsFederateInfoSetFlag "
`helicsFederateInfoSetFlag(helics_federate_info_t fi, int flag, helics_bool_t
    value) -> HELICS_Export helics_status`  
";

%feature("docstring") helicsFederateInfoSetOutputDelay "
`helicsFederateInfoSetOutputDelay(helics_federate_info_t fi, helics_time_t
    outputDelay) -> HELICS_Export helics_status`  
";

%feature("docstring") helicsFederateInfoSetTimeDelta "
`helicsFederateInfoSetTimeDelta(helics_federate_info_t fi, helics_time_t
    timeDelta) -> HELICS_Export helics_status`  
";

%feature("docstring") helicsFederateInfoSetInputDelay "
`helicsFederateInfoSetInputDelay(helics_federate_info_t fi, helics_time_t
    inputDelay) -> HELICS_Export helics_status`  
";

%feature("docstring") helicsFederateInfoSetTimeOffset "
`helicsFederateInfoSetTimeOffset(helics_federate_info_t fi, helics_time_t
    timeOffset) -> HELICS_Export helics_status`  
";

%feature("docstring") helicsFederateInfoSetPeriod "
`helicsFederateInfoSetPeriod(helics_federate_info_t fi, helics_time_t period) ->
    HELICS_Export helics_status`  
";

%feature("docstring") helicsFederateInfoSetMaxIterations "
`helicsFederateInfoSetMaxIterations(helics_federate_info_t fi, int
    maxIterations) -> HELICS_Export helics_status`  
";

%feature("docstring") helicsFederateInfoSetLoggingLevel "
`helicsFederateInfoSetLoggingLevel(helics_federate_info_t fi, int logLevel) ->
    HELICS_Export helics_status`  
";

%feature("docstring") helicsFederateFinalize "
`helicsFederateFinalize(helics_federate fed) -> HELICS_Export helics_status`  

finalize the federate this halts all communication in the federate and
disconnects it from the core  
";

%feature("docstring") helicsFederateFree "
`helicsFederateFree(helics_federate fed) -> HELICS_Export void`  

release the memory associated withe a federate  
";

%feature("docstring") helicsCloseLibrary "
`helicsCloseLibrary() -> HELICS_Export void`  

call when done using the helics library, this function will ensure the threads
are closed properly if possible this should be the last call before exiting,  
";

%feature("docstring") helicsFederateEnterInitializationMode "
`helicsFederateEnterInitializationMode(helics_federate fed) -> HELICS_Export
    helics_status`  

enter the initialization state of a federate  

the initialization state allows initial values to be set and received if the
iteration is requested on entry to the execution state This is a blocking call
and will block until the core allows it to proceed  
";

%feature("docstring") helicsFederateEnterInitializationModeAsync "
`helicsFederateEnterInitializationModeAsync(helics_federate fed) ->
    HELICS_Export helics_status`  

non blocking alternative to  the function
helicsFederateEnterInitializationModeFinalize must be called to finish the
operation  
";

%feature("docstring") helicsFederateIsAsyncOperationCompleted "
`helicsFederateIsAsyncOperationCompleted(helics_federate fed) -> HELICS_Export
    helics_bool_t`  

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
`helicsFederateEnterInitializationModeComplete(helics_federate fed) ->
    HELICS_Export helics_status`  

finalize the entry to initialize mode that was initiated with  
";

%feature("docstring") helicsFederateEnterExecutionMode "
`helicsFederateEnterExecutionMode(helics_federate fed) -> HELICS_Export
    helics_status`  

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
`helicsFederateEnterExecutionModeAsync(helics_federate fed) -> HELICS_Export
    helics_status`  

request that the federate enter the Execution mode  

this call is non-blocking and will return immediately call /ref
helicsFederateEnterExecutionModeComplete to finish the call sequence /ref  
";

%feature("docstring") helicsFederateEnterExecutionModeComplete "
`helicsFederateEnterExecutionModeComplete(helics_federate fed) -> HELICS_Export
    helics_status`  

complete the call to /ref EnterExecutionModeAsync  

Parameters
----------
* `fed` :  
    the federate object to complete the call  
";

%feature("docstring") helicsFederateEnterExecutionModeIterative "
`helicsFederateEnterExecutionModeIterative(helics_federate fed,
    helics_iteration_request iterate, helics_iteration_status *outIterate) ->
    HELICS_Export helics_status`  
";

%feature("docstring") helicsFederateEnterExecutionModeIterativeAsync "
`helicsFederateEnterExecutionModeIterativeAsync(helics_federate fed,
    helics_iteration_request iterate) -> HELICS_Export helics_status`  
";

%feature("docstring") helicsFederateEnterExecutionModeIterativeComplete "
`helicsFederateEnterExecutionModeIterativeComplete(helics_federate fed,
    helics_iteration_status *outIterate) -> HELICS_Export helics_status`  
";

%feature("docstring") helicsFederateGetState "
`helicsFederateGetState(helics_federate fed, federate_state *state) ->
    HELICS_Export helics_status`  

get the current state of a federate  

Parameters
----------
* `fed` :  
    the fed to query  
* `state` :  
    the resulting state if helics_status return helics_ok  
";

%feature("docstring") helicsFederateGetCoreObject "
`helicsFederateGetCoreObject(helics_federate fed) -> HELICS_Export helics_core`  

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
`helicsFederateRequestTime(helics_federate fed, helics_time_t requestTime,
    helics_time_t *timeOut) -> HELICS_Export helics_status`  

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
`helicsFederateRequestTimeIterative(helics_federate fed, helics_time_t
    requestTime, helics_iteration_request iterate, helics_time_t *timeOut,
    helics_iteration_status *outIterate) -> HELICS_Export helics_status`  

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
`helicsFederateRequestTimeAsync(helics_federate fed, helics_time_t requestTime)
    -> HELICS_Export helics_status`  
";

%feature("docstring") helicsFederateRequestTimeComplete "
`helicsFederateRequestTimeComplete(helics_federate fed, helics_time_t *timeOut)
    -> HELICS_Export helics_status`  
";

%feature("docstring") helicsFederateRequestTimeIterativeAsync "
`helicsFederateRequestTimeIterativeAsync(helics_federate fed, helics_time_t
    requestTime, helics_iteration_request iterate) -> HELICS_Export
    helics_status`  
";

%feature("docstring") helicsFederateRequestTimeIterativeComplete "
`helicsFederateRequestTimeIterativeComplete(helics_federate fed, helics_time_t
    *timeOut, helics_iteration_status *outIterate) -> HELICS_Export
    helics_status`  
";

%feature("docstring") helicsFederateGetName "
`helicsFederateGetName(helics_federate fed, char *str, int maxlen) ->
    HELICS_Export helics_status`  

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
`helicsFederateSetTimeDelta(helics_federate fed, helics_time_t time) ->
    HELICS_Export helics_status`  

set the minimum time delta for the federate  

Parameters
----------
* `tdelta` :  
    the minimum time delta to return from a time request function  
";

%feature("docstring") helicsFederateSetOutputDelay "
`helicsFederateSetOutputDelay(helics_federate fed, helics_time_t outputDelay) ->
    HELICS_Export helics_status`  

set the look ahead time  

the look ahead is the propagation time for messages/event to propagate from the
Federate the federate  

Parameters
----------
* `lookAhead` :  
    the look ahead time  
";

%feature("docstring") helicsFederateSetInputDelay "
`helicsFederateSetInputDelay(helics_federate fed, helics_time_t inputDelay) ->
    HELICS_Export helics_status`  

set the impact Window time  

the impact window is the time window around the time request in which other
federates cannot affect the federate  

Parameters
----------
* `lookAhead` :  
    the look ahead time  
";

%feature("docstring") helicsFederateSetPeriod "
`helicsFederateSetPeriod(helics_federate fed, helics_time_t period,
    helics_time_t offset) -> HELICS_Export helics_status`  

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
`helicsFederateSetFlag(helics_federate fed, int flag, helics_bool_t flagValue)
    -> HELICS_Export helics_status`  

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
`helicsFederateSetLoggingLevel(helics_federate fed, int loggingLevel) ->
    HELICS_Export helics_status`  

set the logging level for the federate @ details debug and trace only do
anything if they were enabled in the compilation  

Parameters
----------
* `loggingLevel` :  
    (-1: none, 0: error_only, 1: warnings, 2: normal, 3: debug, 4: trace)  
";

%feature("docstring") helicsFederateGetCurrentTime "
`helicsFederateGetCurrentTime(helics_federate fed, helics_time_t *time) ->
    HELICS_Export helics_status`  

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
`helicsCreateQuery(const char *target, const char *query) -> HELICS_Export
    helics_query`  

create a query object  

a query object consists of a target and query string  
";

%feature("docstring") helicsQueryExecute "
`helicsQueryExecute(helics_query query, helics_federate fed) -> HELICS_Export
    const char *`  

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
`helicsQueryExecuteAsync(helics_query query, helics_federate fed) ->
    HELICS_Export helics_status`  

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
`helicsQueryExecuteComplete(helics_query query) -> HELICS_Export const char *`  

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
`helicsQueryIsCompleted(helics_query query) -> HELICS_Export helics_bool_t`  

check if an asynchronously executed query has completed  

Returns
-------
will return helics_true if an async query has complete or a regular query call
was made with a result and false if an async query has not completed or is
invalid  
";

%feature("docstring") helicsQueryFree "
`helicsQueryFree(helics_query) -> HELICS_Export void`  

free the memory associated with a query object  
";

// File: cpp98_2helics_8hpp.xml

// File: helics_8hpp.xml

// File: helicsExport_8cpp.xml

%feature("docstring") versionStr "
`versionStr(helics::helicsVersionString()) -> const std::string`  
";

%feature("docstring") helicsGetVersion "
`helicsGetVersion(void) -> const char *`  
";

%feature("docstring") helicsIsCoreTypeAvailable "
`helicsIsCoreTypeAvailable(const char *type) -> helics_bool_t`  

Returns true if core/broker type specified is available in current compilation.  
";

%feature("docstring") helicsFederateInfoCreate "
`helicsFederateInfoCreate() -> helics_federate_info_t`  

create a federate info object for specifying federate information when
constructing a federate  

Returns
-------
a helics_federate_info_t object which is a reference to the created object  
";

%feature("docstring") helicsFederateInfoFree "
`helicsFederateInfoFree(helics_federate_info_t fi)`  

delete the memory associated with a federate info object  
";

%feature("docstring") helicsFederateInfoLoadFromArgs "
`helicsFederateInfoLoadFromArgs(helics_federate_info_t fi, int argc, const char
    *const *argv) -> helics_status`  

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
`helicsFederateInfoSetFederateName(helics_federate_info_t fi, const char *name)
    -> helics_status`  

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
`helicsFederateInfoSetCoreName(helics_federate_info_t fi, const char *corename)
    -> helics_status`  

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
`helicsFederateInfoSetCoreInitString(helics_federate_info_t fi, const char
    *coreinit) -> helics_status`  

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
`helicsFederateInfoSetCoreType(helics_federate_info_t fi, int coretype) ->
    helics_status`  
";

%feature("docstring") helicsFederateInfoSetCoreTypeFromString "
`helicsFederateInfoSetCoreTypeFromString(helics_federate_info_t fi, const char
    *coretype) -> helics_status`  

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
`helicsFederateInfoSetFlag(helics_federate_info_t fi, int flag, helics_bool_t
    value) -> helics_status`  
";

%feature("docstring") helicsFederateInfoSetOutputDelay "
`helicsFederateInfoSetOutputDelay(helics_federate_info_t fi, helics_time_t
    outputDelay) -> helics_status`  
";

%feature("docstring") helicsFederateInfoSetTimeDelta "
`helicsFederateInfoSetTimeDelta(helics_federate_info_t fi, helics_time_t
    timeDelta) -> helics_status`  
";

%feature("docstring") helicsFederateInfoSetInputDelay "
`helicsFederateInfoSetInputDelay(helics_federate_info_t fi, helics_time_t
    inputDelay) -> helics_status`  
";

%feature("docstring") helicsFederateInfoSetTimeOffset "
`helicsFederateInfoSetTimeOffset(helics_federate_info_t fi, helics_time_t
    timeOffset) -> helics_status`  
";

%feature("docstring") helicsFederateInfoSetPeriod "
`helicsFederateInfoSetPeriod(helics_federate_info_t fi, helics_time_t period) ->
    helics_status`  
";

%feature("docstring") helicsFederateInfoSetLoggingLevel "
`helicsFederateInfoSetLoggingLevel(helics_federate_info_t fi, int logLevel) ->
    helics_status`  
";

%feature("docstring") helicsFederateInfoSetMaxIterations "
`helicsFederateInfoSetMaxIterations(helics_federate_info_t fi, int
    maxIterations) -> helics_status`  
";

%feature("docstring") getCore "
`getCore(helics_core core) -> helics::Core *`  
";

%feature("docstring") getCoreSharedPtr "
`getCoreSharedPtr(helics_core core) -> std::shared_ptr< helics::Core >`  
";

%feature("docstring") getBroker "
`getBroker(helics_broker broker) -> helics::Broker *`  
";

%feature("docstring") getBrokerSharedPtr "
`getBrokerSharedPtr(helics_broker broker) -> std::shared_ptr< helics::Broker >`  
";

%feature("docstring") helicsCreateCore "
`helicsCreateCore(const char *type, const char *name, const char *initString) ->
    helics_core`  

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
`helicsCreateCoreFromArgs(const char *type, const char *name, int argc, const
    char *const *argv) -> helics_core`  

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
`helicsCreateBroker(const char *type, const char *name, const char *initString)
    -> helics_broker`  

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
`helicsCreateBrokerFromArgs(const char *type, const char *name, int argc, const
    char *const *argv) -> helics_broker`  

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
`helicsBrokerIsConnected(helics_broker broker) -> helics_bool_t`  

check if a broker is connected a connected broker implies is attached to cores
or cores could reach out to communicate return 0 if not connected , something
else if it is connected  
";

%feature("docstring") helicsCoreIsConnected "
`helicsCoreIsConnected(helics_core core) -> helics_bool_t`  

check if a core is connected a connected core implies is attached to federate or
federates could be attached to it return 0 if not connected , something else if
it is connected  
";

%feature("docstring") helicsBrokerGetIdentifier "
`helicsBrokerGetIdentifier(helics_broker broker, char *identifier, int maxlen)
    -> helics_status`  

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
`helicsCoreGetIdentifier(helics_core core, char *identifier, int maxlen) ->
    helics_status`  

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
`helicsBrokerGetAddress(helics_broker broker, char *address, int maxlen) ->
    helics_status`  

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
`helicsCoreDisconnect(helics_core core) -> helics_status`  

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
`helicsBrokerDisconnect(helics_broker broker) -> helics_status`  

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
`helicsCoreFree(helics_core core)`  

release the memory associated with a core  
";

%feature("docstring") helicsBrokerFree "
`helicsBrokerFree(helics_broker broker)`  

release the memory associated with a broker  
";

%feature("docstring") helicsFederateFree "
`helicsFederateFree(helics_federate fed)`  

release the memory associated withe a federate  
";

%feature("docstring") helicsCloseLibrary "
`helicsCloseLibrary()`  

call when done using the helics library, this function will ensure the threads
are closed properly if possible this should be the last call before exiting,  
";

%feature("docstring") helicsCreateQuery "
`helicsCreateQuery(const char *target, const char *query) -> helics_query`  

create a query object  

a query object consists of a target and query string  
";

%feature("docstring") helicsQueryExecute "
`helicsQueryExecute(helics_query query, helics_federate fed) -> const char *`  

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
`helicsQueryExecuteAsync(helics_query query, helics_federate fed) ->
    helics_status`  

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
`helicsQueryExecuteComplete(helics_query query) -> const char *`  

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
`helicsQueryIsCompleted(helics_query query) -> HELICS_Export helics_bool_t`  

check if an asynchronously executed query has completed  

Returns
-------
will return helics_true if an async query has complete or a regular query call
was made with a result and false if an async query has not completed or is
invalid  
";

%feature("docstring") helicsQueryFree "
`helicsQueryFree(helics_query query)`  

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
`helicsFederateRegisterEndpoint(helics_federate fed, const char *name, const
    char *type) -> HELICS_Export helics_endpoint`  
";

%feature("docstring") helicsFederateRegisterGlobalEndpoint "
`helicsFederateRegisterGlobalEndpoint(helics_federate fed, const char *name,
    const char *type) -> HELICS_Export helics_endpoint`  
";

%feature("docstring") helicsEndpointSetDefaultDestination "
`helicsEndpointSetDefaultDestination(helics_endpoint endpoint, const char *dest)
    -> HELICS_Export helics_status`  
";

%feature("docstring") helicsEndpointSendMessageRaw "
`helicsEndpointSendMessageRaw(helics_endpoint endpoint, const char *dest, const
    char *data, int len) -> HELICS_Export helics_status`  
";

%feature("docstring") helicsEndpointSendEventRaw "
`helicsEndpointSendEventRaw(helics_endpoint endpoint, const char *dest, const
    char *data, int len, helics_time_t time) -> HELICS_Export helics_status`  
";

%feature("docstring") helicsEndpointSendMessage "
`helicsEndpointSendMessage(helics_endpoint endpoint, message_t *message) ->
    HELICS_Export helics_status`  
";

%feature("docstring") helicsEndpointSubscribe "
`helicsEndpointSubscribe(helics_endpoint endpoint, const char *key, const char
    *type) -> HELICS_Export helics_status`  

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
`helicsFederateHasMessage(helics_federate fed) -> HELICS_Export int`  

check if the federate has any outstanding messages  
";

%feature("docstring") helicsEndpointHasMessage "
`helicsEndpointHasMessage(helics_endpoint endpoint) -> HELICS_Export int`  
";

%feature("docstring") helicsFederateReceiveCount "
`helicsFederateReceiveCount(helics_federate fed) -> HELICS_Export int`  

Returns the number of pending receives for the specified destination endpoint.  
";

%feature("docstring") helicsEndpointReceiveCount "
`helicsEndpointReceiveCount(helics_endpoint endpoint) -> HELICS_Export int`  

Returns the number of pending receives for all endpoints of particular federate.  
";

%feature("docstring") helicsEndpointGetMessage "
`helicsEndpointGetMessage(helics_endpoint endpoint) -> HELICS_Export message_t`  

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
`helicsFederateGetMessage(helics_federate fed) -> HELICS_Export message_t`  

receive a communication message for any endpoint in the federate  

the return order will be in order of endpoint creation then order of arrival all
messages for the first endpoint, then all for the second, and so on  

Returns
-------
a unique_ptr to a Message object containing the message data  
";

%feature("docstring") helicsEndpointGetType "
`helicsEndpointGetType(helics_endpoint endpoint, char *str, int maxlen) ->
    HELICS_Export helics_status`  

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
`helicsEndpointGetName(helics_endpoint endpoint, char *str, int maxlen) ->
    HELICS_Export helics_status`  

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
`addEndpoint(helics_federate fed, helics::EndpointObject *ept)`  
";

%feature("docstring") helicsFederateRegisterEndpoint "
`helicsFederateRegisterEndpoint(helics_federate fed, const char *name, const
    char *type) -> helics_endpoint`  
";

%feature("docstring") helicsFederateRegisterGlobalEndpoint "
`helicsFederateRegisterGlobalEndpoint(helics_federate fed, const char *name,
    const char *type) -> helics_endpoint`  
";

%feature("docstring") helicsEndpointSetDefaultDestination "
`helicsEndpointSetDefaultDestination(helics_endpoint endpoint, const char *dest)
    -> helics_status`  
";

%feature("docstring") helicsEndpointSendMessageRaw "
`helicsEndpointSendMessageRaw(helics_endpoint endpoint, const char *dest, const
    char *data, int len) -> helics_status`  
";

%feature("docstring") helicsEndpointSendEventRaw "
`helicsEndpointSendEventRaw(helics_endpoint endpoint, const char *dest, const
    char *data, int len, helics_time_t time) -> helics_status`  
";

%feature("docstring") helicsEndpointSendMessage "
`helicsEndpointSendMessage(helics_endpoint endpoint, message_t *message) ->
    helics_status`  
";

%feature("docstring") helicsEndpointSubscribe "
`helicsEndpointSubscribe(helics_endpoint endpoint, const char *key, const char
    *type) -> helics_status`  

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
`helicsFederateHasMessage(helics_federate fed) -> int`  

check if the federate has any outstanding messages  
";

%feature("docstring") helicsEndpointHasMessage "
`helicsEndpointHasMessage(helics_endpoint endpoint) -> int`  
";

%feature("docstring") helicsFederateReceiveCount "
`helicsFederateReceiveCount(helics_federate fed) -> int`  

Returns the number of pending receives for the specified destination endpoint.  
";

%feature("docstring") helicsEndpointReceiveCount "
`helicsEndpointReceiveCount(helics_endpoint endpoint) -> int`  

Returns the number of pending receives for all endpoints of particular federate.  
";

%feature("docstring") emptyMessage "
`emptyMessage() -> message_t`  
";

%feature("docstring") helicsEndpointGetMessage "
`helicsEndpointGetMessage(helics_endpoint endpoint) -> message_t`  

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
`helicsFederateGetMessage(helics_federate fed) -> message_t`  

receive a communication message for any endpoint in the federate  

the return order will be in order of endpoint creation then order of arrival all
messages for the first endpoint, then all for the second, and so on  

Returns
-------
a unique_ptr to a Message object containing the message data  
";

%feature("docstring") helicsEndpointGetType "
`helicsEndpointGetType(helics_endpoint endpoint, char *str, int maxlen) ->
    helics_status`  

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
`helicsEndpointGetName(helics_endpoint endpoint, char *str, int maxlen) ->
    helics_status`  

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
`helicsFederateRegisterSourceFilter(helics_federate fed, helics_filter_type_t
    type, const char *target, const char *name) -> HELICS_Export helics_filter`  

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
`helicsFederateRegisterDestinationFilter(helics_federate fed,
    helics_filter_type_t type, const char *target, const char *name) ->
    HELICS_Export helics_filter`  
";

%feature("docstring") helicsFederateRegisterCloningFilter "
`helicsFederateRegisterCloningFilter(helics_federate fed, const char
    *deliveryEndpoint) -> HELICS_Export helics_filter`  
";

%feature("docstring") helicsCoreRegisterSourceFilter "
`helicsCoreRegisterSourceFilter(helics_core core, helics_filter_type_t type,
    const char *target, const char *name) -> HELICS_Export helics_filter`  
";

%feature("docstring") helicsCoreRegisterDestinationFilter "
`helicsCoreRegisterDestinationFilter(helics_core core, helics_filter_type_t
    type, const char *target, const char *name) -> HELICS_Export helics_filter`  
";

%feature("docstring") helicsCoreRegisterCloningFilter "
`helicsCoreRegisterCloningFilter(helics_core fed, const char *deliveryEndpoint)
    -> HELICS_Export helics_filter`  
";

%feature("docstring") helicsFilterGetTarget "
`helicsFilterGetTarget(helics_filter filt, char *str, int maxlen) ->
    HELICS_Export helics_status`  

get the target of the filter  
";

%feature("docstring") helicsFilterGetName "
`helicsFilterGetName(helics_filter filt, char *str, int maxlen) -> HELICS_Export
    helics_status`  

get the name of the filter  
";

%feature("docstring") helicsFilterSet "
`helicsFilterSet(helics_filter filt, const char *property, double val) ->
    HELICS_Export helics_status`  
";

%feature("docstring") setString "
`setString(helics_filter filt, const char *property, const char *val) ->
    HELICS_Export helics_status`  
";

%feature("docstring") helicsFilterAddDestinationTarget "
`helicsFilterAddDestinationTarget(helics_filter filt, const char *dest) ->
    HELICS_Export helics_status`  
";

%feature("docstring") helicsFilterAddSourceTarget "
`helicsFilterAddSourceTarget(helics_filter filt, const char *dest) ->
    HELICS_Export helics_status`  
";

%feature("docstring") helicsFilterAddDeliveryEndpoint "
`helicsFilterAddDeliveryEndpoint(helics_filter filt, const char *dest) ->
    HELICS_Export helics_status`  
";

%feature("docstring") helicsFilterRemoveDestinationTarget "
`helicsFilterRemoveDestinationTarget(helics_filter filt, const char *dest) ->
    HELICS_Export helics_status`  
";

%feature("docstring") helicsFilterRemoveSourceTarget "
`helicsFilterRemoveSourceTarget(helics_filter filt, const char *dest) ->
    HELICS_Export helics_status`  
";

%feature("docstring") helicsFilterRemoveDeliveryEndpoint "
`helicsFilterRemoveDeliveryEndpoint(helics_filter filt, const char *dest) ->
    HELICS_Export helics_status`  
";

// File: MessageFiltersExport_8cpp.xml

%feature("docstring") federateAddFilter "
`federateAddFilter(helics_federate fed, helics::FilterObject *filt)`  
";

%feature("docstring") coreAddFilter "
`coreAddFilter(helics_core core, helics::FilterObject *filt)`  
";

%feature("docstring") helicsFederateRegisterSourceFilter "
`helicsFederateRegisterSourceFilter(helics_federate fed, helics_filter_type_t
    type, const char *target, const char *name) -> helics_filter`  

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
`helicsFederateRegisterDestinationFilter(helics_federate fed,
    helics_filter_type_t type, const char *target, const char *name) ->
    helics_filter`  
";

%feature("docstring") helicsCoreRegisterSourceFilter "
`helicsCoreRegisterSourceFilter(helics_core cr, helics_filter_type_t type, const
    char *target, const char *name) -> helics_filter`  
";

%feature("docstring") helicsCoreRegisterDestinationFilter "
`helicsCoreRegisterDestinationFilter(helics_core cr, helics_filter_type_t type,
    const char *target, const char *name) -> helics_filter`  
";

%feature("docstring") helicsFederateRegisterCloningFilter "
`helicsFederateRegisterCloningFilter(helics_federate fed, const char
    *deliveryEndpoint) -> helics_filter`  
";

%feature("docstring") helicsCoreRegisterCloningFilter "
`helicsCoreRegisterCloningFilter(helics_core cr, const char *deliveryEndpoint)
    -> helics_filter`  
";

%feature("docstring") getFilter "
`getFilter(helics_filter filt) -> helics::Filter *`  
";

%feature("docstring") getCloningFilter "
`getCloningFilter(helics_filter filt) -> helics::CloningFilter *`  
";

%feature("docstring") helicsFilterGetTarget "
`helicsFilterGetTarget(helics_filter filt, char *str, int maxlen) ->
    helics_status`  

get the target of the filter  
";

%feature("docstring") helicsFilterGetName "
`helicsFilterGetName(helics_filter filt, char *str, int maxlen) ->
    helics_status`  

get the name of the filter  
";

%feature("docstring") helicsFilterSet "
`helicsFilterSet(helics_filter filt, const char *property, double val) ->
    helics_status`  
";

%feature("docstring") setString "
`setString(helics_filter filt, const char *property, const char *val) ->
    helics_status`  
";

%feature("docstring") helicsFilterAddDestinationTarget "
`helicsFilterAddDestinationTarget(helics_filter filt, const char *dest) ->
    helics_status`  
";

%feature("docstring") helicsFilterAddSourceTarget "
`helicsFilterAddSourceTarget(helics_filter filt, const char *src) ->
    helics_status`  
";

%feature("docstring") helicsFilterAddDeliveryEndpoint "
`helicsFilterAddDeliveryEndpoint(helics_filter filt, const char *delivery) ->
    helics_status`  
";

%feature("docstring") helicsFilterRemoveDestinationTarget "
`helicsFilterRemoveDestinationTarget(helics_filter filt, const char *dest) ->
    helics_status`  
";

%feature("docstring") helicsFilterRemoveSourceTarget "
`helicsFilterRemoveSourceTarget(helics_filter filt, const char *source) ->
    helics_status`  
";

%feature("docstring") helicsFilterRemoveDeliveryEndpoint "
`helicsFilterRemoveDeliveryEndpoint(helics_filter filt, const char *delivery) ->
    helics_status`  
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
`playerArgumentParser(int argc, const char *const *argv, po::variables_map
    &vm_map) -> int`  
";

// File: player_8h.xml

// File: playerMain_8cpp.xml

%feature("docstring") main "
`main(int argc, char *argv[]) -> int`  
";

// File: PrecHelper_8cpp.xml

%feature("docstring") getType "
`getType(const std::string &typeString) -> helics_type_t`  
";

%feature("docstring") typeCharacter "
`typeCharacter(helics_type_t type) -> char`  
";

// File: PrecHelper_8h.xml

%feature("docstring") helics::getType "
`getType(const std::string &typeString) -> helics::helics_type_t`  
";

%feature("docstring") helics::typeCharacter "
`typeCharacter(helics::helics_type_t type) -> char`  
";

// File: PublicationInfo_8cpp.xml

// File: PublicationInfo_8hpp.xml

// File: Publications_8cpp.xml

// File: Publications_8hpp.xml

// File: queryFunctions_8cpp.xml

%feature("docstring") vectorizeQueryResult "
`vectorizeQueryResult(std::string &&queryres) -> std::vector< std::string >`  

function takes a query result and vectorizes it if the query is a vector result,
if not the results go into the first element of the vector  
";

%feature("docstring") vectorizeQueryResult "
`vectorizeQueryResult(const std::string &queryres) -> std::vector< std::string
    >`  

function takes a query result and vectorizes it if the query is a vector result,
if not the results go into the first element of the vector  
";

%feature("docstring") vectorizeAndSortQueryResult "
`vectorizeAndSortQueryResult(const std::string &queryres) -> std::vector<
    std::string >`  

function takes a query result, vectorizes and sorts it if the query is a vector
result, if not the results go into the first element of the vector  
";

%feature("docstring") vectorizeAndSortQueryResult "
`vectorizeAndSortQueryResult(std::string &&queryres) -> std::vector< std::string
    >`  

function takes a query result, vectorizes and sorts it if the query is a vector
result, if not the results go into the first element of the vector  
";

%feature("docstring") waitForInit "
`waitForInit(helics::Federate *fed, const std::string &fedName, int timeout) ->
    bool`  

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
`waitForFed(helics::Federate *fed, const std::string &fedName, int timeout) ->
    bool`  

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
`vectorizeQueryResult(std::string &&queryres) -> std::vector< std::string >`  

function takes a query result and vectorizes it if the query is a vector result,
if not the results go into the first element of the vector  
";

%feature("docstring") helics::vectorizeQueryResult "
`vectorizeQueryResult(const std::string &queryres) -> std::vector< std::string
    >`  

function takes a query result and vectorizes it if the query is a vector result,
if not the results go into the first element of the vector  
";

%feature("docstring") helics::vectorizeAndSortQueryResult "
`vectorizeAndSortQueryResult(std::string &&queryres) -> std::vector< std::string
    >`  

function takes a query result, vectorizes and sorts it if the query is a vector
result, if not the results go into the first element of the vector  
";

%feature("docstring") helics::vectorizeAndSortQueryResult "
`vectorizeAndSortQueryResult(const std::string &queryres) -> std::vector<
    std::string >`  

function takes a query result, vectorizes and sorts it if the query is a vector
result, if not the results go into the first element of the vector  
";

%feature("docstring") helics::waitForInit "
`waitForInit(helics::Federate *fed, const std::string &fedName, int
    timeout=10000) -> bool`  

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
`waitForFed(helics::Federate *fed, const std::string &fedName, int
    timeout=10000) -> bool`  

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
`recorderArgumentParser(int argc, const char *const *argv, po::variables_map
    &vm_map) -> int`  
";

// File: recorder_8h.xml

// File: recorderMain_8cpp.xml

%feature("docstring") main "
`main(int argc, char *argv[]) -> int`  
";

// File: searchableObjectHolder_8hpp.xml

// File: simpleQueue_8hpp.xml

// File: source_8cpp.xml

%feature("docstring") helics::sourceArgumentParser "
`sourceArgumentParser(int argc, const char *const *argv, po::variables_map
    &vm_map)`  
";

// File: source_8h.xml

// File: stringOps_8cpp.xml

%feature("docstring") stringOps::convertToLowerCase "
`convertToLowerCase(const std::string &input) -> std::string`  

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
`convertToUpperCase(const std::string &input) -> std::string`  

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
`makeLowerCase(std::string &input)`  

make a string lower case  

Parameters
----------
* `input` :  
    the string to convert  
";

%feature("docstring") stringOps::makeUpperCase "
`makeUpperCase(std::string &input)`  

make a string upper case  

Parameters
----------
* `input` :  
    the string to convert  
";

%feature("docstring") stringOps::while "
`while(tt !=std::string::npos)`  
";

// File: stringOps_8h.xml

%feature("docstring") convertToLowerCase "
`convertToLowerCase(const std::string &input) -> std::string`  

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
`convertToUpperCase(const std::string &input) -> std::string`  

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
`makeLowerCase(std::string &input)`  

make a string lower case  

Parameters
----------
* `input` :  
    the string to convert  
";

%feature("docstring") makeUpperCase "
`makeUpperCase(std::string &input)`  

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
`pow2(unsigned int exponent) -> constexpr double`  

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
`helicsFederateRegisterSubscription(helics_federate fed, const char *key, const
    char *type, const char *units) -> HELICS_Export helics_subscription`  

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
`helicsFederateRegisterTypeSubscription(helics_federate fed, const char *key,
    int type, const char *units) -> HELICS_Export helics_subscription`  

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
`helicsFederateRegisterOptionalSubscription(helics_federate fed, const char
    *key, const char *type, const char *units) -> HELICS_Export
    helics_subscription`  

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
`helicsFederateRegisterOptionalTypeSubscription(helics_federate fed, const char
    *key, int type, const char *units) -> HELICS_Export helics_subscription`  

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
`helicsFederateRegisterPublication(helics_federate fed, const char *key, const
    char *type, const char *units) -> HELICS_Export helics_publication`  
";

%feature("docstring") helicsFederateRegisterTypePublication "
`helicsFederateRegisterTypePublication(helics_federate fed, const char *key, int
    type, const char *units) -> HELICS_Export helics_publication`  
";

%feature("docstring") helicsFederateRegisterGlobalPublication "
`helicsFederateRegisterGlobalPublication(helics_federate fed, const char *key,
    const char *type, const char *units) -> HELICS_Export helics_publication`  
";

%feature("docstring") helicsFederateRegisterGlobalTypePublication "
`helicsFederateRegisterGlobalTypePublication(helics_federate fed, const char
    *key, int type, const char *units) -> HELICS_Export helics_publication`  
";

%feature("docstring") helicsPublicationPublish "
`helicsPublicationPublish(helics_publication pub, const char *data, int len) ->
    HELICS_Export helics_status`  
";

%feature("docstring") helicsPublicationPublishString "
`helicsPublicationPublishString(helics_publication pub, const char *str) ->
    HELICS_Export helics_status`  
";

%feature("docstring") helicsPublicationPublishInteger "
`helicsPublicationPublishInteger(helics_publication pub, int64_t val) ->
    HELICS_Export helics_status`  
";

%feature("docstring") helicsPublicationPublishDouble "
`helicsPublicationPublishDouble(helics_publication pub, double val) ->
    HELICS_Export helics_status`  
";

%feature("docstring") helicsPublicationPublishComplex "
`helicsPublicationPublishComplex(helics_publication pub, double real, double
    imag) -> HELICS_Export helics_status`  
";

%feature("docstring") helicsPublicationPublishVector "
`helicsPublicationPublishVector(helics_publication pub, const double data[], int
    len) -> HELICS_Export helics_status`  
";

%feature("docstring") helicsSubscriptionGetValueSize "
`helicsSubscriptionGetValueSize(helics_subscription sub) -> HELICS_Export int`  
";

%feature("docstring") helicsSubscriptionGetValue "
`helicsSubscriptionGetValue(helics_subscription sub, char *data, int maxlen, int
    *actualLength) -> HELICS_Export helics_status`  
";

%feature("docstring") helicsSubscriptionGetString "
`helicsSubscriptionGetString(helics_subscription sub, char *str, int maxlen) ->
    HELICS_Export helics_status`  
";

%feature("docstring") helicsSubscriptionGetInteger "
`helicsSubscriptionGetInteger(helics_subscription sub, int64_t *val) ->
    HELICS_Export helics_status`  
";

%feature("docstring") helicsSubscriptionGetDouble "
`helicsSubscriptionGetDouble(helics_subscription sub, double *val) ->
    HELICS_Export helics_status`  
";

%feature("docstring") helicsSubscriptionGetComplex "
`helicsSubscriptionGetComplex(helics_subscription sub, double *real, double
    *imag) -> HELICS_Export helics_status`  
";

%feature("docstring") helicsSubscriptionGetVectorSize "
`helicsSubscriptionGetVectorSize(helics_subscription sub) -> HELICS_Export int`  
";

%feature("docstring") helicsSubscriptionGetVector "
`helicsSubscriptionGetVector(helics_subscription sub, double data[], int maxlen,
    int *actualSize) -> HELICS_Export helics_status`  

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
`helicsSubscriptionSetDefault(helics_subscription sub, const char *data, int
    len) -> HELICS_Export helics_status`  
";

%feature("docstring") helicsSubscriptionSetDefaultString "
`helicsSubscriptionSetDefaultString(helics_subscription sub, const char *str) ->
    HELICS_Export helics_status`  
";

%feature("docstring") helicsSubscriptionSetDefaultInteger "
`helicsSubscriptionSetDefaultInteger(helics_subscription sub, int64_t val) ->
    HELICS_Export helics_status`  
";

%feature("docstring") helicsSubscriptionSetDefaultDouble "
`helicsSubscriptionSetDefaultDouble(helics_subscription sub, double val) ->
    HELICS_Export helics_status`  
";

%feature("docstring") helicsSubscriptionSetDefaultComplex "
`helicsSubscriptionSetDefaultComplex(helics_subscription sub, double real,
    double imag) -> HELICS_Export helics_status`  
";

%feature("docstring") helicsSubscriptionSetDefaultVector "
`helicsSubscriptionSetDefaultVector(helics_subscription sub, const double *data,
    int len) -> HELICS_Export helics_status`  
";

%feature("docstring") helicsSubscriptionGetType "
`helicsSubscriptionGetType(helics_subscription sub, char *str, int maxlen) ->
    HELICS_Export helics_status`  
";

%feature("docstring") helicsPublicationGetType "
`helicsPublicationGetType(helics_publication pub, char *str, int maxlen) ->
    HELICS_Export helics_status`  
";

%feature("docstring") helicsSubscriptionGetKey "
`helicsSubscriptionGetKey(helics_subscription sub, char *str, int maxlen) ->
    HELICS_Export helics_status`  
";

%feature("docstring") helicsPublicationGetKey "
`helicsPublicationGetKey(helics_publication pub, char *str, int maxlen) ->
    HELICS_Export helics_status`  
";

%feature("docstring") helicsSubscriptionGetUnits "
`helicsSubscriptionGetUnits(helics_subscription sub, char *str, int maxlen) ->
    HELICS_Export helics_status`  
";

%feature("docstring") helicsPublicationGetUnits "
`helicsPublicationGetUnits(helics_publication pub, char *str, int maxlen) ->
    HELICS_Export helics_status`  
";

%feature("docstring") helicsSubscriptionIsUpdated "
`helicsSubscriptionIsUpdated(helics_subscription sub) -> HELICS_Export
    helics_bool_t`  
";

%feature("docstring") helicsSubscriptionLastUpdateTime "
`helicsSubscriptionLastUpdateTime(helics_subscription sub) -> HELICS_Export
    helics_time_t`  
";

// File: application__api_2ValueFederate_8hpp.xml

// File: cpp98_2ValueFederate_8hpp.xml

// File: ValueFederateExport_8cpp.xml

%feature("docstring") addSubscription "
`addSubscription(helics_federate fed, helics::SubscriptionObject *sub)`  
";

%feature("docstring") addPublication "
`addPublication(helics_federate fed, helics::PublicationObject *pub)`  
";

%feature("docstring") helicsFederateRegisterSubscription "
`helicsFederateRegisterSubscription(helics_federate fed, const char *key, const
    char *type, const char *units) -> helics_subscription`  

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
`helicsFederateRegisterTypeSubscription(helics_federate fed, const char *key,
    int type, const char *units) -> helics_subscription`  

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
`helicsFederateRegisterOptionalSubscription(helics_federate fed, const char
    *key, const char *type, const char *units) -> helics_subscription`  

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
`helicsFederateRegisterOptionalTypeSubscription(helics_federate fed, const char
    *key, int type, const char *units) -> helics_subscription`  

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
`helicsFederateRegisterPublication(helics_federate fed, const char *key, const
    char *type, const char *units) -> helics_publication`  
";

%feature("docstring") helicsFederateRegisterTypePublication "
`helicsFederateRegisterTypePublication(helics_federate fed, const char *key, int
    type, const char *units) -> helics_publication`  
";

%feature("docstring") helicsFederateRegisterGlobalPublication "
`helicsFederateRegisterGlobalPublication(helics_federate fed, const char *key,
    const char *type, const char *units) -> helics_publication`  
";

%feature("docstring") helicsFederateRegisterGlobalTypePublication "
`helicsFederateRegisterGlobalTypePublication(helics_federate fed, const char
    *key, int type, const char *units) -> helics_publication`  
";

%feature("docstring") helicsPublicationPublish "
`helicsPublicationPublish(helics_publication pub, const char *data, int len) ->
    helics_status`  
";

%feature("docstring") helicsPublicationPublishString "
`helicsPublicationPublishString(helics_publication pub, const char *str) ->
    helics_status`  
";

%feature("docstring") helicsPublicationPublishInteger "
`helicsPublicationPublishInteger(helics_publication pub, int64_t val) ->
    helics_status`  
";

%feature("docstring") helicsPublicationPublishDouble "
`helicsPublicationPublishDouble(helics_publication pub, double val) ->
    helics_status`  
";

%feature("docstring") helicsPublicationPublishComplex "
`helicsPublicationPublishComplex(helics_publication pub, double real, double
    imag) -> helics_status`  
";

%feature("docstring") helicsPublicationPublishVector "
`helicsPublicationPublishVector(helics_publication pub, const double data[], int
    len) -> helics_status`  
";

%feature("docstring") helicsSubscriptionGetValueSize "
`helicsSubscriptionGetValueSize(helics_subscription sub) -> int`  
";

%feature("docstring") helicsSubscriptionGetValue "
`helicsSubscriptionGetValue(helics_subscription sub, char *data, int maxlen, int
    *actualSize) -> helics_status`  
";

%feature("docstring") helicsSubscriptionGetString "
`helicsSubscriptionGetString(helics_subscription sub, char *str, int maxlen) ->
    helics_status`  
";

%feature("docstring") helicsSubscriptionGetInteger "
`helicsSubscriptionGetInteger(helics_subscription sub, int64_t *val) ->
    helics_status`  
";

%feature("docstring") helicsSubscriptionGetDouble "
`helicsSubscriptionGetDouble(helics_subscription sub, double *val) ->
    helics_status`  
";

%feature("docstring") helicsSubscriptionGetComplex "
`helicsSubscriptionGetComplex(helics_subscription sub, double *real, double
    *imag) -> helics_status`  
";

%feature("docstring") helicsSubscriptionGetVectorSize "
`helicsSubscriptionGetVectorSize(helics_subscription sub) -> int`  
";

%feature("docstring") helicsSubscriptionGetVector "
`helicsSubscriptionGetVector(helics_subscription sub, double data[], int maxlen,
    int *actualSize) -> helics_status`  

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
`helicsSubscriptionSetDefault(helics_subscription sub, const char *data, int
    len) -> helics_status`  
";

%feature("docstring") helicsSubscriptionSetDefaultString "
`helicsSubscriptionSetDefaultString(helics_subscription sub, const char *str) ->
    helics_status`  
";

%feature("docstring") helicsSubscriptionSetDefaultInteger "
`helicsSubscriptionSetDefaultInteger(helics_subscription sub, int64_t val) ->
    helics_status`  
";

%feature("docstring") helicsSubscriptionSetDefaultDouble "
`helicsSubscriptionSetDefaultDouble(helics_subscription sub, double val) ->
    helics_status`  
";

%feature("docstring") helicsSubscriptionSetDefaultComplex "
`helicsSubscriptionSetDefaultComplex(helics_subscription sub, double real,
    double imag) -> helics_status`  
";

%feature("docstring") helicsSubscriptionSetDefaultVector "
`helicsSubscriptionSetDefaultVector(helics_subscription sub, const double *data,
    int len) -> helics_status`  
";

%feature("docstring") helicsSubscriptionGetType "
`helicsSubscriptionGetType(helics_subscription sub, char *str, int maxlen) ->
    helics_status`  
";

%feature("docstring") helicsPublicationGetType "
`helicsPublicationGetType(helics_publication pub, char *str, int maxlen) ->
    helics_status`  
";

%feature("docstring") helicsSubscriptionGetKey "
`helicsSubscriptionGetKey(helics_subscription sub, char *str, int maxlen) ->
    helics_status`  
";

%feature("docstring") helicsPublicationGetKey "
`helicsPublicationGetKey(helics_publication pub, char *str, int maxlen) ->
    helics_status`  
";

%feature("docstring") helicsSubscriptionGetUnits "
`helicsSubscriptionGetUnits(helics_subscription sub, char *str, int maxlen) ->
    helics_status`  
";

%feature("docstring") helicsPublicationGetUnits "
`helicsPublicationGetUnits(helics_publication pub, char *str, int maxlen) ->
    helics_status`  
";

%feature("docstring") helicsSubscriptionIsUpdated "
`helicsSubscriptionIsUpdated(helics_subscription sub) -> helics_bool_t`  
";

%feature("docstring") helicsSubscriptionLastUpdateTime "
`helicsSubscriptionLastUpdateTime(helics_subscription sub) -> helics_time_t`  
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
`socketTypeFromString(const std::string &socketType) -> socket_type`  
";

// File: zmqHelper_8h.xml

%feature("docstring") zmq::socketTypeFromString "
`socketTypeFromString(const std::string &socketType) -> zmq::socket_type`  
";

// File: zmqProxyHub_8cpp.xml

// File: zmqProxyHub_8h.xml

// File: zmqReactor_8cpp.xml

%feature("docstring") zero "
`zero(0) -> const int`  
";

%feature("docstring") findSocketByName "
`findSocketByName(const std::string &socketName, const std::vector< std::string
    > &socketNames) -> auto`  
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

