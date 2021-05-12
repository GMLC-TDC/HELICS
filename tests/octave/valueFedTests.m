function valueFedTests
   %helics
end


%!function [fedStruct,success]=generateFed()
%! helics;
%! success=true;
%! initstring = '-f 1';
%! fedinitstring = '--broker=mainbroker --federates=1';
%! fedStruct.broker=helicsCreateBroker('zmq','mainbroker',initstring);
%! if (helicsBrokerIsValid(fedStruct.broker)==0)
%!    success=false;
%!    disp('Broker is not valid');
%!    return;
%! endif
%! fedInfo=helicsCreateFederateInfo();
%! try
%!    helicsFederateInfoSetCoreTypeFromString(fedInfo,'zmq');
%!    helicsFederateInfoSetCoreInitString(fedInfo,fedinitstring);
%!   helicsFederateInfoSetTimeProperty(fedInfo,HELICS_PROPERTY_TIME_DELTA, 0.01);
%!    helicsFederateInfoSetIntegerProperty(fedInfo,HELICS_PROPERTY_INT_LOG_LEVEL,1);
%! catch e
%!     disp(e.message)
%!     disp(e.stack(1))
%!    success=false;
%!    helicsBrokerDestroy(fedStruct.broker);
%!    helicsFederateInfoFree(fedInfo);
%!    return
%! end_try_catch
%! try
%!   fedStruct.vFed=helicsCreateValueFederate('fed1',fedInfo);
%!   if (helicsFederateIsValid(fedStruct.vFed)==0)
%!     success=false;
%!   endif
%! catch e
%!     disp(e.message)
%!     disp(e.stack(1))
%!     success=false;
%! end_try_catch
%! helicsFederateInfoFree(fedInfo);
%!endfunction

%!function success=closeStruct(fedStruct)
%! helics;
%! success=true;
%! helicsFederateFinalize(fedStruct.vFed);
%! helicsBrokerWaitForDisconnect(fedStruct.broker,2000);
%!
%! helicsFederateFree(fedStruct.vFed);
%! helicsBrokerFree(fedStruct.broker);
%! helicsCloseLibrary();
%!
%!endfunction

%!function forceCloseStruct(fedStruct)
%! helics;
%! helicsFederateFinalize(fedStruct.vFed);
%!
%! cnt=0;
%! while (helicsBrokerIsConnected(fedStruct.broker))
%!    pause(1);
%!    cnt=cnt+1;
%!    if (cnt>5)
%!        helicsBrokerDisconnect(fedStruct.broker);
%!        break;
%!    endif
%! endwhile
%!
%! helicsFederateFree(fedStruct.vFed);
%! helicsBrokerFree(fedStruct.broker);
%! helicsCloseLibrary();
%!
%!endfunction

%!test
%! ver=helicsGetVersion();
%! assert(length(ver)>10);

%!test
%! [feds,success]=generateFed();
%! assert(success)
%! try
%!    helicsFederateEnterExecutingMode(feds.vFed);
%!    state=helicsFederateGetState(feds.vFed);
%!    assert(state,2);
%!    success=closeStruct(feds);
%!    assert(success)
%! catch err
%!     err
%!     forceCloseStruct(feds);
%!     assert(false)
%! end_try_catch
%
%!test
%! [feds,success]=generateFed();
%! assert(success)
%! try
%!   pubid1 = helicsFederateRegisterTypePublication(feds.vFed, 'pub1', 'string', '');
%!   pubid2 = helicsFederateRegisterGlobalTypePublication(feds.vFed, 'pub2', 'int', '');
%!   pubid3 = helicsFederateRegisterTypePublication(feds.vFed, 'pub3', 'double', 'V');
%!   helicsFederateEnterExecutingMode(feds.vFed);
%!
%!   publication_name = helicsPublicationGetName(pubid1);
%!   assert(publication_name,'fed1/pub1');
%!   publication_name = helicsPublicationGetName(pubid2);
%!   assert(publication_name,'pub2');
%!   publication_name = helicsPublicationGetName(pubid3);
%!   assert(publication_name,'fed1/pub3');
%!   publication_type = helicsPublicationGetType(pubid3);
%!   assert(publication_type,'double');
%!   publication_units = helicsPublicationGetUnits(pubid3);
%!   assert(publication_units,'V');
%!   success=closeStruct(feds);
%!   assert(success);
%! catch e
%!   e
%!   forceCloseStruct(feds);
%!   assert(false);
%! end_try_catch

