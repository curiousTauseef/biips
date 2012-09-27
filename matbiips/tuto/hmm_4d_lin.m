
addpath('../matlab');

%% Model Creation
%% 
%%$$F=\left (\begin{array}{cccc}
%%       1 & 0 & T & 0 \\
%%       0 & 1 & 0 & T \\
%%       0 & 0 & 1 & 0 \\
%%       0 & 0 & 0 & 1
%%     \end{array}\right )$$, 
%% $$G=\left(\begin{array}{cc}
%%            T^2/2 & 0 \\
%%            0 & T^2/2 \\
%%            T & 0 \\
%%            0 & T \\ 
%%           \end{array} \right)$$
%% and $$H=\left(
%%     \begin{array}{cccc}
%%       1 & 0 & 0 & 0 \\
%%       0 & 1 & 0 & 0 \\
%%     \end{array}
%%   \right)$.\\
%%
tmax = 10;
meanxinit = [0; 0; 1; 0];
precxinit = diag(100*ones(4,1));
delta_t = 1;
F = [ 1 0 delta_t 0; 0 1 0 delta_t; 0 0 1 0; 0 0 0 1];
G = [ delta_t.^2/2 0; 0 delta_t.^2/2 ; delta_t 0; 0 delta_t];
H = [ diag(ones(2,1)) zeros(2,2) ];
meanv = zeros(2,1);
precv = diag(1*ones(2,1));
precy = diag([2; 2]);

data={'tmax', 'precxinit', ...
      'precy', 'meanxinit', ...
	  'F', 'G', 'H', 'meanv', 'precv'};

%% intialisation console
biips_init;
p=biips_model('hmm_4d_lin_tracking.bug', data);
data_out = biips_get_data(p);
out_smc=biips_smc_samples(p, {'x'}, 100);

%% on nettoie la console