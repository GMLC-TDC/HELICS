classdef data_t < SwigRef
    %Data to be communicated.
    %
    %Core operates on opaque byte buffers.
    %
    %C++ includes: api-data.h
    %
  methods
    function this = swig_this(self)
      this = helicsMEX(3, self);
    end
    function varargout = data(self, varargin)
      narginchk(1, 2)
      if nargin==1
        nargoutchk(0, 1)
        varargout{1} = helicsMEX(7, self);
      else
        nargoutchk(0, 0)
        helicsMEX(8, self, varargin{1});
      end
    end
    function varargout = length(self, varargin)
      narginchk(1, 2)
      if nargin==1
        nargoutchk(0, 1)
        varargout{1} = helicsMEX(9, self);
      else
        nargoutchk(0, 0)
        helicsMEX(10, self, varargin{1});
      end
    end
    function self = data_t(varargin)
      if nargin==1 && strcmp(class(varargin{1}),'SwigRef')
        if ~isnull(varargin{1})
          self.swigPtr = varargin{1}.swigPtr;
        end
      else
        tmp = helicsMEX(11, varargin{:});
        self.swigPtr = tmp.swigPtr;
        tmp.swigPtr = [];
      end
    end
    function delete(self)
      if self.swigPtr
        helicsMEX(12, self);
        self.swigPtr=[];
      end
    end
  end
  methods(Static)
  end
end
