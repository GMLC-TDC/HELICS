function varargout = helicsFederateRegisterSubscription(varargin)
    %create a subscription
    %
    %the subscription becomes part of the federate and is destroyed when the federate
    %is freed so there are no separate free functions for subscriptions and
    %publications
    %
    %Parameters
    %----------
    %* `fed` :
    %    the federate object in which to create a subscription must have been create
    %    with helicsCreateValueFederate or helicsCreateCombinationFederate
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
  [varargout{1:nargout}] = helicsMEX(126, varargin{:});
end
