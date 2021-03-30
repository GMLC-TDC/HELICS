function messageFedTests

helics
end


%!function [fedStruct,success]=generateFed()
%! helics;
%! success=true;
%! initstring = '-f 1';
%! fedinitstring = '--broker=mainbroker --federates=1';
%! fedStruct.broker=helicsCreateBroker('zmq','mainbroker',initstring);
%! if (helicsBrokerIsValid(fedStruct.broker)==0)
%!    success=false;
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
%!   fedStruct.mFed=helicsCreateMessageFederate('fed1',fedInfo);
%!   if (helicsFederateIsValid(fedStruct.mFed)==0)
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
%! helicsFederateFinalize(fedStruct.mFed);
%! helicsBrokerWaitForDisconnect(fedStruct.broker,2000);
%!
%! helicsFederateFree(fedStruct.mFed);
%! helicsBrokerFree(fedStruct.broker);
%! helicsCloseLibrary();
%!
%!endfunction

%!function forceCloseStruct(fedStruct)
%! helics;
%! helicsFederateFinalize(fedStruct.mFed);
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
%! helicsFederateFree(fedStruct.mFed);
%! helicsBrokerFree(fedStruct.broker);
%! helicsCloseLibrary();
%!
%!endfunction


%!test
%! helics;
%! [feds,success]=generateFed();
%! assert(success)
%! try
%! epid1 = helicsFederateRegisterEndpoint(feds.mFed, 'ep1', '');
%! helicsFederateEnterExecutingMode(feds.mFed);
%! state=helicsFederateGetState(feds.mFed);
%! assert(state,2);
%! success=closeStruct(feds);
%! assert(success);
%! catch e
%!    disp(e.message)
%!     disp(e.stack(1))
%!     forceCloseStruct(feds);
%!     assert(false);
%! end_try_catch
%
% testEndpointRegistration
%!test
%! helics
%! [feds,success]=generateFed();
%! assert(success)
%! try
%! epid1 = helicsFederateRegisterEndpoint(feds.mFed, 'ep1', '');
%!
%! epid2 = helicsFederateRegisterGlobalEndpoint(feds.mFed, 'ep2', 'random');
%!
%! helicsFederateEnterExecutingMode(feds.mFed);
%!
%! ept_key = helicsEndpointGetName(epid1);
%! assert(ept_key,'fed1/ep1');
%!
%! ept_key = helicsEndpointGetName(epid2);
%! assert(ept_key,'ep2');
%!
%! ept_type = helicsEndpointGetType(epid1);
%! assert(isempty(ept_type));
%!
%! ept_type = helicsEndpointGetType(epid2);
%! assert(ept_type,'random');
%! success=closeStruct(feds);
%! assert(success);
%! catch e
%!    disp(e.message)
%!     disp(e.stack(1))
%!     forceCloseStruct(feds);
%!     assert(false);
%! end_try_catch

% testEndpointSend
%!test
%! helics;
%! [feds,success]=generateFed();
%! assert(success)
%! try
%! epid1 = helicsFederateRegisterEndpoint(feds.mFed, 'ep1', '');
%!
%! epid2 = helicsFederateRegisterGlobalEndpoint(feds.mFed, 'ep2', 'random');
%!
%! helicsFederateSetTimeProperty(feds.mFed,int32(137),1.0);
%! helicsFederateEnterExecutingMode(feds.mFed);
%! data = 'this is a random string message';
%!
%! helicsEndpointSendBytesToAt(epid1,data,'ep2',1.0);
%!
%! granted_time=helicsFederateRequestTime(feds.mFed,2.0);
%! assert(granted_time,1.0);
%!
%! res=helicsFederateHasMessage(feds.mFed);
%! assert(res,1);
%! res=helicsEndpointHasMessage(epid1);
%! assert(res,0);
%! res=helicsEndpointHasMessage(epid2);
%! assert(res,1);
%!
%! message = helicsEndpointGetMessage(epid2);
%! assert(helicsMessageGetString(message),data);
%! assert(double(helicsMessageGetByteCount(message)),length(data));
%! assert(helicsMessageGetOriginalSource(message),'fed1/ep1');
%! assert(helicsMessageGetTime(message),1.0);
%! success=closeStruct(feds);
%! assert(success);
%!
%! catch e
%!    disp(e.message)
%!     disp(e.stack(1))
%!     forceCloseStruct(feds);
%!     assert(false);
%! end_try_catch
