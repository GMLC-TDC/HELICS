function varargout = helicsFederateRegisterOptionalSubscription(varargin)
    %create a subscription that is specifically stated to be optional
    %
    %the subscription becomes part of the federate and is destroyed when the federate
    %is freed so there are no separate free functions for subscriptions and
    %publications
    %
    %optional implies that there may or may not be matching publication elsewhere in
    %the federation
    %
    %Parameters
    %----------
    %* `fed` :
    %    the federate object in which to create a subscription
    %* `key` :
    %    the identifier matching a publication to get a subscription for
    %* `type` :
    %    a string describing the expected type of the publication may be NULL
    %* `units` :
    %    a string listing the units of the subscription maybe NULL
    %
    %Returns
    %-------
    %an object containing the subscription
    %
  [varargout{1:nargout}] = helicsMEX(111, varargin{:});
end
