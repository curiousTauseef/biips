%% Matbiips: Tutorial 3
% In this tutorial, we will see how to introduce user-defined functions in the BUGS model.

%% Statistical model
% The statistical model is defined as follows.
%
% $$ x_1\sim \mathcal N\left (\mu_0, \frac{1}{\lambda_0}\right )$$
%
% $$ y_1\sim \mathcal N\left (h(x_1), \frac{1}{\lambda_y}\right )$$
%
% For $t=2:t_{max}$
%
% $$ x_t|x_{t-1} \sim \mathcal N\left ( f(x_{t-1},t-1), \frac{1}{\lambda_x}\right )$$
%
% $$ y_t|x_t \sim \mathcal N\left ( h(x_{t}), \frac{1}{\lambda_y}\right )$$
%
% with $\mathcal N\left (m, S\right )$ stands for the Gaussian distribution 
% of mean $m$ and covariance matrix $S$, $h(x)=x^2/20$, $f(x,t-1)=0.5 x+25 x/(1+x^2)+8 \cos(1.2 (t-1))$, $\mu_0=0$, $\lambda_0 = 5$, $\lambda_x = 0.1$ and $\lambda_y=1$. 

%% Statistical model in BUGS language
% One needs to describe the model in BUGS language. We create the file
%  'hmm_1d_nonlin_funmat.bug':

%%
%
% 
%     var x_true[t_max], x[t_max], y[t_max]
% 
%     data
%     {
%       x_true[1] ~ dnorm(mean_x_init, prec_x_init)
%       y[1] ~ dnorm(x_true[1]^2/20, prec_y)
%       for (t in 2:t_max)
%       {
%         x_true[t] ~ dnorm(funmat(x_true[t-1],t-1), prec_x)
%         y[t] ~ dnorm(x_true[t]^2/20, prec_y)
%       }
%     }
% 
% 
%     model
%     {
%       x[1] ~ dnorm(mean_x_init, prec_x_init)
%       y[1] ~ dnorm(x[1]^2/20, prec_y)
%       for (t in 2:t_max)
%       {
%         x[t] ~ dnorm(funmat(x[t-1],t-1), prec_x)
%         y[t] ~ dnorm(x[t]^2/20, prec_y)
%       }
%     }
%
% Although the nonlinear function f can be defined in BUGS language, we
% choose here to use an external user-defined function 'funmat', which will
% call a Matlab function. 

%% User-defined functions in Matlab
% The BUGS model calls a function funcmat. In order to be able to use this
% function, one needs to create two functions in Matlab. The first
% function, called here 'f_eval.m' provides the evaluation of the function.
%
% *f_eval.m*
%
%     function out = f_eval(x, k)
% 
%     out = .5 * x + 25*x/(1+x^2) + 8*cos(1.2*k);
%
% The second function, f_dim.m, provides the dimensions of the output of f_eval, 
% possibly depending on the dimensions of the inputs.
%
% *f_dim.m* 
%
%     function out_dim = f_dim(x_dim, k_dim)
% 
%     out_dim = [1,1];


%% Installation of Matbiips
% Unzip the Matbiips archive in some folder
% and add the Matbiips folder to the Matlab path
% 

matbiips_path = '../../matbiips/matlab';
addpath(matbiips_path)

%% Load model and data
%

%%
% *Model parameters*
t_max = 20;
mean_x_init = 0;
prec_x_init = 1/5;
prec_x = 1/10;
prec_y = 1;
data = struct('t_max', t_max, 'prec_x_init', prec_x_init,...
    'prec_x', prec_x,  'prec_y', prec_y, 'mean_x_init', mean_x_init);

%%
% *Start BiiPS console*
biips_init;

%%
% *Add the user-defined function 'funmat'*
fun_bugs = 'funmat'; fun_dim = 'f_dim';funeval = 'f_eval';fun_nb_inputs = 2;
biips_add_function(fun_bugs, fun_nb_inputs, fun_dim, funeval)


%%
% *Compile BUGS model and sample data*
model = 'hmm_1d_nonlin_funmat.bug'; % BUGS model filename
sample_data = true; % Boolean
[model_id, data] = biips_model(model, data, 'sample_data', sample_data); % Create biips model and sample data

%% BiiPS Sequential Monte Carlo
% Let now use BiiPS to run a particle filter. 

%%
% *Parameters of the algorithm*. We want to monitor the variable x, and to
% get the filtering and smoothing particle approximations. The algorithm
% will use 10000 particles, stratified resampling, with a threshold of 0.5.
n_part = 10000; % Number of particles
variables = {'x'}; % Variables to be monitored
type = 'fs'; rs_type = 'stratified'; rs_thres = 0.5; % Optional parameters

%%
% *Run SMC*
out_smc = biips_smc_samples(model_id, variables, n_part,...
    'type', type, 'rs_type', rs_type, 'rs_thres', rs_thres);

%%
% *Diagnostic on the algorithm*. 
diag = biips_diagnostic(out_smc);

%%
% *Summary statistics*
summary = biips_summary(out_smc, 'probs', [.025, .975]);

%%
% *Plot Filtering estimates*
x_f_mean = summary.x.f.mean;
x_f_med = summary.x.f.med;
x_f_quant = summary.x.f.quant;
figure('name', 'SMC: Filtering estimates')
fill([1:t_max, t_max:-1:1], [x_f_quant(1,:), fliplr(x_f_quant(2,:))],...
    [.7 .7 1]);
hold on
plot(x_f_mean, 'linewidth', 3)
xlabel('Time')
ylabel('Estimates')
legend({'95 % credible interval', 'Filtering Mean Estimate'})
legend('boxoff')
box off

%%
% *Plot Smoothing estimates*
x_s_mean = summary.x.s.mean;
x_s_quant = summary.x.s.quant;
figure('name', 'SMC: Smoothing estimates')
fill([1:t_max, t_max:-1:1], [x_s_quant(1,:), fliplr(x_s_quant(2,:))],...
    [.7 .7 1]);
hold on
plot(x_s_mean, 'linewidth', 3)
xlabel('Time')
ylabel('Estimates')
legend({'95 % credible interval', 'Smoothing Mean Estimate'})
legend('boxoff')
box off

%%
% Marginal filtering and smoothing densities

kde_estimates = biips_density(out_smc);
time_index = [5, 10, 15, 20];
figure('name', 'SMC: Marginal posteriors')
for k=1:length(time_index)
    tk = time_index(k);
    subplot(2, 2, k)
    plot(kde_estimates.x.f(tk).x, kde_estimates.x.f(tk).f);
    hold on
    plot(kde_estimates.x.s(tk).x, kde_estimates.x.s(tk).f, 'r');
    plot(data.x_true(tk), 0, '*g');
    xlabel(['x_{' num2str(tk) '}']);
    ylabel('posterior density');
    title(['t=', num2str(tk)]);    
end
legend({'filtering density', 'smoothing density', 'True value'});

%% Clear model
% 

biips_clear(model_id)