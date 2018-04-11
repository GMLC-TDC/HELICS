import time
import helics as h
import random

def get_input(grantedtime):

    valid_input = False
    while not valid_input:
        print("Enter request_time (int) (and value to send (float)): ", end="")
        string = input()
        string = string.strip()
        request_time_str = string.replace(",", " ").split(" ")[0]
        try:
            request_time = int(request_time_str)
            if request_time <= grantedtime:
                print("request_time has to be greater than grantedtime.")
                raise RuntimeError("Cannot proceed here because invalid input.")
        except:
            valid_input = False
            continue
        else:
            valid_input = True

        try:
            value_to_send = string.replace(",", " ").split(" ")[1]
        except:
            value_to_send = None
            valid_input = True
            continue

        try:
            value_to_send = float(value_to_send)
        except:
            valid_input = False
            continue
        else:
            valid_input = True

    return request_time, value_to_send



def create_broker():
    initstring = "2 --name=mainbroker"
    broker = h.helicsCreateBroker("zmq", "", initstring)
    isconnected = h.helicsBrokerIsConnected(broker)

    if isconnected == 1:
        pass

    return broker


def create_value_federate(broker, deltat=1.0, fedinitstring="--broker=mainbroker --federates=1"):

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

    print("Entering execution mode")
    h.helicsFederateEnterExecutionMode(fed)

    grantedtime = -1
    while True:
        stop_at_time, value_to_send = get_input(grantedtime)
        while grantedtime < stop_at_time:
            print(">>>>>>>> Requesting time = {}".format(stop_at_time))
            status, grantedtime = h.helicsFederateRequestTime(fed, stop_at_time)
            if grantedtime != stop_at_time:
                status, value = h.helicsSubscriptionGetDouble(subid)
                print("Unexpected value {} from Federate 2".format(value))
            print("<<<<<<<< Granted Time = {}".format(grantedtime))
        assert grantedtime == stop_at_time, "stop_at_time = {}, grantedtime = {}".format(stop_at_time, grantedtime)
        if value_to_send is not None:
            print("Sending {} to Federate 2".format(value_to_send))
            status = h.helicsPublicationPublishDouble(pubid, value_to_send)
        status, value = h.helicsSubscriptionGetDouble(subid)
        print("Received {} from Federate 2".format(value))
        print("----------------------------------")

    destroy_value_federate(fed, broker)


if __name__ == "__main__":
    main()

