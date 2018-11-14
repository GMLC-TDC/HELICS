function varargout = helicsFederateGetMessage(varargin)
    %receive a communication message for any endpoint in the federate
    %
    %the return order will be in order of endpoint creation then order of arrival all
    %messages for the first endpoint, then all for the second, and so on
    %
    %Returns
    %-------
    %a unique_ptr to a Message object containing the message data
    %
  [varargout{1:nargout}] = helicsMEX(199, varargin{:});
end
