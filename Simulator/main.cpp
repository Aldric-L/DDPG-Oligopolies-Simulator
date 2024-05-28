//
//     ___  ___  ___  _____  ____  ___                     ___
//    / _ \/ _ \/ _ \/ ___/ / __ \/ (_)__ ____  ___  ___  / (_)__ ___
//   / // / // / ___/ (_ / / /_/ / / / _ `/ _ \/ _ \/ _ \/ / / -_|_-<
//  /____/____/_/   \___/  \____/_/_/\_, /\___/ .__/\___/_/_/\__/___/
//                                  /___/    /_/
//  DDPG Oligopolies Simulator
//  Created by Aldric Labarthe 2023-2024
//  ENS Paris Saclay
//

#include <iostream>
#include <functional>
#include <thread>
#include <chrono>

#include "AKML-lib/AKML.hpp"
#include "AKML-lib/AgentBasedUtilities/CSV_Saver.hpp"
#include "AKML-lib/AgentBasedUtilities/CLInterface.hpp"

#include "LearningAgent.hpp"

#include "SimulationManager.hpp"

#define MAX_THREADS_USAGE 0.85

// Some of the options that the user is allowed to edit in CLI
struct options {
    SimulationManager::OLIGOPOLY_TYPE type = SimulationManager::OLIGOPOLY_TYPE::COURNOT;
    std::size_t agentsNb = 2;
    std::size_t maxIteration = 120000;
    std::size_t simulationsNb = 12;
    unsigned int maxThreads = (std::thread::hardware_concurrency()*MAX_THREADS_USAGE > 1) ? std::thread::hardware_concurrency()*MAX_THREADS_USAGE : 1;
    float learningRateActor = 0.01;
    float learningRateCritic = 0.1;
    SimulationManager::WN_DECAY_METHOD wnDecayMethod = SimulationManager::WN_DECAY_METHOD::EXP;
    bool exportCritics = true;
    bool inputRenormalization = false;
};

