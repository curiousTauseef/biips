function [sample_param, sample_latent, log_prior, log_marg_like, ...
    accept_rate, accepted, n_fail, rw] = one_update_pmmh(console, param_names, ...
    pn_param, sample_param, sample_latent, latent_names, log_prior, log_marg_like,...
    n_part, rs_thres, rs_type, seed, rw)

n_fail = 0;

% % Remove duplicate entries
% pn_param = cellfun(@parse_varname, param_names);

% Random walk proposal
[rw, prop] = pmmh_rw_proposal(rw, sample_param);

% TODO: Check NA

% Compute log prior density
log_prior_prop = 0;
for i=1:length(param_names)
    var = param_names{i};
    tag = inter_biips('change_data', console, pn_param(i).name, ...
        pn_param(i).lower, pn_param(i).upper, prop{i}, true);
    if ~tag
        log_prior_prop = -Inf;
        n_fail = n_fail + 1;
        warning('Data change failed: %s = %.1f', var, prop{i});
    end
    log_p = inter_biips('get_log_prior_density', console, pn_param(i).name, ...
        pn_param(i).lower, pn_param(i).upper);
    log_prior_prop = log_prior_prop + log_p;
end

% Compute the marginal likelihood: Run SMC sampler
log_marg_like_prop = 0;
if (log_prior_prop ~= -Inf)
    ok = run_smc_forward(console, n_part, rs_thres, rs_type, seed)
    if ~ok
        log_marg_like_prop = -Inf;
        n_fail = n_fail + 1;
        warning('Failure running SMC forward sampler')
    else
        log_marg_like_prop = inter_biips('get_log_norm_const', console);
        if isnan(log_marg_like_prop) || log_marg_like_prop==Inf
            error('Failed to get log marginal likelihood');
        end
    end    
end

% Acceptance rate
accept_rate = min(1, exp(log_marg_like_prop - log_marg_like + ...
    log_prior_prop - log_prior));
if isnan(accept_rate)
    error('Failed to compute acceptance rate: NaN')
end

% Accept-reject step
accepted = (rand < accept_rate);
if accepted
    sample_param = prop;
    log_prior = log_prior_prop;
    log_marg_like = log_marg_like_prop;
    if ~isempty(sample_latent)
        % Sample one realization of the monitored latent variables
        rng_seed = get_seed();
        inter_biips('sample_gen_tree_smooth_particle', console, rng_seed);
        sampled_value = inter_biips('get_sampled_gen_tree_smooth_particle', console);
        for i=1:length(latent_names)
            var = latent_names{i};
            sample_latent{i} = getfield(sampled_value, var);
        end   
    end
end
sample_param
sample_latent

% rescale random walk stepsize
rw = pmmh_rw_rescale(rw, accept_rate);
% Update empirical mean and covariance matrix
rw = pmmh_rw_learn_cov(rw, sample_param, accepted);