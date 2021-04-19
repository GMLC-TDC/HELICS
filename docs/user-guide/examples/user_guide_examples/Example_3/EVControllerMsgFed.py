"""
Created on 5/27/2020

@author: bearcub
"""
# export DYLD_LIBRARY_PATH="/Users/camp426/helics_install/lib"
# or put in bashrc file
import helics as h
import logging
import numpy as np
import sys
import time
#import graph
import argparse

logger = logging.getLogger(__name__)
logger.addHandler(logging.StreamHandler())
logger.setLevel(logging.DEBUG)

#

def destroy_federate(fed):
    status = h.helicsFederateFinalize(fed)
    h.helicsFederateFree(fed)
    h.helicsCloseLibrary()
    print("EVController: Federate finalized")

def create_message_federate(fedinitstring,name,period):
    # Create Federate Info object that describes the federate properties
    fedinfo = h.helicsCreateFederateInfo()
    # Set core type from string
    h.helicsFederateInfoSetCoreTypeFromString(fedinfo, "tcpss")
    #assert status == 0
    # Federate init string
    # you need to tell helics what message bus to use
    h.helicsFederateInfoSetCoreInitString(fedinfo, fedinitstring)
    #assert status == 0
    # Set one second message interval
    h.helicsFederateInfoSetTimeProperty(fedinfo, h.helics_property_time_period, period)
    #assert status == 0
    # set wait for current time update to true
    h.helicsFederateInfoSetFlagOption(fedinfo, h.helics_flag_uninterruptible, True)
    # h.helics_flag_uninterruptible should have integer value of 1
    #assert status == 0
    # see 'helics_federate_flags' in
    # https://docs.helics.org/en/latest/doxygen/helics__enums_8h_source.html
    h.helicsFederateInfoSetIntegerProperty(fedinfo, h.helics_property_int_log_level, 1)
    # more info:
    # https://docs.helics.org/en/latest/user-guide/logging.html
    # https://docs.helics.org/en/latest/doxygen/helics__enums_8h_source.html
    # scroll to section on 'helics_log_levels'
    #print('status is',status)
    # make sure these links aren't dead
    #assert status == 0
    # Create combo federate and give it a name
    fed = h.helicsCreateMessageFederate(name, fedinfo)

    # should this actually be a message federate?
    #fed = h.helicsCreateMessageFederate(name, fedinfo)
    print("Message federate created")

    return fed

if __name__ == "__main__":
    helicsversion = h.helicsGetVersion()
    print("EV_toy: Helics version = {}".format(helicsversion))


    parser = argparse.ArgumentParser(description='EV simulator')
    parser.add_argument('--port', type=int, default=-1,
                    help='port of the HELICS broker')


    args = parser.parse_args()

    if args.port != -1:
        fedinitstring="--brokerport="+str(args.port)
    else:
        fedinitstring=""

    print("Federate Init String = {}".format(fedinitstring))


    name = 'EVController_federate'
    # assume the EV Controller needs 1 minute to determine whether or not to charge
    # the vehicles
    period = 15*60 # 15 min
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
        dest_name = f'EV_federate/EV{EV}.soc'
        enddest_EVsoc.append(
            h.helicsEndpointSetDefaultDestination(
                end_EVsoc[EV-1], dest_name
            )
        )
        print(f"end point {end_name} registered to {dest_name}")

    #end_count = h.helicsFederateGetEndpointCount(fed)
    #print(end_count)
    fed_name = h.helicsFederateGetName(fed)
    print(" Federate {} has been registered".format(fed_name))
    #test = h.helicsEndpointGetName(end_EVsoc[0])
    #print(test)
######################   Entering Execution Mode  ##########################################################

    h.helicsFederateEnterExecutingMode(fed)


    hours = 24*7 # one week
    total_interval = int(60 * 60 * hours)
    update_interval = 30*60 # updates every 10 minutes
    grantedtime = -1
#
## Step through each time period starting from t = 0
    time_sim = [];  instructions = []
    #for t in range(0, total_interval, update_interval): #

    # the EV sent its first message at 15min
    # start the controller at 15min + 7.5min
    grantedtime = h.helicsFederateRequestTime (fed, 22.5*60)
    #print('EV time: ',grantedtime/3600)

    t = grantedtime
    while t < total_interval:

        for j in range(0,len(enddest_EVsoc)):
            # 1. Receive SOC
            #print('endpt name: ',h.helicsEndpointGetName(end_EVsoc[j]))
            if h.helicsEndpointHasMessage(end_EVsoc[j]):
                msg = h.helicsEndpointGetMessageObject(end_EVsoc[j])
                currentsoc = h.helicsMessageGetString(msg)
                #print('currentsoc: ',currentsoc)
                # 2. Send instructions
                #destination_name = str(h.helicsEndpointGetDefaultDestination(end_EVsoc[j]))
                print(t/3600,currentsoc)
                if float(currentsoc) <= 0.9:
                    instructions = 1
                else:
                    instructions = 0
                message = str(instructions)
                h.helicsEndpointSendMessageRaw(end_EVsoc[j], "", message) #
                #print('Sent instructions: {}'.format(instructions))
            else:
                print('NO MESSAGE RECEIVED AT TIME ',t/3600)


        grantedtime = h.helicsFederateRequestTime (fed, (t+update_interval))
        #print('EV time: ',grantedtime/3600)

        t = grantedtime


    #logger.info("Destroying federate")
    destroy_federate(fed)
    #logger.info("Done!")
