import time
import helics as h
import random

def get_input(grantedtime):

    valid_input = False
    while not valid_input:
        print("Enter request_time (int) (and value_to_send (float)) [e.g.: 4, 10.0]: ", end="")
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
            value_to_send = string.replace(",", " ").split(" ")[1]
        except:
            value_to_send = None
            valid_input = True
            continue

        try:
            value_to_send = float(value_to_send)
        except:
            print("value_to_send must be a float or be blank")
            valid_input = False
            continue
        else:
            valid_input = True

    return request_time, value_to_send



def create_value_federate(deltat=1.0, fedinitstring="--federates=1"):

    fedinfo = h.helicsFederateInfoCreate()

    status = h.helicsFederateInfoSetFederateName(fedinfo, "TestB Federate")
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

def destroy_value_federate(fed):
    status = h.helicsFederateFinalize(fed)

    status, state = h.helicsFederateGetState(fed)
    assert state == 3

    h.helicsFederateFree(fed)

    h.helicsCloseLibrary()

def main():

    fed = create_value_federate()

    pubid = h.helicsFederateRegisterGlobalTypePublication(fed, "federate2-to-federate1", h.HELICS_DATA_TYPE_DOUBLE, "")
    subid = h.helicsFederateRegisterSubscription (fed, "federate1-to-federate2", "double", "")
    epid = h.helicsFederateRegisterGlobalEndpoint(fed, "endpoint2", "")

    # print("Setting default value")
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
                print("Interrupt value {} from Federate 1".format(value))
            print("<<<<<<<< Granted Time = {}".format(grantedtime))
        assert grantedtime == stop_at_time, "stop_at_time = {}, grantedtime = {}".format(stop_at_time, grantedtime)
        if value_to_send is not None:
            print("Sending {} to Federate 1".format(value_to_send))
            status = h.helicsPublicationPublishDouble(pubid, value_to_send)
            status = h.helicsEndpointSendMessageRaw(epid, "endpoint1", str(value_to_send))

        status, value = h.helicsSubscriptionGetDouble(subid)
        print("Received value {} from Federate 1".format(value))
        while h.helicsEndpointHasMessage(epid):
            value = h.helicsEndpointGetMessage(epid)
            print("Received message {} from Federate 1".format(value.data))
        print("----------------------------------")


    destroy_value_federate(fed)


if __name__ == "__main__":
    main()

