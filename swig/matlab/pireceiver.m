% PIRECEIVER script demonstrating MATLAB-HELICS interface
%
% See pisender for usage information
%
% This example attempts to request the next time equal to its
% sim_stop_time, but the corresponding pisender will request intermediate
% times such that the pireceiver will be granted times earlier than
% requested
%
% Note to non-programmers: if unfamiliar, the assert functions simply check
% if the condition is true and if so continues. If false, execution stops.

%% Initialize HELICS library in MATLAB
helicsStartup()

%% Configuration
deltat = 0.01;  %Base time interval (seconds)
sim_stop_time = 20;

% HELICS options
helics_core_type = 'zmq'; 
fedinitstring = '--federates=1';  % required with current C interface when using separate processes for each federate

%% Provide summary information
helicsversion = helics.helicsGetVersion();

fprintf('PI RECEIVER: Helics version = %s\n', helicsversion)

%% Create Federate Info object that describes the federate properties
fedinfo = helics.helicsFederateInfoCreate();
assert(not(isempty(fedinfo)))

% Set Federate name
status = helics.helicsFederateInfoSetFederateName(fedinfo, 'MATLAB Pi Receiver Federate');
assert(status==0)

% Set core type from string
status = helics.helicsFederateInfoSetCoreTypeFromString(fedinfo, helics_core_type);
assert(status==0)

% Federate init string
status = helics.helicsFederateInfoSetCoreInitString(fedinfo, fedinitstring);
assert(status==0)

%% Set the message interval (timedelta) for federate. 
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
disp('PI RECEIVER: Value federate created');

% Subscribe to PI SENDER's publication (note: published as global)
sub = helics.helicsFederateRegisterSubscription(vfed, 'testA', 'double', '');

disp('PI RECEIVER: Subscription registered (testA)');

%% Start execution
status = helics.helicsFederateEnterExecutionMode(vfed);
if status == 0
    disp('PI RECEIVER: Entering execution mode');
else
    error('PI RECEIVER: Failed to enter execution mode (status = %d)\n Try running pisender.m first. (or start the broker seperately)', status);
end

%% Execution Loop
granted_time =- 1;  %Force at least one run

%% Continue execution until end of requested simulation time
while granted_time <= sim_stop_time

    [status, granted_time] = helics.helicsFederateRequestTime(vfed, sim_stop_time);
    assert(status==0)

    isupdated = helics.helicsSubscriptionIsUpdated(sub);

    if (isupdated == 1)
        [result, value] = helics.helicsSubscriptionGetDouble(sub);
        fprintf('PI RECEIVER: Received value = %g at time %4.1f from PI SENDER\n', value, granted_time);
    end
end

%% Shutdown
helicsDestroyFederate(vfed);

disp('PI RECEIVER: Federate finalized');

