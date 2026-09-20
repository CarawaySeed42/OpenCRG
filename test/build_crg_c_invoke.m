%
%% ------------------------------------------------------------------------
% User Input
outdir                   = 'mex';
debug                    = true;
UseInterleavedComplexAPI = true;
verbose                  = false;
useAddCompilerFlags      = false;
compiler_flags           = '-std=c++17';
link_lib                 = true; % If true link crg lib, else compile source

this_filepath = mfilename('fullpath');
crg_baselib_dir = fullfile(fileparts(fileparts(fileparts(this_filepath))), 'OpenCRG\c-api\baselib');

%% -----------------------------------------------------------------------
fprintf('-------------------------------------------------------------\n');
crg_inc_dir = fullfile(crg_baselib_dir, 'inc');
crg_src_dir = fullfile(crg_baselib_dir, 'src');
crg_lib_dir = fullfile(crg_baselib_dir, 'lib');

% include folder without and with path separator
includeFolder = crg_inc_dir;
relIncPath    = [includeFolder filesep];

% Get crg lib source files
crg_src_files = dir(fullfile(crg_src_dir, '*.c'));
crg_src_files = fullfile({crg_src_files.folder}, {crg_src_files.name});

% The compiler flags
flags = {};

% Translate user settings to compiler options
if UseInterleavedComplexAPI
    if ~verLessThan('matlab','9.4')
        flags = cat(2, flags, '-R2018a');
    else
        disp(['Compiling without Interleaved Complex API due to ',...
              'Matlab Version being older than 9.4']);
    end
end

if debug
    flags = cat(2, flags, '-g'); %#ok<UNRCH>
end

if verbose
    flags = cat(2, flags, '-v'); %#ok<UNRCH>
end

includePath = sprintf('-I"%s"', includeFolder);
flags = cat(2, flags, includePath);

if useAddCompilerFlags
    flags = cat(2, flags, ['CXXFLAGS=$CXXFLAGS ' compiler_flags]); %#ok<UNRCH>
end

if link_lib
    flags = cat(2, fullfile(crg_lib_dir, 'OpenCRG.lib'), flags); %#ok<UNRCH>
else
    flags = cat(2, flags, crg_src_files); %#ok<UNRCH>
end

% Compile readFile
cflags = cat(2, 'crg_c_invoke.c', flags, '-outdir',  outdir);

% Print chosen options (string joining was introduced with Matlab 2013b)
fprintf(1, 'Compiler Input: ');
fprintf('%s ', cflags{:});
fprintf('\n');

% Compile File
mex(cflags{:})

addpath(fullfile(fileparts(mfilename('fullpath')), 'mex'));

fprintf('-------------------------------------------------------------\n');