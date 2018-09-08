function varargout = helicsFederateEnterExecutionModeAsync(varargin)
    %request that the federate enter the Execution mode
    %
    %this call is non-blocking and will return immediately call /ref
    %helicsFederateEnterExecutionModeComplete to finish the call sequence /ref
    %
  [varargout{1:nargout}] = helicsMEX(86, varargin{:});
end
