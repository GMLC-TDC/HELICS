function varargout = helicsFederateSetInputDelay(varargin)
    %set the impact Window time
    %
    %the impact window is the time window around the time request in which other
    %federates cannot affect the federate
    %
    %Parameters
    %----------
    %* `lookAhead` :
    %    the look ahead time
    %
  [varargout{1:nargout}] = helicsMEX(104, varargin{:});
end
