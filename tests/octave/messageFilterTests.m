function messageFilterTests

helics
end


%!function [fedStruct,success]=generateFeds(count)
%! helics;
%! success=true;
%! initstring = ['-f ',num2str(count)];
%! fedinitstring = ['--broker=mainbroker --federates=',num2str(count)];
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
%!  for ii=1:count
%!   fedStruct.mFed{ii}=helicsCreateMessageFederate(['fed',num2str(ii)],fedInfo);
%!   if (helicsFederateIsValid(fedStruct.mFed{ii})==0)
%!     success=false;
%!   endif
%!  endfor
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
%! for ii=1:numel(fedStruct.mFed)
%!   helicsFederateFinalize(fedStruct.mFed{ii});
%! endfor
%! helicsBrokerWaitForDisconnect(fedStruct.broker,2000);
%!
%!  for ii=1:numel(fedStruct.mFed)
%!    helicsFederateFree(fedStruct.mFed{ii});
%!  endfor
%! helicsBrokerFree(fedStruct.broker);
%! helicsCloseLibrary();
%!
%!endfunction

%!function forceCloseStruct(fedStruct)
%! helics;
%! for ii=1:numel(fedStruct.mFed)
%!   helicsFederateFinalize(fedStruct.mFed{ii});
%! endfor
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
%!  for ii=1:numel(fedStruct.mFed)
%!    helicsFederateFree(fedStruct.mFed{ii});
%!  endfor
%! helicsBrokerFree(fedStruct.broker);
%! helicsCloseLibrary();
%!
%!endfunction


%function to test some of the broker functions
% testBrokerFunctions
%!test
%! helics;
%! initstring = '-f1 --name=mainbroker';
%! broker=helicsCreateBroker('zmq','',initstring);
%! valid=helicsBrokerIsValid(broker);
%! assert(valid,1);
%! ident=helicsBrokerGetIdentifier(broker);
%! assert(ident,'mainbroker');
%! add=helicsBrokerGetAddress(broker);
%! assert(add,'tcp://127.0.0.1:23404');
%! helicsBrokerDisconnect(broker);
%! helicsBrokerFree(broker);
%! helicsCloseLibrary();

%
% testFilterRegistration
%!test
%! helics;
%! [feds,success]=generateFeds(2);
%! assert(success)
%! try
%! mFed=feds.mFed{1};
%! fFed=feds.mFed{2};
%!
%! helicsFederateRegisterGlobalEndpoint(mFed, 'port1', '');
%!
%! helicsFederateRegisterGlobalEndpoint(mFed, 'port2', 'random');
%!
%! f1=helicsFederateRegisterGlobalFilter(fFed,helics.HELICS_FILTER_TYPE_CUSTOM,'filter1');
%! helicsFilterAddSourceTarget(f1,'port1');
%! f2=helicsFederateRegisterGlobalFilter(fFed,helics.HELICS_FILTER_TYPE_DELAY,'filter2');
%! helicsFilterAddDestinationTarget(f2,'port2');
%! helicsFederateRegisterEndpoint(fFed,'fout','');
%! f3=helicsFederateRegisterFilter(fFed,helics.HELICS_FILTER_TYPE_RANDOM_DELAY,'filter3');
%! helicsFilterAddSourceTarget(f3,'fed2/fout');
%!
%! helicsFederateEnterExecutingModeAsync(mFed);
%! helicsFederateEnterExecutingMode(fFed);
%!
%! helicsFederateEnterExecutingModeComplete(mFed);
%!
%! filt_key = helicsFilterGetName(f1);
%! assert(filt_key,'filter1');
%!
%! filt_key = helicsFilterGetName(f2);
%! assert(filt_key,'filter2');
%!
%! success=closeStruct(feds);
%! assert(success);
%! catch e
%!    disp(e.message)
%!     disp(e.stack(1))
%!     forceCloseStruct(feds);
%!     assert(false);
%! end_try_catch
% end
%
% testFilterFunction
%!test
%! helics;
%! [feds,success]=generateFeds(2);
%! assert(success)
%! try
%! mFed=feds.mFed{1};
%! fFed=feds.mFed{2};
%!
%! p1=helicsFederateRegisterGlobalEndpoint(mFed, 'port1', '');
%!
%! p2=helicsFederateRegisterGlobalEndpoint(mFed, 'port2', '');
%!
%! f1=helicsFederateRegisterFilter(fFed,helics.HELICS_FILTER_TYPE_DELAY,'filter1');
%! helicsFilterAddSourceTarget(f1,'port1');
%! helicsFilterSet(f1,'delay',2.5);
%!
%! helicsFederateEnterExecutingModeAsync(mFed);
%! helicsFederateEnterExecutingMode(fFed);
%! helicsFederateEnterExecutingModeComplete(mFed);
%!
%! data='hello world';
%! helicsEndpointSendBytesTo(p1,data,'port2');
%!
%! granted_time=helicsFederateRequestTime(mFed,1.0);
%! assert(granted_time,1.0);
%!
%! res=helicsFederateHasMessage(mFed);
%! assert(res,0);
%!
%! granted_time=helicsFederateRequestTime(mFed,3.0);
%! assert(granted_time,2.5);
%!
%!
%! res=helicsEndpointHasMessage(p2);
%! assert(res,1);
%!
%!
%! success=closeStruct(feds);
%! assert(success);
%! catch e
%!    disp(e.message)
%!     disp(e.stack(1))
%!     forceCloseStruct(feds);
%!     assert(false);
%! end_try_catch
