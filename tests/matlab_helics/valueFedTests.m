function tests=valueFedTests

tests=functiontests(localfunctions);
end

function setup(testCase)  % do not change function name
% open a figure, for example

end

function teardown(testCase)  % do not change function name
% close figure, for example
end

function testBasic(testCase)
import matlab.unittest.constraints.IsGreaterThan
ver=helics.helicsGetVersion();
verifyThat(testCase,length(ver),IsGreaterThan(10),'version length is not valid');
end

function testEnterExecution(testCase)
import matlab.unittest.constraints.IsTrue;
[feds,success]=generateFed();
testCase.verifyThat(success,IsTrue);
helics.helicsFederateEnterExecutionMode(feds.vFed);
[status,state]=helics.helicsFederateGetState(feds.vFed);
testCase.verifyEqual(status,helics.helics_ok);
testCase.verifyEqual(state,helics.helics_execution_state);
success=closeStruct(feds);
testCase.verifyThat(success,IsTrue);
end

function testPublicationRegistration(testCase)
import matlab.unittest.constraints.IsTrue;
import helics.*
[feds,success]=generateFed();
testCase.verifyThat(success,IsTrue);
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
helicsCloseLibrary();

end