function varargout = helicsCreateCombinationFederateFromJson(varargin)
    %create a combination federate from a JSON file or JSON string
    %
    %combination federates are both value federates and message federates, objects
    %can be used in all functions that take a helics_federate,
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
    %an opaque combination federate object
    %
  [varargout{1:nargout}] = helicsMEX(59, varargin{:});
end
