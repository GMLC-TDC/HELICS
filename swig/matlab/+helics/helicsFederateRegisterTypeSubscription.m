function varargout = helicsFederateRegisterTypeSubscription(varargin)
    %create a subscription of a specific known type
    %
    %the subscription becomes part of the federate and is destroyed when the federate
    %is freed so there are no separate free functions for subscriptions and
    %publications
    %
    %Parameters
    %----------
    %* `fed` :
    %    the federate object in which to create a subscription
    %* `key` :
    %    the identifier matching a publication to get a subscription for
    %* `type` :
    %    a known type identifier HELICS_STRING_TYPE, HELICS_INT_TYPE,
    %    HELICS_DOUBLE_TYPE, HELICS_COMPLEX_TYPE, HELICS_VECTOR_TYPE, HELICS_RAW_TYPE
    %* `units` :
    %    a string listing the units of the subscription maybe NULL
    %
    %Returns
    %-------
    %an object containing the subscription
    %
  [varargout{1:nargout}] = helicsMEX(101, varargin{:});
end
