function varargout = helicsEndpointGetType(varargin)
    %get the type specified for an endpoint
    %
    %Parameters
    %----------
    %* `endpoint` :
    %    the endpoint object in question
    %* `str` :
    %    the location where the string is stored
    %* `maxlen` :
    %    the maximum string length that can be stored in str
    %
    %Returns
    %-------
    %a status variable
    %
  [varargout{1:nargout}] = helicsMEX(197, varargin{:});
end
