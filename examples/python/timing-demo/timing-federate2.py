import time
import helics as h
import random

def get_input():

    valid_input = False
    while not valid_input:
        print("Enter request_time (int): ", end="")
        string = input()
        request_time_str = string.strip()
        try:
            request_time = int(request_time_str)
        except:
            valid_input = False
        else:
            valid_input = True

    return request_time



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

    # print("Setting default value")
    h.helicsSubscriptionSetDefaultDouble(subid, 0)

    print("Entering execution mode")
    h.helicsFederateEnterExecutionMode(fed)

    while True:
        stop_at_time = get_input()
        print("Sending {} to Federate 1".format(stop_at_time))
        status = h.helicsPublicationPublishDouble(pubid, stop_at_time)
        print(">>>>>>>> Requesting time = {}".format(stop_at_time))
        status, grantedtime = h.helicsFederateRequestTime (fed, stop_at_time)
        print("<<<<<<<< Granted Time = {}".format(grantedtime))
        status, value = h.helicsSubscriptionGetDouble(subid)
        print("Received {} from Federate 1".format(value))
        print("----------------------------------")

    destroy_value_federate(fed)


if __name__ == "__main__":
    main()
