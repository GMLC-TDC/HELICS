import time
import pytest as pt
import helics as h


def AddBroker(core_type="zmq", number_of_federates=1):

    initstring = "-f {} --name=mainbroker".format(number_of_federates)

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
    fedinfo = h.helicsCreateFederateInfo()

    # Set Federate name #
    h.helicsFederateInfoSetCoreName(fedinfo, name_prefix)

    # Set core type from string #
    h.helicsFederateInfoSetCoreTypeFromString(fedinfo, "zmq")

    # Federate init string #
    fedinitstring = "--broker=mainbroker --federates={}".format(count)
    h.helicsFederateInfoSetCoreInitString(fedinfo, fedinitstring)

    # Set the message interval (timedelta) for federate. Note th#
    # HELICS minimum message time interval is 1 ns and by default
    # it uses a time delta of 1 second. What is provided to the
    # setTimedelta routine is a multiplier for the default timedelta.

    # Set one second message interval #
    h.helicsFederateInfoSetTimeProperty(fedinfo, h.helics_property_time_delta, deltat)

    h.helicsFederateInfoSetIntegerProperty(fedinfo, h.helics_property_int_log_level, 1)

    mFed = h.helicsCreateMessageFederate(name_prefix, fedinfo)

    return mFed, fedinfo


def FreeFederate(fed, fedinfo):
    h.helicsFederateFinalize(fed)
    state = h.helicsFederateGetState(fed)
    assert state == 3  # TODO: should this be 3?

    h.helicsFederateInfoFree(fedinfo)
    h.helicsFederateFree(fed)


@pt.fixture()
def broker():
    broker = AddBroker("zmq", 1)
    yield broker
    h.helicsBrokerDisconnect(broker)
    h.helicsCloseLibrary()


def test_broker_functions(broker):

    initstring = "--broker="
    identifier = h.helicsBrokerGetIdentifier(broker)
    initstring = initstring + identifier
    initstring = initstring + " --broker_address"
    address = h.helicsBrokerGetAddress(broker)
    initstring = initstring + address


def test_message_filter_registration(broker):

    fFed, ffedinfo = AddFederate(broker, "zmq", 1, 1, "filter")
    mFed, mfedinfo = AddFederate(broker, "zmq", 1, 1, "message")

    h.helicsFederateRegisterGlobalEndpoint(mFed, "port1", "")
    h.helicsFederateRegisterGlobalEndpoint(mFed, "port2", None)

    f1 = h.helicsFederateRegisterFilter(fFed, h.helics_filter_type_custom, "filter1")
    f2 = h.helicsFederateRegisterFilter(fFed, h.helics_filter_type_custom, "filter2")
    h.helicsFederateRegisterEndpoint(fFed, "fout", "")
    h.helicsFederateRegisterFilter(fFed, h.helics_filter_type_custom, "filter0/fout")
    h.helicsFederateEnterExecutingModeAsync(fFed)
    h.helicsFederateEnterExecutingMode(mFed)
    h.helicsFederateEnterExecutingModeComplete(fFed)

    filter_name = h.helicsFilterGetName(f1)
    assert filter_name == "filter/filter1"

    filter_name = h.helicsFilterGetName(f2)
    assert filter_name == "filter/filter2"

    # filter_target = h.helicsFilterGetTarget(f2)
    # assert filter_target == "port2"

    h.helicsFederateFinalize(mFed)
    h.helicsFederateFinalize(fFed)

    FreeFederate(fFed, ffedinfo)
    FreeFederate(mFed, mfedinfo)
    time.sleep(1.0)


def test_message_filter_function(broker):

    fFed, ffedinfo = AddFederate(broker, "zmq", 1, 1, "filter")
    mFed, mfedinfo = AddFederate(broker, "zmq", 1, 1, "message")

    p1 = h.helicsFederateRegisterGlobalEndpoint(mFed, "port1", "")
    p2 = h.helicsFederateRegisterGlobalEndpoint(mFed, "port2", "random")

    f1 = h.helicsFederateRegisterGlobalFilter(fFed, h.helics_filter_type_custom, "filter1")
    h.helicsFilterAddSourceTarget(f1, "port1")
    f2 = h.helicsFederateRegisterGlobalFilter(fFed, h.helics_filter_type_delay, "filter2")
    h.helicsFilterAddSourceTarget(f2, "port1")
    h.helicsFederateRegisterEndpoint(fFed, "fout", "")
    f3 = h.helicsFederateRegisterFilter(fFed, h.helics_filter_type_random_delay, "filter3")
    h.helicsFilterAddSourceTarget(f3, "filter/fout")

    h.helicsFilterSet(f2, "delay", 2.5)
    h.helicsFederateEnterExecutingModeAsync(fFed)
    h.helicsFederateEnterExecutingMode(mFed)
    h.helicsFederateEnterExecutingModeComplete(fFed)
    state = h.helicsFederateGetState(fFed)
    assert state == 2
    data = "hello world"

    filt_key = h.helicsFilterGetName(f1)
    assert filt_key == "filter1"

    filt_key = h.helicsFilterGetName(f2)
    assert filt_key == "filter2"

    h.helicsEndpointSendMessageRaw(p1, "port2", data)
    h.helicsFederateRequestTimeAsync(mFed, 1.0)
    grantedtime = h.helicsFederateRequestTime(fFed, 1.0)
    assert grantedtime == 1.0
    grantedtime = h.helicsFederateRequestTimeComplete(mFed)
    assert grantedtime == 1.0
    res = h.helicsFederateHasMessage(mFed)
    assert res == 0
    res = h.helicsEndpointHasMessage(p2)
    assert res == 0
    # grantedtime = h.helicsFederateRequestTime(fFed, 3.0)
    # assert res==h.helics_true

    h.helicsFederateFinalize(mFed)
    h.helicsFederateFinalize(fFed)
    # f2 = h.helicsFederateRegisterDestinationFilter (fFed, h.helics_custom_filter, "filter2", "port2")
    # ep1 = h.helicsFederateRegisterEndpoint (fFed, "fout", "")
    # f3 = h.helicsFederateRegisterSourceFilter (fFed, h.helics_custom_filter, "", "filter0/fout")

    FreeFederate(fFed, ffedinfo)
    FreeFederate(mFed, mfedinfo)
    time.sleep(1.0)