int main(int argc, const char * argv[]) {
    std::cout << "DDPG-Oligopolies-Simulator v0.1 - Welcome!\n";
    
    options localoptions;
    //localoptions.simulationsNb = 1;
    
    auto CLOptionsTuple = std::make_tuple
    (akml::CLSelectOption<SimulationManager::OLIGOPOLY_TYPE> (&localoptions.type, "o", "oligopolyModel", "Choose the model (COURNOT|STACKELBERG|TEMPORAL_COURNOT)", {{"COURNOT", SimulationManager::COURNOT}, {"STACKELBERG", SimulationManager::STACKELBERG}, {"TEMPORAL_COURNOT", SimulationManager::TEMPORAL_COURNOT}}),
     akml::CLOption<std::size_t> (&localoptions.maxIteration, "N", "maxIterations", "Set the number of iterations"),
     akml::CLOption<std::size_t> (&localoptions.agentsNb, "n", "agentsNb", "Set the number of agents"),
     akml::CLOption<std::size_t> (&localoptions.simulationsNb, "S", "simulationsNb", "Set the number of simulations to process"),
     akml::CLOption<unsigned int> (&localoptions.maxThreads, "T", "maxThreads", "Set the number of simulations to process in parallel"),
     akml::CLOption<float> (&LearningAgent::gamma, "g", "gamma", "Set the gamma parameter"),
     akml::CLOption<std::size_t> (&LearningAgent::maxBufferSize, "m", "maxBufferSize", "Set the size of the memory buffer"),
     akml::CLOption<bool> (&localoptions.inputRenormalization, "p", "profitNorm", "Enable/Disable profit normalization"),
     akml::CLOption<float> (&SimulationManager::K, "f", "profitFactor", "Reward scaling factor"),
     akml::CLOption<float> (&SimulationManager::B, "F", "profitScale", "Reward scaling constant"),
     akml::CLOption<float> (&LearningAgent::decayRate, "d", "decayRate", "Edit the decayRate parameter"),
     akml::CLSelectOption<SimulationManager::WN_DECAY_METHOD> (&localoptions.wnDecayMethod, "w", "wnDecay", "Choose the Whitenoise decayMethod (LIN|EXP|SIG|TRUNC_EXP|TRUNC_EXP_RES)", {{"LIN", SimulationManager::WN_DECAY_METHOD::LIN}, {"EXP", SimulationManager::WN_DECAY_METHOD::EXP}, {"SIG", SimulationManager::WN_DECAY_METHOD::SIG}, {"TRUNC_EXP", SimulationManager::WN_DECAY_METHOD::TRUNC_EXP}, {"TRUNC_EXP_RES", SimulationManager::WN_DECAY_METHOD::TRUNC_EXP_RES}}),
     akml::CLOption<float> (&SimulationManager::maxWhiteNoise, "s", "maxWhiteNoise", "Edit the maximum std. deviation of the whitenoise process"),
     akml::CLOption<float> (&localoptions.learningRateActor, "a", "actorLR", "Edit the actor learning rate"),
     akml::CLOption<float> (&localoptions.learningRateCritic, "c", "criticLR", "Edit the critic learning rate"),
     akml::CLOption<float> (&SimulationManager::C, "C", "modC", "Edit the marginal cost of the model"),
     akml::CLOption<float> (&SimulationManager::D, "D", "modD", "Edit the demand hyperparameter of the model"),
     akml::CLOption<bool> (&localoptions.exportCritics, "E", "exportCritics", "Should we dump critics of agents at the end of simulation ?"),
     akml::CLOption<bool> (&LearningAgent::adamOptimizer, "a", "adam", "Enable/Disable Adam optimizer"));
        
    try {
        akml::CLManager localCLManager(argc, argv, CLOptionsTuple);
    }catch (...) {
        return -1;
    }
    
    if (std::thread::hardware_concurrency() > 1 && std::thread::hardware_concurrency()*MAX_THREADS_USAGE < localoptions.maxThreads){
        std::cerr << "Too many threads requested, maxThreads set to " << std::thread::hardware_concurrency()*MAX_THREADS_USAGE << "\n";
        localoptions.maxThreads = std::thread::hardware_concurrency()*MAX_THREADS_USAGE;
    }
    unsigned int maxThreads = std::max((unsigned int)(localoptions.maxThreads/localoptions.agentsNb), (unsigned int)1);
    std::vector<SimulationManager*> managers;
    std::vector<std::thread> workers;
    
    // Input renormalization, in temporal Cournot, the price is the input, needing a higher scaling factor
    SimulationManager::A = (localoptions.type == SimulationManager::TEMPORAL_COURNOT) ? 0.45 : 1;

    
    managers.reserve(maxThreads); workers.reserve(maxThreads);
    for (std::size_t s(0); s < localoptions.simulationsNb;){
        for (unsigned int conc_s(0); s+conc_s < std::min(localoptions.simulationsNb, s+maxThreads); conc_s++){
            std::cout << "Initialization of simulation " << s+conc_s+1 << " / " << localoptions.simulationsNb << "\n";
            managers.push_back(new SimulationManager(localoptions.type, localoptions.agentsNb, localoptions.maxIteration));
            switch (localoptions.wnDecayMethod) {
                case SimulationManager::LIN: managers.back()->setWNMethod(&SimulationManager::LINDECAYED_WHITENOISE); break;
                case SimulationManager::EXP: managers.back()->setWNMethod(&SimulationManager::EXPDECAYED_WHITENOISE); break;
                case SimulationManager::SIG: managers.back()->setWNMethod(&SimulationManager::SIGDECAYED_WHITENOISE); break;
                case SimulationManager::TRUNC_EXP: managers.back()->setWNMethod(&SimulationManager::TRUNCEXPDECAYED_WHITENOISE); break;
                case SimulationManager::TRUNC_EXP_RES: managers.back()->setWNMethod(&SimulationManager::TRUNCRESEXPDECAYED_WHITENOISE); break;
            }
            managers.back()->setLearningRates(localoptions.learningRateActor, localoptions.learningRateCritic);
            if (localoptions.inputRenormalization)
                managers.back()->enableProfitRenormalization();
            else
                managers.back()->disableProfitRenormalization();
        }
        
        for (unsigned int conc_s(0); conc_s < managers.size(); conc_s++){
            std::cout << "Processing simulation " << s+conc_s+1 << " / " << localoptions.simulationsNb << "\n";
            workers.emplace_back([localoptions, &CLOptionsTuple](SimulationManager* simul, bool mute, unsigned int simulId) {
                auto start = std::chrono::high_resolution_clock::now();
                std::string output = simul->processSimulation(mute);
                if (localoptions.exportCritics){
                    std::cout << "Critics dump of simulation " << simulId+1 << " - Log file: " << "DDPG-Critics-" << output << ".csv" << "\n";
                    simul->saveCritics(output);
                }
                auto end = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double> duration = end - start;
                
                std::ofstream cout(std::string("DDPG-SimulInfos-" + output +".txt"));
                std::ios_base::sync_with_stdio(false);
                auto *coutbuf = std::cout.rdbuf();
                std::cout.rdbuf(cout.rdbuf());
                akml::CLManager::printOptionsValues(false, CLOptionsTuple);
                std::cout << "--executionTime=" << duration.count() << "\n";
                std::cout.rdbuf(coutbuf);
                std::cout << "End of simulation " << simulId+1 << " - Log file: " << "DDPG-" << SimulationManager::getOligopolyName(localoptions.type) << "-" << std::to_string(localoptions.agentsNb) << "-" << output << ".csv" << "\n";
            }, managers[conc_s], maxThreads>1&&localoptions.simulationsNb>1, s+conc_s);
            
        }
        for (unsigned int conc_s(0); conc_s < managers.size(); conc_s++){
            if (workers[conc_s].joinable())
                workers[conc_s].join();
            delete managers[conc_s];
        }
        s += managers.size();
        workers.clear(); managers.clear();
        
    }
    return 0;
}
