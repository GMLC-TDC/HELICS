loadlibrary('./../../../helics_install/lib/helics/libhelicsSharedLib.dylib');

fedinitstring = '--broker=mainbroker --federates=1';
deltat = 0.01;

helicsversion = helics.helicsGetVersion();

display(strcat('PI RECEIVER: Helics version = ', helicsversion));

% Create Federate Info object that describes the federate properties
fedinfo = helics.helicsFederateInfoCreate();

% Set Federate name
status = helics.helicsFederateInfoSetFederateName(fedinfo, 'TestB Federate');

% Set core type from string
status = helics.helicsFederateInfoSetCoreTypeFromString(fedinfo, 'zmq');

% Federate init string
status = helics.helicsFederateInfoSetCoreInitString(fedinfo, fedinitstring);

% Set the message interval (timedelta) for federate. Note that
% HELICS minimum message time interval is 1 ns and by default
% it uses a time delta of 1 second. What is provided to the
% setTimedelta routine is a multiplier for the default timedelta.

% Set one second message interval
status = helics.helicsFederateInfoSetTimeDelta(fedinfo, deltat);

status = helics.helicsFederateInfoSetLoggingLevel(fedinfo, 1);

% Create value federate
vfed = helics.helicsCreateValueFederate(fedinfo);
display('PI RECEIVER: Value federate created');

% Subscribe to PI SENDER's publication
sub = helics.helicsRegisterSubscription(vfed, 'testA', 'double', '');

display('PI RECEIVER: Subscription registered');

status = helics.helicsEnterExecutionMode(vfed);
display('PI RECEIVER: Entering execution mode');

value = 0.0;
prevtime = 0;

currenttime = helics.helicsRequestTime(vfed, 0.19);

while currenttime <= 0.19

    currenttime = helics.helicsRequestTime(vfed, 0.19);

    isupdated = helics.helicsIsValueUpdated(sub);

    if (isupdated == 1) && (currenttime > prevtime)
        [result, value] = helics.helicsGetDouble(sub);
        display(strcat('PI RECEIVER: Received value = ', num2str(value), '', ' at time ', num2str(currenttime), ' from PI SENDER'));
    end

    prevtime = currenttime;
end

status = helics.helicsFinalize(vfed);

helics.helicsFreeFederate(vfed);
helics.helicsCloseLibrary();

display('PI RECEIVER: Federate finalized');
