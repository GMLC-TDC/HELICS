function varargout = helicsEndpointGetMessage(varargin)
    %receive a packet from a particular endpoint
    %
    %Parameters
    %----------
    %* `endpoint` :
    %    the identifier for the endpoint
    %
    %Returns
    %-------
    %a message object
    %
  [varargout{1:nargout}] = helicsMEX(148, varargin{:});
end
