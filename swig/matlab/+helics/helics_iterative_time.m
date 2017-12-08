classdef helics_iterative_time < SwigRef
  methods
    function this = swig_this(self)
      this = helicsMEX(3, self);
    end
    function varargout = time(self, varargin)
      narginchk(1, 2)
      if nargin==1
        nargoutchk(0, 1)
        varargout{1} = helicsMEX(5, self);
      else
        nargoutchk(0, 0)
        helicsMEX(6, self, varargin{1});
      end
    end
    function varargout = status(self, varargin)
      narginchk(1, 2)
      if nargin==1
        nargoutchk(0, 1)
        varargout{1} = helicsMEX(7, self);
      else
        nargoutchk(0, 0)
        helicsMEX(8, self, varargin{1});
      end
    end
    function self = helics_iterative_time(varargin)
      if nargin==1 && strcmp(class(varargin{1}),'SwigRef')
        if ~isnull(varargin{1})
          self.swigPtr = varargin{1}.swigPtr;
        end
      else
        tmp = helicsMEX(9, varargin{:});
        self.swigPtr = tmp.swigPtr;
        tmp.swigPtr = [];
      end
    end
    function delete(self)
      if self.swigPtr
        helicsMEX(10, self);
        self.swigPtr=[];
      end
    end
  end
  methods(Static)
  end
end
