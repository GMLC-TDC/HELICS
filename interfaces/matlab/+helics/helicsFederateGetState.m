function varargout = helicsFederateGetState(varargin)
    %get the current state of a federate
    %
    %Parameters
    %----------
    %* `fed` :
    %    the fed to query
    %* `state` :
    %    the resulting state if helics_status return helics_ok
    %
  [varargout{1:nargout}] = helicsMEX(99, varargin{:});
end
