function varargout = helicsFederateRequestTimeIterative(varargin)
    %request an iterative time
    %
    %this call allows for finer grain control of the iterative process then /ref
    %helicsFederateRequestTime it takes a time and and iteration request and return a
    %time and iteration status
    %
    %Parameters
    %----------
    %* `fed` :
    %    the federate to make the request of
    %* `requestTime` :
    %    the next desired time
    %* `iterate` :
    %    the requested iteration mode
    %* `timeOut` :
    %    the granted time
    %* `outIterate` :
    %    the iteration specification of the result
    %
    %Returns
    %-------
    %a helics_status object with a return code of the result
    %
  [varargout{1:nargout}] = helicsMEX(94, varargin{:});
end
