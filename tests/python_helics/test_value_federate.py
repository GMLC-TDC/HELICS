import time
import pytest as pt
import helics as h

@pt.fixture
def vFed():
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
    assert status == 0

    # Set core type from string #
    status = h.helicsFederateInfoSetCoreTypeFromString(fedinfo, "zmq")
    assert status == 0

    # Federate init string #
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

    vFed = h.helicsCreateValueFederate(fedinfo)

    yield vFed

    status = h.helicsFederateFinalize(vFed)
    assert status == 0

    status, state = h.helicsFederateGetState(vFed)
    assert status == 0
    assert state == 3

    while (h.helicsBrokerIsConnected(broker)):
        time.sleep(1)

    h.helicsFederateFree(vFed)
    h.helicsCloseLibrary()




def test_value_federate_initialize(vFed):
    status, state = h.helicsFederateGetState(vFed)
    assert status == 0
    assert state == 0

    h.helicsFederateEnterExecutionMode(vFed)

    status, state = h.helicsFederateGetState(vFed)
    assert status == 0
    assert state == 2

def test_value_federate_publication_registration(vFed):
    pubid1 = h.helicsFederateRegisterPublication(vFed, "pub1", "string", "")
    pubid2 = h.helicsFederateRegisterGlobalPublication(vFed, "pub2", "int", "")
    pubid3 = h.helicsFederateRegisterPublication(vFed, "pub3", "double", "V")
    h.helicsFederateEnterExecutionMode(vFed)

    status, publication_key = h.helicsPublicationGetKey(pubid1)
    assert status == 0
    assert publication_key == 'TestA Federate/pub1'
    status, publication_key = h.helicsPublicationGetKey(pubid2)
    assert status == 0
    assert publication_key == 'pub2'
    status, publication_key = h.helicsPublicationGetKey(pubid3)
    assert status == 0
    assert publication_key == 'TestA Federate/pub3'
    status, publication_type = h.helicsPublicationGetType(pubid3)
    assert status == 0
    assert publication_type == 'double'
    status, publication_units = h.helicsPublicationGetUnits(pubid3)
    assert status == 0
    assert publication_units == 'V'

def test_value_federate_publisher_registration(vFed):
    pubid1 = h.helicsFederateRegisterTypePublication(vFed, "pub1", h.HELICS_DATA_TYPE_STRING, "")
    pubid2 = h.helicsFederateRegisterGlobalTypePublication(vFed, "pub2", h.HELICS_DATA_TYPE_INT, "")
    pubid3 = h.helicsFederateRegisterTypePublication(vFed, "pub3", h.HELICS_DATA_TYPE_DOUBLE, "V")
    h.helicsFederateEnterExecutionMode(vFed)

    # federate_state state;
    # CE(helicsFederateGetState(vFed1, &state));
    # BOOST_CHECK(state == helics_execution_state);

    # char sv[HELICS_SIZE_MAX];
    # CE(helicsPublicationGetKey(pubid, sv, HELICS_SIZE_MAX));
    # char sv2[HELICS_SIZE_MAX];
    # CE(helicsPublicationGetKey(pubid2, sv2, HELICS_SIZE_MAX));
    # BOOST_CHECK_EQUAL(sv, "fed0/pub1");
    # BOOST_CHECK_EQUAL(sv2, "pub2");
    # char pub3name[HELICS_SIZE_MAX];
    # CE(helicsPublicationGetKey(pubid3, pub3name, HELICS_SIZE_MAX));
    # BOOST_CHECK_EQUAL(pub3name, "fed0/pub3");

    # char tmp[HELICS_SIZE_MAX];
    # CE(helicsPublicationGetType(pubid3, tmp, HELICS_SIZE_MAX));
    # BOOST_CHECK_EQUAL(tmp, "double");
    # CE(helicsPublicationGetUnits(pubid3, tmp, HELICS_SIZE_MAX));
    # BOOST_CHECK_EQUAL(tmp, "V");

def test_value_federate_subscription_registration(vFed):
    subid1 = h.helicsFederateRegisterOptionalSubscription(vFed, "sub1", "double", "V")
    subid2 = h.helicsFederateRegisterOptionalTypeSubscription(vFed, "sub2", h.HELICS_DATA_TYPE_INT, "")
    subid3 = h.helicsFederateRegisterOptionalSubscription(vFed, "sub3", "double", "V")
    h.helicsFederateEnterExecutionMode(vFed)

    # h.helicsSubscriptionGetKey(subid1)

def test_value_federate_subscription_and_publication_registration(vFed):

    pubid1 = h.helicsFederateRegisterTypePublication(vFed, "pub1", h.HELICS_DATA_TYPE_STRING, "")
    pubid2 = h.helicsFederateRegisterGlobalTypePublication(vFed, "pub2", h.HELICS_DATA_TYPE_INT, "")

    pubid3 = h.helicsFederateRegisterPublication(vFed, "pub3", "double", "V")

    subid1 = h.helicsFederateRegisterOptionalSubscription(vFed, "sub1", "double", "V")
    subid2 = h.helicsFederateRegisterOptionalTypeSubscription(vFed, "sub2", h.HELICS_DATA_TYPE_INT, "")

    subid3 = h.helicsFederateRegisterOptionalSubscription(vFed, "sub3", "double", "V")

def test_value_federate_single_transfer(vFed):

    pubid = h.helicsFederateRegisterGlobalTypePublication (vFed, "pub1", h.HELICS_DATA_TYPE_STRING, "");
    subid = h.helicsFederateRegisterSubscription (vFed, "pub1", "string", "");

    h.helicsFederateEnterExecutionMode(vFed)

    h.helicsPublicationPublishString(pubid, "string1")

    status, grantedtime = h.helicsFederateRequestTime(vFed, 1.0)
    assert status == 0
    assert grantedtime == 0.01

    status, s = h.helicsSubscriptionGetString(subid)
    assert status == 0
    assert s == "string1"

