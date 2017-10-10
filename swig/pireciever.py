import helics as h

fedinitstring = "--broker=mainbroker --federates=1"
deltat = 0.01


helicsversion = h.helicsGetVersion()

print("PI RECEIVER: Helics version = {}".format(helicsversion))

# Create Federate Info object that describes the federate properties */
fedinfo = h.helicsFederateInfoCreate()

# Set Federate name
status = h.helicsFederateInfoSetFederateName(fedinfo, "TestB Federate")

# Set core type from string
status = h.helicsFederateInfoSetCoreTypeFromString(fedinfo, "zmq")

# Federate init string
status = h.helicsFederateInfoSetCoreInitString(fedinfo, fedinitstring)

# Set the message interval (timedelta) for federate. Note that
# HELICS minimum message time interval is 1 ns and by default
# it uses a time delta of 1 second. What is provided to the
# setTimedelta routine is a multiplier for the default timedelta.

# Set one second message interval
status = h.helicsFederateInfoSetTimeDelta(fedinfo, deltat)

status = h.helicsFederateInfoSetLoggingLevel(fedinfo, 1)

# Create value federate
vfed = h.helicsCreateValueFederate(fedinfo)
print("PI RECEIVER: Value federate created")

# Subscribe to PI SENDER's publication
sub = h.helicsRegisterSubscription(vfed, "testA", "double", "")
print("PI RECEIVER: Subscription registered")

status = h.helicsEnterExecutionMode(vfed)
print("PI RECEIVER: Entering execution mode")

value = 0.0

currenttime = h.helicsRequestTime(vfed, 0.19)

