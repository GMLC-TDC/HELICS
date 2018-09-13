classdef message_t < SwigRef
    %Message_t mapped to a c compatible structure
    %
    %C++ includes: api-data.h
    %
  methods
    function this = swig_this(self)
      this = helicsMEX(3, self);
    end
    function varargout = time(self, varargin)
      narginchk(1, 2)
      if nargin==1
        nargoutchk(0, 1)
        varargout{1} = helicsMEX(16, self);
      else
        nargoutchk(0, 0)
        helicsMEX(17, self, varargin{1});
      end
    end
    function varargout = data(self, varargin)
      narginchk(1, 2)
      if nargin==1
        nargoutchk(0, 1)
        varargout{1} = helicsMEX(18, self);
      else
        nargoutchk(0, 0)
        helicsMEX(19, self, varargin{1});
      end
    end
    function varargout = length(self, varargin)
      narginchk(1, 2)
      if nargin==1
        nargoutchk(0, 1)
        varargout{1} = helicsMEX(20, self);
      else
        nargoutchk(0, 0)
        helicsMEX(21, self, varargin{1});
      end
    end
    function varargout = messageID(self, varargin)
      narginchk(1, 2)
      if nargin==1
        nargoutchk(0, 1)
        varargout{1} = helicsMEX(22, self);
      else
        nargoutchk(0, 0)
        helicsMEX(23, self, varargin{1});
      end
    end
    function varargout = flags(self, varargin)
      narginchk(1, 2)
      if nargin==1
        nargoutchk(0, 1)
        varargout{1} = helicsMEX(24, self);
      else
        nargoutchk(0, 0)
        helicsMEX(25, self, varargin{1});
      end
    end
    function varargout = original_source(self, varargin)
      narginchk(1, 2)
      if nargin==1
        nargoutchk(0, 1)
        varargout{1} = helicsMEX(26, self);
      else
        nargoutchk(0, 0)
        helicsMEX(27, self, varargin{1});
      end
    end
    function varargout = source(self, varargin)
      narginchk(1, 2)
      if nargin==1
        nargoutchk(0, 1)
        varargout{1} = helicsMEX(28, self);
      else
        nargoutchk(0, 0)
        helicsMEX(29, self, varargin{1});
      end
    end
    function varargout = dest(self, varargin)
      narginchk(1, 2)
      if nargin==1
        nargoutchk(0, 1)
        varargout{1} = helicsMEX(30, self);
      else
        nargoutchk(0, 0)
        helicsMEX(31, self, varargin{1});
      end
    end
    function varargout = original_dest(self, varargin)
      narginchk(1, 2)
      if nargin==1
        nargoutchk(0, 1)
        varargout{1} = helicsMEX(32, self);
      else
        nargoutchk(0, 0)
        helicsMEX(33, self, varargin{1});
      end
    end
    function self = message_t(varargin)
      if nargin==1 && strcmp(class(varargin{1}),'SwigRef')
        if ~isnull(varargin{1})
          self.swigPtr = varargin{1}.swigPtr;
        end
      else
        tmp = helicsMEX(34, varargin{:});
        self.swigPtr = tmp.swigPtr;
        tmp.swigPtr = [];
      end
    end
    function delete(self)
      if self.swigPtr
        helicsMEX(35, self);
        self.swigPtr=[];
      end
    end
  end
  methods(Static)
  end
end
