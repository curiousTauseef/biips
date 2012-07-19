x=load('x.dat');
y=load('y.dat');
data=struct('y', y, 'tmax', 100, 'precxinit', 1,'precx', 100, 'precy', 1, 'meanxinit', 0);
bool_et_un = inter_biips('load_module', 'basemod'); 
p0=inter_biips('make_console'); 
inter_biips('check_model', p0, 'hmm_1d_lin.bug'); 
inter_biips('compile_model', p0, data, false, 12);
debilos = inter_biips('get_data',p0);
inter_biips('clear_console',p0); 
