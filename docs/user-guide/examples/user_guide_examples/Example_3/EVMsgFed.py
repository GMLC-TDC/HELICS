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
import argparse


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

def create_message_federate(fedinitstring,name,period):
    fedinfo = h.helicsCreateFederateInfo()
    h.helicsFederateInfoSetCoreTypeFromString(fedinfo, "tcpss")
    h.helicsFederateInfoSetCoreInitString(fedinfo, fedinitstring)
    # Set one second message interval
    h.helicsFederateInfoSetTimeProperty(fedinfo, h.helics_property_time_period, period)
    # set wait for current time update to true
    h.helicsFederateInfoSetFlagOption(fedinfo, h.helics_flag_uninterruptible, True)
    h.helicsFederateInfoSetIntegerProperty(fedinfo, h.helics_property_int_log_level, 1)
    fed = h.helicsCreateMessageFederate(name, fedinfo)
    #print("Message federate created")
    return fed


def get_new_EV(numEVs):
    # numEVs is the number of EVs to return to the main program

    lvl1 = np.random.poisson(np.random.normal(30,np.random.uniform(1,3)),1)
    lvl2 = np.random.poisson(np.random.normal(50,np.random.uniform(1,2)),1)
    lvl3 = np.random.poisson(np.random.normal(20,np.random.uniform(.05,.25)),1)
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
    parser = argparse.ArgumentParser(description='EV simulator')
    parser.add_argument('--seed', type=int, default=867530,
                    help='The seed that will be used for our random distribution')
    parser.add_argument('--port', type=int, default=-1,
                    help='port of the HELICS broker')


    args = parser.parse_args()
    np.random.seed(args.seed)

    if args.port != -1:
        fedinitstring="--brokerport="+str(args.port)
    else:
        fedinitstring=""

    print("Federate Init String = {}".format(fedinitstring)) 
    print("Random seed = {}".format(args.seed))



    name = 'EV_federate'
    period = 15*60  # 15 min  # this is the default time step for the EV federate
    fed = create_message_federate(fedinitstring,name,period)

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
    charge_rate = [1.8,7.2,50]
    # [120V*15A, 240V*30A, 50kW DC charging]
    batt_size = 62 # leaf capacity is 62 kWh
    hours = 24*7 # one week
    total_interval = int(60 * 60 * hours)
    update_interval = 30*60 # updates every hour
    grantedtime = -1

    numLvl1,numLvl2,numLvl3,EVlist = get_new_EV(end_count)
##
## Step through each time period starting from t = 0
    time_sim = []; currentsoc = np.random.rand(end_count)
    #t = 0

    # this will be the first message that is "waiting" for the
    # EV Controller

    t = h.helicsFederateRequestTime (fed, 0)
    for j in range(0,len(enddest_EVsoc)):
        end_name = str(h.helicsEndpointGetName(end_EVsoc[j]))
        destination_name = str(h.helicsEndpointGetDefaultDestination(end_EVsoc[j]))
        h.helicsEndpointSendMessageRaw(end_EVsoc[j], "", str(currentsoc[j])) #
        #print('destination: ',destination_name)
        #print('message sent: ',str(currentsoc[j]))
        #print('at time: ',t/3600)
        #print(t/3600,currentsoc[j])

    grantedtime = h.helicsFederateRequestTime (fed, (t+update_interval))
    t = grantedtime

    #logger.info('time: ',t)
    time_since_last_msg = np.zeros(end_count)
    print('time power')
    next_time = []
    while t < total_interval:



        for j in range(0,len(enddest_EVsoc)):
            # 1. Receive instructions
            if h.helicsEndpointHasMessage(end_EVsoc[j]):
                msg = h.helicsEndpointGetMessageObject(end_EVsoc[j])
                instructions = h.helicsMessageGetString(msg)
            # 2. Change SOC based on instructions
                if int(instructions) == 1:
                    addenergy = charge_rate[(EVlist[j]-1)]*(update_interval/3600)   #time_since_last_msg[j]
                    #print('old soc: ',currentsoc[j])
                    currentsoc[j] = currentsoc[j] + addenergy/batt_size
                    #print('new soc: ',currentsoc[j])
                else:
                    _,_,_,newEVtype = get_new_EV(1)
                    EVlist[j] = newEVtype[0]
                    currentsoc[j] = np.random.uniform(.05,.5)
                    #print('new soc: ',currentsoc[j])
                # 3. Send SOC
                #end_name = str(h.helicsEndpointGetName(end_EVsoc[j]))
                destination_name = str(h.helicsEndpointGetDefaultDestination(end_EVsoc[j]))
                h.helicsEndpointSendMessageRaw(end_EVsoc[j], "", str(currentsoc[j])) #
                #print(t/3600,currentsoc[j])
            else:
                #currentsoc[j] = np.random.rand(end_count)
                #print('new value: ',currentsoc[j])
                print('error AT TIME ',t,' with endpoint ',str(h.helicsEndpointGetDefaultDestination(end_EVsoc[j])))
                #print('maybe we need to advance in time by this federates time delta, ',h.helicsFederateRequestTime (fed, t))

        total_power = 0
        for j in range(0,len(enddest_EVsoc)):
            total_power += charge_rate[(EVlist[j]-1)]
        print(t/3600,total_power)



        grantedtime = h.helicsFederateRequestTime (fed, (t+update_interval))
        #print('EV time: ',grantedtime/3600)

        t = grantedtime






    #print("Destroying federate")
    destroy_federate(fed)
    #print("Done!")
