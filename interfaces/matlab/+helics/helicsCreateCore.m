function varargout = helicsCreateCore(varargin)
    %create a core object
    %
    %Parameters
    %----------
    %* `type` :
    %    the type of the core to create
    %* `name` :
    %    the name of the core , may be a nullptr or empty string to have a name
    %    automatically assigned
    %* `initString` :
    %    an initialization string to send to the core-the format is similar to
    %    command line arguments typical options include a broker address
    %    --broker="XSSAF" or the number of federates or the address
    %
    %Returns
    %-------
    %a helics_core object
    %
  [varargout{1:nargout}] = helicsMEX(38, varargin{:});
end
