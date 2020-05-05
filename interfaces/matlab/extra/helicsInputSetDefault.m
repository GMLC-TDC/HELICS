function  helicsInputSetDefault(varargin)
    pubdata=varargin{2};
    import helics.*
   switch (class(pubdata))
       case {'char'}
           if (length(pubdata)==1)
               helicsInputSetDefaultChar(varargin{:});
           else
               helicsInputSetDefaultString(varargin{:});
           end
       case {'int8'}
           helicsInputSetDefaultChar(varargin{:});
       case {'double'}
           if (isreal(pubdata))
               if (numel(pubdata)>1)
                   helicsInputSetDefaultVector(varargin{:});
               else
                   helicsInputSetDefaultDouble(varargin{:});
               end
           else
               if (numel(pubdata)>1)
                   helicsInputSetDefaultVector(varargin{:});
               else
                   helicsInputSetDefaultComplex(varargin{:});
               end
           end
       case {'single'}
           if (isreal(pubdata))
                if (numel(pubdata)>1)
                   helicsInputSetDefaultVector(varargin{1},double(pubdata));
               else
                   helicsInputSetDefaultDouble(varargin{1}, double(pubdata));
               end
           else
               if (numel(pubdata)>1)
                   helicsInputSetDefaultVector(varargin{:});
               else
                   helicsInputSetDefaultComplex(varargin{1},double(pubdata));
               end
           end
       case {'int32','uint64','uint32','int16','uint16'}
           helicsInputSetDefaultInteger(varargin{1},int64(pubdata));
       case {'int64'}
           helicsInputSetDefaultInteger(varargin{:});
       case {'logical'}
           if (pubdata)
               helicsInputSetDefaultBoolean(varargin{1},int32(1));
           else
               helicsInputSetDefaultBoolean(varargin{1},int32(0));
           end
   end
end
