% test_crgMex
% Runs some crg api functions using matlab and c api and compares results 
fprintf('--- test_crg_c_invoke ---\n');

% add paths
filedir = fileparts(mfilename('fullpath'));
addpath(fullfile(filedir, 'mex'));
addpath(fullfile(fileparts(filedir), 'matlab', 'lib'));

% logical to strip map
true_false = {'true', 'false'};

% crg filepath
crg_file = 'D:\Software_Development\Public_Git\OpenCRGFork\crg-bin\belgian_block.crg';

%% Matlab API
crg = crg_read(crg_file);
[pu, pv] = meshgrid(crg.head.ubeg-1:crg.head.uinc:crg.head.uend+1,crg.head.vmin-1:crg.head.vinc:crg.head.vmax+1);
puv = [pu(:), pv(:)];

% shuffle puv with fixed seed to test history
rng(1);
rand_idx = 1:length(puv);
rand_idx = rand_idx(randperm(length(rand_idx)));
puv = puv(rand_idx, :);

% evaluate
zm  = crg_eval_uv2z (crg, puv);
xym = crg_eval_uv2xy(crg, puv);
uvm = crg_eval_xy2uv(crg, xym);

%% C-API
crg_id = crgMex('open', crg_file);
zc     = crgMex('eval_uv2z',  crg_id, puv);
xyc    = crgMex('eval_uv2xy', crg_id, puv);
uvc    = crgMex('eval_xy2uv', crg_id, xym);
closed = crgMex('close', crg_id);

%% Compare results
dz  = zm-zc;
dxy = xym-xyc;
duv = uvm-uvc;

fprintf('Comparison Matlab- C-API:\n');
fprintf('Same nan returns from uv2z: %s\n', true_false{1+any(xor(isnan(zm), isnan(zc)))});
fprintf('Max. difference uv2z: %g\n', max(abs(dz)));
fprintf('Max. difference uv2xy: %g , %g\n', max(abs(dxy)));
fprintf('Max. difference xy2uv: %g , %g\n', max(abs(duv)));
