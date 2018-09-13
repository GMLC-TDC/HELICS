function varargout = helicsBrokerGetAddress(varargin)
    %get the network address associated with a broker
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
  [varargout{1:nargout}] = helicsMEX(51, varargin{:});
end
