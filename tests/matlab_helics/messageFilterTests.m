function tests=messageFilterTests

tests=functiontests(localfunctions);
end

function setup(testCase)  % do not change function name
% open a figure, for example

end

function teardown(testCase)  % do not change function name
% close figure, for example
end


function [fedStruct,success]=generateFeds(count)
import helics.*
success=true;
initstring = '1 --name=mainbroker';
fedinitstring = '--broker=mainbroker --federates=1';
fedStruct.broker=helicsCreateBroker('zmq','',initstring);
if (fedStruct.broker==0)
    success=false;
    return;
end
fedInfo=helicsFederateInfoCreate();
if (fedInfo==0)
    success=false;
    return;
end
status=helicsFederateInfoSetFederateName(fedInfo,'fed1');
if (status~=0)
    success=false;
end
status=helicsFederateInfoSetCoreTypeFromString(fedInfo,'zmq');
if (status~=0)
    success=false;
end
status=helicsFederateInfoSetCoreInitString(fedInfo,fedinitstring);
if (status~=0)
    success=false;
end
status=helicsFederateInfoSetTimeDelta(fedInfo, 0.01);
if (status~=0)
    success=false;
end
status=helicsFederateInfoSetLoggingLevel(fedInfo,1);
if (status~=0)
    success=false;
end
for ii=1:count
    helicsFederateInfoSetFederateName(fedInfo,['fed',num2str(ii)]);
fedStruct.mFed{ii}=helicsCreateMessageFederate(fedInfo);
if (fedStruct.mFed{ii}==0)
    success=false;
end
end
helicsFederateInfoFree(fedInfo);
end

function success=closeStruct(fedStruct)
import helics.*
success=true;
for ii=1:numel(fedStruct.mFed)
status=helicsFederateFinalize(fedStruct.mFed{ii});
if (status~=0)
    success=false;
end
end
while (helicsBrokerIsConnected(fedStruct.broker))
    pause(1);
end
for ii=1:numel(fedStruct.mFed)
helicsFederateFree(fedStruct.mFed{ii});
end
helicsBrokerFree(fedStruct.broker);
helicsCloseLibrary();

end

function forceCloseStruct(fedStruct)
import helics.*
for ii=1:numel(fedStruct.mFed)
helicsFederateFinalize(fedStruct.mFed{ii});
end
cnt=0;
while (helicsBrokerIsConnected(fedStruct.broker))
    pause(1);
    cnt=cnt+1;
    if (cnt>5)
        helicsBrokerDisconnect(fedStruct.broker);
        break;
    end
end
for ii=1:numel(fedStruct.mFed)
helicsFederateFree(fedStruct.mFed{ii});
end
helicsBrokerFree(fedStruct.broker);
helicsCloseLibrary();

end

%function to test some of the broker functions
function testBrokerFunctions(testCase)
import matlab.unittest.constraints.IsTrue;
import helics.*
initstring = '1 --name=mainbroker';
broker=helicsCreateBroker('zmq','',initstring);
[status, ident]=helicsBrokerGetIdentifier(broker);
testCase.verifyEqual(status,helics.helics_ok);
testCase.verifyEqual(ident,'mainbroker');
[status, add]=helicsBrokerGetAddress(broker);
testCase.verifyEqual(status,helics.helics_ok);
testCase.verifyEqual(add,'tcp://127.0.0.1:23404');
status=helicsBrokerDisconnect(broker);
testCase.verifyEqual(status,helics.helics_ok);
helicsBrokerFree(broker);
helicsCloseLibrary();
end

function testFilterRegistration(testCase)
import matlab.unittest.constraints.IsTrue;
import helics.*
[feds,success]=generateFeds(2);
testCase.verifyThat(success,IsTrue);
try
mFed=feds.mFed{1};
fFed=feds.mFed{2};

helicsFederateRegisterGlobalEndpoint(mFed, 'port1', '');

helicsFederateRegisterGlobalEndpoint(mFed, 'port2', 'random');

f1=helicsFederateRegisterSourceFilter(fFed,helics.helics_custom_filter,'port1','filter1');
f2=helicsFederateRegisterDestinationFilter(fFed,helics.helics_delay_filter,'port2','filter2');
helicsFederateRegisterEndpoint(fFed,'fout','');
helicsFederateRegisterSourceFilter(fFed,helics.helics_randomDelay_filter,'fed2/fout','filter3');

status=helicsFederateEnterExecutionModeAsync(mFed);
testCase.verifyEqual(status,helics.helics_ok);
status=helicsFederateEnterExecutionMode(fFed);
testCase.verifyEqual(status,helics.helics_ok);
status=helicsFederateEnterExecutionModeComplete(mFed);
testCase.verifyEqual(status,helics.helics_ok);

[status, filt_key] = helicsFilterGetName(f1);
testCase.verifyEqual(status,helics.helics_ok);
testCase.verifyEqual(filt_key,'filter1');

[status, filt_key] = helicsFilterGetName(f2);
testCase.verifyEqual(status,helics.helics_ok);
testCase.verifyEqual(filt_key,'filter2');

[status, filt_key] = helicsFilterGetTarget(f2);
testCase.verifyEqual(status,helics.helics_ok);
testCase.verifyEqual(filt_key,'port2');

success=closeStruct(feds);
testCase.verifyThat(success,IsTrue);
catch e
    testCase.verifyThat(false,IsTrue);
   disp(e.message)
    disp(e.stack(1))
    forceCloseStruct(feds);
end
end

function testFilterFunction(testCase)
import matlab.unittest.constraints.IsTrue;
import helics.*
[feds,success]=generateFeds(2);
testCase.verifyThat(success,IsTrue);
try
mFed=feds.mFed{1};
fFed=feds.mFed{2};

p1=helicsFederateRegisterGlobalEndpoint(mFed, 'port1', '');

p2=helicsFederateRegisterGlobalEndpoint(mFed, 'port2', '');

f1=helicsFederateRegisterSourceFilter(fFed,helics.helics_delay_filter,'port1','filter1');
status=helicsFilterSet(f1,'delay',2.5);
testCase.verifyEqual(status,helics.helics_ok);

status=helicsFederateEnterExecutionModeAsync(mFed);
testCase.verifyEqual(status,helics.helics_ok);
status=helicsFederateEnterExecutionMode(fFed);
testCase.verifyEqual(status,helics.helics_ok);
status=helicsFederateEnterExecutionModeComplete(mFed);
testCase.verifyEqual(status,helics.helics_ok);

data='hello world';
helicsEndpointSendMessageRaw(p1,'port2',data);

[status,granted_time]=helicsFederateRequestTime(mFed,1.0);
testCase.verifyEqual(status,helics.helics_ok);
testCase.verifyEqual(granted_time,1.0);

res=helicsFederateHasMessage(mFed);
testCase.verifyEqual(res,helics_false);

[status,granted_time]=helicsFederateRequestTime(mFed,3.0);
testCase.verifyEqual(status,helics.helics_ok);
testCase.verifyEqual(granted_time,2.5);


res=helicsEndpointHasMessage(p2);
testCase.verifyEqual(res,helics_true);


success=closeStruct(feds);
testCase.verifyThat(success,IsTrue);
catch e
    testCase.verifyThat(false,IsTrue);
   disp(e.message)
    disp(e.stack(1))
    forceCloseStruct(feds);
end
end
