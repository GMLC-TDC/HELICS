function varargout = helicsBrokerGetIdentifier(varargin)
    %get an identifier for the broker
    %
    %Parameters
    %----------
    %* `broker` :
    %    the broker to query
    %* `identifier` :
    %    storage space to place the identifier string
    %* `maxlen` :
    %    the maximum space available in identifier
    %
    %Returns
    %-------
    %a helics_status enumeration indicating any error condition
    %
  [varargout{1:nargout}] = helicsMEX(37, varargin{:});
end
