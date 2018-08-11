function varargout = helicsEndpointReceiveCount(varargin)
    %Returns the number of pending receives for all endpoints of particular federate.
    %
  [varargout{1:nargout}] = helicsMEX(172, varargin{:});
end