% test named point
%!test
%! helics;
%! [feds,success]=generateFed();
%! assert(success)
%!
%! defaultValue = 'start';
%!     defVal = 5.3;
%!     testValue1 = 'inside of the functional relationship of helics';
%!     testVal1 = 45.7823;
%!     testValue2 = 'I am a string';
%!     testVal2 = 0.0;
%! try
%!     pubid = helicsFederateRegisterGlobalPublication(feds.vFed, 'pub1', HELICS_DATA_TYPE_NAMED_POINT, '');
%!     subid = helicsFederateRegisterSubscription(feds.vFed, 'pub1', '');
%!
%!     helicsInputSetDefaultNamedPoint(subid, defaultValue, defVal);
%!
%!     helicsFederateEnterExecutingMode(feds.vFed);
%!
%!     % publish string1 at time=0.0;
%!     helicsPublicationPublishNamedPoint(pubid, testValue1, testVal1);
%!
%!     % double val;
%!     [value,val] = helicsInputGetNamedPoint(subid);
%!     assert(value,defaultValue);
%!     assert(val,defVal);
%!
%!     grantedtime = helicsFederateRequestTime(feds.vFed, 1.0);
%!     assert(grantedtime,0.01);
%!
%!     % get the value
%!     [value,val] = helicsInputGetNamedPoint(subid);
%!     % make sure the string is what we expect
%!     assert(value,testValue1);
%!     assert(val,testVal1);
%!
%!     % publish a second string
%!     helicsPublicationPublishNamedPoint(pubid, testValue2, testVal2);
%!
%!     % make sure the value is still what we expect
%!     [value, val] = helicsInputGetNamedPoint(subid);
%!     % make sure the string is what we expect
%!     assert(value,testValue1);
%!     assert(val,testVal1);
%!
%!     % advance time
%!     grantedtime = helicsFederateRequestTime(feds.vFed, 2.0);
%!     assert(grantedtime,0.02);
%!
%!     % make sure the value was updated
%!     [value, val] = helicsInputGetNamedPoint(subid);
%!     % make sure the string is what we expect
%!     assert(value,testValue2);
%!     assert(val,testVal2);
%!     success=closeStruct(feds);
%!     assert(success);
%! catch err
%!    err
%!     forceCloseStruct(feds);
%!     assert(false);
%! end_try_catch
%
% test boolean
%!test
%! helics;
%! [feds,success]=generateFed();
%! assert(success)
%!
%! try
%! defaultValue = 1;
%!     testValue1 = 1;
%!     testValue2 = 0;
%!
%!     pubid = helicsFederateRegisterGlobalPublication(feds.vFed, 'pub1', HELICS_DATA_TYPE_BOOLEAN, '');
%!     subid = helicsFederateRegisterSubscription(feds.vFed, 'pub1', '');
%!
%!     helicsInputSetDefaultBoolean(subid, defaultValue);
%!
%!     helicsFederateEnterExecutingMode(feds.vFed);
%!
%!     % publish string1 at time=0.0;
%!      helicsPublicationPublishBoolean(pubid, testValue1);
%!
%!     % double val;
%!     value = helicsInputGetBoolean(subid);
%!
%!     assert(value,defaultValue);
%!
%!     grantedtime = helicsFederateRequestTime(feds.vFed, 1.0);
%!
%!     assert(grantedtime,0.01);
%!
%!     % get the value
%!     value = helicsInputGetBoolean(subid);
%!
%!     % make sure the string is what we expect
%!     assert(value,testValue1);
%!
%!     % publish a second string
%!     helicsPublicationPublishBoolean(pubid, testValue2);
%!
%!
%!     % make sure the value is still what we expect
%!     value = helicsInputGetBoolean(subid);
%!
%!     % make sure the string is what we expect
%!     assert(value,testValue1);
%!
%!     % advance time
%!     grantedtime = helicsFederateRequestTime(feds.vFed, 2.0);
%!
%!     assert(grantedtime,0.02);
%!
%!     % make sure the value was updated
%!     value = helicsInputGetBoolean(subid);
%!
%!     % make sure the string is what we expect
%!     assert(value,testValue2);
%!     success=closeStruct(feds);
%!     assert(success);
%! catch e
%!     disp(e.message)
%!     disp(e.stack(1))
%!     forceCloseStruct(feds);
%!     assert(false);
%! end_try_catch
%
% testPublisherRegistration
%!test
%! helics;
%! [feds,success]=generateFed();
%! assert(success)
%!
%! try
%!     pubid1 = helicsFederateRegisterPublication(feds.vFed, 'pub1', HELICS_DATA_TYPE_STRING, '');
%!     pubid2 = helicsFederateRegisterGlobalPublication(feds.vFed, 'pub2', HELICS_DATA_TYPE_INT, '');
%!     pubid3 = helicsFederateRegisterPublication(feds.vFed, 'pub3', HELICS_DATA_TYPE_DOUBLE, 'V');
%!
%!     publication_name = helicsPublicationGetName(pubid1);
%!
%! assert(publication_name,'fed1/pub1');
%! publication_type = helicsPublicationGetType(pubid1);
%!
%! assert(publication_type,'string');
%! publication_name = helicsPublicationGetName(pubid2);
%!
%! assert(publication_name,'pub2');
%! publication_type = helicsPublicationGetType(pubid2);
%!
%! assert(publication_type,'int64');
%! publication_name = helicsPublicationGetName(pubid3);
%!
%! assert(publication_name,'fed1/pub3');
%! publication_type = helicsPublicationGetType(pubid3);
%!
%! assert(publication_type,'double');
%! publication_units = helicsPublicationGetUnits(pubid3);
%!
%! assert(publication_units,'V');
%!
%!     helicsFederateEnterExecutingMode(feds.vFed);
%!
%!
%!    %% add state and some type checks
%!     success=closeStruct(feds);
%!     assert(success);
%! catch e
%!     disp(e.message)
%!     disp(e.stack(1))
%!     forceCloseStruct(feds);
%!    assert(false);
%! end_try_catch

