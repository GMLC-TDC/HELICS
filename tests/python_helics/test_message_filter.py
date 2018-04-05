
import time
import pytest as pt
import helics as h

def AddBroker(core_type="zmq", number_of_federates=1):

    initstring = "{} --name=mainbroker".format(number_of_federates)
    deltat = 0.01

    helicsversion = h.helicsGetVersion()

    # Create broker #
    broker = h.helicsCreateBroker(core_type, "", initstring)

    isconnected = h.helicsBrokerIsConnected(broker)

    if isconnected == 1:
        pass

    return broker

def AddFederate(broker, core_type="zmq", count=1, deltat=1.0, name_prefix="fed"):

    # Create Federate Info object that describes the federate properties #
    fedinfo = h.helicsFederateInfoCreate()

    # Set Federate name #
    status = h.helicsFederateInfoSetFederateName(fedinfo, "TestA Federate")

    # Set core type from string #
    status = h.helicsFederateInfoSetCoreTypeFromString(fedinfo, "zmq")

    # Federate init string #
    fedinitstring = "--broker=mainbroker --federates={}".format(count)
    status = h.helicsFederateInfoSetCoreInitString(fedinfo, fedinitstring)

    # Set the message interval (timedelta) for federate. Note th#
    # HELICS minimum message time interval is 1 ns and by default
    # it uses a time delta of 1 second. What is provided to the
    # setTimedelta routine is a multiplier for the default timedelta.

    # Set one second message interval #
    status = h.helicsFederateInfoSetTimeDelta(fedinfo, deltat)

    status = h.helicsFederateInfoSetLoggingLevel(fedinfo, 1)

    mFed = h.helicsCreateMessageFederate(fedinfo)

    return mFed

def FreeFederate(fed):
    status = h.helicsFederateFinalize(fed)

    status, state = h.helicsFederateGetState(fed)
    assert state == 3

    h.helicsFederateFree(fed)


@pt.fixture()
def broker():
    broker = AddBroker("zmq", 1)
    yield broker
    h.helicsBrokerDisconnect(broker)
    h.helicsCloseLibrary()

def test_broker_functions(broker):

    initstring = "--broker="
    status, identifier = h.helicsBrokerGetIdentifier(broker)
    assert status == 0
    initstring = initstring + identifier
    initstring = initstring + " --broker_address"
    status, address = h.helicsBrokerGetAddress(broker)
    assert status == 0
    initstring = initstring + address

def test_message_filter_registration(broker):

    fFed = AddFederate(broker, "zmq", 1, 1, "filter")
    mFed = AddFederate(broker, "zmq", 1, 1, "message")

    h.helicsFederateRegisterGlobalEndpoint(mFed, "port1", "")
    h.helicsFederateRegisterGlobalEndpoint(mFed, "port2", None)

    f1 = h.helicsFederateRegisterSourceFilter (fFed, h.helics_custom_filter, "filter1", "port1")
    f2 = h.helicsFederateRegisterDestinationFilter (fFed, h.helics_custom_filter, "filter2", "port2")
    ep1 = h.helicsFederateRegisterEndpoint (fFed, "fout", "")
    f3 = h.helicsFederateRegisterSourceFilter (fFed, h.helics_custom_filter, "", "filter0/fout")

    FreeFederate(fFed)
    FreeFederate(mFed)

def test_message_filter_function(broker):

    fFed = AddFederate(broker, "zmq", 1, 1, "filter")
    mFed = AddFederate(broker, "zmq", 1, 1, "message")

    p1 = h.helicsFederateRegisterGlobalEndpoint(mFed, "port1", "")
    p2 = h.helicsFederateRegisterGlobalEndpoint(mFed, "port2", "")

    f1 = h.helicsFederateRegisterSourceFilter (fFed, h.helics_delay_filter, "port1", "filter1")
    h.helicsFilterSet(f1, "delay", 2.5)

    h.helicsFederateEnterExecutionModeAsync(fFed)
    h.helicsFederateEnterExecutionMode(mFed)
    h.helicsFederateEnterExecutionModeComplete(fFed)

    status, state = h.helicsFederateGetState(fFed)
    assert state == 2

    data = "hello world"
    # TODO: Fix segfaults on the next line
    # h.helicsEndpointSendMessageRaw(p1, "port2", data)

    # f2 = h.helicsFederateRegisterDestinationFilter (fFed, h.helics_custom_filter, "filter2", "port2")
    # ep1 = h.helicsFederateRegisterEndpoint (fFed, "fout", "")
    # f3 = h.helicsFederateRegisterSourceFilter (fFed, h.helics_custom_filter, "", "filter0/fout")

    FreeFederate(fFed)
    FreeFederate(mFed)



