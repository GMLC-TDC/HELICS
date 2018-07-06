import time
import pytest as pt
import helics as h

@pt.fixture
def mFed():
    initstring = "1 --name=mainbroker"
    fedinitstring = "--broker=mainbroker --federates=1"
    deltat = 0.01

    h.helicsGetVersion()

    # Create broker #
    broker = h.helicsCreateBroker("zmq", "", initstring)

    isconnected = h.helicsBrokerIsConnected(broker)

    if isconnected == 1:
        pass

    # Create Federate Info object that describes the federate properties #
    fedinfo = h.helicsFederateInfoCreate()

    # Set Federate name #
    h.helicsFederateInfoSetFederateName(fedinfo, "TestA Federate")

    # Set core type from string #
    h.helicsFederateInfoSetCoreTypeFromString(fedinfo, "zmq")

    # Federate init string #
    h.helicsFederateInfoSetCoreInitString(fedinfo, fedinitstring)

    # Set the message interval (timedelta) for federate. Note th#
    # HELICS minimum message time interval is 1 ns and by default
    # it uses a time delta of 1 second. What is provided to the
    # setTimedelta routine is a multiplier for the default timedelta.

    # Set one second message interval #
    h.helicsFederateInfoSetTimeDelta(fedinfo, deltat)

    h.helicsFederateInfoSetLoggingLevel(fedinfo, 1)

    mFed = h.helicsCreateMessageFederate(fedinfo)

    yield mFed

    status = h.helicsFederateFinalize(mFed)
    assert status == h.helics_ok
    status, state = h.helicsFederateGetState(mFed)
    assert state == 3
    assert status == h.helics_ok
    while (h.helicsBrokerIsConnected(broker)):
        time.sleep(1)

    h.helicsFederateFree(mFed)
    h.helicsCloseLibrary()


def test_message_federate_initialize(mFed):
    status, state = h.helicsFederateGetState(mFed)
    assert state == 0
    assert status == 0
    h.helicsFederateEnterExecutionMode(mFed)

    status, state = h.helicsFederateGetState(mFed)
    assert state == 2

def test_message_federate_endpoint_registration(mFed):
    epid1 = h.helicsFederateRegisterEndpoint(mFed, "ep1", None)
    epid2 = h.helicsFederateRegisterGlobalEndpoint(mFed, "ep2", "random")

    h.helicsFederateEnterExecutionMode(mFed)

    status, endpoint_name = h.helicsEndpointGetName(epid1)
    assert status == 0
    assert endpoint_name == "TestA Federate/ep1"

    status, endpoint_name = h.helicsEndpointGetName(epid2)
    assert status == 0
    assert endpoint_name == "ep2"

    status, endpoint_name = h.helicsEndpointGetType(epid1)
    assert status == 0
    assert endpoint_name == ""

    status, endpoint_name = h.helicsEndpointGetType(epid2)
    assert status == 0
    assert endpoint_name == "random"


def test_message_federate_send(mFed):
    epid1 = h.helicsFederateRegisterEndpoint(mFed, "ep1", None)
    epid2 = h.helicsFederateRegisterGlobalEndpoint(mFed, "ep2", "random")

    h.helicsFederateSetTimeDelta(mFed, 1.0)
    h.helicsFederateEnterExecutionMode(mFed)

    data = "random-data"

    status = h.helicsEndpointSendEventRaw(epid1, "ep2", data, 1.0)

    status, granted_time = h.helicsFederateRequestTime(mFed, 2.0)
    assert status == 0
    assert granted_time == 1.0

    res = h.helicsFederateHasMessage(mFed)
    assert res == 1

    res = h.helicsEndpointHasMessage(epid1)
    assert res == 0

    res = h.helicsEndpointHasMessage(epid2)
    assert res == 1

    message = h.helicsEndpointGetMessage(epid2)

    assert message.data == 'random-data'
    assert message.length == 11
    assert message.original_dest == ''
    assert message.original_source == 'TestA Federate/ep1'
    assert message.source == 'TestA Federate/ep1'
    assert message.time == 1.0

