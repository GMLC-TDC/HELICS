function varargout = helicsQueryExecute(varargin)
    %Execute a query
    %
    %the call will block until the query finishes which may require communication or
    %other delays
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
    %a pointer to a string. the string will remain valid until the query is freed or
    %executed again the return will be nullptr if fed or query is an invalid object
    %
  [varargout{1:nargout}] = helicsMEX(112, varargin{:});
end
