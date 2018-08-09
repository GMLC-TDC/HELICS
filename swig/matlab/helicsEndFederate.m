function helicsDestroyFederate(fed_info)
% helicsDestroyFederate helper function to abstract steps to close a HELICS federate
%
% TODO: mauy eventually become part of the C library and called via mex
if isempty(fed_info)
    warning('Empty fed_info, will not finalize or free HELICS federate. Will still close library')
else
    status = helics.helicsFederateFinalize(fed_info);
    if status ~= 0
        warning('Non-zero status (%d) returned in call to helicsFederateFinalize. Ignoring.', status)
    end


    helics.helicsFederateFree(fed_info);
end

%Note: helicsCloseLibrary is unrelated to the MATLAB library loading (from
%helicsStartup)
helics.helicsCloseLibrary();