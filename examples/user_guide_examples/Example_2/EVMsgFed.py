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
    print("EVController: Federate finalized")

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
    print("Message federate created")
    return fed

if __name__ == "__main__":
    random.seed(8675309)
    fedinitstring = " --federates=1"# --broker="+broker
    name = 'EV_federate'
    deltat = 60.0
    fed = create_message_federate(fedinitstring,name,deltat)

    #### Register interfaces #####
# Register the endpoints and their destinations
# the EVController will subscribe to each EV
    num_EVs = 10
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
        print(f"end point {end_name} registered to {dest_name}")

    end_count = h.helicsFederateGetEndpointCount(fed)
    print(end_count)

    fed_name = h.helicsFederateGetName(fed)
    print(" Federate {} has been registered".format(fed_name))
    test = h.helicsEndpointGetName(end_EVsoc[0])
    print(test)
#
######################   Entering Execution Mode  ##########################################################

    h.helicsFederateEnterExecutingMode(fed)

#
# controller's initial guess of each EV's soc
    #currentsoc = []
### np.random.rand(pub_count)
    #instructions = np.ones(end_count)

    batt_size = 62*60 # leaf capacity is 62 kWh
    hours = 1
    total_interval = int(60 * 60 * hours)
    update_interval = 10*60 # updates every 10 minutes
    grantedtime = -1
##
## Step through each time period starting from t = 0
    time_sim = []; currentsoc = np.random.rand(end_count)
    send_soc = 1; t = 0
    while t < total_interval:

        grantedtime = h.helicsFederateRequestTime (fed, t)
        print('granted time: ',str(grantedtime))

        # current minute
        time_sim.append(t/60)

        if send_soc:
            # First send SOC from each EV in the fleet to the EVController
            for j in range(0,len(enddest_EVsoc)):
                print(' soc: ',currentsoc[j])
                # log the source of the message
                end_name = str(h.helicsEndpointGetName(end_EVsoc[j]))
                # this is the name of the source endpoint
                print('Sending endpoint name: ',end_name)
                destination_name = str(h.helicsEndpointGetDefaultDestination(end_EVsoc[j]))
                print('destination endpoint name: ',destination_name)
                #destination_federate = str(destination_name.split('/EV')[0])
                #print('Endpoint destination federate: ',destination_federate)
                h.helicsEndpointSendMessageRaw(end_EVsoc[j], "", str(currentsoc[j])) #

                print('Sent SOC: {}'.format(currentsoc[j]))

                print('')
            # after sending the soc of each EV,
            send_soc = 0
            # wait 1 min for instructions
            t = grantedtime
        else:
            # ask for instructions
            instructions = []
            for j in range(0,len(enddest_EVsoc)):
                # log the source of the message
                print('endpt name: ',h.helicsEndpointGetName(end_EVsoc[j]))
                msg = h.helicsEndpointGetMessageObject(end_EVsoc[j])
                #test = h.helicsMessageGetString(msg)
                if h.helicsEndpointHasMessage(msg):
                    instructions.append(
                        h.helicsMessageGetString(msg)
                    )
                    print('msg: ',instructions[j])
                    # right now assume
                    if instructions[j] == 1:
                        # add 19.2 kW times time difference
                        addenergy = 19.2*time_sim[]
                        # convert to %
                        currentsoc[j] = currentsoc[j] + addenergy/batt_size

            if instructions:
                send_soc = 1


        t = grantedtime

    logger.info("Destroying federate")
    destroy_federate(fed)
    logger.info("Done!")
