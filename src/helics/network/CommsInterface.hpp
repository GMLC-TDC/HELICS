/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "NetworkBrokerData.hpp"
#include "gmlc/concurrency/TriggerVariable.hpp"
#include "gmlc/concurrency/TripWire.hpp"
#include "gmlc/containers/BlockingPriorityQueue.hpp"
#include "helics/core/ActionMessage.hpp"

#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <utility>

namespace helics {

/** implementation of a generic communications interface
 */
class CommsInterface {
  public:
    /** enumeration of whether the threading system should generate a single thread or multiple
     * threads*/
    enum class thread_generation {
        single,  //!< indicate that a single thread is used for transmitting and receiving
        dual  //!< indicate that separate threads are used, 1 for transmission and 1 for reception
    };
    /** default constructor*/
    CommsInterface() = default;
    explicit CommsInterface(thread_generation threads);
    /** destructor*/
    virtual ~CommsInterface();

    /** load network information into the comms object*/
    virtual void loadNetworkInfo(const NetworkBrokerData& netInfo);
    void loadTargetInfo(std::string_view localTarget,
                        std::string_view brokerTarget,
                        gmlc::networking::InterfaceNetworks targetNetwork =
                            gmlc::networking::InterfaceNetworks::LOCAL);
    /** transmit a message along a particular route
     */
    void transmit(route_id rid, const ActionMessage& cmd);
    /** transmit a message along a particular route
     */
    void transmit(route_id rid, ActionMessage&& cmd);
    /** add a new route assigned to the appropriate id
     */
    void addRoute(route_id rid, std::string_view routeInfo);
    /** remove a route from use*/
    void removeRoute(route_id rid);
    /** connect the commsInterface
    @return true if the connection was successful false otherwise
    */
    bool connect();

    /** disconnected the comms interface
     */
    void disconnect();

    /** try reconnected from a mismatched or disconnection*/
    bool reconnect();
    /** set the name of the communicator*/
    void setName(const std::string& commName);

    /** set a flag indicating that a broker connection is required and all Connection
    fields are targeted at a broker*/
    void setRequireBrokerConnection(bool requireBrokerConnection);

    /** set the callback for processing the messages
     */
    void setCallback(std::function<void(ActionMessage&&)> callback);
    /** set the callback for processing the messages
     */
    void setLoggingCallback(
        std::function<void(int level, std::string_view name, std::string_view message)> callback);
    /** set the max message size and max Queue size
     */
    void setMessageSize(int maxMsgSize, int maxCount);
    /** check if the commInterface is connected
     */
    bool isConnected() const;

    /** set the timeout for the initial broker connection
    @param timeOut the value is in milliseconds
    */
    void setTimeout(std::chrono::milliseconds timeOut);
    /** set a flag for the comms system*/
    virtual void setFlag(std::string_view flag, bool val);
    /** enable or disable the server mode for the comms*/
    void setServerMode(bool serverActive);

    /** generate a log message as a warning*/
    void logWarning(std::string_view message) const;
    /** generate a log message as an error*/
    void logError(std::string_view message) const;
    /** generate a log message as a level above warning or error*/
    void logMessage(std::string_view message) const;

  protected:
    /// enumeration of the connection status flags for more immediate feedback from the processing
    /// threads
    enum class ConnectionStatus : int {

        STARTUP = -1,  //!< the connection is in STARTUP mode
        CONNECTED = 0,  //!< we are CONNECTED
        RECONNECTING = 1,  //!< we are trying reconnect
        TERMINATED = 2,  //!< the connection has been TERMINATED
        ERRORED = 4  //!< some ERRORED occurred on the connection
    };

  private:
    /// the status of the receiver thread
    std::atomic<ConnectionStatus> rxStatus{ConnectionStatus::STARTUP};

  protected:
    gmlc::concurrency::TriggerVariable rxTrigger;

    std::string name;  //!< the name of the object
    std::string localTargetAddress;  //!< the base for the receive address
    std::string brokerTargetAddress;  //!< the base for the broker address
    std::string brokerName;  //!< the identifier for the broker
    /// the initialization string for any automatically generated broker
    std::string brokerInitString;

  private:
    std::string randomID;  //!< randomized id for preventing crosstalk in some situations
    /// the status of the transmitter thread
    std::atomic<ConnectionStatus> txStatus{ConnectionStatus::STARTUP};
    gmlc::concurrency::TriggerVariable txTrigger;
    std::atomic<bool> operating{false};  //!< the comms interface is in STARTUP mode
    const bool singleThread{false};  //!< specify that the interface should operate a single thread

