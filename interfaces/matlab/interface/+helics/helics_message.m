classdef helics_message < SwigRef
  methods
    function this = swig_this(self)
      this = helicsMEX(3, self);
    end
    function varargout = time(self, varargin)
      narginchk(1, 2)
      if nargin==1
        nargoutchk(0, 1)
        varargout{1} = helicsMEX(11, self);
      else
        nargoutchk(0, 0)
        helicsMEX(12, self, varargin{1});
      end
    end
    function varargout = data(self, varargin)
      narginchk(1, 2)
      if nargin==1
        nargoutchk(0, 1)
        varargout{1} = helicsMEX(13, self);
      else
        nargoutchk(0, 0)
        helicsMEX(14, self, varargin{1});
      end
    end
    function varargout = length(self, varargin)
      narginchk(1, 2)
      if nargin==1
        nargoutchk(0, 1)
        varargout{1} = helicsMEX(15, self);
      else
        nargoutchk(0, 0)
        helicsMEX(16, self, varargin{1});
      end
    end
    function varargout = messageID(self, varargin)
      narginchk(1, 2)
      if nargin==1
        nargoutchk(0, 1)
        varargout{1} = helicsMEX(17, self);
      else
        nargoutchk(0, 0)
        helicsMEX(18, self, varargin{1});
      end
    end
    function varargout = flags(self, varargin)
      narginchk(1, 2)
      if nargin==1
        nargoutchk(0, 1)
        varargout{1} = helicsMEX(19, self);
      else
        nargoutchk(0, 0)
        helicsMEX(20, self, varargin{1});
      end
    end
    function varargout = original_source(self, varargin)
      narginchk(1, 2)
      if nargin==1
        nargoutchk(0, 1)
        varargout{1} = helicsMEX(21, self);
      else
        nargoutchk(0, 0)
        helicsMEX(22, self, varargin{1});
      end
    end
    function varargout = source(self, varargin)
      narginchk(1, 2)
      if nargin==1
        nargoutchk(0, 1)
        varargout{1} = helicsMEX(23, self);
      else
        nargoutchk(0, 0)
        helicsMEX(24, self, varargin{1});
      end
    end
    function varargout = dest(self, varargin)
      narginchk(1, 2)
      if nargin==1
        nargoutchk(0, 1)
        varargout{1} = helicsMEX(25, self);
      else
        nargoutchk(0, 0)
        helicsMEX(26, self, varargin{1});
      end
    end
    function varargout = original_dest(self, varargin)
      narginchk(1, 2)
      if nargin==1
        nargoutchk(0, 1)
        varargout{1} = helicsMEX(27, self);
      else
        nargoutchk(0, 0)
        helicsMEX(28, self, varargin{1});
      end
    end
    function self = helics_message(varargin)
      if nargin==1 && strcmp(class(varargin{1}),'SwigRef')
        if ~isnull(varargin{1})
          self.swigPtr = varargin{1}.swigPtr;
        end
      else
        tmp = helicsMEX(29, varargin{:});
        self.swigPtr = tmp.swigPtr;
        tmp.SwigClear();
      end
    end
    function delete(self)
      if self.swigPtr
        helicsMEX(30, self);
        self.SwigClear();
      end
    end
  end
  methods(Static)
  end
end
