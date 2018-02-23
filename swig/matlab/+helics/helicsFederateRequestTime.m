function varargout = helicsFederateRequestTime(varargin)
    %request the next time for federate execution
    %
    %Parameters
    %----------
    %* `fed` :
    %    the federate to make the request of
    %* `requestTime` :
    %    the next requested time
    %* `timeOut` :
    %    the time granted to the federate
    %
    %Returns
    %-------
    %a helics_status if the return value is equal to helics_ok the timeOut will
    %contain the new granted time, otherwise timeOut is invalid
    %
  [varargout{1:nargout}] = helicsMEX(80, varargin{:});
end
