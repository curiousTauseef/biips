function diagn = biips_diagnostic(parts, varargin)

%
% BIIPS_DIAGNOSTIC returns a diagnostic structure on the SMC algorithm
% diagn = biips_diagnostic(parts, 'Propertyname', propertyvalue, ...)
%
%   INPUT
%   - parts:    input structure containing the particles of different variables.
%               usually returned by biips_smc_samples function
%   Optional Inputs:
%   - variable_names:   cell of strings. subset of the fields of particles struct
%                       argument
%   - type:             string containing the characters 'f', 's' and/or 'b'
%   - ess_thres :       integer. Threshold on the Effective Sample Size (ESS) of the
%                       examined particles. If all the ESS components are over the
%                       threshold, the diagnostic is valid, otherwise it is not
%                       valid. default is 30
%   - quiet:            flag. deactivate message display. default is 0
%
%   OUTPUT
%   - diagn:   output structure providing the minimum value of the
%              effective sample size for each variable
%
%   See also BIIPS_SMC_SAMPLES
%--------------------------------------------------------------------------
% EXAMPLE:
% data = struct('var1', 0, 'var2', 1.2);
% model_id = biips_model('model.bug', data)
% npart = 100; variables = {'x'}; 
% out_smc = biips_smc_samples(model_id, variables, npart);
% diag = biips_diagnostic(out_smc);
%--------------------------------------------------------------------------

% BiiPS Project - Bayesian Inference with interacting Particle Systems
% MatBiips interface
% Authors: Adrien Todeschini, Marc Fuentes, Fran�ois Caron
% Copyright (C) Inria
% License: GPL-3
% Jan 2014; Last revision: 18-03-2014
%--------------------------------------------------------------------------
% 
%% PROCESS AND CHECK INPUTS
%%% Process and check optional arguments
optarg_names = {'variable_names', 'type', 'ess_thres', 'quiet'};
optarg_default = {{}, '', 30, false};
optarg_valid = {{}, {'f', 's', 'b', 'fs', 'fb', 'sb', 'fsb'}, [0, intmax],...
    {true, false}};
optarg_type = {'char', 'char', 'numeric', 'logical'};
[variable_names, type, ess_thres, quiet] = parsevar(varargin, optarg_names, optarg_type,...
    optarg_valid, optarg_default);

if (isempty(variable_names))
    variable_names = fieldnames(parts); % vars = {}, take all fields
end

if (isempty(type))
    type='fsb';
end

% retrieve only the field presents in the first variable
present = fieldnames(getfield(parts, variable_names{1}));
present = strcat(present{:});
indices = arrayfun(@(x) strfind(present, x), type, 'UniformOutput', 0);
indices = sort([indices{:}]);
type = present(indices);

% Select only the wanted variables
s = cell2struct_weaknames(cellfun(@(x) getfield(parts, x), variable_names,'UniformOutput',0), variable_names);
cell_fsb = num2cell(type);
cell_diagn = cell(size(variable_names));

for i=1:length(variable_names)
    if ~quiet
        disp(['* Diagnosing variable: ' , variable_names{i}]);
    end
    ctemp = cell(size(type));
    for j=1:length(type)
        ctemp{j} =  diagnostic(getfield(getfield(s, variable_names{i}), type(j)), ess_thres, quiet, type(j));
    end
    cell_diagn{i} = cell2struct_weaknames(ctemp, cell_fsb);
end
diagn = cell2struct_weaknames(cell_diagn, variable_names);