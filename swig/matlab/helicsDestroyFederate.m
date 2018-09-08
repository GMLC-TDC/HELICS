function helicsDestroyFederate(fed_info, close_library)
% helicsDestroyFederate helper function to abstract steps to close a HELICS federate
%
% helicsDestroyFederate(fed_info)  Gets rid of all HELICS data and C
%   objects associated with the federate described by fed_info. This
%   includes "closing" the HELICS library so a new federate can be created
%   in the same MATLAB session.
%
% helicsDestroyFederate(fed_info, false) Ends the federate, but does not
%   close the HELICS library. This option is used when a single MATLAB
%   session/program is managing multiple HELICS objects, such as multiple
%   federates. In the multi-object case, the library should only be closed
%   when destroying the last object.
%
% TODO: mauy eventually become part of the C library and called via SWIG mex

if nargin <2 || isempty(close_library)
    close_library = true;
end

if nargin <1 || isempty(fed_info)
    if close_library
        warning('Empty fed_info, will not finalize or free HELICS federate. Will still close library')
    else
        warning('Nothing to do: Empty fed_info, not closing library')
    end        
else
    status = helics.helicsFederateFinalize(fed_info);
    if status ~= 0
        warning('Non-zero status (%d) returned in call to helicsFederateFinalize. Ignoring.', status)
    end


    helics.helicsFederateFree(fed_info);
end

if close_library
    %Note: helicsCloseLibrary is unrelated to the MATLAB library loading (from
    %helicsStartup)
    helics.helicsCloseLibrary();
end