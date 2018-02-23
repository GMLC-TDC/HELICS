function varargout = helicsFederateEnterInitializationMode(varargin)
    %enter the initialization state of a federate
    %
    %the initialization state allows initial values to be set and received if the
    %iteration is requested on entry to the execution state This is a blocking call
    %and will block until the core allows it to proceed
    %
  [varargout{1:nargout}] = helicsMEX(68, varargin{:});
end
