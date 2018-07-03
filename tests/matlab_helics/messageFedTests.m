function tests=messageFedTests

tests=functiontests(localfunctions);
end

function setup(testCase)  % do not change function name
% open a figure, for example

end

function teardown(testCase)  % do not change function name
% close figure, for example
end


function [fedStruct,success]=generateFed()
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
fedStruct.mFed=helicsCreateMessageFederate(fedInfo);
if (fedStruct.mFed==0)
    success=false;
end
helicsFederateInfoFree(fedInfo);
end

function success=closeStruct(fedStruct)
import helics.*
success=true;
status=helicsFederateFinalize(fedStruct.mFed);
if (status~=0)
    success=false;
end
while (helicsBrokerIsConnected(fedStruct.broker))
    pause(1);
end
helicsFederateFree(fedStruct.mFed);
helicsBrokerFree(fedStruct.broker);
helicsCloseLibrary();

end

function forceCloseStruct(fedStruct)
import helics.*
helicsFederateFinalize(fedStruct.mFed);

cnt=0;
while (helicsBrokerIsConnected(fedStruct.broker))
    pause(1);
    cnt=cnt+1;
    if (cnt>5)
        helicsBrokerDisconnect(fedStruct.broker);
        break;
    end
end

helicsFederateFree(fedStruct.mFed);
helicsBrokerFree(fedStruct.broker);
helicsCloseLibrary();

end

function testEndpointRegistration(testCase)
import matlab.unittest.constraints.IsTrue;
import helics.*
[feds,success]=generateFed();
testCase.verifyThat(success,IsTrue);
try
epid1 = helicsFederateRegisterEndpoint(feds.mFed, 'ep1', '');

epid2 = helicsFederateRegisterGlobalEndpoint(feds.mFed, 'ep2', 'random');


status=helicsFederateEnterExecutionMode(feds.mFed);
testCase.verifyEqual(status,helics.helics_ok);

[status, ept_key] = helicsEndpointGetName(epid1);
testCase.verifyEqual(status,helics.helics_ok);
testCase.verifyEqual(ept_key,'fed1/ep1');

[status, ept_key] = helicsEndpointGetName(epid2);
testCase.verifyEqual(status,helics.helics_ok);
testCase.verifyEqual(ept_key,'ep2');

[status, ept_type] = helicsEndpointGetType(epid1);
testCase.verifyEqual(status,helics.helics_ok);
testCase.verifyThat(isempty(ept_type),IsTrue);

[status, ept_type] = helicsEndpointGetType(epid2);
testCase.verifyEqual(status,helics.helics_ok);
testCase.verifyEqual(ept_type,'random');

catch e
    testCase.verifyThat(false,IsTrue);
   disp(e.message)
    disp(e.stack(1))
    forceCloseStruct(feds);
end
end

function testEndpointSend(testCase)
import matlab.unittest.constraints.IsTrue;
import helics.*
[feds,success]=generateFed();
testCase.verifyThat(success,IsTrue);
try
epid1 = helicsFederateRegisterEndpoint(feds.mFed, 'ep1', '');

epid2 = helicsFederateRegisterGlobalEndpoint(feds.mFed, 'ep2', 'random');

helicsFederateSetTimeDelta(feds.mFed,1.0);
status=helicsFederateEnterExecutionMode(feds.mFed);
testCase.verifyEqual(status,helics.helics_ok);
data = 'this is a random string message';

status=helicsEndpointSendEventRaw(epid1,'ep2',data,1.0);
testCase.verifyEqual(status,helics.helics_ok);



[status,granted_time]=helicsFederateRequestTime(feds.mFed,2.0);
testCase.verifyEqual(status,helics.helics_ok);
testCase.verifyEqual(granted_time,1.0);

res=helicsFederateHasMessage(feds.mFed);
testCase.verifyEqual(res,helics_true);
res=helicsEndpointHasMessage(epid1);
testCase.verifyEqual(res,helics_false);
res=helicsEndpointHasMessage(epid2);
testCase.verifyEqual(res,helics_true);

message = helicsEndpointGetMessage(epid2);
testCase.verifyEqual(message.data,data);
testCase.verifyEqual(double(message.length),length(data));
testCase.verifyEqual(message.original_source,'fed1/ep1');
testCase.verifyEqual(message.time,1.0);
success=closeStruct(feds);
testCase.verifyThat(success,IsTrue);


catch e
    testCase.verifyThat(false,IsTrue);
   disp(e.message)
    disp(e.stack(1))
    forceCloseStruct(feds);
end
end

