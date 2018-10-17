function varargout = helicsQueryExecuteAsync(varargin)
    %Execute a query in a non-blocking call
    %
    %Parameters
    %----------
    %* `query` :
    %    the query object to use in the query
    %* `fed` :
    %    a federate to send the query through
    %
    %Returns
    %-------
    %a helics status enumeration with the result of the query specification
    %
  [varargout{1:nargout}] = helicsMEX(115, varargin{:});
end
