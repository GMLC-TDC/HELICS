import time
import helics as h
import random

def create_value_federate(deltat=0, fedinitstring="--federates=1"):

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

def get_input():

    valid_input = False
    while not valid_input:
        print("Enter request_time (int) and value to publish (double): ", end="")
        string = input()
        string_list = string.strip().replace(",", " ").split(" ")
        if len(string_list) != 2:
            valid_input = False
            continue
        request_time_str, double_value_str = string_list
        try:
            request_time = int(request_time_str)
            double_value = float(double_value_str)
        except:
            valid_input = False
        else:
            valid_input = True

    return request_time, double_value

def main():

    fed = create_value_federate()

    pubid = h.helicsFederateRegisterGlobalTypePublication(fed, "federate2-to-federate1", h.HELICS_DATA_TYPE_DOUBLE, "")
    subid = h.helicsFederateRegisterSubscription (fed, "federate1-to-federate2", "double", "")

    # print("Setting default value")
    h.helicsSubscriptionSetDefaultDouble(subid, 0)

    # print("Entering execution mode")
    h.helicsFederateEnterExecutionMode(fed)

    hours = 1
    seconds = int(60 * 60 * hours)
    grantedtime = -1
    random.seed(0)
    # for t in range(1, seconds + 1, 60 * 5):
    stop_at_time = 6000
    while grantedtime < stop_at_time:
        stop_at_time, double_value = get_input()
        status = h.helicsPublicationPublishDouble(pubid, double_value)
        print(">>>>>>>> Requesting time = {}".format(stop_at_time))
        status, grantedtime = h.helicsFederateRequestTime (fed, stop_at_time)
        print("<<<<<<<< Granted Time = {}".format(grantedtime))
        status, receivedValue = h.helicsSubscriptionGetDouble(subid)
        print("----------------------------------")

    destroy_value_federate(fed)

if __name__ == "__main__":
    main()