%
% testDouble
%!test
%! helics;
%! [feds,success]=generateFed();
%! assert(success)
%!
%! try
%!     defaultValue = 1.0;
%!     testValue1 = 2.7586;
%!     testValue2 = 1e27;
%!
%!     pubid = helicsFederateRegisterGlobalPublication(feds.vFed, 'pub1', HELICS_DATA_TYPE_DOUBLE, '');
%!     subid = helicsFederateRegisterSubscription(feds.vFed, 'pub1', '');
%!
%!     helicsInputSetDefaultDouble(subid, defaultValue);
%!
%!     helicsFederateEnterExecutingMode(feds.vFed);
%!
%!     % publish string1 at time=0.0;
%!     helicsPublicationPublishDouble(pubid, testValue1);
%!
%!     % double val;
%!     value = helicsInputGetDouble(subid);
%!
%!     assert(value,defaultValue);
%!
%!     grantedtime = helicsFederateRequestTime(feds.vFed, 1.0);
%!
%!     assert(grantedtime,0.01);
%!
%!     % get the value
%!     value = helicsInputGetDouble(subid);
%!
%!     % make sure the string is what we expect
%!     assert(value,testValue1);
%!
%!     % publish a second string
%!     helicsPublicationPublishDouble(pubid, testValue2);
%!
%!
%!     % make sure the value is still what we expect
%!     value = helicsInputGetDouble(subid);
%!
%!     % make sure the string is what we expect
%!     assert(value,testValue1);
%!
%!     % advance time
%!     grantedtime = helicsFederateRequestTime(feds.vFed, 2.0);
%!
%!     assert(grantedtime,0.02);
%!
%!     % make sure the value was updated
%!     value = helicsInputGetDouble(subid);
%!
%!     % make sure the string is what we expect
%!     assert(value,testValue2);
%!     success=closeStruct(feds);
%!     assert(success);
%! catch e
%!     disp(e.message)
%!     disp(e.stack(1))
%!     forceCloseStruct(feds);
%!     assert(false)
%! end_try_catch
% end
%
%
% testComplex
%!test
%! helics;
%! [feds,success]=generateFed();
%! assert(success)
%!
%! try
%!     defaultValue = 1.0-1.0j;
%!     testValue1 = 2.7586+ 342.25626j;
%!     testValue2 = 1e27-0.3e-2j;
%!
%!     pubid = helicsFederateRegisterGlobalPublication(feds.vFed, 'pub1', HELICS_DATA_TYPE_COMPLEX, '');
%!     subid = helicsFederateRegisterSubscription(feds.vFed, 'pub1', '');
%!
%!     helicsInputSetDefaultComplex(subid, defaultValue);
%!
%!     helicsFederateEnterExecutingMode(feds.vFed);
%!
%!     % publish string1 at time=0.0;
%!     helicsPublicationPublishComplex(pubid, testValue1);
%!
%!     % double val;
%!     value = helicsInputGetComplex(subid);
%!
%!     assert(value,defaultValue);
%!
%!     grantedtime = helicsFederateRequestTime(feds.vFed, 1.0);
%!
%!     assert(grantedtime,0.01);
%!
%!    % get the value
%!     value = helicsInputGetComplex(subid);
%!
%!     % make sure the string is what we expect
%!     assert(value,testValue1);
%!
%!     % publish a second string
%!     helicsPublicationPublishComplex(pubid, testValue2);
%!
%!
%!     % make sure the value is still what we expect
%!     value = helicsInputGetComplex(subid);
%!
%!     % make sure the string is what we expect
%!     assert(value,testValue1);
%!     % advance time
%!     grantedtime = helicsFederateRequestTime(feds.vFed, 2.0);
%!
%!     assert(grantedtime,0.02);
%!
%!     % make sure the value was updated
%!     value = helicsInputGetComplex(subid);
%!
%!     % make sure the string is what we expect
%!     assert(value,testValue2);
%!     success=closeStruct(feds);
%!     assert(success);
%! catch e
%!     disp(e.message)
%!     disp(e.stack(1))
%!     forceCloseStruct(feds);
%!     assert(false)
%! end_try_catch
%
%
% testInteger
%!test
%! helics;
%! [feds,success]=generateFed();
%! assert(success)
%!
%! try
%! defaultValue = int64(45626678);
%!     testValue1 = int64(-27);
%!     testValue2 = int64(0);
%!
%!     pubid = helicsFederateRegisterGlobalPublication(feds.vFed, 'pub1', HELICS_DATA_TYPE_INT, '');
%!     subid = helicsFederateRegisterSubscription(feds.vFed, 'pub1', '');
%!
%!     helicsInputSetDefaultInteger(subid, defaultValue);
%!
%!     helicsFederateEnterExecutingMode(feds.vFed);
%!
%!     % publish string1 at time=0.0;
%!     helicsPublicationPublishInteger(pubid, testValue1);
%!
%!     % double val;
%!     value = helicsInputGetInteger(subid);
%!
%!     assert(value,defaultValue);
%!
%!     grantedtime = helicsFederateRequestTime(feds.vFed, 1.0);
%!
%!     assert(grantedtime,0.01);
%!
%!     % get the value
%!     value = helicsInputGetInteger(subid);
%!
%!     % make sure the string is what we expect
%!     assert(value,testValue1);
%!
%!     % publish a second string
%!     helicsPublicationPublishInteger(pubid, testValue2);
%!
%!     % make sure the value is still what we expect
%!     value = helicsInputGetInteger(subid);
%!
%!     % make sure the string is what we expect
%!    assert(value,testValue1);
%!
%!     % advance time
%!     grantedtime = helicsFederateRequestTime(feds.vFed, 2.0);
%!
%!     assert(grantedtime,0.02);
%!
%!     % make sure the value was updated
%!     value = helicsInputGetInteger(subid);
%!
%!     % make sure the value is what we expect
%!     assert(value,testValue2);
%!     success=closeStruct(feds);
%!     assert(success);
%! catch e
%!     disp(e.message)
%!     disp(e.stack(1))
%!     forceCloseStruct(feds);
%!     assert(false)
%! end_try_catch
%
%
% testString
%!test
%! helics;
%! [feds,success]=generateFed();
%! assert(success)
%!
%! try
%! defaultValue = 'string1';
%!     testValue1 = 'this is a longer test string to bypass sso';
%!     testValue2 = '';
%!
%!     pubid = helicsFederateRegisterGlobalPublication(feds.vFed, 'pub1', HELICS_DATA_TYPE_STRING, '');
%!     subid = helicsFederateRegisterSubscription(feds.vFed, 'pub1', '');
%!
%!     helicsInputSetDefaultString(subid, defaultValue);
%!
%!     helicsFederateEnterExecutingMode(feds.vFed);
%!
%!     % publish string1 at time=0.0;
%!     helicsPublicationPublishString(pubid, testValue1);
%!
%!     % double val;
%!     value = helicsInputGetString(subid);
%!     assert(value,defaultValue);
%!
%!     grantedtime = helicsFederateRequestTime(feds.vFed, 1.0);
%!
%!     assert(grantedtime,0.01);
%!
%!     % get the value
%!     value = helicsInputGetString(subid);
%!
%!     % make sure the string is what we expect
%!     assert(value,testValue1);
%!
%!     % publish a second string
%!     helicsPublicationPublishString(pubid, testValue2);
%!
%!
%!     % make sure the value is still what we expect
%!     value = helicsInputGetString(subid);
%!
%!     % make sure the string is what we expect
%!     assert(value,testValue1);
%!
%!     % advance time
%!     grantedtime = helicsFederateRequestTime(feds.vFed, 2.0);
%!
%!     assert(grantedtime,0.02);
%!
%!     % make sure the value was updated
%!     value = helicsInputGetString(subid);
%!
%!     % make sure the value is what we expect
%!     assert(isempty(value),isempty(testValue2));
%!     success=closeStruct(feds);
%!     assert(success);
%! catch e
%!     disp(e.message)
%!     disp(e.stack(1))
%!     forceCloseStruct(feds);
%!     assert(false)
%! end_try_catch

