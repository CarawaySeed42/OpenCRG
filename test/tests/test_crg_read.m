function test_suite = test_crg_read()
try % assignment of 'localfunctions' is necessary in Matlab >= 2016
    test_functions = localfunctions();
catch % no problem; early Matlab versions can use initTestSuite fine
end
initTestSuite;

function test_crg_bin_exists
% test_assert_crg_bin_exists Assert crg_bin directory exists
mfiledir = fileparts(mfilename('fullpath'));
crg_bin_dir = fullfile(fileparts(fileparts(mfiledir)), 'crg-bin');

assertTrue(isdir(crg_bin_dir)); %#ok<ISDIR> % isdir for backwards comp

function test_crg_read_no_exception
% test_crg_read_no_exception Assert crg_read not throwing
mfiledir = fileparts(mfilename('fullpath'));
crg_file = fullfile(fileparts(fileparts(mfiledir)), 'crg-bin', 'belgian_block.crg');

error('Simulate test fail');
crg = crg_read(crg_file); %#ok<NASGU>