function varargout = helicsFederateInfoSetCoreTypeFromString(varargin)
    %set the core type from a string
    %
    %Parameters
    %----------
    %* `fi` :
    %    the federate info object to alter
    %* `coretype` :
    %    a string naming a core type
    %
    %Returns
    %-------
    %a helics_status enumeration helics_ok on success helicsInvalidReference if fi is
    %not a valid reference helics_discard if the string is not recognized
    %
  [varargout{1:nargout}] = helicsMEX(55, varargin{:});
end
