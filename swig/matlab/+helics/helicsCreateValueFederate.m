function varargout = helicsCreateValueFederate(varargin)
    %create a value federate from a federate info object
    %
    %helics_federate objects can be used in all functions that take a helics_federate
    %or helics_federate object as an argument
    %
    %Parameters
    %----------
    %* `fi` :
    %    the federate info object that contains details on the federate
    %
    %Returns
    %-------
    %an opaque value federate object
    %
  [varargout{1:nargout}] = helicsMEX(53, varargin{:});
end
