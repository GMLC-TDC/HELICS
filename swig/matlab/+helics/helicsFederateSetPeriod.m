function varargout = helicsFederateSetPeriod(varargin)
    %set the period and offset of the federate
    %
    %the federate will on grant time on N*period+offset interval
    %
    %Parameters
    %----------
    %* `period` :
    %    the length of time between each subsequent grants
    %* `offset` :
    %    the shift of the period from 0 offset must be < period
    %
  [varargout{1:nargout}] = helicsMEX(105, varargin{:});
end
