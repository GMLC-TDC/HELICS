function tests=valueFedTests

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
fedStruct.vFed=helicsCreateValueFederate(fedInfo);
if (fedStruct.vFed==0)
    success=false;
end
helicsFederateInfoFree(fedInfo);
end

function success=closeStruct(fedStruct)
import helics.*
success=true;
status=helicsFederateFinalize(fedStruct.vFed);
if (status~=0)
    success=false;
end
while (helicsBrokerIsConnected(fedStruct.broker))
    pause(1);
end
helicsFederateFree(fedStruct.vFed);
helicsBrokerFree(fedStruct.broker);
helicsCloseLibrary();

end

function forceCloseStruct(fedStruct)
import helics.*
helicsFederateFinalize(fedStruct.vFed);

cnt=0;
while (helicsBrokerIsConnected(fedStruct.broker))
    pause(1);
    cnt=cnt+1;
    if (cnt>5)
        helicsBrokerDisconnect(fedStruct.broker);
        break;
    end
end

helicsFederateFree(fedStruct.vFed);
helicsBrokerFree(fedStruct.broker);
helicsCloseLibrary();

end

function testBasic(testCase)
import matlab.unittest.constraints.IsGreaterThan
ver=helics.helicsGetVersion();
verifyThat(testCase,length(ver),IsGreaterThan(10),'version length is not valid');
end

function testEnterExecution(testCase)
import matlab.unittest.constraints.IsTrue;
[feds,success]=generateFed();
try
testCase.verifyThat(success,IsTrue);
helics.helicsFederateEnterExecutionMode(feds.vFed);
[status,state]=helics.helicsFederateGetState(feds.vFed);
testCase.verifyEqual(status,helics.helics_ok);
testCase.verifyEqual(state,helics.helics_execution_state);
success=closeStruct(feds);
testCase.verifyThat(success,IsTrue);
catch e
    testCase.verifyThat(false,IsTrue);
    disp(e.message)
    disp(e.stack(1))
    forceCloseStruct(feds);
end
end

function testPublicationRegistration(testCase)
import matlab.unittest.constraints.IsTrue;
import helics.*
[feds,success]=generateFed();
testCase.verifyThat(success,IsTrue);
try
pubid1 = helicsFederateRegisterPublication(feds.vFed, 'pub1', 'string', '');
pubid2 = helicsFederateRegisterGlobalPublication(feds.vFed, 'pub2', 'int', '');
pubid3 = helicsFederateRegisterPublication(feds.vFed, 'pub3', 'double', 'V');
helicsFederateEnterExecutionMode(feds.vFed);

[status, publication_key] = helicsPublicationGetKey(pubid1);
testCase.verifyEqual(status,helics.helics_ok);
testCase.verifyEqual(publication_key,'fed1/pub1');
[status, publication_key] = helicsPublicationGetKey(pubid2);
testCase.verifyEqual(status,helics.helics_ok);
testCase.verifyEqual(publication_key,'pub2');
[status, publication_key] = helicsPublicationGetKey(pubid3);
testCase.verifyEqual(status,helics.helics_ok);
testCase.verifyEqual(publication_key,'fed1/pub3');
[status, publication_type] = helicsPublicationGetType(pubid3);
testCase.verifyEqual(status,helics.helics_ok);
testCase.verifyEqual(publication_type,'double');
[status, publication_units] = helicsPublicationGetUnits(pubid3);
testCase.verifyEqual(status,helics.helics_ok);
testCase.verifyEqual(publication_units,'V');
success=closeStruct(feds);
testCase.verifyThat(success,IsTrue);
catch e
    testCase.verifyThat(false,IsTrue);
   disp(e.message)
    disp(e.stack(1))
    forceCloseStruct(feds);
end
end

function testNamedPoint(testCase)
import matlab.unittest.constraints.IsTrue;
import helics.*
[feds,success]=generateFed();
testCase.verifyThat(success,IsTrue);

defaultValue = 'start';
    defVal = 5.3;
    testValue1 = 'inside of the functional relationship of helics';
    testVal1 = 45.7823;
    testValue2 = 'I am a string';
    testVal2 = 0.0;
