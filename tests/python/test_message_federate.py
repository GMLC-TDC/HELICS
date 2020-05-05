import time
import pytest as pt
import helics as h


@pt.fixture
def mFed():
    initstring = "-f 1 --name=mainbroker"
    fedinitstring = "--broker=mainbroker --federates=1"
    deltat = 0.01

    h.helicsGetVersion()

    # Create broker #
    broker = h.helicsCreateBroker("zmq", "", initstring)

    isconnected = h.helicsBrokerIsConnected(broker)

    if isconnected == 1:
        pass

    # Create Federate Info object that describes the federate properties #
    fedinfo = h.helicsCreateFederateInfo()

    # Set Federate name #
    h.helicsFederateInfoSetCoreName(fedinfo, "CoreA Federate")

    # Set core type from string #
    h.helicsFederateInfoSetCoreTypeFromString(fedinfo, "zmq")

    # Federate init string #
    h.helicsFederateInfoSetCoreInitString(fedinfo, fedinitstring)

    # Set the message interval (timedelta) for federate. Note th#
    # HELICS minimum message time interval is 1 ns and by default
    # it uses a time delta of 1 second. What is provided to the
    # setTimedelta routine is a multiplier for the default timedelta.

    # Set one second message interval #
    h.helicsFederateInfoSetTimeProperty(fedinfo, h.helics_property_time_delta, deltat)

    h.helicsFederateInfoSetIntegerProperty(fedinfo, h.helics_property_int_log_level, 1)

    mFed = h.helicsCreateMessageFederate("TestA Federate", fedinfo)

    yield mFed

    h.helicsFederateFinalize(mFed)
    state = h.helicsFederateGetState(mFed)
    assert state == 3
    while h.helicsBrokerIsConnected(broker):
        time.sleep(1)

    h.helicsFederateInfoFree(fedinfo)
    h.helicsFederateFree(mFed)
    h.helicsCloseLibrary()


def test_message_federate_initialize(mFed):
    state = h.helicsFederateGetState(mFed)
    assert state == 0
    h.helicsFederateEnterExecutingMode(mFed)

    state = h.helicsFederateGetState(mFed)
    assert state == 2


def test_message_federate_endpoint_registration(mFed):
    epid1 = h.helicsFederateRegisterEndpoint(mFed, "ep1", None)
    epid2 = h.helicsFederateRegisterGlobalEndpoint(mFed, "ep2", "random")

    h.helicsFederateEnterExecutingMode(mFed)

    endpoint_name = h.helicsEndpointGetName(epid1)
    assert endpoint_name == "TestA Federate/ep1"

    endpoint_name = h.helicsEndpointGetName(epid2)
    assert endpoint_name == "ep2"

    endpoint_name = h.helicsEndpointGetType(epid1)
    assert endpoint_name == ""

    endpoint_name = h.helicsEndpointGetType(epid2)
    assert endpoint_name == "random"


def test_message_federate_send(mFed):
    epid1 = h.helicsFederateRegisterEndpoint(mFed, "ep1", None)
    epid2 = h.helicsFederateRegisterGlobalEndpoint(mFed, "ep2", "random")

    h.helicsFederateSetTimeProperty(mFed, h.helics_property_time_delta, 1.0)
    h.helicsFederateEnterExecutingMode(mFed)

    data = "random-data"

    h.helicsEndpointSendEventRaw(epid1, "ep2", data, 1.0)

    granted_time = h.helicsFederateRequestTime(mFed, 2.0)
    assert granted_time == 1.0

    res = h.helicsFederateHasMessage(mFed)
    assert res == 1

    res = h.helicsEndpointHasMessage(epid1)
    assert res == 0

    res = h.helicsEndpointHasMessage(epid2)
    assert res == 1

    message = h.helicsEndpointGetMessage(epid2)

    assert message.data == "random-data"
    assert message.length == 11
    assert message.original_dest == ""
    assert message.original_source == "TestA Federate/ep1"
    assert message.source == "TestA Federate/ep1"
    assert message.time == 1.0
