
loadlibrary(GetFullPath('~/local/helics-develop/lib/libhelicsSharedLib.dylib'));

initstring = '2 --name=mainbroker';
fedinitstring = '--broker=mainbroker --federates=1';
deltat = 0.01;

helicsversion = helics.helicsGetVersion();

display(strcat('PI SENDER: Helics version = ', helicsversion));

% Create broker
display('Creating Broker');
broker = helics.helicsCreateBroker('zmq', '', initstring);
display('Created Broker');

display('Checking if Broker is connected');
isconnected = helics.helicsBrokerIsConnected(broker);
display('Checked if Broker is connected');

if isconnected == 1;
    display('Broker created and connected');
end;

% Create Federate Info object that describes the federate properties
fedinfo = helics.helicsFederateInfoCreate();

% Set Federate name
status = helics.helicsFederateInfoSetFederateName(fedinfo, 'TestA Federate');

% Set core type from string
status = helics.helicsFederateInfoSetCoreTypeFromString(fedinfo, 'zmq');

% Federate init string
status = helics.helicsFederateInfoSetCoreInitString(fedinfo, fedinitstring);

% Set the message interval (timedelta) for federate. Note th#
% HELICS minimum message time interval is 1 ns and by default
% it uses a time delta of 1 second. What is provided to the
% setTimedelta routine is a multiplier for the default timedelta.

% Set one second message interval
status = helics.helicsFederateInfoSetTimeDelta(fedinfo, deltat);

status = helics.helicsFederateInfoSetLoggingLevel(fedinfo, 1);

% Create value federate
vfed = helics.helicsCreateValueFederate(fedinfo);
display('PI SENDER: Value federate created');

% Register the publication
pub = helics.helicsFederateRegisterGlobalPublication(vfed, 'testA', 'double', '');
display('PI SENDER: Publication registered');

% Enter execution mode
status = helics.helicsFederateEnterExecutionMode(vfed);
display('PI SENDER: Entering execution mode');

% This federate will be publishing deltat*pi for numsteps steps
this_time = 0.0;
value = 22.0 / 7.0;

for i = 1:20
    val = value;

    currenttime = helics.helicsFederateRequestTime(vfed, i);

    display(['PI SENDER: Sending value pi = ', '', num2str(val), ' at time ', '', num2str(this_time + (deltat * i)),' to PI RECEIVER']);
    status = helics.helicsPublicationPublishDouble(pub, val);
end

status = helics.helicsFederateFinalize(vfed);
display('PI SENDER: Federate finalized');


while helics.helicsBrokerIsConnected(broker)
    pause(1);
end

helics.helicsFederateFree(vfed);
helics.helicsCloseLibrary();

display('PI SENDER: Broker disconnected');
