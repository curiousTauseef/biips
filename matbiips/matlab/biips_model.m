function [p, data] = biips_model(filename, data, varargin)

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% BIIPS_MODEL instantiate a stochastic model under a DAG form 
% function [p, data] = biips_model(filename, data, 'Propertyname', propertyvalue, ...)
%  INPUT: 
%  - filename:      name of the BUGS file which describes the stochastic model
%  - data:          either a struct containing constants and observed values
%                   or a cell of strings containing the names of the variables whose
%                   value will be looked for in the workspace
%  Optional inputs:
%  - sample_data: boolean to choose if data is generated by the 'data'
%                   block of the BUGS model. default is 'true'
%  - seed:        seed of the random number generator used to sample the data
%                   if 'sample_data' is true. default is random.
%  - quiet:       boolean to deactivate verbosity. default is 'false'
%  OUTPUT:
%  - p:             integer which is an index of the compiled model object in the
%                   internal table of models.
%  - data:          structure containing the data. useful if 'sample_data' is true
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% EXAMPLES:
% data = struct('var1', 0, 'var2', 1.2);
% [model_id, data] = biips_model('model.bug', data, 'sample_data', true);
%
% var1 = 0; var2 = 1.2;
% data_names = {'var1', 'var2'};
% [model_id, data] = biips_model('model.bug', data_names, 'sample_data', true);
% 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% BiiPS Project - Bayesian Inference with interacting Particle Systems
%
% Reference: A. Todeschini, M. Fuentes, F. Caron, P. Legrand, P. Del Moral.
% BiiPS: a software for Bayesian inference with interacting particle
% systems. Technical Report, INRIA. February 2014.
%
% Authors: Adrien Todeschini, Marc Fuentes
% INRIA Bordeaux, France
% email: biips-project@lists.gforge.inria.fr
% Website: https://alea.bordeaux.inria.fr/biips
% Jan 2014; Last revision: 24-01-2014
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


%% PROCESS AND CHECK INPUTS
%%% Check filename
if ~ischar(filename)
    error('Invalid filename: must be a character array.\n');
elseif ~exist(filename, 'file')
    error('Undefined BUGS file ''%s''\n', filename);
end
%%% Process and check optional arguments
optarg_names = {'sample_data', 'seed', 'quiet'};
optarg_default = {true, get_seed(), false};
optarg_valid = {{true, false}, [0, intmax], {'true', 'false'}};
optarg_type = {'logical', 'integer', 'logical'};
[sample_data, seed, quiet] = parsevar(varargin, optarg_names, optarg_type,...
    optarg_valid, optarg_default);

%%% Processing data argument
if (isa(data, 'cell'))
    data = reshape(data, length(data), 1);
    
    isch = cellfun(@(x) ischar(x), data);
    ignored_var = data(~isch);
    if (~isempty(ignored_var))
        warning('ignored non character elements in ''data'' cell argument.');
    end
    data = data(isch);
    
    isnum = cellfun(@(x) isnumeric(evalin('base', x)), data, 'ErrorHandler', @(S, varargin) false);
    ignored_var = data(~isnum);
    if (~isempty(ignored_var))
        varnames = sprintf('%s ',ignored_var{:});
        warning('ignored the following (either non numeric or non existent) variables given in ''data'' cell argument: %s ', varnames);
    end
    
    data = data(isnum);
    
    data = cell2struct(cellfun(@(x) evalin('base',x), data, 'UniformOutput', false), data, 1);
end

%% Create console
p = inter_biips('make_console');
if (quiet)
  old_verb = inter_biips('verbosity', 0);
end

%% Load model and do some checks
inter_biips('check_model', p, filename)
inter_biips('compile_model', p, data, sample_data, seed);

data = inter_biips('get_data', p);

if (quiet)
  inter_biips('verbosity', old_verb);
end