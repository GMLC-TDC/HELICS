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
try
fedStruct.vFed=helicsCreateValueFederate('fed1',fedInfo);
if (~helicsFederateIsValid(fedStruct.vFed))
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
helicsBrokerWaitForDisconnect(fedStruct.broker,2000);

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
testCase.verifyEqual(state,helics.HELICS_STATE_EXECUTION);
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
pubid1 = helicsFederateRegisterTypePublication(feds.vFed, 'pub1', 'string', '');
pubid2 = helicsFederateRegisterGlobalTypePublication(feds.vFed, 'pub2', 'int', '');
pubid3 = helicsFederateRegisterTypePublication(feds.vFed, 'pub3', 'double', 'V');
helicsFederateEnterExecutingMode(feds.vFed);

publication_key = helicsPublicationGetName(pubid1);
testCase.verifyEqual(publication_key,'fed1/pub1');
publication_key = helicsPublicationGetName(pubid2);
testCase.verifyEqual(publication_key,'pub2');
publication_key = helicsPublicationGetName(pubid3);
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
    pubid = helicsFederateRegisterGlobalPublication(feds.vFed, 'pub1', HELICS_DATA_TYPE_NAMED_POINT, '');
    subid = helicsFederateRegisterSubscription(feds.vFed, 'pub1', '');

    helicsInputSetDefaultNamedPoint(subid, defaultValue, defVal);

    helicsFederateEnterExecutingMode(feds.vFed);

    % publish string1 at time=0.0;
    helicsPublicationPublishNamedPoint(pubid, testValue1, testVal1);

    % double val;
    [value,val] = helicsInputGetNamedPoint(subid);
    testCase.verifyEqual(value,defaultValue);
    testCase.verifyEqual(val,defVal);

    grantedtime = helicsFederateRequestTime(feds.vFed, 1.0);
    testCase.verifyEqual(grantedtime,0.01);

    % get the value
    [value,val] = helicsInputGetNamedPoint(subid);
    % make sure the string is what we expect
    testCase.verifyEqual(value,testValue1);
    testCase.verifyEqual(val,testVal1);

    % publish a second string
    helicsPublicationPublishNamedPoint(pubid, testValue2, testVal2);

    % make sure the value is still what we expect
    [value, val] = helicsInputGetNamedPoint(subid);
    % make sure the string is what we expect
    testCase.verifyEqual(value,testValue1);
    testCase.verifyEqual(val,testVal1);

    % advance time
    grantedtime = helicsFederateRequestTime(feds.vFed, 2.0);
    testCase.verifyEqual(grantedtime,0.02);

    % make sure the value was updated
    [value, val] = helicsInputGetNamedPoint(subid);
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
defaultValue = HELICS_TRUE;
    testValue1 = HELICS_TRUE;

    pubid = helicsFederateRegisterGlobalPublication(feds.vFed, 'pub1', HELICS_DATA_TYPE_BOOLEAN, '');
    subid = helicsFederateRegisterSubscription(feds.vFed, 'pub1', '');

    helicsInputSetDefaultBoolean(subid, defaultValue);


    helicsFederateEnterExecutingMode(feds.vFed);

    % publish string1 at time=0.0;
     helicsPublicationPublishBoolean(pubid, testValue1);


    % double val;
    value = helicsInputGetBoolean(subid);

    testCase.verifyEqual(value,defaultValue);

    grantedtime = helicsFederateRequestTime(feds.vFed, 1.0);

    testCase.verifyEqual(grantedtime,0.01);

    % get the value
    value = helicsInputGetBoolean(subid);

    % make sure the string is what we expect
    testCase.verifyEqual(value,testValue1);

    % publish a second boolean
    helicsPublicationPublish(pubid, false);


    % make sure the value is still what we expect
    value = helicsInputGetBoolean(subid);

    % make sure the string is what we expect
    testCase.verifyEqual(value,testValue1);

    % advance time
    grantedtime = helicsFederateRequestTime(feds.vFed, 2.0);

    testCase.verifyEqual(grantedtime,0.02);

    % make sure the value was updated
    value = helicsInputGetBoolean(subid);

    % make sure the string is what we expect
    testCase.verifyEqual(value,HELICS_FALSE);
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
    pubid1 = helicsFederateRegisterPublication(feds.vFed, 'pub1', HELICS_DATA_TYPE_STRING, '');
    pubid2 = helicsFederateRegisterGlobalPublication(feds.vFed, 'pub2', HELICS_DATA_TYPE_INT, '');
    pubid3 = helicsFederateRegisterPublication(feds.vFed, 'pub3', HELICS_DATA_TYPE_DOUBLE, 'V');

    publication_key = helicsPublicationGetName(pubid1);

