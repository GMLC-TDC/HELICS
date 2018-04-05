
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


def AddFederate(broker, core_type="zmq", count=1, time_delta=1.0, name_prefix="fed"):

    initstring = "--broker="
    initstring = initstring + h.helicsBrokerGetIdentifier(broker)
    initstring = initstring + " --broker_address" + h.helicsBrokerGetAddress(broker)

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

    yield mFed

    status = h.helicsFederateFinalize(mFed)

    status, state = h.helicsFederateGetState(mFed)
    assert state == 3

    while (h.helicsBrokerIsConnected(broker)):
        time.sleep(1)

    h.helicsFederateFree(mFed)
    h.helicsCloseLibrary()

@pt.fixture()
def broker():
    broker = AddBroker("zmq", 1)
    return broker

def test_broker_functions(broker):

    initstring = "--broker="
    initstring = initstring + h.helicsBrokerGetIdentifier(broker)
    initstring = initstring + " --broker_address" + h.helicsBrokerGetAddress(broker)


