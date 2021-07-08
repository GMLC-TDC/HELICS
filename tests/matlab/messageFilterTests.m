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
initstring = '-f1';
fedinitstring = '--broker=mainbroker --federates=1';
fedStruct.broker=helicsCreateBroker('zmq','mainbroker',initstring);
if (~helicsBrokerIsValid(fedStruct.broker))
    success=false;
    return;
end
fedInfo=helicsCreateFederateInfo();
if (fedInfo==0)
    success=false;
    return;
end
try
helicsFederateInfoSetCoreTypeFromString(fedInfo,'zmq');
helicsFederateInfoSetCoreInitString(fedInfo,fedinitstring);
helicsFederateInfoSetTimeProperty(fedInfo,HELICS_PROPERTY_TIME_DELTA, 0.01);
helicsFederateInfoSetIntegerProperty(fedInfo,HELICS_PROPERTY_INT_LOG_LEVEL,HELICS_LOG_LEVEL_WARNING);
catch ec
    success=false;
    helicsBrokerDestroy(fedStruct.broker);
    helicsFederateInfoFree(fedInfo);
    return
end
for ii=1:count
fedStruct.mFed{ii}=helicsCreateMessageFederate(['fed',num2str(ii)],fedInfo);
if (fedStruct.mFed{ii}==0)
    success=false;
end
end
helicsFederateInfoFree(fedInfo);
end

function success=closeStruct(fedStruct)
import helics.*
success=true;
try
for ii=1:numel(fedStruct.mFed)
helicsFederateFinalize(fedStruct.mFed{ii});
end
helicsBrokerWaitForDisconnect(fedStruct.broker,2000);

for ii=1:numel(fedStruct.mFed)
helicsFederateFree(fedStruct.mFed{ii});
end
helicsBrokerFree(fedStruct.broker);
helicsCloseLibrary();
catch
    success=false;
end
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
initstring = '-f 1 --name=mainbroker';
broker=helicsCreateBroker('zmq','',initstring);
ident=helicsBrokerGetIdentifier(broker);
testCase.verifyEqual(ident,'mainbroker');
add=helicsBrokerGetAddress(broker);
testCase.verifyEqual(add,'tcp://127.0.0.1:23404');
helicsBrokerDisconnect(broker);
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

f1=helicsFederateRegisterGlobalFilter(fFed,helics.HELICS_FILTER_TYPE_CUSTOM,'filter1');
helicsFilterAddSourceTarget(f1,'port1');
f2=helicsFederateRegisterGlobalFilter(fFed,helics.HELICS_FILTER_TYPE_DELAY,'filter2');
helicsFilterAddDestinationTarget(f2,'port2');
helicsFederateRegisterEndpoint(fFed,'fout','');
f3=helicsFederateRegisterFilter(fFed,helics.HELICS_FILTER_TYPE_RANDOM_DELAY,'filter3');
helicsFilterAddSourceTarget(f3,'fed2/fout');

helicsFederateEnterExecutingModeAsync(mFed);
helicsFederateEnterExecutingMode(fFed);

helicsFederateEnterExecutingModeComplete(mFed);

filt_key = helicsFilterGetName(f1);
testCase.verifyEqual(filt_key,'filter1');

filt_key = helicsFilterGetName(f2);
testCase.verifyEqual(filt_key,'filter2');

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

f1=helicsFederateRegisterFilter(fFed,helics.HELICS_FILTER_TYPE_DELAY,'filter1');
helicsFilterAddSourceTarget(f1,'port1');
helicsFilterSet(f1,'delay',2.5);

helicsFederateEnterExecutingModeAsync(mFed);
helicsFederateEnterExecutingMode(fFed);
helicsFederateEnterExecutingModeComplete(mFed);

data='hello world';
helicsEndpointSendBytesTo(p1,data,'port2');

granted_time=helicsFederateRequestTime(mFed,1.0);
testCase.verifyEqual(granted_time,1.0);

res=helicsFederateHasMessage(mFed);
testCase.verifyEqual(res,HELICS_FALSE);

granted_time=helicsFederateRequestTime(mFed,3.0);
testCase.verifyEqual(granted_time,2.5);


res=helicsEndpointHasMessage(p2);
testCase.verifyEqual(res,HELICS_TRUE);


success=closeStruct(feds);
testCase.verifyThat(success,IsTrue);
catch e
    testCase.verifyThat(false,IsTrue);
   disp(e.message)
    disp(e.stack(1))
    forceCloseStruct(feds);
end
end
