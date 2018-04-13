import time
import helics as h
import random

def get_input(grantedtime):

    valid_input = False
    while not valid_input:
        print("Enter request_time (int) (and value_to_send (str)) [e.g.: 4 hello, world]: ", end="")
        string = input()
        string = string.strip()
        request_time_str = string.replace(",", " ").split(" ")[0]
        try:
            request_time = int(request_time_str)
            if request_time <= grantedtime:
                raise RuntimeError("Cannot proceed here because invalid input.")
        except:
            print("request_time has to be an 'int' and has to be greater than grantedtime.")
            valid_input = False
            continue
        else:
            valid_input = True

        try:
            value_to_send = string.replace(request_time_str, "").strip().strip(",").strip()
        except:
            value_to_send = None
            valid_input = True
            continue

        try:
            value_to_send = str(value_to_send)
        except:
            print("value_to_send must be a str or be blank")
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

    pubid = h.helicsFederateRegisterGlobalTypePublication(fed, "federate1-to-federate2", h.HELICS_DATA_TYPE_STRING, "")
    subid = h.helicsFederateRegisterSubscription(fed, "federate2-to-federate1", "double", "")
    epid = h.helicsFederateRegisterGlobalEndpoint(fed, "endpoint1", "")
    # fid = h.helicsFederateRegisterSourceFilter(fed, h.helics_delay_filter, "endpoint2", "filter-name")

    h.helicsSubscriptionSetDefaultDouble(subid, 0)

    print("Entering execution mode")
    h.helicsFederateEnterExecutionMode(fed)

    # h.helicsFilterSet(fid, "delay", 2.0)

    grantedtime = -1
    while True:
        stop_at_time, value_to_send = get_input(grantedtime)
        while grantedtime < stop_at_time:
            print(">>>>>>>> Requesting time = {}".format(stop_at_time))
            status, grantedtime = h.helicsFederateRequestTime(fed, stop_at_time)
            if grantedtime != stop_at_time:
                status, value = h.helicsSubscriptionGetString(subid)
                print("Interrupt value '{}' from Federate 2".format(value))
            print("<<<<<<<< Granted Time = {}".format(grantedtime))
        assert grantedtime == stop_at_time, "stop_at_time = {}, grantedtime = {}".format(stop_at_time, grantedtime)
        if value_to_send is not None and value_to_send != '':
            print("Sending '{}' to Federate 2".format(value_to_send))
            status = h.helicsPublicationPublishString(pubid, str(value_to_send))
            status = h.helicsEndpointSendMessageRaw(epid, "endpoint2", str(value_to_send))
        status, value = h.helicsSubscriptionGetString(subid)
        print("Received value '{}' from Federate 2".format(value))
        while h.helicsEndpointHasMessage(epid):
            value = h.helicsEndpointGetMessage(epid)
            print("Received message '{}' from Federate 2".format(value.data))
        print("----------------------------------")

    destroy_value_federate(fed, broker)


if __name__ == "__main__":
    main()

