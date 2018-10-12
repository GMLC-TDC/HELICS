function varargout = helicsCreateBrokerFromArgs(varargin)
    %create a core object by passing command line arguments
    %
    %Parameters
    %----------
    %* `type` :
    %    the type of the core to create
    %* `name` :
    %    the name of the core , may be a nullptr or empty string to have a name
    %    automatically assigned
    %* `argc` :
    %    the number of arguments
    %* `argv` :
    %    the string values from a command line
    %
    %Returns
    %-------
    %a helics_core object
    %
  [varargout{1:nargout}] = helicsMEX(44, varargin{:});
end
