function varargout = helicsFederateSetLoggingLevel(varargin)
    %set the logging level for the federate @ details debug and trace only do
    %anything if they were enabled in the compilation
    %
    %Parameters
    %----------
    %* `loggingLevel` :
    %    (-1: none, 0: error_only, 1: warnings, 2: normal, 3: debug, 4: trace)
    %
  [varargout{1:nargout}] = helicsMEX(94, varargin{:});
end
