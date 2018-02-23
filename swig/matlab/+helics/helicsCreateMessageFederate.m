function varargout = helicsCreateMessageFederate(varargin)
    %create a message federate from a federate info object
    %
    %helics_message_federate objects can be used in all functions that take a
    %helics_message_federate or helics_federate object as an argument
    %
    %Parameters
    %----------
    %* `fi` :
    %    the federate info object that contains details on the federate
    %
    %Returns
    %-------
    %an opaque message federate object
    %
  [varargout{1:nargout}] = helicsMEX(45, varargin{:});
end
