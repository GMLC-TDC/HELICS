function  helicsPublicationPublish(varargin)
    pubdata=varargin{2};
   switch (pubdata)
       case {'char'}
           if (length(pubdata)==1)
               helicsPublicationPublishChar(varargin{:});
           else
               helicsPublicationPublishString(varargin{:});
           end
       case {'int8'}
           helicsPublicationPublishChar(varargin{:});
       case {'double'}
           if (isComplex(pubdata))
               if (numel(pubdata)>1)
                   helicsPublicationPublishVector(varargin{:});
               else
                   helicsPublicationPublishComplex(varargin{:});
               end
           else
               if (numel(pubdata)>1)
                   helicsPublicationPublishVector(varargin{:});
               else
                   helicsPublicationPublishDouble(varargin{:});
               end
           end
       case {'single'}
           if (isComplex(pubdata))
                if (numel(pubdata)>1)
                   helicsPublicationPublishVector(varargin{:});
               else
                   helicsPublicationPublishComplex(varargin{1},double(pubdata));
               end
           else
               if (numel(pubdata)>1)
                   helicsPublicationPublishVector(varargin{1},double(pubdata));
               else
                   helicsPublicationPublishDouble(varargin{1}, double(pubdata));
               end
           end
       case {'int32','uint64','uint32','int16','uint16'}
           helicsPublicationPublishInteger(varargin{1},int64(pubdata));
       case {'int64'}
           helicsPublicationPublishInteger(varargin{:});
       case {'logical'}
           if (pubdata)
               helicsPublicationPublishBoolean(varargin{1},uint32_t(1));
           else
               helicsPublicationPublishBoolean(varargin{1},uint32_t(0));
           end
   end
end