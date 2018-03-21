function varargout = helicsFederateEnterExecutionMode(varargin)
    %request that the federate enter the Execution mode
    %
    %this call is blocking until granted entry by the core object for an asynchronous
    %alternative call /ref helicsFederateEnterExecutionModeAsync
    %
    %Parameters
    %----------
    %* `fed` :
    %    a federate to change modes
    %
    %Returns
    %-------
    %a helics_status enumeration helics_error if something went wrong
    %helicsInvalidReference if fed is invalid
    %
  [varargout{1:nargout}] = helicsMEX(80, varargin{:});
end
