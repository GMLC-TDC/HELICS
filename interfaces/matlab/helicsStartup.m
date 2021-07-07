function success = helicsStartup(helicsLibPath)
% HELICSSTARTUP configures MATLAB for HELICS use

if nargin < 1
    %If not specified use our location, which should have required lib once
    %compiled/installed
    helicsLibPath = fileparts(mfilename('fullpath'));
end

fprintf('Loading HELICS (from %s)...\n', helicsLibPath)

%% Extract HELICS library name in a cross-platform way
listing = dir(fullfile(helicsLibPath, '*helics.*'));

libraryName = '';

%TODO: vectorize (make MATLAB-esque)
for i=1:numel(listing)
    [~, ~, ext]=fileparts(listing(i).name);
    if isequal(ext, '.h')
        continue;
    end
    if isequal(ext, '.lib')
        continue;
    end
     if isequal(ext, '.a')
        continue;
    end
    libraryName = listing(i).name;
end

%% Try some backups if not found
if isempty(libraryName)
    %if we are empty try for a debug version
    listing = dir(fullfile(helicsLibPath, '*helics.*'));

    for i=1:numel(listing)
        [~, ~, ext]=fileparts(listing(i).name);
        if isequal(ext, '.h')
            continue;
        end
        if isequal(ext, '.lib')
            continue;
        end
        if isequal(ext, '.a')
            continue;
        end
        libraryName = listing(i).name;
    end

end

if (~isempty(libraryName))
    [~,name]=fileparts(libraryName);
    if ~libisloaded(name)
        loadlibrary(GetFullPath(fullfile(helicsLibPath, libraryName)));
    end
else
    disp('Unable to find library for HELICS')
end

if nargout >1
    success = ~isempty(libraryName);
end
