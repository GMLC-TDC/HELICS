function varargout = helicsFederateSetTimeDelta(varargin)
    %set the minimum time delta for the federate
    %
    %Parameters
    %----------
    %* `tdelta` :
    %    the minimum time delta to return from a time request function
    %
  [varargout{1:nargout}] = helicsMEX(102, varargin{:});
end
