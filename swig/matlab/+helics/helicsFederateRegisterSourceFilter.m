function varargout = helicsFederateRegisterSourceFilter(varargin)
    %create a source Filter on the specified federate
    %
    %filters can be created through a federate or a core , linking through a federate
    %allows a few extra features of name matching to function on the federate
    %interface but otherwise equivalent behavior
    %
    %Parameters
    %----------
    %* `fed` :
    %    the fed to register through
    %* `name` :
    %    the name of the filter (can be nullptr)
    %* `inputType` :
    %    the input type of the filter, used for ordering (can be nullptr)
    %* `outputType` :
    %    the output type of the filter, used for ordering (can be nullptr)
    %
    %Returns
    %-------
    %a helics_source_filter object
    %
  [varargout{1:nargout}] = helicsMEX(154, varargin{:});
end
