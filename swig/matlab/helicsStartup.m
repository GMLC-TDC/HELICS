display('Loading HELICS ...')
directory = split(mfilename('fullpath'), '/');
directory = strjoin({directory{1:end-1}}, '/');

listing = dir([directory, '/', '*helicsSharedLib.*']);

libraryname = '';

for i=1:numel(listing)
    if ( endsWith(listing(i).name, '.h') == 0 )
        libraryname = listing(i).name;
    end
end

if length(libraryname) == 0
    display('Unable to find library for helics')
end

loadlibrary(GetFullPath([directory, '/', libraryname]));
