
import helics as h

initstring = "2 --name=mainbroker"
fedinitstring = "--broker=mainbroker --federates=1"
deltat = 0.01

helicsversion = h.helicsGetVersion()

print("PI SENDER: Helics version = %s\n", helicsversion)
print("%s", help)

# Create broker #
broker = h.helicsCreateBroker("zmq", "", initstring)

isconnected = h.helicsBrokerIsConnected(broker)

if isconnected:
    print("PI SENDER: Broker created and connected")

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
pub = h.helicsRegisterGlobalPublication(vfed, "testA", "double", "")
print("PI SENDER: Publication registered")

# Enter execution mode #
status = h.helicsEnterExecutionMode(vfed)
print("PI SENDER: Entering execution mode")

# This federate will be publishing deltat*pi for numsteps steps #
this_time = 0.0
value = 22.0 / 7.0
