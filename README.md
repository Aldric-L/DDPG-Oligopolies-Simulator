#  DDPG-Oligopolies
### Aldric L. 2024 ENS Paris Saclay

This repository contains the source code and the simulations data used in my master thesis *Strategies and equilibria on selected markets: a multi-agent simulation and stochastic modeling approach* for the Ecole Normale Supérieure Paris-Saclay (France).

The simulator is based on the work of Timothy P. Lillicrap et al (10.48550/arXiv.1509.02971) and features self-learning agents in standard oligopoly models (Cournot, Stackelberg).  The simulator is able to simulate various simulation lengths with almost every parameter being tunable via the CLI.

## CLI Usage 

```
-h, --help                  Show this help message and exit 
-o, --oligopolyModel=value  Choose the model (COURNOT|STACKELBERG|TEMPORAL_COURNOT)  
-N, --maxIterations=value   Set the number of iterations 
-n, --agentsNb=value        Set the number of agents 
-S, --simulationsNb=value   Set the number of simulations to process 
-T, --maxThreads=value      Set the number of simulations to process in parallel 
-g, --gamma=value           Set the gamma parameter 
-m, --maxBufferSize=value   Set the size of the memory buffer 
-p, --profitNorm=value      Enable/Disable profit normalization 
-f, --profitFactor=value    Reward scaling factor 
-F, --profitScale=value     Reward scaling constant 
-d, --decayRate=value       Edit the decayRate parameter 
-w, --wnDecay=value         Choose the Whitenoise decayMethod (LIN|EXP|SIG|TRUNC_EXP|TRUNC_EXP_RES) 
-s, --maxWhiteNoise=value   Edit the maximum std. deviation of the whitenoise process 
-a, --actorLR=value         Edit the actor learning rate 
-c, --criticLR=value        Edit the critic learning rate 
-C, --modC=value            Edit the marginal cost of the model 
-D, --modD=value            Edit the demand hyperparameter of the model 
-E, --exportCritics         Should we dump critics of agents at the end of simulation ? 
```

## Repository organization

```
- Simulator/ : Where the C++ source files are

- DataProcessing/ : Where data and R scripts are
|-- Calibration/ : Python scripts used to determine optimal hyperparameter and to study analytic stability
|-- Outputs/ : csv files for data analysis (simulations data)
|-- plots/ : png files of the plots in the paper
| autoImportCSV.R : R script to import all the simulations output in a folder
| autoProcessVisualization.R : R script to regenerate plots
| evaluateGammaOnCollusion.R : features all the regressions used to assess the causis of algorithmic collusion
| utilsOligopolies.R : hyperparameters and other R functions

- Strategies and equilibria on selected markets.pdf : The paper
```

## Cite this work
```
@mastersthesis{Labarthe_2024,
    author = {Aldric Labarthe},
    date-added = {2024-04-22 16:42:42 +0200},
    school = {Ecole Normale Sup{\'e}rieure Paris-Saclay},
    title = {Strategies and equilibria on selected markets: a multi-agent simulation and stochastic modeling approach},
    year = {2024}}
```
