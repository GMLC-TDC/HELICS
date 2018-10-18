function varargout = helicsSubscriptionGetVector(varargin)
    %get a vector from a subscription
    %
    %Parameters
    %----------
    %* `sub` :
    %    the subscription to get the result for
    %* `data` :
    %    the location to store the data
    %* `maxlen` :
    %    the maximum size of the vector
    %* `actualSize` :
    %    pointer to variable to store the actual size
    %
  [varargout{1:nargout}] = helicsMEX(149, varargin{:});
end
