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

def create_broker():
    initstring = "-f 2 --name=thisbroker"
    broker = h.helicsCreateBroker("zmq", "", initstring)
    isconnected = h.helicsBrokerIsConnected(broker)

    if isconnected == 1:
        print("Broker created and connected")
        pass

    return broker

def destroy_federate(fed, broker):
    status = h.helicsFederateFinalize(fed)
    while h.helicsBrokerIsConnected(broker):
        print('broker is still connected')
        time.sleep(1)

    h.helicsFederateFree(fed)
    h.helicsCloseLibrary()
    print("EVController: Federate finalized")

def create_message_federate(fedinitstring,name,deltat):
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
    h.helicsFederateInfoSetTimeProperty(fedinfo, h.helics_property_time_delta, deltat)
    #assert status == 0
    # set wait for current time update to true
    h.helicsFederateInfoSetFlagOption(fedinfo, h.helics_flag_wait_for_current_time_update, False)
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
    deltat = 60*60 # 60 seconds
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

    print("Just entered execution mode")

    hours = 24*7
    total_interval = int(60 * 60 * hours)
    update_interval = 60*60 # updates every 10 minutes
    grantedtime = -1
#
## Step through each time period starting from t = 0
    time_sim = [];  instructions = []
    #for t in range(0, total_interval, update_interval): #

    t = 10; query_fleet = 1
    while t < total_interval:

        #while grantedtime < t:
        # if t is 10 minutes, grantedtime could be 5 minutes
        print("Requesting Time ", t)
        grantedtime = h.helicsFederateRequestTime (fed, t)
        # default minimum timestep is very very small
        # print('NEXT TIME WILL BE: ',grantedtime/3600)

        # current minute
        time_sim.append(t/3600)
        print('CURRENT TIME in HOURS: ',t/3600)
        if query_fleet:
            #  get SOC from each EV in the fleet
            # if we're querying the fleet, then
            # we need to respond to the fleet
            # in ~ 1 minute
            currentsoc = []
            for j in range(0,len(enddest_EVsoc)):
                # log the source of the message
                print('endpt name: ',h.helicsEndpointGetName(end_EVsoc[j]))
                if h.helicsEndpointHasMessage(end_EVsoc[j]):
                    msg = h.helicsEndpointGetMessageObject(end_EVsoc[j])
                #test = h.helicsMessageGetString(msg)
                    currentsoc.append(
                        h.helicsMessageGetString(msg)
                    )
                    print('msg: ',h.helicsMessageGetString(msg),h.helicsMessageIsValid(msg))
                #else:

                    #continue
                    query_fleet = 0
                else:
                    print('NO MESSAGE RECEIVED AT TIME ',t)
            t = grantedtime

            #if currentsoc:
            #    # in 1 minute,
            #    t = grantedtime
            #    # don't query the fleet, rather,
            #    # tell them whether or not to continue charging
            #    query_fleet = 0
            #else:
            #    # otherwise, increment time by update_interval = 10 min
            #    t += update_interval

        else:
            # the most recent soc is known, send instructions to EVs
            for j in range(0,len(enddest_EVsoc)):
                print(' soc: ',currentsoc[j],type(currentsoc[j]))
                # log the source of the message
                end_name = str(h.helicsEndpointGetName(end_EVsoc[j]))
                # this is the name of the source endpoint
                print('Sending endpoint name: ',end_name)
                destination_name = str(h.helicsEndpointGetDefaultDestination(end_EVsoc[j]))
                print('destination endpoint name: ',destination_name)
                #destination_federate = str(destination_name.split('/EV')[0])
                #print('Endpoint destination federate: ',destination_federate)
                print('currentsoc: ',currentsoc[j])
                if float(currentsoc[j]) <= 0.9:
                    instructions = 1
                else:
                    instructions = 0
                h.helicsEndpointSendMessageRaw(end_EVsoc[j], "", str(instructions)) #

                print('Sent instructions: {}'.format(instructions))


            # query the EV fleet again
            query_fleet = 1
            # after 10 minutes
            t = grantedtime




        # set t to 5 minutes
        # if there is something you need to deal with, then you want
        # t to be equal to granted time plus min timestep
        #t = grantedtime #+ update_interval

    #logger.info("Destroying federate")
    destroy_federate(fed, broker)
    #logger.info("Done!")