%
%
% testVector
%!test
%! helics;
%! [feds,success]=generateFed();
%! assert(success)
%!
%! try
%!     defaultValue = [36.5;22.1;-10.4];
%!     testValue1 = ones(22,1);
%!     testValue2 = [99.1;-99;2;0.0;-1e35;4.56e-7];
%!
%!     pubid = helicsFederateRegisterGlobalPublication(feds.vFed, 'pub1', HELICS_DATA_TYPE_VECTOR, '');
%!     subid = helicsFederateRegisterSubscription(feds.vFed, 'pub1', '');
%!
%!     helicsInputSetDefaultVector(subid, defaultValue);
%!
%!
%!     helicsFederateEnterExecutingMode(feds.vFed);
%!
%!     % publish string1 at time=0.0;
%!     helicsPublicationPublishVector(pubid, testValue1);
%!
%!
%!     % double val;
%!     value = helicsInputGetVector(subid);
%!     assert(value,defaultValue);
%!
%!     grantedtime = helicsFederateRequestTime(feds.vFed, 1.0);
%!
%!     assert(grantedtime,0.01);
%!
%!     % get the value
%!     value = helicsInputGetVector(subid);
%!
%!     % make sure the string is what we expect
%!     assert(value,testValue1);
%!
%!     % publish a second string
%!     helicsPublicationPublishVector(pubid, testValue2);
%!
%!
%!     % make sure the value is still what we expect
%!     value = helicsInputGetVector(subid);
%!
%!     % make sure the string is what we expect
%!     assert(value,testValue1);
%!
%!     % advance time
%!     grantedtime = helicsFederateRequestTime(feds.vFed, 2.0);
%!
%!     assert(grantedtime,0.02);
%!
%!     % make sure the value was updated
%!     value = helicsInputGetVector(subid);
%!
%!     % make sure the value is what we expect
%!     assert(isempty(value),isempty(testValue2));
%!     success=closeStruct(feds);
%!     assert(success);
%! catch e
%!     disp(e.message)
%!     disp(e.stack(1))
%!     forceCloseStruct(feds);
%!     assert(false)
%! end_try_catch