  protected:
    bool mRequireBrokerConnection{
        false};  //!< specify that the comms should assume we have a broker
    bool serverMode{true};  //!< some comms have a server mode and non-server mode
    bool autoBroker{false};  //!< the broker should be automatically generated if needed
    bool useJsonSerialization{false};  //!< true to make all connections use JSON serialization
    bool observer{false};  //!< true for connections that are for observation only
    /** timeout for the initial connection to a broker or to bind a broker port(in ms)*/
    std::chrono::milliseconds connectionTimeout{4000};
    int maxMessageSize = 16 * 1024;  //!< the maximum message size for the queues (if needed)
    int maxMessageCount = 512;  //!< the maximum number of message to buffer (if needed)
    std::atomic<bool> requestDisconnect{false};  //!< flag gets set when disconnect is called
    std::function<void(ActionMessage&&)>
        ActionCallback;  //!< the callback for what to do with a received message
    std::function<void(int level, std::string_view name, std::string_view message)>
        loggingCallback;  //!< callback for logging
    gmlc::containers::BlockingPriorityQueue<std::pair<route_id, ActionMessage>>
        txQueue;  //!< set of messages waiting to be transmitted
    // closing the files or connection can take some time so there is a need for inter-thread
    // communication to not spit out warning messages if it is in the process of disconnecting
    std::atomic<bool> disconnecting{
        false};  //!< flag indicating that the comm system is in the process of disconnecting
    gmlc::networking::InterfaceNetworks interfaceNetwork{
        gmlc::networking::InterfaceNetworks::LOCAL};

  private:
    std::thread queue_transmitter;  //!< single thread for sending data
    std::thread queue_watcher;  //!< thread monitoring the receive queue
    std::mutex threadSyncLock;  //!< lock to handle thread operations
    virtual void queue_rx_function() = 0;  //!< the functional loop for the receive queue
    virtual void queue_tx_function() = 0;  //!< the loop for transmitting data
    virtual void closeTransmitter();  //!< function to instruct the transmitter loop to close
    virtual void closeReceiver();  //!< function to instruct the receiver loop to close
    virtual void reconnectTransmitter();  //!< function to reconnect the transmitter
    virtual void reconnectReceiver();  //!< function to reconnect the receiver
  protected:
    void setTxStatus(ConnectionStatus status);
    void setRxStatus(ConnectionStatus status);
    ConnectionStatus getRxStatus() const { return rxStatus.load(); }
    ConnectionStatus getTxStatus() const { return txStatus.load(); }
    /** function to protect certain properties in a threaded environment
    these functions should be called in a pair*/
    bool propertyLock();
    void propertyUnLock();
    /** function to join the processing threads*/
    void join_tx_rx_thread();
    /** get the generated randomID for this comm interface*/
    const std::string& getRandomID() const { return randomID; }

  private:
    gmlc::concurrency::TripWireDetector
        tripDetector;  //!< try to detect if everything is shutting down
};

namespace CommFactory {
    /** builder for Comm Objects*/
    class CommBuilder {
      public:
        virtual std::unique_ptr<CommsInterface> build() = 0;
    };

    /** template for making a Core builder*/
    template<class CommTYPE>
    class CommTypeBuilder final: public CommBuilder {
      public:
        static_assert(std::is_base_of<CommsInterface, CommTYPE>::value,
                      "Type does not inherit from helics::CommsInterface");

        using comm_build_type = CommTYPE;
        virtual std::unique_ptr<CommsInterface> build() override
        {
            return std::make_unique<CommTYPE>();
        }
    };

    /** define a new Comm Builder from the builder give a name and build code*/
    void defineCommBuilder(std::shared_ptr<CommBuilder> builder,
                           std::string_view commTypeName,
                           int code);

    /** template function to create a builder and link it into the library*/
    template<class CommTYPE>
    std::shared_ptr<CommBuilder> addCommType(std::string_view commTypeName, int code)
    {
        auto bld = std::make_shared<CommTypeBuilder<CommTYPE>>();
        std::shared_ptr<CommBuilder> cbld = std::static_pointer_cast<CommBuilder>(bld);
        defineCommBuilder(cbld, commTypeName, code);
        return cbld;
    }

    std::unique_ptr<CommsInterface> create(CoreType type);
    std::unique_ptr<CommsInterface> create(std::string_view type);

}  // namespace CommFactory

template<class X>
class ConditionalChangeOnDestroy {
  private:
    std::atomic<X>& aref;
    X fval;
    X expectedValue;

  public:
    ConditionalChangeOnDestroy(std::atomic<X>& var, X finalValue, X expValue):
        aref(var), fval(std::move(finalValue)), expectedValue(std::move(expValue))
    {
    }
    ~ConditionalChangeOnDestroy() { aref.compare_exchange_strong(expectedValue, fval); }
};

}  // namespace helics
