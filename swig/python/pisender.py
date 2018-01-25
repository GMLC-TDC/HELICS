import time
import helics as h

initstring = "2 --name=mainbroker"
fedinitstring = "--broker=mainbroker --federates=1"
deltat = 0.01

helicsversion = h.helicsGetVersion()

print("PI SENDER: Helics version = {}".format(helicsversion))

# Create broker #
print("Creating Broker")
broker = h.helicsCreateBroker("zmq", "", initstring)
print("Created Broker")

print("Checking if Broker is connected")
isconnected = h.helicsBrokerIsConnected(broker)
print("Checked if Broker is connected")

if isconnected == 1:
    print("Broker created and connected")

# Create Federate Info object that describes the federate properties #
fedinfo = h.helicsFederateInfoCreate()

# Set Federate name #
status = h.helicsFederateInfoSetFederateName(fedinfo, "TestA Federate")

# Set core type from string #
status = h.helicsFederateInfoSetCoreTypeFromString(fedinfo, "zmq")

# Federate init string #
status = h.helicsFederateInfoSetCoreInitString(fedinfo, fedinitstring)

# Set the message interval (timedelta) for federate. Note th#
# HELICS minimum message time interval is 1 ns and by default
# it uses a time delta of 1 second. What is provided to the
# setTimedelta routine is a multiplier for the default timedelta.

# Set one second message interval #
status = h.helicsFederateInfoSetTimeDelta(fedinfo, deltat)

status = h.helicsFederateInfoSetLoggingLevel(fedinfo, 1)

# Create value federate #
vfed = h.helicsCreateValueFederate(fedinfo)
print("PI SENDER: Value federate created")

# Register the publication #
pub = h.helicsFederateRegisterGlobalPublication(vfed, "testA", "double", "")
print("PI SENDER: Publication registered")

# Enter execution mode #
status = h.helicsFederateEnterExecutionMode(vfed)
print("PI SENDER: Entering execution mode")

# This federate will be publishing deltat*pi for numsteps steps #
this_time = 0.0
value = 22.0 / 7.0

for t in range(5, 10):
    val = value

    currenttime = h.helicsFederateRequestTime(vfed, t)

    status = h.helicsPublicationPublishDouble(pub, val)
    print("PI SENDER: Sending value pi = {} at time {} to PI RECEIVER".format(val, currenttime[-1]))

    time.sleep(1)

status = h.helicsFederateFinalize(vfed)
print("PI SENDER: Federate finalized")

while (h.helicsBrokerIsConnected(broker)):
    time.sleep(1)

h.helicsFederateFree(vfed)
h.helicsCloseLibrary()

print("PI SENDER: Broker disconnected")
