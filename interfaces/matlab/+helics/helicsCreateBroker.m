function varargout = helicsCreateBroker(varargin)
    %create a broker object
    %
    %Parameters
    %----------
    %* `type` :
    %    the type of the broker to create
    %* `name` :
    %    the name of the broker , may be a nullptr or empty string to have a name
    %    automatically assigned
    %* `initString` :
    %    an initialization string to send to the core-the format is similar to
    %    command line arguments typical options include a broker address
    %    --broker="XSSAF" if this is a subbroker or the number of federates or the
    %    address
    %
    %Returns
    %-------
    %a helics_core object
    %
  [varargout{1:nargout}] = helicsMEX(37, varargin{:});
end