def test_value_federate_runFederateTestDouble(vFed):
    defaultValue = 1.0
    testValue = 2.0
    pubid = h.helicsFederateRegisterGlobalTypePublication (vFed, "pub1", h.HELICS_DATA_TYPE_DOUBLE, "")
    subid = h.helicsFederateRegisterSubscription (vFed, "pub1", "double", "")
    h.helicsSubscriptionSetDefaultDouble(subid, defaultValue)

    h.helicsFederateEnterExecutionMode (vFed)

    # publish string1 at time=0.0;
    h.helicsPublicationPublishDouble(pubid, testValue)

    status, value = h.helicsSubscriptionGetDouble(subid)
    assert value == defaultValue

    status, grantedtime = h.helicsFederateRequestTime (vFed, 1.0)
    assert grantedtime == 0.01

    status, value = h.helicsSubscriptionGetDouble(subid)
    assert value == testValue

    # publish string1 at time=0.0;
    h.helicsPublicationPublishDouble(pubid, testValue + 1)

    status, grantedtime = h.helicsFederateRequestTime (vFed, 2.0)
    assert grantedtime == 0.02

    status, value = h.helicsSubscriptionGetDouble(subid)
    assert value == testValue + 1

def test_value_federate_runFederateTestComplex(vFed):
    rDefaultValue = 1.0
    iDefaultValue = 1.0
    rTestValue = 2.0
    iTestValue = 2.0
    pubid = h.helicsFederateRegisterGlobalTypePublication (vFed, "pub1", h.HELICS_DATA_TYPE_COMPLEX, "")
    subid = h.helicsFederateRegisterSubscription (vFed, "pub1", "double", "")
    h.helicsSubscriptionSetDefaultComplex(subid, rDefaultValue, iDefaultValue)

    h.helicsFederateEnterExecutionMode (vFed)

    # publish string1 at time=0.0;
    h.helicsPublicationPublishComplex(pubid, rTestValue, iTestValue)

    status, value1, value2 = h.helicsSubscriptionGetComplex(subid)
    assert value1 == rDefaultValue
    assert value2 == iDefaultValue

    status, grantedtime = h.helicsFederateRequestTime (vFed, 1.0)
    assert grantedtime == 0.01

    status, value1, value2 = h.helicsSubscriptionGetComplex(subid)
    assert value1 == rTestValue
    assert value2 == iTestValue


def test_value_federate_runFederateTestInteger(vFed):
    defaultValue = 1
    testValue = 2
    pubid = h.helicsFederateRegisterGlobalTypePublication (vFed, "pub1", h.HELICS_DATA_TYPE_INT, "")
    subid = h.helicsFederateRegisterSubscription (vFed, "pub1", "int", "")
    h.helicsSubscriptionSetDefaultInteger(subid, defaultValue)

    h.helicsFederateEnterExecutionMode (vFed)

    h.helicsPublicationPublishInteger(pubid, testValue)

    status, value = h.helicsSubscriptionGetInteger(subid)
    assert status == 0
    assert value == defaultValue

    status, grantedtime = h.helicsFederateRequestTime(vFed, 1.0)
    assert status == 0
    assert grantedtime == 0.01

    status, value = h.helicsSubscriptionGetInteger(subid)
    assert status == 0
    assert value == testValue

    h.helicsPublicationPublishInteger(pubid, testValue + 1)

    status, grantedtime = h.helicsFederateRequestTime (vFed, 2.0)
    assert status == 0
    assert grantedtime == 0.02

    status, value = h.helicsSubscriptionGetInteger(subid)
    assert status == 0
    assert value == testValue + 1


def test_value_federate_runFederateTestString(vFed):
    defaultValue = "String1"
    testValue = "String2"
    pubid = h.helicsFederateRegisterGlobalTypePublication (vFed, "pub1", h.HELICS_DATA_TYPE_STRING, "")
    subid = h.helicsFederateRegisterSubscription (vFed, "pub1", "string", "")
    h.helicsSubscriptionSetDefaultString(subid, defaultValue)

    h.helicsFederateEnterExecutionMode(vFed)

    # TODO: Fix error with the following function
    h.helicsPublicationPublishString(pubid, testValue)

    status, value = h.helicsSubscriptionGetString(subid)
    assert status == 0
    assert value == defaultValue

    status, grantedtime = h.helicsFederateRequestTime (vFed, 1.0)
    assert status == 0
    assert grantedtime == 0.01

    status, value = h.helicsSubscriptionGetString(subid)
    assert status == 0
    assert value == testValue

def test_value_federate_runFederateTestVectorD(vFed):
    defaultValue = [0, 1, 2]
    testValue = [3, 4, 5]
    pubid = h.helicsFederateRegisterGlobalTypePublication (vFed, "pub1", h.HELICS_DATA_TYPE_VECTOR, "")
    subid = h.helicsFederateRegisterSubscription (vFed, "pub1", "vector", "")
    h.helicsSubscriptionSetDefaultVector(subid, defaultValue)

    h.helicsFederateEnterExecutionMode(vFed)

    # TODO: Fix error with the following function
    h.helicsPublicationPublishVector(pubid, testValue)

    status, value = h.helicsSubscriptionGetVector(subid)
    assert status == 0
    assert value == [0, 1, 2]

    status, grantedtime = h.helicsFederateRequestTime(vFed, 1.0)
    assert status == 0
    assert grantedtime == 0.01

    status, value = h.helicsSubscriptionGetVector(subid)
    assert status == 0
    assert value == [3, 4, 5]
