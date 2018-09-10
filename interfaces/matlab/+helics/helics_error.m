classdef helics_error < SwigRef
  methods
    function this = swig_this(self)
      this = helicsMEX(3, self);
    end
    function varargout = error_code(self, varargin)
      narginchk(1, 2)
      if nargin==1
        nargoutchk(0, 1)
        varargout{1} = helicsMEX(38, self);
      else
        nargoutchk(0, 0)
        helicsMEX(39, self, varargin{1});
      end
    end
    function varargout = message(self, varargin)
      narginchk(1, 2)
      if nargin==1
        nargoutchk(0, 1)
        varargout{1} = helicsMEX(40, self);
      else
        nargoutchk(0, 0)
        helicsMEX(41, self, varargin{1});
      end
    end
    function self = helics_error(varargin)
      if nargin==1 && strcmp(class(varargin{1}),'SwigRef')
        if ~isnull(varargin{1})
          self.swigPtr = varargin{1}.swigPtr;
        end
      else
        tmp = helicsMEX(42, varargin{:});
        self.swigPtr = tmp.swigPtr;
        tmp.swigPtr = [];
      end
    end
    function delete(self)
      if self.swigPtr
        helicsMEX(43, self);
        self.swigPtr=[];
      end
    end
  end
  methods(Static)
  end
end
