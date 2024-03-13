//
//     ___  ___  ___  _____  ____  ___                     ___
//    / _ \/ _ \/ _ \/ ___/ / __ \/ (_)__ ____  ___  ___  / (_)__ ___
//   / // / // / ___/ (_ / / /_/ / / / _ `/ _ \/ _ \/ _ \/ / / -_|_-<
//  /____/____/_/   \___/  \____/_/_/\_, /\___/ .__/\___/_/_/\__/___/
//                                  /___/    /_/
//  DDPG Oligopolies Simulator
//  Created by Aldric Labarthe 2023-2024.
//

#include <iostream>
#include <functional>
#include <thread>
#include <cstring>

#include "AKML-lib/AKML.hpp"
#include "AKML-lib/AgentBasedUtilities/CSV_Saver.hpp"

#include "LearningAgent.hpp"
#include "SimulationManager.hpp"

#define MAX_THREADS_USAGE 0.85

void printProgramUsage(const std::string &programName) {
    std::cout << "Usage: " << programName << " [options]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -h, --help                  Show this help message and exit\n";
    std::cout << "  -m, --oligopolyModel=value  Choose the model (COURNOT|STACKELBERG|TEMPORAL_COURNOT) \n";
    std::cout << "  -N, --maxIterations=value   Set the number of iterations\n";
    std::cout << "  -n, --agentsNb=value        Set the number of agents\n";
    std::cout << "  -S, --simulationsNb=value   Set the number of simulations to process\n";
    std::cout << "  -T, --maxThreads=value      Set the number of simulations to process in parallel\n";
    std::cout << "  -g, --gamma=value           Set the gamma parameter\n";
    std::cout << "  -p, --profitNorm=value      Enable/Disable profit normalization\n";
    std::cout << "  -d, --decayRate=value       Edit the decayRate parameter\n";
}

struct options {
    SimulationManager::OLIGOPOLY_TYPE type = SimulationManager::OLIGOPOLY_TYPE::TEMPORAL_COURNOT;
    std::size_t agentsNb = 2;
    std::size_t maxIteration = 120000;
    std::size_t simulationsNb = 12;
    unsigned int maxThreads = (std::thread::hardware_concurrency()*MAX_THREADS_USAGE > 1) ? std::thread::hardware_concurrency()*MAX_THREADS_USAGE : 1;
    float gamma = 0.99;
    bool profitNormalization = true;
    float decayRate = 0.9998;
};

void printOptionsValues(const options &localoptions) {
    std::cout << "Options:" << std::endl;
    std::cout << "--oligopolyModel="<< SimulationManager::getOligopolyName(localoptions.type) << "\n";
    std::cout << "--maxIterations=" << localoptions.maxIteration << "\n";
    std::cout << "--agentsNb=" << localoptions.agentsNb << "\n";
    std::cout << "--simulationsNb=" << localoptions.simulationsNb << "\n";
    std::cout << "--maxThreads=" << localoptions.maxThreads << "\n";
    std::cout << "--gamma=" << localoptions.gamma << "\n";
    std::cout << "--profitNorm=" << localoptions.profitNormalization << "\n";
    std::cout << "--decayRate=" << localoptions.decayRate << "\n";
}

