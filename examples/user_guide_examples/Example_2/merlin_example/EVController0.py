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

#logger = logging.getLogger(__name__)
#logger.addHandler(logging.StreamHandler())
#logger.setLoggingLevel(logging.DEBUG)

#

helicsversion = h.helicsGetVersion()

print("EV_toy: Helics version = {}".format(helicsversion))
broker = sys.argv[1]
#################################  Creating Broker  ########################################

initstring = "-f 2 --name=thisbroker"
# see command line
# https://docs.helics.org/en/latest/apps/Broker.html
fedinitstring = " --federates=1 --broker="+broker
deltat = 0.1

# print("Creating Broker")
# broker = h.helicsCreateBroker("zmq", "", initstring)
# print("Created Broker")

# print("Checking if Broker is connected")
# isconnected = h.helicsBrokerIsConnected(broker)
# print("Checked if Broker is connected")

# if isconnected == 1:
#     print("Broker created and connected")

# Create Federate Info object that describes the federate properties
fedinfo = h.helicsCreateFederateInfo()
# Set Federate name
h.helicsFederateInfoSetCoreName(fedinfo, 'EVController_federate')
# Set core type from string
h.helicsFederateInfoSetCoreTypeFromString(fedinfo, "zmq")
# Federate init string
h.helicsFederateInfoSetCoreInitString(fedinfo, fedinitstring)
# Set one second message interval
h.helicsFederateInfoSetTimeProperty(fedinfo, h.helics_property_time_delta, deltat)
# Create value federate
vfed = h.helicsCreateValueFederate('EVController_federate', fedinfo)
print("Value federate created")

# Register the publications
# the controller will publish
# to each EV
pub_EV = []
num_EVs = 10
EVs = range(1,num_EVs+1)
for EV in EVs:
    pub_name = f'Instruction.to.EV{EV}'
    pub_EV.append(
        h.helicsFederateRegisterGlobalTypePublication(
            vfed, pub_name, 'double', ''
        )
    )
    print(f'publication {pub_name} registered')

# Register the subscriptions
# each EV will subscribe to the EVController
sub_EVsoc = []
for EV in EVs:
    sub_name = f'EV{EV}.soc'
    sub_EVsoc.append(
        h.helicsFederateRegisterSubscription(
            vfed, sub_name, 'double'
        )
    )
    print(f"subscription {sub_name} registered")


pub_count = h.helicsFederateGetPublicationCount(vfed)
sub_count = h.helicsFederateGetInputCount(vfed)
print(pub_count,sub_count)


# Enter execution mode
h.helicsFederateEnterExecutingMode(vfed)
print("Controller: Entering execution mode")


currentsoc = np.full(sub_count,0.5)
# np.random.rand(pub_count)
instructions = np.ones(pub_count)

num_timesteps = 5
# Step through each time period starting from t = 0
for t in range(0, num_timesteps):
    currenttime = h.helicsFederateRequestTime(vfed, t)

    # Get all subscriptions
    # EVController asks all EVs for their current soc
    for i in range(0,sub_count):
        # Get input id
        sid = h.helicsFederateGetInputByIndex(vfed, i)
        key = h.helicsInputGetKey(sid)
        currentsoc[i] = h.helicsInputGetDouble(sid)
        print('Message from ' + key + ' received: {} at time {}'.format(currentsoc[i],currenttime))

    # Send all publications
    # EVController instructs EV to continue charging 1
    # or halt charging 0
    # depending on currentsoc
    for j in range(0,pub_count):
        # Get publication id
        pid = h.helicsFederateGetPublicationByIndex(vfed, j)
        key = h.helicsPublicationGetKey(pid)
        # generate the publication value and publish it
        if currentsoc[j] <= 0.9:
            instructions[j] = 1
        else:
            instructions[j] = 0
        print(instructions[j])
        h.helicsPublicationPublishDouble(pid, instructions[j])
        print('Message to ' + key + ' published: {} at time {}'.format(instructions[j], currenttime))

    print(' ')

h.helicsFederateFinalize(vfed)

h.helicsFederateFree(vfed)
h.helicsCloseLibrary()
print("EVController: Federate finalized")

quit()
