# -*- coding: utf-8 -*-
"""
Created on 5/27/2020

@author: bearcub
"""

import helics as h
import random
import string
import time
from datetime import datetime, timedelta
import json
import logging
import numpy as np
import sys

#logging.basicConfig(filename='cosim.log')
logger = logging.getLogger(__name__)
logger.addHandler(logging.StreamHandler())
logger.setLevel(logging.DEBUG)

def destroy_federate(fed):
    status = h.helicsFederateFinalize(fed)
    #while h.helicsBrokerIsConnected(broker):
    #    print('broker is still connected')
    #    time.sleep(1)

    h.helicsFederateFree(fed)
    h.helicsCloseLibrary()
    #print("EVController: Federate finalized")

def create_message_federate(fedinitstring,name,deltat):
    fedinfo = h.helicsCreateFederateInfo()
    h.helicsFederateInfoSetCoreTypeFromString(fedinfo, "zmq")
    h.helicsFederateInfoSetCoreInitString(fedinfo, fedinitstring)
    # Set one second message interval
    h.helicsFederateInfoSetTimeProperty(fedinfo, h.helics_property_time_delta, deltat)
    # set wait for current time update to true
    h.helicsFederateInfoSetFlagOption(fedinfo, h.helics_flag_wait_for_current_time_update, True)
    h.helicsFederateInfoSetIntegerProperty(fedinfo, h.helics_property_int_log_level, 1)
    fed = h.helicsCreateMessageFederate(name, fedinfo)
    #print("Message federate created")
    return fed


def get_new_EV(numEVs):
    # numEVs is the number of EVs to return to the main program

    lvl2 = np.random.lognormal(np.random.normal(5,1),np.random.uniform(0,2),1)
    lvl1 = np.random.lognormal(np.random.normal(3,1),np.random.uniform(0,10),1)
    lvl3 = np.random.lognormal(np.random.normal(2,1),np.random.uniform(0,.1),1)
    total = lvl1+lvl2+lvl3
    #print(lvl1,lvl2,lvl3,total)
    p1,p2,p3 = lvl1/total,lvl2/total,lvl3/total
    #print(p1,p2,p3)
    listOfEVs = np.random.choice([1,2,3],numEVs,p=[p1[0],p2[0],p3[0]]).tolist()
    numLvl1 = listOfEVs.count(1)
    numLvl2 = listOfEVs.count(2)
    numLvl3 = listOfEVs.count(3)

    return numLvl1,numLvl2,numLvl3,listOfEVs


if __name__ == "__main__":
    random.seed(8675309)
    fedinitstring = " --federates=1"# --broker="+broker
    name = 'EV_federate'
    deltat = 60*60  # this is the default time step for the EV federate
    fed = create_message_federate(fedinitstring,name,deltat)

    #### Register interfaces #####
# Register the endpoints and their destinations
# the EVController will subscribe to each EV
    num_EVs = 100
    end_EVsoc = []
    enddest_EVsoc = []
    EVs = range(1,num_EVs+1)
    for EV in EVs:
        end_name = f'EV{EV}.soc'
        end_EVsoc.append(
            h.helicsFederateRegisterEndpoint(
                fed, end_name, 'double'
            )
        )


        dest_name = f'EVController_federate/EV{EV}.soc'
        enddest_EVsoc.append(
            h.helicsEndpointSetDefaultDestination(
                end_EVsoc[EV-1], dest_name
            )
        )
        #print(f"end point {end_name} registered to {dest_name}")

    end_count = h.helicsFederateGetEndpointCount(fed)
    #print(end_count)

    fed_name = h.helicsFederateGetName(fed)
    #print(" Federate {} has been registered".format(fed_name))
    test = h.helicsEndpointGetName(end_EVsoc[0])
    #print(test)
#
######################   Entering Execution Mode  ##########################################################

    h.helicsFederateEnterExecutingMode(fed)

#
# controller's initial guess of each EV's soc
    #currentsoc = []
### np.random.rand(pub_count)
    #instructions = np.ones(end_count)

    # we will explore the question: what is the distribution of instantaneous power draw,
    # given unrestricted access by 10 vehicles.
    # in each iteration, we will draw from a distribution of lvl1, lvl2, lvl3 chargers
    # placing a gaussian over each.
    # eg: out of 100 EVs, ~ 60 will be lvl2, 10 will be lvl3, 30 will be lvl1
    # But this is the expectation of each.
    # We can assign stddev to each of these.
    # lvl2 ~ norm(60,unif(0,5))  -- assumes that most of lvl2 EVs are around 60% of population
    # lvl1 ~ norm(30,unif(0,10)) -- assumes that there is a large amount of spread in # of lvl1 EVs
    # lvl3 ~ norm(10,unif(0,1)) -- assumes not many lvl3 EVs, with small spread
    # in each iteration, the number of EVs connected to the controller may not addup to 100
    # that's ok -- we're just calculating the prior predictive check of the
    # power draw from a distribution of EVs

    # when the simulation starts, draw 100 samples from this joint distribution
    # the simulation starts in a snapshot of existing power discharge to the vehicles
    # i.e., each vehicle is at a random soc, and is connected to the EV controller.
    # if an EV reaches >95% SOC, it is disconnected, and the EV controller
    # selects a new EV from the joint distribution
    # each vehicle will have its own characteristics:
    # lvl1 = leaf, 120V
    # lvl2 = leaf, 240V
    # lvl3 = leaf, 480V
    # assumes 15amp outlet, same battery size
    charge_rate = [1.8,3.6,7.2]
    batt_size = 62*60 # leaf capacity is 62 kWh
    hours = 24*7
    total_interval = int(60 * 60 * hours)
    update_interval = 60*60 # updates every hour
    grantedtime = -1

    numLvl1,numLvl2,numLvl3,EVlist = get_new_EV(end_count)
