import helics as h

fedinitstring = "--federates=1"
deltat = 0.01

helicsversion = h.helicsGetVersion()

print("PI RECEIVER: Helics version = {}".format(helicsversion))

# Create Federate Info object that describes the federate properties */
print("PI RECEIVER: Creating Federate Info")
fedinfo = h.helicsFederateInfoCreate()

# Set Federate name
print("PI RECEIVER: Setting Federate Info Name")
status = h.helicsFederateInfoSetFederateName(fedinfo, "TestB Federate")

# Set core type from string
print("PI RECEIVER: Setting Federate Info Core Type")
status = h.helicsFederateInfoSetCoreTypeFromString(fedinfo, "zmq")

# Federate init string
print("PI RECEIVER: Setting Federate Info Init String")
status = h.helicsFederateInfoSetCoreInitString(fedinfo, fedinitstring)

# Set the message interval (timedelta) for federate. Note that
# HELICS minimum message time interval is 1 ns and by default
# it uses a time delta of 1 second. What is provided to the
# setTimedelta routine is a multiplier for the default timedelta.

# Set one second message interval
print("PI RECEIVER: Setting Federate Info Time Delta")
status = h.helicsFederateInfoSetTimeDelta(fedinfo, deltat)

print("PI RECEIVER: Setting Federate Info Logging")
status = h.helicsFederateInfoSetLoggingLevel(fedinfo, 1)

# Create value federate
print("PI RECEIVER: Creating Value Federate")
vfed = h.helicsCreateValueFederate(fedinfo)
print("PI RECEIVER: Value federate created")

# Subscribe to PI SENDER's publication
sub = h.helicsFederateRegisterSubscription(vfed, "testA", "double", "")
print("PI RECEIVER: Subscription registered")

status = h.helicsFederateEnterExecutionMode(vfed)
print("PI RECEIVER: Entering execution mode")

value = 0.0
prevtime = 0

currenttime = h.helicsFederateRequestTime(vfed, 100)[-1]
print("PI RECEIVER: Current time is {} ".format(currenttime))

isupdated = h.helicsSubscriptionIsUpdated(sub)

if (isupdated == 1):
    result, value = h.helicsSubscriptionGetDouble(sub)
    print("PI RECEIVER: Received value = {} at time {} from PI SENDER".format(value, currenttime))

while (currenttime <= 100):

    currenttime = h.helicsFederateRequestTime(vfed, 100)[-1]

    isupdated = h.helicsSubscriptionIsUpdated(sub)

    if (isupdated == 1):
        result, value = h.helicsSubscriptionGetDouble(sub)
        print("PI RECEIVER: Received value = {} at time {} from PI SENDER".format(value, currenttime))

status = h.helicsFederateFinalize(vfed)

h.helicsFederateFree(vfed)
h.helicsCloseLibrary()
print("PI RECEIVER: Federate finalized")