try
    pubid = helicsFederateRegisterGlobalTypePublication(feds.vFed, 'pub1', HELICS_DATA_TYPE_NAMEDPOINT, '');
    subid = helicsFederateRegisterSubscription(feds.vFed, 'pub1', 'named_point', '');

    status = helicsSubscriptionSetDefaultNamedPoint(subid, defaultValue, defVal);
    testCase.verifyEqual(status,helics.helics_ok);

    status = helicsFederateEnterExecutionMode(feds.vFed);
    testCase.verifyEqual(status,helics.helics_ok);

    % publish string1 at time=0.0;
    status = helicsPublicationPublishNamedPoint(pubid, testValue1, testVal1);
    testCase.verifyEqual(status,helics.helics_ok);

    % double val;
    [status, value, val] = helicsSubscriptionGetNamedPoint(subid);
    testCase.verifyEqual(status,helics.helics_ok);
    testCase.verifyEqual(value,defaultValue);
    testCase.verifyEqual(val,defVal);

    [status,grantedtime] = helicsFederateRequestTime(feds.vFed, 1.0);
    testCase.verifyEqual(status,helics.helics_ok);
    testCase.verifyEqual(grantedtime,0.01);

    % get the value
    [status, value, val] = helicsSubscriptionGetNamedPoint(subid);
    testCase.verifyEqual(status,helics.helics_ok);
    % make sure the string is what we expect
    testCase.verifyEqual(value,testValue1);
    testCase.verifyEqual(val,testVal1);

    % publish a second string
    status = helicsPublicationPublishNamedPoint(pubid, testValue2, testVal2);
    testCase.verifyEqual(status,helics.helics_ok);

    % make sure the value is still what we expect
    [status, value, val] = helicsSubscriptionGetNamedPoint(subid);
    testCase.verifyEqual(status,helics.helics_ok);
    % make sure the string is what we expect
    testCase.verifyEqual(value,testValue1);
    testCase.verifyEqual(val,testVal1);

    % advance time
    [status, grantedtime] = helicsFederateRequestTime(feds.vFed, 2.0);
    testCase.verifyEqual(status,helics.helics_ok);
    testCase.verifyEqual(grantedtime,0.02);

    % make sure the value was updated
    [status, value, val] = helicsSubscriptionGetNamedPoint(subid);
    testCase.verifyEqual(status,helics.helics_ok);
    % make sure the string is what we expect
    testCase.verifyEqual(value,testValue2);
    testCase.verifyEqual(val,testVal2);
    success=closeStruct(feds);
    testCase.verifyThat(success,IsTrue);
catch e
    testCase.verifyThat(false,IsTrue);
    disp(e.message)
    disp(e.stack(1))
    forceCloseStruct(feds);
end
end

function testBool(testCase)
import matlab.unittest.constraints.IsTrue;
import helics.*
[feds,success]=generateFed();
testCase.verifyThat(success,IsTrue);

try
defaultValue = helics_true;
    testValue1 = helics_true;
    testValue2 = helics_false;

    pubid = helicsFederateRegisterGlobalTypePublication(feds.vFed, 'pub1', HELICS_DATA_TYPE_BOOLEAN, '');
    subid = helicsFederateRegisterSubscription(feds.vFed, 'pub1', 'bool', '');

    status = helicsSubscriptionSetDefaultBoolean(subid, defaultValue);
    testCase.verifyEqual(status,helics.helics_ok);

    status = helicsFederateEnterExecutionMode(feds.vFed);
    testCase.verifyEqual(status,helics.helics_ok);

    % publish string1 at time=0.0;
    status = helicsPublicationPublishBoolean(pubid, testValue1);
    testCase.verifyEqual(status,helics.helics_ok);

    % double val;
    [status, value] = helicsSubscriptionGetBoolean(subid);
    testCase.verifyEqual(status,helics.helics_ok);
    testCase.verifyEqual(value,defaultValue);

    [status,grantedtime] = helicsFederateRequestTime(feds.vFed, 1.0);
    testCase.verifyEqual(status,helics.helics_ok);
    testCase.verifyEqual(grantedtime,0.01);

    % get the value
    [status, value] = helicsSubscriptionGetBoolean(subid);
    testCase.verifyEqual(status,helics.helics_ok);
    % make sure the string is what we expect
    testCase.verifyEqual(value,testValue1);

    % publish a second string
    status = helicsPublicationPublishBoolean(pubid, testValue2);
    testCase.verifyEqual(status,helics.helics_ok);

    % make sure the value is still what we expect
    [status, value] = helicsSubscriptionGetBoolean(subid);
    testCase.verifyEqual(status,helics.helics_ok);
    % make sure the string is what we expect
    testCase.verifyEqual(value,testValue1);

    % advance time
    [status, grantedtime] = helicsFederateRequestTime(feds.vFed, 2.0);
    testCase.verifyEqual(status,helics.helics_ok);
    testCase.verifyEqual(grantedtime,0.02);

    % make sure the value was updated
    [status, value] = helicsSubscriptionGetBoolean(subid);
    testCase.verifyEqual(status,helics.helics_ok);
    % make sure the string is what we expect
    testCase.verifyEqual(value,testValue2);
    success=closeStruct(feds);
    testCase.verifyThat(success,IsTrue);
