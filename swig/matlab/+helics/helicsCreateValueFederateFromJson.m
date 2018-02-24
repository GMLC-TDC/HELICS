function varargout = helicsCreateValueFederateFromJson(varargin)
    %create a value federate from a JSON file or JSON string
    %
    %helics_federate objects can be used in all functions that take a helics_federate
    %or helics_federate object as an argument
    %
    %Parameters
    %----------
    %* `JSON` :
    %    a JSON file or a JSON string that contains setup and configuration
    %    information
    %
    %Returns
    %-------
    %an opaque value federate object
    %
  [varargout{1:nargout}] = helicsMEX(46, varargin{:});
end
