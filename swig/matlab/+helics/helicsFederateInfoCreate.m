function varargout = helicsFederateInfoCreate(varargin)
    %create a federate info object for specifying federate information when
    %constructing a federate
    %
    %Returns
    %-------
    %a helics_federate_info_t object which is a reference to the created object
    %
  [varargout{1:nargout}] = helicsMEX(51, varargin{:});
end