testCase.verifyEqual(publication_key,'fed1/pub1');
publication_type = helicsPublicationGetType(pubid1);

testCase.verifyEqual(publication_type,'string');
publication_key = helicsPublicationGetName(pubid2);

testCase.verifyEqual(publication_key,'pub2');
publication_type = helicsPublicationGetType(pubid2);

testCase.verifyEqual(publication_type,'int64');
publication_key = helicsPublicationGetName(pubid3);

testCase.verifyEqual(publication_key,'fed1/pub3');
publication_type = helicsPublicationGetType(pubid3);

testCase.verifyEqual(publication_type,'double');
publication_units = helicsPublicationGetUnits(pubid3);

testCase.verifyEqual(publication_units,'V');

    helicsFederateEnterExecutingMode(feds.vFed);


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

    pubid = helicsFederateRegisterGlobalPublication(feds.vFed, 'pub1', HELICS_DATA_TYPE_DOUBLE, '');
    subid = helicsFederateRegisterSubscription(feds.vFed, 'pub1', '');

    helicsInputSetDefault(subid, defaultValue);


    helicsFederateEnterExecutingMode(feds.vFed);


    % publish string1 at time=0.0;
    helicsPublicationPublishDouble(pubid, testValue1);


    % double val;
    value = helicsInputGetDouble(subid);

    testCase.verifyEqual(value,defaultValue);

    grantedtime = helicsFederateRequestTime(feds.vFed, 1.0);

    testCase.verifyEqual(grantedtime,0.01);

    % get the value
    value = helicsInputGetDouble(subid);

    % make sure the string is what we expect
    testCase.verifyEqual(value,testValue1);

    % publish a second double
    helicsPublicationPublish(pubid, testValue2);


    % make sure the value is still what we expect
    value = helicsInputGetDouble(subid);

    % make sure the string is what we expect
    testCase.verifyEqual(value,testValue1);

    % advance time
    grantedtime = helicsFederateRequestTime(feds.vFed, 2.0);

    testCase.verifyEqual(grantedtime,0.02);

    % make sure the value was updated
    value = helicsInputGetDouble(subid);

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
defaultValue = 1.0-1.0j;
    testValue1 = 2.7586+ 342.25626j;
    testValue2 = 1e27-0.3e-2j;

    pubid = helicsFederateRegisterGlobalPublication(feds.vFed, 'pub1', HELICS_DATA_TYPE_COMPLEX, '');
    subid = helicsFederateRegisterSubscription(feds.vFed, 'pub1', '');

    helicsInputSetDefault(subid, defaultValue);


    helicsFederateEnterExecutingMode(feds.vFed);


    % publish string1 at time=0.0;
    helicsPublicationPublishComplex(pubid, testValue1);


    % double val;
    value = helicsInputGetComplex(subid);

    testCase.verifyEqual(value,defaultValue);

    grantedtime = helicsFederateRequestTime(feds.vFed, 1.0);

    testCase.verifyEqual(grantedtime,0.01);

    % get the value
    value = helicsInputGetComplex(subid);

    % make sure the value is what we expect
    testCase.verifyEqual(value,testValue1);

    % publish a second complex value
    helicsPublicationPublish(pubid, testValue2);


    % make sure the value is still what we expect
    value = helicsInputGetComplex(subid);

    % make sure the string is what we expect
    testCase.verifyEqual(value,testValue1);
    % advance time
    grantedtime = helicsFederateRequestTime(feds.vFed, 2.0);

    testCase.verifyEqual(grantedtime,0.02);

    % make sure the value was updated
    value = helicsInputGetComplex(subid);

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


