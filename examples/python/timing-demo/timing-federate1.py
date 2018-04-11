import time
import helics as h
import random


def create_broker():
    initstring = "2 --name=mainbroker"
    broker = h.helicsCreateBroker("zmq", "", initstring)
    isconnected = h.helicsBrokerIsConnected(broker)

    if isconnected == 1:
        pass

    return broker


def create_value_federate(broker, deltat=0, fedinitstring="--broker=mainbroker --federates=1"):

    fedinfo = h.helicsFederateInfoCreate()

    status = h.helicsFederateInfoSetFederateName(fedinfo, "TestA Federate")
    assert status == 0

    status = h.helicsFederateInfoSetCoreTypeFromString(fedinfo, "zmq")
    assert status == 0

    status = h.helicsFederateInfoSetCoreInitString(fedinfo, fedinitstring)
    assert status == 0

    status = h.helicsFederateInfoSetTimeDelta(fedinfo, deltat)
    assert status == 0

    status = h.helicsFederateInfoSetLoggingLevel(fedinfo, 1)
    assert status == 0

    fed = h.helicsCreateCombinationFederate(fedinfo)

    return fed

def destroy_value_federate(fed, broker):
    status = h.helicsFederateFinalize(fed)

    status, state = h.helicsFederateGetState(fed)
    assert state == 3

    while (h.helicsBrokerIsConnected(broker)):
        time.sleep(1)

    h.helicsFederateFree(fed)

    h.helicsCloseLibrary()


def main():

    broker = create_broker()
    fed = create_value_federate(broker)

    pubid = h.helicsFederateRegisterGlobalTypePublication(fed, "federate1-to-federate2", h.HELICS_DATA_TYPE_DOUBLE, "")
    subid = h.helicsFederateRegisterSubscription(fed, "federate2-to-federate1", "double", "")

    h.helicsSubscriptionSetDefaultDouble(subid, 0)

    h.helicsFederateEnterExecutionMode(fed)

    hours = 1
    seconds = int(60 * 60 * hours)
    grantedtime = -1
    random.seed(0)
    while True:
        status = h.helicsPublicationPublishDouble(pubid, 1.0)
        while grantedtime < t:
            status, grantedtime = h.helicsFederateRequestTime (fed, t)
            print("<<<<<<<< Granted Time = {}".format(grantedtime))
        status, value = h.helicsSubscriptionGetDouble(subid)
        print("----------------------------------")

    destroy_value_federate(fed, broker)


if __name__ == "__main__":
    main()