catch e
    testCase.verifyThat(false,IsTrue);
    disp(e.message)
    disp(e.stack(1))
    forceCloseStruct(feds);
end
end

function testPublisherRegistration(testCase)
import matlab.unittest.constraints.IsTrue;
import helics.*
[feds,success]=generateFed();
testCase.verifyThat(success,IsTrue);

try
    pubid1 = helicsFederateRegisterTypePublication(feds.vFed, 'pub1', HELICS_DATA_TYPE_STRING, '');
    pubid2 = helicsFederateRegisterGlobalTypePublication(feds.vFed, 'pub2', HELICS_DATA_TYPE_INT, '');
    pubid3 = helicsFederateRegisterTypePublication(feds.vFed, 'pub3', HELICS_DATA_TYPE_DOUBLE, 'V');

    [status, publication_key] = helicsPublicationGetKey(pubid1);
testCase.verifyEqual(status,helics.helics_ok);
testCase.verifyEqual(publication_key,'fed1/pub1');
[status, publication_type] = helicsPublicationGetType(pubid1);
testCase.verifyEqual(status,helics.helics_ok);
testCase.verifyEqual(publication_type,'string');
[status, publication_key] = helicsPublicationGetKey(pubid2);
testCase.verifyEqual(status,helics.helics_ok);
testCase.verifyEqual(publication_key,'pub2');
[status, publication_type] = helicsPublicationGetType(pubid2);
testCase.verifyEqual(status,helics.helics_ok);
testCase.verifyEqual(publication_type,'int64');
[status, publication_key] = helicsPublicationGetKey(pubid3);
testCase.verifyEqual(status,helics.helics_ok);
testCase.verifyEqual(publication_key,'fed1/pub3');
[status, publication_type] = helicsPublicationGetType(pubid3);
testCase.verifyEqual(status,helics.helics_ok);
testCase.verifyEqual(publication_type,'double');
[status, publication_units] = helicsPublicationGetUnits(pubid3);
testCase.verifyEqual(status,helics.helics_ok);
testCase.verifyEqual(publication_units,'V');

    status = helicsFederateEnterExecutionMode(feds.vFed);
    testCase.verifyEqual(status,helics.helics_ok);

   %% add state and some type checks
    success=closeStruct(feds);
    testCase.verifyThat(success,IsTrue);
catch e
    testCase.verifyThat(false,IsTrue);
    disp(e.message)
    disp(e.stack(1))
    forceCloseStruct(feds);
end
end

function testDouble(testCase)
import matlab.unittest.constraints.IsTrue;
import helics.*
[feds,success]=generateFed();
testCase.verifyThat(success,IsTrue);

