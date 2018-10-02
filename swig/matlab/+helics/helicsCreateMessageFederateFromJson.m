function varargout = helicsCreateMessageFederateFromJson(varargin)
    %create a message federate from a JSON file or JSON string
    %
    %helics_message_federate objects can be used in all functions that take a
    %helics_message_federate or helics_federate object as an argument
    %
    %Parameters
    %----------
    %* `JSON` :
    %    a JSON file or a JSON string that contains setup and configuration
    %    information
    %
    %Returns
    %-------
    %an opaque message federate object
    %
  [varargout{1:nargout}] = helicsMEX(57, varargin{:});
end
