disp('Loading HELICS ...')
if (exists('helicsLibPath','var')==1)
    directory=helicsLibPath;
else
    directory = fileparts(mfilename('fullpath'));
end


listing = dir(fullfile(directory, '*helicsSharedLib.*'));

libraryName = '';

for i=1:numel(listing)
    if ( endsWith(listing(i).name, '.h') == 1 )
        continue;
    end
    if ( endsWith(listing(i).name, '.lib') == 1 )
        continue;
    end
    librarynName = listing(i).name;
end

if isempty(librarynName)
    %if we are empty try for a debug version
    listing = dir(fullfile(directory, '*helicsSharedLibd.*'));
    
    for i=1:numel(listing)
        if ( endsWith(listing(i).name, '.h') == 1 )
            continue;
        end
        if ( endsWith(listing(i).name, '.lib') == 1 )
            continue;
        end
        librarynName = listing(i).name;
    end
    
end

if (~isempty(libraryName))
    loadlibrary(GetFullPath(fullfile(directory, librarynName)));
else
    disp('Unable to find library for helics')
end
