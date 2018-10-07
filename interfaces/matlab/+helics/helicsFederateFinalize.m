function varargout = helicsFederateFinalize(varargin)
    %finalize the federate this halts all communication in the federate and
    %disconnects it from the core
    %
  [varargout{1:nargout}] = helicsMEX(82, varargin{:});
end
