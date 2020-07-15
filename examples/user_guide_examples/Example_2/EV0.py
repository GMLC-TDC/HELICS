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

#logger = logging.getLogger(__name__)
#logger.addHandler(logging.StreamHandler())
#logger.setLoggingLevel(logging.DEBUG)

broker = sys.argv[1]
fedinitstring = " --federates=1 --broker="+broker

deltat = 0.1
# Create Federate Info object that describes the federate properties
fedinfo = h.helicsCreateFederateInfo()
# Set Federate name
h.helicsFederateInfoSetCoreName(fedinfo, 'EV_federate')
# Set core type from string
h.helicsFederateInfoSetCoreTypeFromString(fedinfo, "zmq")
# Federate init string
h.helicsFederateInfoSetCoreInitString(fedinfo, fedinitstring)
# Set one second message interval
h.helicsFederateInfoSetTimeProperty(fedinfo, h.helics_property_time_delta, deltat)
# Create value federate
vfed = h.helicsCreateValueFederate('EV_federate', fedinfo)
print("Value federate created")

# Register the publications
# each EV will publish their SOC
pub_EVsoc = []
num_EVs = 10
EVs = range(1,num_EVs+1)
for EV in EVs:
    pub_name = f'EV{EV}.soc'
    pub_EVsoc.append(
        h.helicsFederateRegisterGlobalTypePublication(
            vfed, pub_name, 'double', ''
        )
    )
    print(f'publication {pub_name} registered')

# Register the subscriptions
# each EV will subscribe to the EVController
sub_EV = []
for EV in EVs:
    sub_name = f'Instruction.to.EV{EV}'
    sub_EV.append(
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

num_timesteps = 5


#currentsoc = #np.full(pub_count,0.5)
currentsoc = np.random.rand(pub_count)
instructions = np.ones(sub_count)
# Step through each time period starting from t = 0

for t in range(0, num_timesteps):
    currenttime = h.helicsFederateRequestTime(vfed, t)

    # Send all publications
    for j in range(0,pub_count):
        # Get publication id
        pid = h.helicsFederateGetPublicationByIndex(vfed, j)
        key = h.helicsPublicationGetKey(pid)
        # generate the publication value and publish it
        if instructions[j] == 1:
            currentsoc[j] = currentsoc[j] + 0.1
        h.helicsPublicationPublishDouble(pid, currentsoc[j])
        print('Message to ' + key + ' published: {} at time {}'.format(currentsoc[j], currenttime))

    # Get all subscriptions
    for i in range(0,sub_count):
        # Get input id
        sid = h.helicsFederateGetInputByIndex(vfed, i)
        key = h.helicsInputGetKey(sid)
        instructions[i] = h.helicsInputGetDouble(sid)
        print('Message from ' + key + ' received: {} at time {}'.format(instructions[i],currenttime))

    print(' ')
    time.sleep(1)

h.helicsFederateFinalize(vfed)

h.helicsFederateFree(vfed)
h.helicsCloseLibrary()
print("EVs: Federate finalized")

quit()
