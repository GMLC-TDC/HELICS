function varargout = helicsFederateGetCurrentTime(varargin)
    %get the current time of the federate
    %
    %Parameters
    %----------
    %* `fed` :
    %    the federate object to query
    %* `time` :
    %    storage location for the time variable
    %
    %Returns
    %-------
    %helics_status object indicating success or error
    %
  [varargout{1:nargout}] = helicsMEX(108, varargin{:});
end
