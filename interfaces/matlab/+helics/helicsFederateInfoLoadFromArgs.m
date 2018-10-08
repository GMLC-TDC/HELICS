function varargout = helicsFederateInfoLoadFromArgs(varargin)
    %load a federate info from command line arguments
    %
    %Parameters
    %----------
    %* `fi` :
    %    a federateInfo object
    %* `argc` :
    %    the number of command line arguments
    %* `argv` :
    %    an array of strings from the command line
    %
    %Returns
    %-------
    %a helics_status enumeration indicating success or any potential errors
    %
  [varargout{1:nargout}] = helicsMEX(75, varargin{:});
end