// Returns false if there is an error
bool handleOptions(int argc, const char * argv[], options& localoptions) {
    std::vector<std::string> args(argv, argv + argc);
    for (int i = 1; i < argc; ++i) {
        const std::string &arg = args[i];
        
        std::size_t pos = arg.find('=');
        std::string option, value;
    
        if (arg.substr(0, 2) == "--" && pos > 2 && pos != std::string::npos) {
            option = arg.substr(2, pos - 2);
            value = arg.substr(pos + 1);
        }else if (arg.substr(0, 1) == "-" && pos != std::string::npos) {
            option = arg.substr(1, pos - 1);
            value = arg.substr(pos + 1);
        }else {
            printProgramUsage(args[0]);
            return false;
        }
        
        //std::cout << "Debug: " << option << " " << value << "\n";
        try {
            if (option == "m" ||option == "oligopolyModel"){
                if (value == "STACKELBERG" ||value == "Stackelberg" ||value == "S"){
                    localoptions.type = SimulationManager::OLIGOPOLY_TYPE::STACKELBERG;
                }else if (value == "COURNOT" ||value == "Cournot" ||value == "C"){
                    localoptions.type = SimulationManager::OLIGOPOLY_TYPE::COURNOT;
                }else if (value == "TEMPORAL_COURNOT" ||value == "TEMPORALCOURNOT" ||value == "Temporal_Cournot" ||value == "TEMPORAL" ||value == "TC" ||value == "T"){
                    localoptions.type = SimulationManager::OLIGOPOLY_TYPE::TEMPORAL_COURNOT;
                }else {
                    std::cerr << "Alert! Unable to parse oligopoly model type.\n";
                    printProgramUsage(args[0]);
                    return false;
                }
            }else if (option == "N" ||option == "maxIterations"){
                localoptions.maxIteration = (std::size_t)std::stoi(value);
            }else if (option == "n" ||option == "agentsNb"){
                localoptions.agentsNb = (std::size_t)std::stoi(value);
            }else if (option == "S" ||option == "simulationsNb"){
                localoptions.simulationsNb = (std::size_t)std::stoi(value);
            }else if (option == "T" ||option == "maxThreads"){
                localoptions.maxThreads = (unsigned int)std::stoi(value);
                if (std::thread::hardware_concurrency()*MAX_THREADS_USAGE < localoptions.maxThreads){
                    std::cerr << "Too many threads requested, maxThreads set to " << std::thread::hardware_concurrency()*MAX_THREADS_USAGE << "\n";
                    localoptions.maxThreads = std::thread::hardware_concurrency()*MAX_THREADS_USAGE;
                }
            }else if (option == "g" ||option == "gamma"){
                localoptions.gamma = std::stof(value);
            }else if (option == "p" ||option == "profitNorm"){
                localoptions.profitNormalization = (value == "true" || value == "T" || value == "1" ||value == "True" || value == "TRUE");
            }else if (option == "d" ||option == "decayRate"){
                localoptions.decayRate = std::stof(value);
            }else {
                printProgramUsage(args[0]);
                return false;
            }
        }catch (const std::invalid_argument& e) {
            std::cerr << "Invalid argument: " << e.what() << std::endl;
            return -1;
        }catch (const std::out_of_range& e) {
            std::cerr << "Out of range argument: " << e.what() << std::endl;
            return -1;
        }
    }
    if (localoptions.type == SimulationManager::OLIGOPOLY_TYPE::TEMPORAL_COURNOT && !localoptions.profitNormalization)
        std::cerr << "Alert! Profit normalization is disabled. Temporal Cournot should be implemented with profit normalization. \n";
    return true;
}


int main(int argc, const char * argv[]) {
    std::cout << "DDPG-Oligopolies-Simulator v0.1 - Welcome!\n";
    options localoptions = {
      // If we want to force default values:
        .type=SimulationManager::STACKELBERG,
        .simulationsNb = 2,
        .gamma = 0.f,
        .profitNormalization = false,
        .decayRate = 1.f // or 0.9999
    };
    
    if (!handleOptions(argc, argv, localoptions))
        return -1;
    printOptionsValues(localoptions); std::cout << std::endl;
    
    unsigned int maxThreads = std::max((unsigned int)(localoptions.maxThreads/localoptions.agentsNb), (unsigned int)1);
    std::vector<SimulationManager> managers;
    std::vector<std::thread> workers;
    
    // Input renormalization, in temporal Cournot, the price is the input, needing a higher scaling factor
    SimulationManager::A = (localoptions.type == SimulationManager::TEMPORAL_COURNOT) ? 0.45 : 1;
    
    managers.reserve(maxThreads); workers.reserve(maxThreads);
    for (std::size_t s(0); s < localoptions.simulationsNb;){
        for (unsigned int conc_s(0); s+conc_s < std::min(localoptions.simulationsNb, s+maxThreads); conc_s++){
            std::cout << "Initialization of simulation " << s+conc_s+1 << " / " << localoptions.simulationsNb << "\n";
            managers.emplace_back(localoptions.type, localoptions.agentsNb, localoptions.maxIteration);
            managers.back().setGamma(localoptions.gamma);
            managers.back().setDecayrate(localoptions.decayRate);
            managers.back().setWNMethod(&SimulationManager::LINDECAYED_WHITENOISE);
            if (!localoptions.profitNormalization)
                managers.back().disableProfitRenormalization();
        }
        
        for (unsigned int conc_s(0); conc_s < managers.size(); conc_s++){
            std::cout << "Processing simulation " << s+conc_s+1 << " / " << localoptions.simulationsNb << "\n";
            workers.emplace_back(&SimulationManager::processSimulation, managers[conc_s], maxThreads>1&&localoptions.simulationsNb>1);
        }
        for (unsigned int conc_s(0); conc_s < managers.size(); conc_s++){
            if (workers[conc_s].joinable())
                workers[conc_s].join();
        }
        s += managers.size();
        workers.clear(); managers.clear();
        
    }
    
    return 0;
}
