function varargout = helicsQueryExecuteComplete(varargin)
    %complete the return from a query called with /ref helicsExecuteQueryAsync
    %
    %the function will block until the query completes /ref isQueryComplete can be
    %called to determine if a query has completed or not
    %
    %Parameters
    %----------
    %* `query` :
    %    the query object to
    %
    %Returns
    %-------
    %a pointer to a string. the string will remain valid until the query is freed or
    %executed again the return will be nullptr if query is an invalid object
    %
  [varargout{1:nargout}] = helicsMEX(96, varargin{:});
end