##
## Step through each time period starting from t = 0
    time_sim = []; currentsoc = np.random.rand(end_count)
    send_soc = 1; t = 0
    time_since_last_msg = np.zeros(end_count)
    print('time,power')
    while t < total_interval:

        total_power = 0
        for j in range(0,len(enddest_EVsoc)):
            total_power += charge_rate[(EVlist[j]-1)]

        print(t/3600,total_power)
        grantedtime = h.helicsFederateRequestTime (fed, t)
        #print('NEXT TIME WILL BE: ',grantedtime/3600)

        # current minute
        time_sim.append(t/3600)
        #print('CURRENT TIME in HOURS: ',t/3600)

        if send_soc:
            # First send SOC from each EV in the fleet to the EVController
            for j in range(0,len(enddest_EVsoc)):
                #print('sending soc to controller: ',currentsoc[j])
                # log the source of the message
                end_name = str(h.helicsEndpointGetName(end_EVsoc[j]))
                # this is the name of the source endpoint
                #print('Sending endpoint name: ',end_name)
                destination_name = str(h.helicsEndpointGetDefaultDestination(end_EVsoc[j]))
                #print('destination endpoint name: ',destination_name)
                #destination_federate = str(destination_name.split('/EV')[0])
                #print('Endpoint destination federate: ',destination_federate)
                h.helicsEndpointSendMessageRaw(end_EVsoc[j], "", str(currentsoc[j])) #

                #print('Sent SOC: {}'.format(currentsoc[j]))

                #print('')
            # after sending the soc of each EV,
            send_soc = 0
            # wait 1 min for instructions
            t = grantedtime
        else:
            # ask for instructions
            instructions = []
            for j in range(0,len(enddest_EVsoc)):
                # log the source of the message
                #print('endpt name: ',h.helicsEndpointGetName(end_EVsoc[j]))

                if h.helicsEndpointHasMessage(end_EVsoc[j]):
                    msg = h.helicsEndpointGetMessageObject(end_EVsoc[j])
                    #print('MESSAGE RECEIVED: ',h.helicsMessageGetString(msg))
                    instructions.append(h.helicsMessageGetString(msg))
                    #print('msg: ',instructions[j])
                    # right now assume
                    if int(instructions[j]) == 1:
                        # add  kW times time difference
                        # EVlist[j] = 1, 2, or 3
                        # subtract 1 to get the index for the
                        # charge rate of that level EV
                        #print('OLD SOC: ',currentsoc[j])
                        #print('TIME DIFF IN HOURS: ',time_since_last_msg[j])
                        #print('EVLIST: ',EVlist[j])
                        #print('INDEX: ',(EVlist[j]-1))
                        # add e.g., 1.9 kW for the past minute of charging
                        addenergy = charge_rate[(EVlist[j]-1)]*time_since_last_msg[j]
                        #    # convert to %
                        #print('              ADDING THIS AMOUNT OF ENERGY: ',addenergy)
                        #print('              AS PERCENT OF BATT SIZE: ',addenergy/batt_size)
                        currentsoc[j] = currentsoc[j] + addenergy/batt_size
                        #print('NEW SOC: ',currentsoc[j])
                    else:
                        # this EV has enough energy stored
                        # the controller has "disengaged" the EV,
                        # and the EV federation should select a new random
                        # EV
                        # reassign the j-th EV this new information
                        #print('WAS A ',EVlist[j])
                        _,_,_,newEVtype = get_new_EV(1)
                        EVlist[j] = newEVtype[0]
                        #print('CONGRATS, ITS A ',EVlist[j])
                        currentsoc[j] = np.random.uniform(.05,.5)
                        #print('SOC: ',currentsoc[j])
                    send_soc = 1
                    time_since_last_msg[j] = 0
                else:
                    #print('NO MESSAGE AT TIME ',t/3600)
                    time_since_last_msg[j] += 1
                    #print('TIME SINCE LAST MSG: ',time_since_last_msg[j])
                #if h.helicsEndpointHasMessage(msg):
                #    instructions.append(
                #        h.helicsMessageGetString(msg)
                #    )
                #    print('msg: ',instructions[j])
                #    # right now assume
                #    if instructions[j] == 1:
                #        # add  kW times time difference
                #        # EVlist[j] = 1, 2, or 3
                #        # subtract 1 to get the index for the
                #        # charge rate of that level EV
                #        addenergy = charge_rate[(EVlist[j]-1)]*(time_sim[-1]-time_sim[-2])
                #    #    # convert to %
                #        currentsoc[j] = currentsoc[j] + addenergy/batt_size
                #    else:
                #        # this EV has enough energy stored
                #        # the controller has "disengaged" the EV,
                #        # and the EV federation should select a new random
                #        # EV
                #        # reassign the j-th EV this new information
                #        _,_,_,EVlist[j] = get_new_EV(1)



            t = grantedtime

    #print("Destroying federate")
    destroy_federate(fed)
    #print("Done!")
