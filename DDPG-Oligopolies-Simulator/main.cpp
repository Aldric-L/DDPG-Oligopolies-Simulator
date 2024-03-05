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

int main(int argc, const char * argv[]) {
    SimulationManager::OLIGOPOLY_TYPE type = SimulationManager::TEMPORAL_COURNOT;
    std::size_t agentsNb = 4;
    std::size_t maxIteration = 120000;
    if (argc > 1){
        if (argc >= 2){
            if (std::strcmp(argv[1],"STACKELBERG"))
                type = SimulationManager::STACKELBERG;
            if (std::strcmp(argv[1], "COURNOT"))
                type = SimulationManager::COURNOT;
                
            if (argc >= 3){
                try {
                    agentsNb = (std::size_t)std::stoi(argv[2]);
                } catch (const std::invalid_argument& e) {
                    std::cerr << "Invalid argument: " << e.what() << std::endl;
                    return -1;
                } catch (const std::out_of_range& e) {
                    std::cerr << "Out of range argument: " << e.what() << std::endl;
                    return -1;
                }
                
                if (argc >= 4){
                    try {
                        maxIteration = (std::size_t)std::stoi(argv[3]);
                    } catch (const std::invalid_argument& e) {
                        std::cerr << "Invalid argument: " << e.what() << std::endl;
                        return -1;
                    } catch (const std::out_of_range& e) {
                        std::cerr << "Out of range argument: " << e.what() << std::endl;
                        return -1;
                    }
                }
            }
            
        }
    }
    
    std::cout << "\nSimulation parameters: " << type << " - " << agentsNb << " - " << maxIteration;
    SimulationManager manager(type, agentsNb, maxIteration);
    manager.setGamma(0.9);
    manager.setDecayrate(0.9995);
    manager.setWNMethod(&SimulationManager::SIGDECAYED_WHITENOISE);
    manager.processSimulation();

    return 0;
}
