function varargout = helicsCoreGetIdentifier(varargin)
    %get an identifier for the core
    %
    %Parameters
    %----------
    %* `core` :
    %    the core to query
    %* `identifier` :
    %    storage space to place the identifier string
    %* `maxlen` :
    %    the maximum space available in identifier
    %
    %Returns
    %-------
    %a helics_status enumeration indicating any error condition
    %
  [varargout{1:nargout}] = helicsMEX(42, varargin{:});
end
