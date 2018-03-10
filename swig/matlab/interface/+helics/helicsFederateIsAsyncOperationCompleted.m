function varargout = helicsFederateIsAsyncOperationCompleted(varargin)
    %check if the current Asynchronous operation has completed
    %
    %Parameters
    %----------
    %* `fed` :
    %    the federate to operate on
    %
    %Returns
    %-------
    %0 if not completed, 1 if completed
    %
  [varargout{1:nargout}] = helicsMEX(72, varargin{:});
end
