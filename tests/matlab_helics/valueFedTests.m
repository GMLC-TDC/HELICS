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
initstring = '1';
fedinitstring = '--broker=mainbroker --federates=1';
fedStruct.broker=helicsCreateBroker('zmq','mainbroker',initstring);
if (fedStruct.broker==0)
    success=false;
    return;
end
fedInfo=helicsCreateFederateInfo();
if (fedInfo==0)
    helicsBrokerDestroy(fedStruct.broker);
    success=false;
    return;
end
try
helicsFederateInfoSetCoreTypeFromString(fedInfo,'zmq');
helicsFederateInfoSetCoreInitString(fedInfo,fedinitstring);
helicsFederateInfoSetTimeProperty(fedInfo,TIME 0.01);
helicsFederateInfoSetLoggingLevel(fedInfo,1);
catch ec
    success=false;
    helicsBrokerDestroy(fedStruct.broker);
    helicsFederateInfoFree(fedInfo);
    return
end
try
fedStruct.vFed=helicsCreateValueFederate('fed1',fedInfo);
if (fedStruct.vFed==0)
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
helicsFederateFinalize(fedStruct.vFed);
helicsBrokerWaitForDisconnect(fedStruct.broker);

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
helics.helicsFederateEnterExecutingMode(feds.vFed);
state=helics.helicsFederateGetState(feds.vFed);
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
helicsFederateEnterExecutingMode(feds.vFed);

publication_key = helicsPublicationGetKey(pubid1);
testCase.verifyEqual(publication_key,'fed1/pub1');
publication_key = helicsPublicationGetKey(pubid2);
testCase.verifyEqual(publication_key,'pub2');
publication_key = helicsPublicationGetKey(pubid3);
testCase.verifyEqual(publication_key,'fed1/pub3');
publication_type = helicsPublicationGetType(pubid3);
testCase.verifyEqual(publication_type,'double');
publication_units = helicsPublicationGetUnits(pubid3);
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

    helicsSubscriptionSetDefaultNamedPoint(subid, defaultValue, defVal);

    helicsFederateEnterExecutingMode(feds.vFed);

    % publish string1 at time=0.0;
    helicsPublicationPublishNamedPoint(pubid, testValue1, testVal1);

    % double val;
    [value, val] = helicsSubscriptionGetNamedPoint(subid);
    testCase.verifyEqual(value,defaultValue);
    testCase.verifyEqual(val,defVal);

    grantedtime = helicsFederateRequestTime(feds.vFed, 1.0);
    testCase.verifyEqual(grantedtime,0.01);

    % get the value
    [value, val] = helicsSubscriptionGetNamedPoint(subid);
    % make sure the string is what we expect
    testCase.verifyEqual(value,testValue1);
    testCase.verifyEqual(val,testVal1);

    % publish a second string
    helicsPublicationPublishNamedPoint(pubid, testValue2, testVal2);

    % make sure the value is still what we expect
    [value, val] = helicsSubscriptionGetNamedPoint(subid);
    % make sure the string is what we expect
    testCase.verifyEqual(value,testValue1);
    testCase.verifyEqual(val,testVal1);

    % advance time
    grantedtime = helicsFederateRequestTime(feds.vFed, 2.0);
    testCase.verifyEqual(grantedtime,0.02);

    % make sure the value was updated
    [value, val] = helicsSubscriptionGetNamedPoint(subid);
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

    helicsSubscriptionSetDefaultBoolean(subid, defaultValue);
    

    helicsFederateEnterExecutionMode(feds.vFed);

    % publish string1 at time=0.0;
     helicsPublicationPublishBoolean(pubid, testValue1);
    

    % double val;
    value = helicsSubscriptionGetBoolean(subid);
    
    testCase.verifyEqual(value,defaultValue);

    grantedtime = helicsFederateRequestTime(feds.vFed, 1.0);
    
    testCase.verifyEqual(grantedtime,0.01);

    % get the value
    value = helicsSubscriptionGetBoolean(subid);
    
    % make sure the string is what we expect
    testCase.verifyEqual(value,testValue1);

    % publish a second string
    helicsPublicationPublishBoolean(pubid, testValue2);
    

    % make sure the value is still what we expect
    value = helicsSubscriptionGetBoolean(subid);
    
    % make sure the string is what we expect
    testCase.verifyEqual(value,testValue1);

    % advance time
    grantedtime = helicsFederateRequestTime(feds.vFed, 2.0);
    
    testCase.verifyEqual(grantedtime,0.02);

    % make sure the value was updated
    value = helicsSubscriptionGetBoolean(subid);
    
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

    publication_key = helicsPublicationGetKey(pubid1);

