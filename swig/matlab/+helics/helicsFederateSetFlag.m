function varargout = helicsFederateSetFlag(varargin)
    %set a flag for the federate
    %
    %Parameters
    %----------
    %* `fed` :
    %    the federate to alter a flag for
    %* `flag` :
    %    the flag to change
    %* `flagValue` :
    %    the new value of the flag 0 for false !=0 for true
    %
  [varargout{1:nargout}] = helicsMEX(99, varargin{:});
end
