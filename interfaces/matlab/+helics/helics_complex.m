classdef helics_complex < SwigRef
  methods
    function this = swig_this(self)
      this = helicsMEX(3, self);
    end
    function varargout = real(self, varargin)
      narginchk(1, 2)
      if nargin==1
        nargoutchk(0, 1)
        varargout{1} = helicsMEX(16, self);
      else
        nargoutchk(0, 0)
        helicsMEX(17, self, varargin{1});
      end
    end
    function varargout = imag(self, varargin)
      narginchk(1, 2)
      if nargin==1
        nargoutchk(0, 1)
        varargout{1} = helicsMEX(18, self);
      else
        nargoutchk(0, 0)
        helicsMEX(19, self, varargin{1});
      end
    end
    function self = helics_complex(varargin)
      if nargin==1 && strcmp(class(varargin{1}),'SwigRef')
        if ~isnull(varargin{1})
          self.swigPtr = varargin{1}.swigPtr;
        end
      else
        tmp = helicsMEX(20, varargin{:});
        self.swigPtr = tmp.swigPtr;
        tmp.swigPtr = [];
      end
    end
    function delete(self)
      if self.swigPtr
        helicsMEX(21, self);
        self.swigPtr=[];
      end
    end
  end
  methods(Static)
  end
end
