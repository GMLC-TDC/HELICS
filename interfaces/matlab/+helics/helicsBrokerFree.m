function varargout = helicsBrokerFree(varargin)
    %release the memory associated with a broker
    %
  [varargout{1:nargout}] = helicsMEX(60, varargin{:});
end