function testInteger(testCase)
import matlab.unittest.constraints.IsTrue;
import helics.*
[feds,success]=generateFed();
testCase.verifyThat(success,IsTrue);

try
defaultValue = int64(45626678);
    testValue1 = int64(-27);
    testValue2 = int64(0);

    pubid = helicsFederateRegisterGlobalPublication(feds.vFed, 'pub1', HELICS_DATA_TYPE_INT, '');
    subid = helicsFederateRegisterSubscription(feds.vFed, 'pub1', '');

    helicsInputSetDefault(subid, defaultValue);


    helicsFederateEnterExecutingMode(feds.vFed);


    % publish string1 at time=0.0;
    helicsPublicationPublishInteger(pubid, testValue1);


    % double val;
    value = helicsInputGetInteger(subid);

    testCase.verifyEqual(value,defaultValue);

    grantedtime = helicsFederateRequestTime(feds.vFed, 1.0);

    testCase.verifyEqual(grantedtime,0.01);

    % get the value
    value = helicsInputGetInteger(subid);

    % make sure the string is what we expect
    testCase.verifyEqual(value,testValue1);

    % publish a second string
    helicsPublicationPublish(pubid, testValue2);

    % make sure the value is still what we expect
    value = helicsInputGetInteger(subid);

    % make sure the string is what we expect
    testCase.verifyEqual(value,testValue1);

    % advance time
    grantedtime = helicsFederateRequestTime(feds.vFed, 2.0);

    testCase.verifyEqual(grantedtime,0.02);

    % make sure the value was updated
    value = helicsInputGetInteger(subid);

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

    pubid = helicsFederateRegisterGlobalPublication(feds.vFed, 'pub1', HELICS_DATA_TYPE_STRING, '');
    subid = helicsFederateRegisterSubscription(feds.vFed, 'pub1', '');

    helicsInputSetDefault(subid, defaultValue);

    helicsFederateEnterExecutingMode(feds.vFed);

    % publish string1 at time=0.0;
    helicsPublicationPublish(pubid, testValue1);


    % double val;
    value = helicsInputGetString(subid);

    testCase.verifyEqual(value,defaultValue);

    grantedtime = helicsFederateRequestTime(feds.vFed, 1.0);

    testCase.verifyEqual(grantedtime,0.01);

    % get the value
    value = helicsInputGetString(subid);

    % make sure the string is what we expect
    testCase.verifyEqual(value,testValue1);

    % publish a second string
    helicsPublicationPublishString(pubid, testValue2);


    % make sure the value is still what we expect
    value = helicsInputGetString(subid);

    % make sure the string is what we expect
    testCase.verifyEqual(value,testValue1);

    % advance time
    grantedtime = helicsFederateRequestTime(feds.vFed, 2.0);

    testCase.verifyEqual(grantedtime,0.02);

    % make sure the value was updated
    value = helicsInputGetString(subid);

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

    pubid = helicsFederateRegisterGlobalPublication(feds.vFed, 'pub1', HELICS_DATA_TYPE_VECTOR, '');
    subid = helicsFederateRegisterSubscription(feds.vFed, 'pub1', '');

    helicsInputSetDefault(subid, defaultValue);


    helicsFederateEnterExecutingMode(feds.vFed);


    % publish string1 at time=0.0;
    helicsPublicationPublishVector(pubid, testValue1);


    % double val;
    value = helicsInputGetVector(subid);

    testCase.verifyEqual(value,defaultValue);

    grantedtime = helicsFederateRequestTime(feds.vFed, 1.0);

    testCase.verifyEqual(grantedtime,0.01);

    % get the value
    value = helicsInputGetVector(subid);

    % make sure the string is what we expect
    testCase.verifyEqual(value,testValue1);

    % publish a second string
    helicsPublicationPublish(pubid, testValue2);


    % make sure the value is still what we expect
    value = helicsInputGetVector(subid);

    % make sure the string is what we expect
    testCase.verifyEqual(value,testValue1);

    % advance time
    grantedtime = helicsFederateRequestTime(feds.vFed, 2.0);

    testCase.verifyEqual(grantedtime,0.02);

    % make sure the value was updated
    value = helicsInputGetVector(subid);

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
