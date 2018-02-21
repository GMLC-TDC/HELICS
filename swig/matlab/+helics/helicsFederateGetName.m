function varargout = helicsFederateGetName(varargin)
    %get the name of the federate
    %
    %Parameters
    %----------
    %* `fed` :
    %    the federate object to query
    %* `str` :
    %    memory buffer to store the result
    %* `maxlen` :
    %    the maximum size of the buffer
    %
    %Returns
    %-------
    %helics_status object indicating success or error
    %
  [varargout{1:nargout}] = helicsMEX(86, varargin{:});
end