try
defaultValue = 1.0;
    testValue1 = 2.7586;
    testValue2 = 1e27;

    pubid = helicsFederateRegisterGlobalTypePublication(feds.vFed, 'pub1', HELICS_DATA_TYPE_DOUBLE, '');
    subid = helicsFederateRegisterSubscription(feds.vFed, 'pub1', 'double', '');

    status = helicsSubscriptionSetDefaultDouble(subid, defaultValue);
    testCase.verifyEqual(status,helics.helics_ok);

    status = helicsFederateEnterExecutionMode(feds.vFed);
    testCase.verifyEqual(status,helics.helics_ok);

    % publish string1 at time=0.0;
    status = helicsPublicationPublishDouble(pubid, testValue1);
    testCase.verifyEqual(status,helics.helics_ok);

    % double val;
    [status, value] = helicsSubscriptionGetDouble(subid);
    testCase.verifyEqual(status,helics.helics_ok);
    testCase.verifyEqual(value,defaultValue);

    [status,grantedtime] = helicsFederateRequestTime(feds.vFed, 1.0);
    testCase.verifyEqual(status,helics.helics_ok);
    testCase.verifyEqual(grantedtime,0.01);

    % get the value
    [status, value] = helicsSubscriptionGetDouble(subid);
    testCase.verifyEqual(status,helics.helics_ok);
    % make sure the string is what we expect
    testCase.verifyEqual(value,testValue1);

    % publish a second string
    status = helicsPublicationPublishDouble(pubid, testValue2);
    testCase.verifyEqual(status,helics.helics_ok);

    % make sure the value is still what we expect
    [status, value] = helicsSubscriptionGetDouble(subid);
    testCase.verifyEqual(status,helics.helics_ok);
    % make sure the string is what we expect
    testCase.verifyEqual(value,testValue1);

    % advance time
    [status, grantedtime] = helicsFederateRequestTime(feds.vFed, 2.0);
    testCase.verifyEqual(status,helics.helics_ok);
    testCase.verifyEqual(grantedtime,0.02);

    % make sure the value was updated
    [status, value] = helicsSubscriptionGetDouble(subid);
    testCase.verifyEqual(status,helics.helics_ok);
    % make sure the string is what we expect
    testCase.verifyEqual(value,testValue2);
    success=closeStruct(feds);
    testCase.verifyThat(success,IsTrue);
catch e
    testCase.verifyThat(false,IsTrue);
    disp(e.message)
    disp(e.stack(1))
    forceCloseStruct(feds);
end
end


function testComplex(testCase)
import matlab.unittest.constraints.IsTrue;
import helics.*
[feds,success]=generateFed();
testCase.verifyThat(success,IsTrue);

try
defaultValue_r = 1.0;
defaultValue_j = -1.0;
    testValue1_r = 2.7586;
    testValue1_j = 342.25626;
    testValue2_r = 1e27;
    testValue2_j = -0.3e-2;

    pubid = helicsFederateRegisterGlobalTypePublication(feds.vFed, 'pub1', HELICS_DATA_TYPE_COMPLEX, '');
    subid = helicsFederateRegisterSubscription(feds.vFed, 'pub1', 'double', '');

    status = helicsSubscriptionSetDefaultComplex(subid, defaultValue_r, defaultValue_j);
    testCase.verifyEqual(status,helics.helics_ok);

    status = helicsFederateEnterExecutionMode(feds.vFed);
    testCase.verifyEqual(status,helics.helics_ok);

    % publish string1 at time=0.0;
    status = helicsPublicationPublishComplex(pubid, testValue1_r,testValue1_j);
    testCase.verifyEqual(status,helics.helics_ok);

    % double val;
    [status, value_r,value_j] = helicsSubscriptionGetComplex(subid);
    testCase.verifyEqual(status,helics.helics_ok);
    testCase.verifyEqual(value_r,defaultValue_r);
    testCase.verifyEqual(value_j,defaultValue_j);

    [status,grantedtime] = helicsFederateRequestTime(feds.vFed, 1.0);
    testCase.verifyEqual(status,helics.helics_ok);
    testCase.verifyEqual(grantedtime,0.01);

    % get the value
    [status, value_r, value_j] = helicsSubscriptionGetComplex(subid);
    testCase.verifyEqual(status,helics.helics_ok);
    % make sure the string is what we expect
    testCase.verifyEqual(value_r,testValue1_r);
    testCase.verifyEqual(value_j,testValue1_j);
    % publish a second string
    status = helicsPublicationPublishComplex(pubid, testValue2_r,testValue2_j);
    testCase.verifyEqual(status,helics.helics_ok);

    % make sure the value is still what we expect
    [status, value_r,value_j] = helicsSubscriptionGetComplex(subid);
    testCase.verifyEqual(status,helics.helics_ok);
    % make sure the string is what we expect
    testCase.verifyEqual(value_r,testValue1_r);
    testCase.verifyEqual(value_j,testValue1_j);
    % advance time
    [status, grantedtime] = helicsFederateRequestTime(feds.vFed, 2.0);
    testCase.verifyEqual(status,helics.helics_ok);
    testCase.verifyEqual(grantedtime,0.02);

    % make sure the value was updated
    [status, value_r,value_j] = helicsSubscriptionGetComplex(subid);
    testCase.verifyEqual(status,helics.helics_ok);
    % make sure the string is what we expect
    testCase.verifyEqual(value_r,testValue2_r);
    testCase.verifyEqual(value_j,testValue2_j);
    success=closeStruct(feds);
    testCase.verifyThat(success,IsTrue);
