function varargout = helicsFederateRegisterOptionalTypeSubscription(varargin)
    %create a subscription of a specific known type that is specifically stated to be
    %optional
    %
    %the subscription becomes part of the federate and is destroyed when the federate
    %is freed so there are no separate free functions for subscriptions and
    %publications optional implies that there may or may not be matching publication
    %elsewhere in the federation
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
  [varargout{1:nargout}] = helicsMEX(102, varargin{:});
end
