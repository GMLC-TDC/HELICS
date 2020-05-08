# -*- coding: utf-8 -*-
"""
Created on Thu Oct 11 10:08:26 2018

@author: monish.mukherjee
"""
import scipy.io as spio
from pypower.api import case118, ppoption, runpf, runopf
import math
import numpy
import matplotlib.pyplot as plt
import time
import helics as h
import random
import logging

logger = logging.getLogger(__name__)
logger.addHandler(logging.StreamHandler())
logger.setLevel(logging.DEBUG)


def create_broker():
    initstring = "--federates=3 --name=mainbroker"
    broker = h.helicsCreateBroker("zmq", "", initstring)
    isconnected = h.helicsBrokerIsConnected(broker)

    if isconnected == 1:
        pass

    return broker


def create_federate(deltat=1.0, fedinitstring="--federates=1"):

    fedinfo = h.helicsFederateInfoCreate()

    status = h.helicsFederateInfoSetFederateName(fedinfo, "Combination Federate")
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


def destroy_federate(fed):
    h.helicsFederateFinalize(fed)

    #    status, state = h.helicsFederateGetState(fed)
    #    assert state == 3

    while h.helicsBrokerIsConnected(broker):
        time.sleep(1)

    h.helicsFederateFree(fed)
    h.helicsCloseLibrary()