catch e
    testCase.verifyThat(false,IsTrue);
    disp(e.message)
    disp(e.stack(1))
    forceCloseStruct(feds);
end
end


function testInteger(testCase)
import matlab.unittest.constraints.IsTrue;
import helics.*
[feds,success]=generateFed();
testCase.verifyThat(success,IsTrue);

try
defaultValue = int64(45626678);
    testValue1 = int64(-27);
    testValue2 = int64(0);

    pubid = helicsFederateRegisterGlobalTypePublication(feds.vFed, 'pub1', HELICS_DATA_TYPE_INT, '');
    subid = helicsFederateRegisterSubscription(feds.vFed, 'pub1', 'int', '');

    status = helicsSubscriptionSetDefaultInteger(subid, defaultValue);
    testCase.verifyEqual(status,helics.helics_ok);

    status = helicsFederateEnterExecutionMode(feds.vFed);
    testCase.verifyEqual(status,helics.helics_ok);

    % publish string1 at time=0.0;
    status = helicsPublicationPublishInteger(pubid, testValue1);
    testCase.verifyEqual(status,helics.helics_ok);

    % double val;
    [status, value] = helicsSubscriptionGetInteger(subid);
    testCase.verifyEqual(status,helics.helics_ok);
    testCase.verifyEqual(value,defaultValue);

    [status,grantedtime] = helicsFederateRequestTime(feds.vFed, 1.0);
    testCase.verifyEqual(status,helics.helics_ok);
    testCase.verifyEqual(grantedtime,0.01);

    % get the value
    [status, value] = helicsSubscriptionGetInteger(subid);
    testCase.verifyEqual(status,helics.helics_ok);
    % make sure the string is what we expect
    testCase.verifyEqual(value,testValue1);

    % publish a second string
    status = helicsPublicationPublishInteger(pubid, testValue2);
    testCase.verifyEqual(status,helics.helics_ok);

    % make sure the value is still what we expect
    [status, value] = helicsSubscriptionGetInteger(subid);
    testCase.verifyEqual(status,helics.helics_ok);
    % make sure the string is what we expect
    testCase.verifyEqual(value,testValue1);

    % advance time
    [status, grantedtime] = helicsFederateRequestTime(feds.vFed, 2.0);
    testCase.verifyEqual(status,helics.helics_ok);
    testCase.verifyEqual(grantedtime,0.02);

    % make sure the value was updated
    [status, value] = helicsSubscriptionGetInteger(subid);
    testCase.verifyEqual(status,helics.helics_ok);
    % make sure the value is what we expect
    testCase.verifyEqual(value,testValue2);
    success=closeStruct(feds);
    testCase.verifyThat(success,IsTrue);
catch e
    testCase.verifyThat(false,IsTrue);
    disp(e.message)
    disp(e.stack(1))
    forceCloseStruct(feds);
end
end


function testString(testCase)
import matlab.unittest.constraints.IsTrue;
import helics.*
[feds,success]=generateFed();
testCase.verifyThat(success,IsTrue);

