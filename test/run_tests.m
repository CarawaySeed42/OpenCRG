% Script used to run tests

IS_CI = ~isempty(getenv('CI'));

if IS_CI
   disp('script is running during CI'); 
end

mfiledir = fileparts(mfilename('fullpath'));
addpath(fullfile(mfiledir, 'MOxUnit', 'MOxUnit'));
moxunit_set_path();
addpath(fullfile(fileparts(mfiledir), 'matlab'));
crg_init();

% addpath(fullfile(pwd, 'MOcov', 'MOcov'));
all_passed = moxunit_runtests('tests', '-verbose') ;%-with_coverage -cover MOxUnit -cover_xml_file coverage.xml;

if IS_CI
    exit(double(~all_passed));
end