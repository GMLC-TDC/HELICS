function varargout = helicsBrokerIsConnected(varargin)
    %check if a broker is connected a connected broker implies is attached to cores
    %or cores could reach out to communicate return 0 if not connected , something
    %else if it is connected
    %
  [varargout{1:nargout}] = helicsMEX(39, varargin{:});
end
