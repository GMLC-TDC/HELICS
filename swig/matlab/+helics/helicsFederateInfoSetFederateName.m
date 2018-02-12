function varargout = helicsFederateInfoSetFederateName(varargin)
    %set the federate name in the Federate Info structure
    %
    %Parameters
    %----------
    %* `fi` :
    %    the federate info object to alter
    %* `name` :
    %    the new identifier for the federate
    %
    %Returns
    %-------
    %a helics_status enumeration helics_ok on success
    %
  [varargout{1:nargout}] = helicsMEX(51, varargin{:});
end
