function varargout = helicsFederateSetOutputDelay(varargin)
    %set the look ahead time
    %
    %the look ahead is the propagation time for messages/event to propagate from the
    %Federate the federate
    %
    %Parameters
    %----------
    %* `lookAhead` :
    %    the look ahead time
    %
  [varargout{1:nargout}] = helicsMEX(96, varargin{:});
end
