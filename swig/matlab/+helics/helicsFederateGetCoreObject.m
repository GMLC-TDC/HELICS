function varargout = helicsFederateGetCoreObject(varargin)
    %get the core object associated with a federate
    %
    %Parameters
    %----------
    %* `fed` :
    %    a federate object
    %
    %Returns
    %-------
    %a core object, nullptr if invalid
    %
  [varargout{1:nargout}] = helicsMEX(92, varargin{:});
end
