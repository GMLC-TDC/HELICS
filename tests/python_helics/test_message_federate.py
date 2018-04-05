import time
import pytest as pt
import helics as h

@pt.fixture
def mFed():
    import helics as h

    initstring = "1 --name=mainbroker"
    fedinitstring = "--broker=mainbroker --federates=1"
    deltat = 0.01

    helicsversion = h.helicsGetVersion()

    # Create broker #
    broker = h.helicsCreateBroker("zmq", "", initstring)

    isconnected = h.helicsBrokerIsConnected(broker)

    if isconnected == 1:
        pass

    # Create Federate Info object that describes the federate properties #
    fedinfo = h.helicsFederateInfoCreate()

    # Set Federate name #
    status = h.helicsFederateInfoSetFederateName(fedinfo, "TestA Federate")

    # Set core type from string #
    status = h.helicsFederateInfoSetCoreTypeFromString(fedinfo, "zmq")

    # Federate init string #
    status = h.helicsFederateInfoSetCoreInitString(fedinfo, fedinitstring)

    # Set the message interval (timedelta) for federate. Note th#
    # HELICS minimum message time interval is 1 ns and by default
    # it uses a time delta of 1 second. What is provided to the
    # setTimedelta routine is a multiplier for the default timedelta.

    # Set one second message interval #
    status = h.helicsFederateInfoSetTimeDelta(fedinfo, deltat)

    status = h.helicsFederateInfoSetLoggingLevel(fedinfo, 1)

    mFed = h.helicsCreateMessageFederate(fedinfo)

    yield mFed

    status = h.helicsFederateFinalize(mFed)

    status, state = h.helicsFederateGetState(mFed)
    assert state == 3

    while (h.helicsBrokerIsConnected(broker)):
        time.sleep(1)

    h.helicsFederateFree(mFed)
    h.helicsCloseLibrary()


def test_message_federate_initialize(mFed):
    status, state = h.helicsFederateGetState(mFed)
    assert state == 0

    h.helicsFederateEnterExecutionMode(mFed)

    status, state = h.helicsFederateGetState(mFed)
    assert state == 2

def test_message_federate_endpoint_registration(mFed):
    epid1 = h.helicsFederateRegisterEndpoint(mFed, "ep1", None)
    epid2 = h.helicsFederateRegisterGlobalEndpoint(mFed, "ep2", "random")

    h.helicsFederateEnterExecutionMode(mFed)

    status, endpoint_name = h.helicsEndpointGetName(epid1, 100)
    assert status == 0
    assert endpoint_name == "TestA Federate/ep1"

    status, endpoint_name = h.helicsEndpointGetName(epid2, 100)
    assert status == 0
    assert endpoint_name == "ep2"

    status, endpoint_name = h.helicsEndpointGetType(epid1, 100)
    assert status == 0
    assert endpoint_name == ""

    status, endpoint_name = h.helicsEndpointGetType(epid2, 100)
    assert status == 0
    assert endpoint_name == "random"


def test_message_federate_endpoint_registration(mFed):
    epid1 = h.helicsFederateRegisterEndpoint(mFed, "ep1", None)
    epid2 = h.helicsFederateRegisterGlobalEndpoint(mFed, "ep2", "random")

    h.helicsFederateSetTimeDelta(mFed, 1.0)
    h.helicsFederateEnterExecutionMode(mFed)

    data = "random-data"

    status = h.helicsEndpointSendEventRaw(epid1, "ep2", data, len(data))

    status, granted_time = h.helicsFederateRequestTime(mFed, 1.0)

    assert granted_time == 1.0

    res = h.helicsFederateHasMessage (mFed)
    assert res == 0

    res = h.helicsEndpointHasMessage (epid1)
    # TODO: Figure out why this is returning zero
    assert res != 0

    res = h.helicsEndpointHasMessage (epid2)
    assert res == 0

    # This causes a segfault
    res = h.helicsEndpointGetMessage(epid2)
    assert res == 0

