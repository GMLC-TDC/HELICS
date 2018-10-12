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
        varargout{1} = helicsMEX(17, self);
      else
        nargoutchk(0, 0)
        helicsMEX(18, self, varargin{1});
      end
    end
    function varargout = data(self, varargin)
      narginchk(1, 2)
      if nargin==1
        nargoutchk(0, 1)
        varargout{1} = helicsMEX(19, self);
      else
        nargoutchk(0, 0)
        helicsMEX(20, self, varargin{1});
      end
    end
    function varargout = length(self, varargin)
      narginchk(1, 2)
      if nargin==1
        nargoutchk(0, 1)
        varargout{1} = helicsMEX(21, self);
      else
        nargoutchk(0, 0)
        helicsMEX(22, self, varargin{1});
      end
    end
    function varargout = messageID(self, varargin)
      narginchk(1, 2)
      if nargin==1
        nargoutchk(0, 1)
        varargout{1} = helicsMEX(23, self);
      else
        nargoutchk(0, 0)
        helicsMEX(24, self, varargin{1});
      end
    end
    function varargout = flags(self, varargin)
      narginchk(1, 2)
      if nargin==1
        nargoutchk(0, 1)
        varargout{1} = helicsMEX(25, self);
      else
        nargoutchk(0, 0)
        helicsMEX(26, self, varargin{1});
      end
    end
    function varargout = original_source(self, varargin)
      narginchk(1, 2)
      if nargin==1
        nargoutchk(0, 1)
        varargout{1} = helicsMEX(27, self);
      else
        nargoutchk(0, 0)
        helicsMEX(28, self, varargin{1});
      end
    end
    function varargout = source(self, varargin)
      narginchk(1, 2)
      if nargin==1
        nargoutchk(0, 1)
        varargout{1} = helicsMEX(29, self);
      else
        nargoutchk(0, 0)
        helicsMEX(30, self, varargin{1});
      end
    end
    function varargout = dest(self, varargin)
      narginchk(1, 2)
      if nargin==1
        nargoutchk(0, 1)
        varargout{1} = helicsMEX(31, self);
      else
        nargoutchk(0, 0)
        helicsMEX(32, self, varargin{1});
      end
    end
    function varargout = original_dest(self, varargin)
      narginchk(1, 2)
      if nargin==1
        nargoutchk(0, 1)
        varargout{1} = helicsMEX(33, self);
      else
        nargoutchk(0, 0)
        helicsMEX(34, self, varargin{1});
      end
    end
    function self = message_t(varargin)
      if nargin==1 && strcmp(class(varargin{1}),'SwigRef')
        if ~isnull(varargin{1})
          self.swigPtr = varargin{1}.swigPtr;
        end
      else
        tmp = helicsMEX(35, varargin{:});
        self.swigPtr = tmp.swigPtr;
        tmp.swigPtr = [];
      end
    end
    function delete(self)
      if self.swigPtr
        helicsMEX(36, self);
        self.swigPtr=[];
      end
    end
  end
  methods(Static)
  end
end
