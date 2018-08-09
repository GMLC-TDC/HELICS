% PISENDER script demonstrating MATLAB-HELICS interface
%
% Usage:
%  1. Start two separate MATLAB terminals 
%  2. In the first: 
%     >> pisender
%  3. In the second:
%     >> pireciever


%% Initialize HELICS library in MATLAB
helicsStartup()

%% Configuration
deltat = 1;  %Base time interval (seconds)
numsteps = 20;

% HELICS options
% Note: these configure this matlab process to host the main broker and 1
% federate
pisend_start_broker = true;
helics_core_type = 'zmq'; 
broker_initstring = '2 --name=mainbroker';
fed_initstring = '--broker=mainbroker --federates=1';

%% Provide summary information
helicsversion = helics.helicsGetVersion();

fprintf('PI SENDER (with main broker): Helics version = %s\n', helicsversion)

%% Create broker (if desired)
if pisend_start_broker
    disp('Creating Broker');
    broker = helics.helicsCreateBroker(helics_core_type, '', broker_initstring);
    disp('Created Broker');

    fprintf('Checking if Broker is connected...');
    isconnected = helics.helicsBrokerIsConnected(broker);

    if isconnected == 1
        fprintf('SUCCESS, Broker created and connected\n');
    else
        fprintf('\n')
        error('NOT CONNECTED (helicsBrokerIsConnected return = %d)', isconnected)
    end
end

%% Create Federate Info object that describes the federate properties
fedinfo = helics.helicsFederateInfoCreate();
assert(not(isempty(fedinfo)))

% Set Federate name
status = helics.helicsFederateInfoSetFederateName(fedinfo, 'MATLAB Pi SENDER Federate');
assert(status==0)

% Set core type from string
status = helics.helicsFederateInfoSetCoreTypeFromString(fedinfo, helics_core_type);
assert(status==0)

% Federate init string
status = helics.helicsFederateInfoSetCoreInitString(fedinfo, fed_initstring);
assert(status==0)


% Note:
% HELICS minimum message time interval is 1 ns and by default
% it uses a time delta of 1 second. What is provided to the
% setTimedelta routine is a multiplier for the default timedelta 
% (default unit = seconds).

% Set one message interval
status = helics.helicsFederateInfoSetTimeDelta(fedinfo, deltat);
assert(status==0)

status = helics.helicsFederateInfoSetLoggingLevel(fedinfo, 1);
assert(status==0)

%% Actually create value federate
vfed = helics.helicsCreateValueFederate(fedinfo);
disp('PI SENDER: Value federate created');

%% Register our value to publish
pub = helics.helicsFederateRegisterGlobalPublication(vfed, 'testA', 'double', '');
disp('PI SENDER: Publication registered (testA)');

%% Start execution
status = helics.helicsFederateEnterExecutionMode(vfed);
if status == 0
    disp('PI SENDER: Entering execution mode');
else
    error('PI SENDER: Failed to enter execution mode (status = %d)\n Try running pisender.m first. (or start the broker seperately)', status);
end

%% Execution Loop
% This federate will be publishing deltat*pi for numsteps steps
this_time = 0.0;
value = pi;

for i = 1:numsteps
    val = value;

    [status, granted_time] = helics.helicsFederateRequestTime(vfed, i);
    assert(status==0)

%    fprintf('PI SENDER: Sending value pi = %g at time %f to PI RECEIVER\n', val, this_time + (deltat * i));
    fprintf('PI SENDER: Publishing value pi = %g at time %4.1f... ', val, granted_time);
    status = helics.helicsPublicationPublishDouble(pub, val);
    fprintf('DONE (status=%d)\n', status);
end

%% Shutdown

if pisend_start_broker
    % If we started the broker in this thread, we have to be careful
    % sequencing the shutdown in hopes of doing so cleanly
    status = helics.helicsFederateFinalize(vfed);
    disp('PI SENDER: Federate finalized');

    %Make sure the broker is gone in case we have a lingering low-level
    %reference (to avoid memory leaks)
    for foo = 1:60
        if not(helics.helicsBrokerIsConnected(broker))
            break
        end
        pause(1);
    end
    disp('PI SENDER: Broker disconnected');

    helics.helicsFederateFree(vfed);
    helics.helicsCloseLibrary();
else
    %But if we just setup the federate, we can simply call endFederate
    helicsDestroyFederate(vfed); %#ok<UNRCH>  
    disp('PI SENDER: Federate finalized');
end