try
defaultValue = 'string1';
    testValue1 = 'this is a longer test string to bypass sso';
    testValue2 = '';

    pubid = helicsFederateRegisterGlobalTypePublication(feds.vFed, 'pub1', HELICS_DATA_TYPE_STRING, '');
    subid = helicsFederateRegisterSubscription(feds.vFed, 'pub1', 'string', '');

    status = helicsSubscriptionSetDefaultString(subid, defaultValue);
    testCase.verifyEqual(status,helics.helics_ok);

    status = helicsFederateEnterExecutionMode(feds.vFed);
    testCase.verifyEqual(status,helics.helics_ok);

    % publish string1 at time=0.0;
    status = helicsPublicationPublishString(pubid, testValue1);
    testCase.verifyEqual(status,helics.helics_ok);

    % double val;
    [status, value] = helicsSubscriptionGetString(subid);
    testCase.verifyEqual(status,helics.helics_ok);
    testCase.verifyEqual(value,defaultValue);

    [status,grantedtime] = helicsFederateRequestTime(feds.vFed, 1.0);
    testCase.verifyEqual(status,helics.helics_ok);
    testCase.verifyEqual(grantedtime,0.01);

    % get the value
    [status, value] = helicsSubscriptionGetString(subid);
    testCase.verifyEqual(status,helics.helics_ok);
    % make sure the string is what we expect
    testCase.verifyEqual(value,testValue1);

    % publish a second string
    status = helicsPublicationPublishString(pubid, testValue2);
    testCase.verifyEqual(status,helics.helics_ok);

    % make sure the value is still what we expect
    [status, value] = helicsSubscriptionGetString(subid);
    testCase.verifyEqual(status,helics.helics_ok);
    % make sure the string is what we expect
    testCase.verifyEqual(value,testValue1);

    % advance time
    [status, grantedtime] = helicsFederateRequestTime(feds.vFed, 2.0);
    testCase.verifyEqual(status,helics.helics_ok);
    testCase.verifyEqual(grantedtime,0.02);

    % make sure the value was updated
    [status, value] = helicsSubscriptionGetString(subid);
    testCase.verifyEqual(status,helics.helics_ok);
    % make sure the value is what we expect
    testCase.verifyEqual(isempty(value),isempty(testValue2));
    success=closeStruct(feds);
    testCase.verifyThat(success,IsTrue);
catch e
    testCase.verifyThat(false,IsTrue);
    disp(e.message)
    disp(e.stack(1))
    forceCloseStruct(feds);
end
end


function testVector(testCase)
import matlab.unittest.constraints.IsTrue;
import helics.*
[feds,success]=generateFed();
testCase.verifyThat(success,IsTrue);

try
defaultValue = [34.5,22.1,-10.4];
    testValue1 = ones(22,1);
    testValue2 = [99.1,-99,2];

    pubid = helicsFederateRegisterGlobalTypePublication(feds.vFed, 'pub1', HELICS_DATA_TYPE_VECTOR, '');
    subid = helicsFederateRegisterSubscription(feds.vFed, 'pub1', 'vector', '');

    status = helicsSubscriptionSetDefaultVector(subid, defaultValue);
    testCase.verifyEqual(status,helics.helics_ok);

    status = helicsFederateEnterExecutionMode(feds.vFed);
    testCase.verifyEqual(status,helics.helics_ok);

    % publish string1 at time=0.0;
    status = helicsPublicationPublishVector(pubid, testValue1);
    testCase.verifyEqual(status,helics.helics_ok);

    % double val;
    [status, value] = helicsSubscriptionGetVector(subid);
    testCase.verifyEqual(status,helics.helics_ok);
    testCase.verifyEqual(value,defaultValue);

    [status,grantedtime] = helicsFederateRequestTime(feds.vFed, 1.0);
    testCase.verifyEqual(status,helics.helics_ok);
    testCase.verifyEqual(grantedtime,0.01);

    % get the value
    [status, value] = helicsSubscriptionGetString(subid);
    testCase.verifyEqual(status,helics.helics_ok);
    % make sure the string is what we expect
    testCase.verifyEqual(value,testValue1);

    % publish a second string
    status = helicsPublicationPublishVector(pubid, testValue2);
    testCase.verifyEqual(status,helics.helics_ok);

    % make sure the value is still what we expect
    [status, value] = helicsSubscriptionGetVector(subid);
    testCase.verifyEqual(status,helics.helics_ok);
    % make sure the string is what we expect
    testCase.verifyEqual(value,testValue1);

    % advance time
    [status, grantedtime] = helicsFederateRequestTime(feds.vFed, 2.0);
    testCase.verifyEqual(status,helics.helics_ok);
    testCase.verifyEqual(grantedtime,0.02);

    % make sure the value was updated
    [status, value] = helicsSubscriptionGetVector(subid);
    testCase.verifyEqual(status,helics.helics_ok);
    % make sure the value is what we expect
    testCase.verifyEqual(isempty(value),isempty(testValue2));
    success=closeStruct(feds);
    testCase.verifyThat(success,IsTrue);
catch e
    testCase.verifyThat(false,IsTrue);
    disp(e.message)
    disp(e.stack(1))
    forceCloseStruct(feds);
end
end
