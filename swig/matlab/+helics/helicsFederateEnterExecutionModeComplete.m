function varargout = helicsFederateEnterExecutionModeComplete(varargin)
    %complete the call to /ref EnterExecutionModeAsync
    %
    %Parameters
    %----------
    %* `fed` :
    %    the federate object to complete the call
    %
  [varargout{1:nargout}] = helicsMEX(82, varargin{:});
end