if __name__ == "__main__":

    broker = create_broker()
    # fed = create_federate()

    #################################  Registering  federate from json  ########################################

    fed = h.helicsCreateValueFederateFromConfig("Transmission_json.json")
    h.helicsFederateRegisterInterfaces(fed, "Transmission_json.json")
    federate_name = h.helicsFederateGetName(fed)[-1]
    print(" Federate {} has been registered".format(federate_name))
    pubkeys_count = h.helicsFederateGetPublicationCount(fed)
    subkeys_count = h.helicsFederateGetInputCount(fed)
    print(subkeys_count)
    ######################   Reference to Publications and Subscription form index  #############################
    pubid = {}
    subid = {}
    for i in range(0, pubkeys_count):
        pubid["m{}".format(i)] = h.helicsFederateGetPublicationByIndex(fed, i)
        pubtype = h.helicsPublicationGetType(pubid["m{}".format(i)])
        print(pubtype)
    for i in range(0, subkeys_count):
        subid["m{}".format(i)] = h.helicsFederateGetInputByIndex(fed, i)
        h.helicsInputSetDefaultComplex(subid["m{}".format(i)], 0, 0)
        sub_key = h.helicsSubscriptionGetKey(subid["m{}".format(i)])
        print("Registered Subscription ---> {}".format(sub_key))

    ######################   Entering Execution Mode  ##########################################################
    h.helicsFederateEnterInitializingMode(fed)
    status = h.helicsFederateEnterExecutingMode(fed)

    # Pypower Processing (inputs)
    hours = 24
    total_inteval = int(60 * 60 * hours)
    grantedtime = -1
    pf_interval = 5 * 60  # in seconds (minimim_resolution)
    acopf_interval = 15 * 60  # in seconds (minimim_resolution)
    random.seed(0)

    peak_demand = []
    ppc = []
    case_format = case118()
    peak_demand = case_format["bus"][:, 2][:].copy()
    ppc = case_format.copy()

    ######################   creating fixed load profiles for each bus based on PF interval #############################

    # load profiles (inputs)
    profiles = spio.loadmat(
        "normalized_load_data_1min_ORIGINAL.mat", squeeze_me=True, struct_as_record=False
    )
    load_profiles_1min = profiles["my_data"]
    resolution_load = numpy.floor(total_inteval / pf_interval)
    points = numpy.floor(numpy.linspace(0, len(load_profiles_1min) - 1, resolution_load + 1))
    time_pf = numpy.linspace(0, total_inteval, resolution_load + 1)
    load_profiles = load_profiles_1min[points.astype(int), :]

    ###################   Creating a fixed profile for buses    ##################

    bus_profiles_index = []
    profile_number = 0
    for i in range(len(ppc["bus"])):
        bus_profiles_index.append(profile_number)
        if profile_number == 8:
            profile_number = 0
        else:
            profile_number = profile_number + 1
    ###################   Asserting Profiles to buses    ############################

    # bus_profiles_index = numpy.random.random_integers(0,load_profiles.shape[1]-1,len(ppc['bus']))
    bus_profiles = load_profiles[:, bus_profiles_index]
    time_opf = numpy.linspace(0, total_inteval, numpy.floor(total_inteval / acopf_interval) + 1)

    ###########################   Cosimulation Bus and Load Amplification Factor #########################################

    # Co-sim Bus  (inputs)
    Cosim_bus_number = 118
    cosim_bus = Cosim_bus_number - 1  ## Do not change this line
    load_amplification_factor = 15

    # power_flow
    fig = plt.figure()
    ax1 = fig.add_subplot(2, 1, 1)
    ax2 = fig.add_subplot(2, 1, 2)
    voltage_plot = []
    x = 0
    k = 0
    voltage_cosim_bus = (ppc["bus"][cosim_bus, 7] * ppc["bus"][cosim_bus, 9]) * 1.043

    #########################################   Starting Co-simulation  ####################################################

    for t in range(0, total_inteval, pf_interval):
        ############################   Publishing Voltage to GridLAB-D #######################################################
        voltage_gld = complex(voltage_cosim_bus * 1000)
        logger.info("Voltage value = {} kV".format(abs(voltage_gld) / 1000))
        for i in range(0, pubkeys_count):
            pub = pubid["m{}".format(i)]
            status = h.helicsPublicationPublishComplex(pub, voltage_gld.real, voltage_gld.imag)
        # status = h.helicsEndpointSendEventRaw(epid, "fixed_price", 10, t)

        while grantedtime < t:
            grantedtime = h.helicsFederateRequestTime(fed, t)
        time.sleep(0.1)

        #############################   Subscribing to Feeder Load from to GridLAB-D ##############################################

        for i in range(0, subkeys_count):
            sub = subid["m{}".format(i)]
            rload, iload = h.helicsInputGetComplex(sub)
        logger.info("Python Federate grantedtime = {}".format(grantedtime))
        logger.info("Load value = {} kW".format(complex(rload, iload) / 1000))
        # print(voltage_plot,real_demand)

        actual_demand = peak_demand * bus_profiles[x, :]
        ppc["bus"][:, 2] = actual_demand
        ppc["bus"][:, 3] = actual_demand * math.tan(math.acos(0.85))
        ppc["bus"][cosim_bus, 2] = rload * load_amplification_factor / 1000000
        ppc["bus"][cosim_bus, 3] = iload * load_amplification_factor / 1000000
        ppopt = ppoption(PF_ALG=1)

        print("PF TIme is {} and ACOPF time is {}".format(time_pf[x], time_opf[k]))

        ############################  Running OPF For optimal power flow intervals   ##############################

        if time_pf[x] == time_opf[k]:
            results_opf = runopf(ppc, ppopt)
            if results_opf["success"]:
                ppc["bus"] = results_opf["bus"]
                ppc["gen"] = results_opf["gen"]
                if k == 0:
                    LMP_solved = results_opf["bus"][:, 13]
                else:
                    LMP_solved = numpy.vstack((LMP_solved, results_opf["bus"][:, 13]))
                    opf_time = time_opf[0 : k + 1] / 3600
            k = k + 1

        ################################  Running PF For optimal power flow intervals   ##############################

        solved_pf = runpf(ppc, ppopt)
        results_pf = solved_pf[0]
        ppc["bus"] = results_pf["bus"]
        ppc["gen"] = results_pf["gen"]

        if results_pf["success"] == 1:
            if x == 0:
                voltages = results_pf["bus"][:, 7]
                real_demand = results_pf["bus"][:, 2]
                distribution_load = [rload / 1000000]
            else:
                voltages = numpy.vstack((voltages, results_pf["bus"][:, 7]))
                real_demand = numpy.vstack((real_demand, results_pf["bus"][:, 2]))
                distribution_load.append(rload / 1000000)
                pf_time = time_pf[0 : x + 1] / 3600

            voltage_cosim_bus = results_pf["bus"][cosim_bus, 7] * results_pf["bus"][cosim_bus, 9]
            voltage_plot.append(voltage_cosim_bus)

        ######################### Plotting the Voltages and Load of the Co-SIM bus ##############################################

        if x > 0:
            ax1.clear()
            ax1.plot(pf_time, voltage_plot, "r--")
            ax1.set_xlim([0, 25])
            ax1.set_ylabel("Voltage [in kV]")
            ax1.set_xlabel("Time [in hours]")
            ax2.clear()
            ax2.plot(pf_time, real_demand[:, cosim_bus], "k")
            ax2.set_xlim([0, 25])
            ax2.set_ylabel("Load from distribution [in MW]")
            ax2.set_xlabel("Time [in hours]")
            plt.show(block=False)
            plt.pause(0.1)
        x = x + 1

    ##########################   Creating headers and Printing results to CSVs #####################################

    head = str("Time(in Hours)")
    for i in range(voltages.shape[1]):
        head = head + "," + ("Bus" + str(i + 1))

    numpy.savetxt(
        "Transmission_Voltages.csv",
        numpy.column_stack((pf_time, voltages)),
        delimiter=",",
        fmt="%s",
        header=head,
        comments="",
    )
    numpy.savetxt(
        "Transmission_MW_demand.csv",
        numpy.column_stack((pf_time, real_demand)),
        delimiter=",",
        fmt="%s",
        header=head,
        comments="",
    )
    numpy.savetxt(
        "Transmission_LMP.csv",
        numpy.column_stack((opf_time, LMP_solved)),
        delimiter=",",
        fmt="%s",
        header=head,
        comments="",
    )

    ##############################   Terminating Federate   ########################################################
    t = 60 * 60 * 24
    while grantedtime < t:
        grantedtime = h.helicsFederateRequestTime(fed, t)
    logger.info("Destroying federate")
    destroy_federate(fed)
    logger.info("Done!")
