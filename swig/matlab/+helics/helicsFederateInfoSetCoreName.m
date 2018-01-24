function varargout = helicsFederateInfoSetCoreName(varargin)
    %set the name of the core to link to for a federate
    %
    %Parameters
    %----------
    %* `fi` :
    %    the federate info object to alter
    %* `corename` :
    %    the identifier for a core to link to
    %
    %Returns
    %-------
    %a helics_status enumeration helics_ok on success helicsInvalidReference if fi is
    %not a valid reference
    %
  [varargout{1:nargout}] = helicsMEX(52, varargin{:});
end
