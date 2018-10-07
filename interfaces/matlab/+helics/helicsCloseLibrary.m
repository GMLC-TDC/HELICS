function varargout = helicsCloseLibrary(varargin)
    %call when done using the helics library, this function will ensure the threads
    %are closed properly if possible this should be the last call before exiting,
    %
  [varargout{1:nargout}] = helicsMEX(84, varargin{:});
end
