function varargout = helicsQueryIsCompleted(varargin)
    %check if an asynchronously executed query has completed
    %
    %Returns
    %-------
    %will return helics_true if an async query has complete or a regular query call
    %was made with a result and false if an async query has not completed or is
    %invalid
    %
  [varargout{1:nargout}] = helicsMEX(114, varargin{:});
end
