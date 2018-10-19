function varargout = helicsEndpointSubscribe(varargin)
    %subscribe an endpoint to a publication
    %
    %Parameters
    %----------
    %* `endpoint` :
    %    the endpoint to use
    %* `key` :
    %    the name of the publication
    %* `type` :
    %    the type of the publication that is expected (nullptr or "" for DON'T
    %    CARE)
    %
  [varargout{1:nargout}] = helicsMEX(190, varargin{:});
end
