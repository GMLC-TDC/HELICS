function varargout = helicsFederateEnterInitializationModeAsync(varargin)
    %non blocking alternative to  the function
    %helicsFederateEnterInitializationModeFinalize must be called to finish the
    %operation
    %
  [varargout{1:nargout}] = helicsMEX(68, varargin{:});
end
