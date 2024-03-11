//
//  main.cpp
//  DDPG Oligopolies Simulator
//
//  Created by Aldric Labarthe on 26/01/2024.
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

int main(int argc, const char * argv[]) {
    SimulationManager::OLIGOPOLY_TYPE type = SimulationManager::TEMPORAL_COURNOT;
    std::size_t agentsNb = 2;
    // Formerly 90000...
    std::size_t maxIteration = 120000;
    std::size_t simulationsNb = 10;
    unsigned int maxThreads = 1;
    maxThreads = (std::thread::hardware_concurrency()*MAX_THREADS_USAGE > 1) ? std::thread::hardware_concurrency()*MAX_THREADS_USAGE : 1;

    // Arg 1 : STACKELBERG | COURNOT | TEMPORAL
    // Arg 2 : agents number
    // Arg 3 : maxIteration
    // Arg 4 : simulationsNb
    // Arg 5 : maxThreads
    std::cout << "DDPG-Oligopolies-Simulator v0.1 - Welcome!\n";
    std::cout << "Usage: STACKELBERG|COURNOT|TEMPORAL agentsNumber maxIteration simulationsNb maxThreads \n";
    if (argc >= 2){
        if (std::strcmp(argv[1],"STACKELBERG"))
            type = SimulationManager::STACKELBERG;
        if (std::strcmp(argv[1], "COURNOT"))
            type = SimulationManager::COURNOT;
            
        try {
            if (argc >= 3)
                agentsNb = (std::size_t)std::stoi(argv[2]);
            if (argc >= 4)
                maxIteration = (std::size_t)std::stoi(argv[3]);
            if (argc >= 5)
                simulationsNb = (std::size_t)std::stoi(argv[4]);
            if (argc >= 6)
                maxThreads = (unsigned int)std::stoi(argv[5]);
        } catch (const std::invalid_argument& e) {
                std::cerr << "Invalid argument: " << e.what() << std::endl;
                return -1;
        } catch (const std::out_of_range& e) {
                std::cerr << "Out of range argument: " << e.what() << std::endl;
                return -1;
        }
            
    }
    maxThreads = std::max((unsigned int)(maxThreads/agentsNb), (unsigned int)1);
    std::cout << "Simulation parameters: " << type << " - " << agentsNb << " - " << maxIteration << " (" << simulationsNb << "-" << maxThreads << ")\n";
    std::vector<SimulationManager> managers;
    std::vector<std::thread> workers;
    managers.reserve(maxThreads); workers.reserve(maxThreads);
    for (std::size_t s(0); s < simulationsNb;){
        for (unsigned int conc_s(0); s+conc_s < std::min(simulationsNb, s+maxThreads); conc_s++){
            std::cout << "Initialization of simulation " << s+conc_s+1 << " / " << simulationsNb << "\n";
            managers.emplace_back(type, agentsNb, maxIteration);
            //manager.setGamma(0.9);
            managers.back().setGamma(0);
            managers.back().setWNMethod(&SimulationManager::TRUNCRESEXPDECAYED_WHITENOISE);
            managers.back().disableProfitRenormalization();
            //manager.setDecayrate(0.9999);
            //manager.setWNMethod(&SimulationManager::SIGDECAYED_WHITENOISE);
            
        }
        
        for (unsigned int conc_s(0); conc_s < managers.size(); conc_s++){
            std::cout << "Processing simulation " << s+conc_s+1 << " / " << simulationsNb << "\n";
            workers.emplace_back(&SimulationManager::processSimulation, managers[conc_s], true);
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
