
import time
import pytest as pt
import helics as h

def AddBroker(core_type="zmq", number_of_federates=1):

    initstring = "{} --name=mainbroker".format(number_of_federates)
    deltat = 0.01

    helicsversion = h.helicsGetVersion()
    print("HELICS version = {}".format(helicsversion))

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
    status = h.helicsFederateInfoSetFederateName(fedinfo, name_prefix)
    assert status == 0

    # Set core type from string #
    status = h.helicsFederateInfoSetCoreTypeFromString(fedinfo, "zmq")
    assert status == 0

    # Federate init string #
    fedinitstring = "--broker=mainbroker --federates={}".format(count)
    status = h.helicsFederateInfoSetCoreInitString(fedinfo, fedinitstring)
    assert status == 0

    # Set the message interval (timedelta) for federate. Note th#
    # HELICS minimum message time interval is 1 ns and by default
    # it uses a time delta of 1 second. What is provided to the
    # setTimedelta routine is a multiplier for the default timedelta.

    # Set one second message interval #
    status = h.helicsFederateInfoSetTimeDelta(fedinfo, deltat)
    assert status == 0

    status = h.helicsFederateInfoSetLoggingLevel(fedinfo, 1)
    assert status == 0

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

    f1 = h.helicsFederateRegisterSourceFilter (fFed, h.helics_custom_filter, "port1", "filter1")
    f2 = h.helicsFederateRegisterDestinationFilter (fFed, h.helics_custom_filter, "port2", "filter2")
    ep1 = h.helicsFederateRegisterEndpoint (fFed, "fout", "")
    f3 = h.helicsFederateRegisterSourceFilter (fFed, h.helics_custom_filter,  "filter0/fout", "")
    status=h.helicsFederateEnterExecutionModeAsync(fFed)
    assert status == 0
    status=h.helicsFederateEnterExecutionMode(mFed)
    assert status == 0
    status=h.helicsFederateEnterExecutionModeComplete(fFed)
    assert status == 0

    status, filter_name = h.helicsFilterGetName(f1)
    assert status == 0
    assert filter_name == "filter1"

    status, filter_name = h.helicsFilterGetName(f2)
    assert status == 0
    assert filter_name == "filter2"

    status, filter_target = h.helicsFilterGetTarget(f2)
    assert status == 0
    assert filter_target == "port2"

    h.helicsFederateFinalize(mFed)
    h.helicsFederateFinalize(fFed)

    FreeFederate(fFed)
    FreeFederate(mFed)
    time.sleep(1.0)

def test_message_filter_function(broker):

    fFed = AddFederate(broker, "zmq", 1, 1, "filter")
    mFed = AddFederate(broker, "zmq", 1, 1, "message")

    p1 = h.helicsFederateRegisterGlobalEndpoint(mFed, "port1", "")
    p2 = h.helicsFederateRegisterGlobalEndpoint(mFed, "port2", "")

    f1 = h.helicsFederateRegisterSourceFilter (fFed, h.helics_delay_filter, "port1", "filter1")
    status=h.helicsFilterSet(f1, "delay", 2.5)
    assert status == 0
    status=h.helicsFederateEnterExecutionModeAsync(fFed)
    assert status == 0
    status=h.helicsFederateEnterExecutionMode(mFed)
    assert status == 0
    status=h.helicsFederateEnterExecutionModeComplete(fFed)
    assert status == 0
    status, state = h.helicsFederateGetState(fFed)
    assert status == 0
    assert state == 2
    data = "hello world"
    h.helicsEndpointSendMessageRaw(p1, "port2", data)
    status = h.helicsFederateRequestTimeAsync (mFed, 1.0)
    assert status == h.helics_ok
    status, grantedtime = h.helicsFederateRequestTime(fFed, 1.0)
    assert status == 0
    assert grantedtime == 1.0
    status, grantedtime = h.helicsFederateRequestTimeComplete (mFed)
    assert status == 0
    assert grantedtime == 1.0
    res=h.helicsFederateHasMessage(mFed)
    assert res==0
    #status, grantedtime = h.helicsFederateRequestTime(fFed, 3.0)	
    #res=h.helicsendpointHasMessage(p2)
    #assert res==h.helics_true	

    h.helicsFederateFinalize(mFed)
    h.helicsFederateFinalize(fFed)
    #f2 = h.helicsFederateRegisterDestinationFilter (fFed, h.helics_custom_filter, "filter2", "port2")
    #ep1 = h.helicsFederateRegisterEndpoint (fFed, "fout", "")
    #f3 = h.helicsFederateRegisterSourceFilter (fFed, h.helics_custom_filter, "", "filter0/fout")

    FreeFederate(fFed)
    FreeFederate(mFed)
    time.sleep(1.0)
