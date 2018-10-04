/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "../common/BlockingPriorityQueue.hpp"
#include "../common/TriggerVariable.hpp"
#include "../common/TripWire.hpp"
#include "ActionMessage.hpp"
#include "NetworkBrokerData.hpp"
#include <functional>
#include <thread>

namespace helics
{
enum class interface_networks : char;

constexpr int control_route = (-1);
  /** implementation of a generic communications interface
 */
class CommsInterface
{
  public:
    /** default constructor*/
    CommsInterface () = default;
    /** destructor*/
    virtual ~CommsInterface ();

	/** load network information into the comms object*/
    virtual void loadNetworkInfo (const NetworkBrokerData &netInfo);
	void loadTargetInfo(const std::string &localTarget,
                    const std::string &brokerTarget,
                    interface_networks targetNetwork = interface_networks::local);
    /** transmit a message along a particular route
     */
    void transmit (int route_id, const ActionMessage &cmd);
    /** transmit a message along a particular route
     */
    void transmit (int route_id, ActionMessage &&cmd);
    /** add a new route assigned to the appropriate id
     */
    void addRoute (int route_id, const std::string &routeInfo);
    /** remove a route from use*/
    void removeRoute(int route_id);
    /** connect the commsInterface
    @return true if the connection was successful false otherwise
    */
    bool connect ();

    /** disconnected the comms interface
     */
    void disconnect ();

    /** try reconnected from a mismatched or disconnection*/
    bool reconnect ();
    /** set the name of the communicator*/
    void setName (const std::string &name);
    /** set the callback for processing the messages
     */
    void setCallback (std::function<void(ActionMessage &&)> callback);
    /** set the callback for processing the messages
     */
    void setLoggingCallback (
      std::function<void(int level, const std::string &name, const std::string &message)> callback);
    /** set the max message size and max Queue size
     */
    void setMessageSize (int maxMessageSize, int maxMessageCount);
    /** check if the commInterface is connected
     */
    bool isConnected () const;

    /** set the timeout for the initial broker connection
    @param timeout the value is in milliseconds
    */
	void setTimeout(int timeout);

	/** enable or disable the server more for the comms*/
	void setServerMode(bool serverActive);

  protected:
    void logWarning (const std::string &message) const;
    void logError (const std::string &message) const;
  protected:
    // enumeration of the connection status flags for more immediate feedback from the processing threads
    enum class connection_status : int
    {

        startup = -1,  //!< the connection is in startup mode
        connected = 0,  //!< we are connected
        reconnecting = 1,  //!< we are trying reconnect
        terminated = 2,  //!< the connection has been terminated
        error = 4,  //!< some error occurred on the connection

    };

  private:
    std::atomic<connection_status> rx_status{connection_status::startup};  //!< the status of the receiver thread
  protected:
    TriggerVariable rxTrigger;

    std::string name;  //!< the name of the object
    std::string localTarget_;  //!< the base for the receive address
    std::string brokerTarget_;  //!< the base for the broker address
    std::string brokerName_;  //!< the identifier for the broker
  private:
    std::atomic<connection_status> tx_status{
      connection_status::startup};  //!< the status of the transmitter thread
    TriggerVariable txTrigger;
    std::atomic<bool> operating;  //!< the comms interface is in startup mode
  protected:
	 bool serverMode = true;  //!< some comms have a server mode and non-server mode
    int connectionTimeout =
      4000;  // timeout for the initial connection to a broker or to bind a broker port(in ms)
    int maxMessageSize_ = 16 * 1024;  //!< the maximum message size for the queues (if needed)
    int maxMessageCount_ = 512;  //!< the maximum number of message to buffer (if needed)

    std::function<void(ActionMessage &&)> ActionCallback;  //!< the callback for what to do with a received message
    std::function<void(int level, const std::string &name, const std::string &message)>
      loggingCallback;  //!< callback for logging
    BlockingPriorityQueue<std::pair<int, ActionMessage>> txQueue;  //!< set of messages waiting to be transmitted
    // closing the files or connection can take some time so there is a need for inter-thread communication to not
    // spit out warning messages if it is in the process of disconnecting
    std::atomic<bool> disconnecting{
      false};  //!< flag indicating that the comm system is in the process of disconnecting
    interface_networks interfaceNetwork;

  private:
    std::thread queue_transmitter;  //!< single thread for sending data
    std::thread queue_watcher;  //!< thread monitoring the receive queue
    std::mutex threadSyncLock;  //!< lock to handle thread operations
    virtual void queue_rx_function () = 0;  //!< the functional loop for the receive queue
    virtual void queue_tx_function () = 0;  //!< the loop for transmitting data
    virtual void closeTransmitter ();  //!< function to instruct the transmitter loop to close
    virtual void closeReceiver () = 0;  //!< function to instruct the receiver loop to close
    virtual void reconnectTransmitter ();  //!< function to reconnect the transmitter
    virtual void reconnectReceiver ();  //!< function to reconnect the receiver
  protected:
    void setTxStatus (connection_status txStatus);
    void setRxStatus (connection_status rxStatus);
    connection_status getRxStatus () const { return rx_status.load (); }
    connection_status getTxStatus () const { return tx_status.load (); }
    /** function to protect certain properties in a threaded environment
	these functions should be called in a pair*/
    bool propertyLock ();
    void propertyUnLock ();

  private:
    tripwire::TripWireDetector tripDetector;  //!< try to detect if everything is shutting down
};

template <class X>
class conditionalChangeOnDestroy
{
  private:
    std::atomic<X> &aref;
    X fval;
    X expectedValue;

  public:
    conditionalChangeOnDestroy (std::atomic<X> &var, X finalValue, X expValue)
        : aref (var), fval (std::move (finalValue)), expectedValue (std::move (expValue))
    {
    }
    ~conditionalChangeOnDestroy () { aref.compare_exchange_strong (expectedValue, fval); }
};

}  // namespace helics