testCase.verifyEqual(publication_key,'fed1/pub1');
publication_type = helicsPublicationGetType(pubid1);

testCase.verifyEqual(publication_type,'string');
publication_key = helicsPublicationGetKey(pubid2);

testCase.verifyEqual(publication_key,'pub2');
publication_type = helicsPublicationGetType(pubid2);

testCase.verifyEqual(publication_type,'int64');
publication_key = helicsPublicationGetKey(pubid3);

testCase.verifyEqual(publication_key,'fed1/pub3');
publication_type = helicsPublicationGetType(pubid3);

testCase.verifyEqual(publication_type,'double');
publication_units = helicsPublicationGetUnits(pubid3);

testCase.verifyEqual(publication_units,'V');

    helicsFederateEnterExecutionMode(feds.vFed);
    

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

    helicsSubscriptionSetDefaultDouble(subid, defaultValue);
    

    helicsFederateEnterExecutionMode(feds.vFed);
    

    % publish string1 at time=0.0;
    helicsPublicationPublishDouble(pubid, testValue1);
    

    % double val;
    value = helicsSubscriptionGetDouble(subid);
    
    testCase.verifyEqual(value,defaultValue);

    grantedtime = helicsFederateRequestTime(feds.vFed, 1.0);
    
    testCase.verifyEqual(grantedtime,0.01);

    % get the value
    value = helicsSubscriptionGetDouble(subid);
    
    % make sure the string is what we expect
    testCase.verifyEqual(value,testValue1);

    % publish a second string
    helicsPublicationPublishDouble(pubid, testValue2);
    

    % make sure the value is still what we expect
    value = helicsSubscriptionGetDouble(subid);
    
    % make sure the string is what we expect
    testCase.verifyEqual(value,testValue1);

    % advance time
    grantedtime = helicsFederateRequestTime(feds.vFed, 2.0);
    
    testCase.verifyEqual(grantedtime,0.02);

    % make sure the value was updated
    value = helicsSubscriptionGetDouble(subid);
    
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

    helicsSubscriptionSetDefaultComplex(subid, defaultValue_r, defaultValue_j);
    

    helicsFederateEnterExecutionMode(feds.vFed);
    

    % publish string1 at time=0.0;
    helicsPublicationPublishComplex(pubid, testValue1_r,testValue1_j);
    

    % double val;
    value = helicsSubscriptionGetComplex(subid);
    
    testCase.verifyEqual(value.real,defaultValue_r);
    testCase.verifyEqual(value.imag,defaultValue_j);

    grantedtime = helicsFederateRequestTime(feds.vFed, 1.0);
    
    testCase.verifyEqual(grantedtime,0.01);

    % get the value
    [value_r, value_j] = helicsSubscriptionGetComplex(subid);
    
    % make sure the string is what we expect
    testCase.verifyEqual(value_r,testValue1_r);
    testCase.verifyEqual(value_j,testValue1_j);
    % publish a second string
    helicsPublicationPublishComplex(pubid, testValue2_r,testValue2_j);
    

    % make sure the value is still what we expect
    [value_r,value_j] = helicsSubscriptionGetComplex(subid);
    
    % make sure the string is what we expect
    testCase.verifyEqual(value_r,testValue1_r);
    testCase.verifyEqual(value_j,testValue1_j);
    % advance time
    grantedtime = helicsFederateRequestTime(feds.vFed, 2.0);
    
    testCase.verifyEqual(grantedtime,0.02);

    % make sure the value was updated
    [value_r,value_j] = helicsSubscriptionGetComplex(subid);
    
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

    helicsSubscriptionSetDefaultInteger(subid, defaultValue);
    

    helicsFederateEnterExecutionMode(feds.vFed);
    

    % publish string1 at time=0.0;
    helicsPublicationPublishInteger(pubid, testValue1);
    

    % double val;
    value = helicsSubscriptionGetInteger(subid);
    
    testCase.verifyEqual(value,defaultValue);

    grantedtime = helicsFederateRequestTime(feds.vFed, 1.0);
    
    testCase.verifyEqual(grantedtime,0.01);

    % get the value
    value = helicsSubscriptionGetInteger(subid);
    
    % make sure the string is what we expect
    testCase.verifyEqual(value,testValue1);

    % publish a second string
    helicsPublicationPublishInteger(pubid, testValue2);
    
    % make sure the value is still what we expect
    value = helicsSubscriptionGetInteger(subid);
    
    % make sure the string is what we expect
    testCase.verifyEqual(value,testValue1);

    % advance time
    grantedtime = helicsFederateRequestTime(feds.vFed, 2.0);
    
    testCase.verifyEqual(grantedtime,0.02);

    % make sure the value was updated
    value = helicsSubscriptionGetInteger(subid);
    
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

    helicsSubscriptionSetDefaultString(subid, defaultValue);
    
    helicsFederateEnterExecutionMode(feds.vFed);
    
    % publish string1 at time=0.0;
    helicsPublicationPublishString(pubid, testValue1);
    

    % double val;
    value = helicsSubscriptionGetString(subid);
    
    testCase.verifyEqual(value,defaultValue);

    grantedtime = helicsFederateRequestTime(feds.vFed, 1.0);
    
    testCase.verifyEqual(grantedtime,0.01);

    % get the value
    value = helicsSubscriptionGetString(subid);
    
    % make sure the string is what we expect
    testCase.verifyEqual(value,testValue1);

    % publish a second string
    helicsPublicationPublishString(pubid, testValue2);
    

    % make sure the value is still what we expect
    value = helicsSubscriptionGetString(subid);
    
    % make sure the string is what we expect
    testCase.verifyEqual(value,testValue1);

    % advance time
    grantedtime = helicsFederateRequestTime(feds.vFed, 2.0);
    
    testCase.verifyEqual(grantedtime,0.02);

    % make sure the value was updated
    value = helicsSubscriptionGetString(subid);
    
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
defaultValue = [34.5;22.1;-10.4];
    testValue1 = ones(22,1);
    testValue2 = [99.1;-99;2;0.0;-1e35;4.56e-7];

    pubid = helicsFederateRegisterGlobalTypePublication(feds.vFed, 'pub1', HELICS_DATA_TYPE_VECTOR, '');
    subid = helicsFederateRegisterSubscription(feds.vFed, 'pub1', 'vector', '');

    helicsSubscriptionSetDefaultVector(subid, defaultValue);
    

    helicsFederateEnterExecutionMode(feds.vFed);
    

    % publish string1 at time=0.0;
    helicsPublicationPublishVector(pubid, testValue1);
    

    % double val;
    value = helicsSubscriptionGetVector(subid);
    
    testCase.verifyEqual(value,defaultValue);

    grantedtime = helicsFederateRequestTime(feds.vFed, 1.0);
    
    testCase.verifyEqual(grantedtime,0.01);

    % get the value
    value = helicsSubscriptionGetVector(subid);
    
    % make sure the string is what we expect
    testCase.verifyEqual(value,testValue1);

    % publish a second string
    helicsPublicationPublishVector(pubid, testValue2);
    

    % make sure the value is still what we expect
    value = helicsSubscriptionGetVector(subid);
    
    % make sure the string is what we expect
    testCase.verifyEqual(value,testValue1);

    % advance time
    grantedtime = helicsFederateRequestTime(feds.vFed, 2.0);
    
    testCase.verifyEqual(grantedtime,0.02);

    % make sure the value was updated
    value = helicsSubscriptionGetVector(subid);
    
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
