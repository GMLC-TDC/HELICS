import pytest as pt
import helics as h

def test_value_federate_initialize_test(vFed):
    status, state = h.helicsFederateGetState(vFed)
    assert state == 0

    h.helicsFederateEnterExecutionMode(vFed)

    status, state = h.helicsFederateGetState(vFed)
    assert state == 2

def test_value_federate_initialize_test(vFed):
    pubid1 = h.helicsFederateRegisterPublication(vFed, "pub1", "string", "");
    pubid2 = h.helicsFederateRegisterGlobalPublication(vFed, "pub2", "int", "");
    pubid3 = h.helicsFederateRegisterPublication(vFed, "pub3", "double", "V");
    h.helicsFederateEnterExecutionMode(vFed);


@pt.fixture
def vFed():
    import helics as h

    initstring = "1 --name=mainbroker"
    fedinitstring = "--broker=mainbroker --federates=1"
    deltat = 0.01

    helicsversion = h.helicsGetVersion()

    print("PI SENDER: Helics version = {}".format(helicsversion))

    # Create broker #
    print("Creating Broker")
    broker = h.helicsCreateBroker("zmq", "", initstring)
    print("Created Broker")

    print("Checking if Broker is connected")
    isconnected = h.helicsBrokerIsConnected(broker)
    print("Checked if Broker is connected")

    if isconnected == 1:
        print("Broker created and connected")

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

    vFed = h.helicsCreateValueFederate(fedinfo)
