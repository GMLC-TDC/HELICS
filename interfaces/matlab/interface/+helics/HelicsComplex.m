classdef HelicsComplex < SwigRef
  methods
    function this = swig_this(self)
      this = helicsMEX(3, self);
    end
    function varargout = real(self, varargin)
      narginchk(1, 2)
      if nargin==1
        nargoutchk(0, 1)
        varargout{1} = helicsMEX(13, self);
      else
        nargoutchk(0, 0)
        helicsMEX(14, self, varargin{1});
      end
    end
    function varargout = imag(self, varargin)
      narginchk(1, 2)
      if nargin==1
        nargoutchk(0, 1)
        varargout{1} = helicsMEX(15, self);
      else
        nargoutchk(0, 0)
        helicsMEX(16, self, varargin{1});
      end
    end
    function self = HelicsComplex(varargin)
      if nargin==1 && strcmp(class(varargin{1}),'SwigRef')
        if ~isnull(varargin{1})
          self.swigPtr = varargin{1}.swigPtr;
        end
      else
        tmp = helicsMEX(17, varargin{:});
        self.swigPtr = tmp.swigPtr;
        tmp.SwigClear();
      end
    end
    function delete(self)
      if self.swigPtr
        helicsMEX(18, self);
        self.SwigClear();
      end
    end
  end
  methods(Static)
  end
end
