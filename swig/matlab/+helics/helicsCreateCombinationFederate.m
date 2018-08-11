function varargout = helicsCreateCombinationFederate(varargin)
    %create a combination federate from a federate info object
    %
    %combination federates are both value federates and message federates, objects
    %can be used in all functions that take a helics_federate,
    %helics_message_federate or helics_federate object as an argument
    %
    %Parameters
    %----------
    %* `fi` :
    %    the federate info object that contains details on the federate
    %
    %Returns
    %-------
    %an opaque value federate object nullptr if the object creation failed
    %
  [varargout{1:nargout}] = helicsMEX(57, varargin{:});
end
