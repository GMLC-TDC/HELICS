function varargout = helicsFederateInfoSetCoreInitString(varargin)
    %set the initialization string for the core usually in the form of command line
    %arguments
    %
    %Parameters
    %----------
    %* `fi` :
    %    the federate info object to alter
    %* `coreInit` :
    %    a string with the core initialization strings
    %
    %Returns
    %-------
    %a helics_status enumeration helics_ok on success helicsInvalidReference if fi is
    %not a valid reference
    %
  [varargout{1:nargout}] = helicsMEX(65, varargin{:});
end
