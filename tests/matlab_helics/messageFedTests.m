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
helicsFederateInfoSetTimeProperty(fedInfo,helics_property_time_delta, 0.01);
helicsFederateInfoSetIntegerProperty(fedInfo,helics_property_int_log_level,1);
catch ec
    success=false;
    helicsBrokerDestroy(fedStruct.broker);
    helicsFederateInfoFree(fedInfo);
    return
end
try
fedStruct.mFed=helicsCreateMessageFederate('fed1',fedInfo);
if (~helicsFederateIsValid(fedStruct.mFed))
    success=false;
end
catch ec
    success=false;
end
helicsFederateInfoFree(fedInfo);
end

function success=closeStruct(fedStruct)
import helics.*
success=true;
helicsFederateFinalize(fedStruct.mFed);
helicsBrokerWaitForDisconnect(fedStruct.broker,2000);

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
function testEndpointInitialize(testCase)
import matlab.unittest.constraints.IsTrue;
import helics.*
[feds,success]=generateFed();
testCase.verifyThat(success,IsTrue);
try
epid1 = helicsFederateRegisterEndpoint(feds.mFed, 'ep1', '');
testCase.verifyNotEqual(epid1,0);
helicsFederateEnterExecutingMode(feds.mFed);
state=helicsFederateGetState(feds.mFed);
testCase.verifyEqual(state,helics.helics_state_execution);
success=closeStruct(feds);
testCase.verifyThat(success,IsTrue);
catch e
    testCase.verifyThat(false,IsTrue);
   disp(e.message)
    disp(e.stack(1))
    forceCloseStruct(feds);
end
end

function testEndpointRegistration(testCase)
import matlab.unittest.constraints.IsTrue;
import helics.*
[feds,success]=generateFed();
testCase.verifyThat(success,IsTrue);
try
epid1 = helicsFederateRegisterEndpoint(feds.mFed, 'ep1', '');

epid2 = helicsFederateRegisterGlobalEndpoint(feds.mFed, 'ep2', 'random');


helicsFederateEnterExecutingMode(feds.mFed);

ept_key = helicsEndpointGetName(epid1);
testCase.verifyEqual(ept_key,'fed1/ep1');

ept_key = helicsEndpointGetName(epid2);
testCase.verifyEqual(ept_key,'ep2');

ept_type = helicsEndpointGetType(epid1);
testCase.verifyThat(isempty(ept_type),IsTrue);

ept_type = helicsEndpointGetType(epid2);
testCase.verifyEqual(ept_type,'random');
success=closeStruct(feds);
testCase.verifyThat(success,IsTrue);
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

helicsFederateSetTimeProperty(feds.mFed,int32(137),1.0);
helicsFederateEnterExecutingMode(feds.mFed);
data = 'this is a random string message';

helicsEndpointSendEventRaw(epid1,'ep2',data,1.0);

granted_time=helicsFederateRequestTime(feds.mFed,2.0);
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
