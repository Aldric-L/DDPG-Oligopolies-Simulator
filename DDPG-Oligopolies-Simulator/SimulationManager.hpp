//
//  SimulationManager.hpp
//  CournotDDPG
//
//  Created by Aldric Labarthe on 28/02/2024.
//

#ifndef SimulationManager_hpp
#define SimulationManager_hpp

#include <stdio.h>
#include <functional>
#include <vector>
#include <thread>
#include "AKML-lib/AgentBasedUtilities/CSV_Saver.hpp"
#include "LearningAgent.hpp"


class SimulationManager {
public:
    enum OLIGOPOLY_TYPE {STACKELBERG, COURNOT, TEMPORAL_COURNOT};
    
    // Hyperparameters
    inline static float C = 0.2;
    inline static float D = 2.2;
    inline static float K = 0.98;
    inline static float B = 0;
    inline static float A = 0.45;
    
    inline static const float STANDARD_LIN_DEMAND (float p) { return (float)(D - p); }
    inline static const float STANDARD_LIN_INVDEMAND (float q) { return (float)(D - q); }
    inline static const float LIN_COST (float q) { return (float)(C*q); }
    inline static const float QUAD_COST (float q) { return (float)(C*std::pow(q,2)); }
    inline static const float EXPDECAYED_WHITENOISE (std::size_t it, std::size_t maxiterations) {
        return 1.f - std::powf(1.0001, (float)((float)it -(float)maxiterations)); }
    inline static const float LINDECAYED_WHITENOISE (std::size_t it, std::size_t maxiterations) {
        return 1.f-(float)it/(float)maxiterations; }
    inline static const float SIGDECAYED_WHITENOISE (std::size_t it, std::size_t maxiterations) {
        return 1.f-(1 / (1 + (float)std::exp(0.65 * (float)maxiterations - (float)it))); }
    
    
protected:
    // Local parameters
    std::function<float(float)> demand;
    std::function<float(float)> inverseDemand;
    std::function<float(float)> cost;
    std::function<float(std::size_t, std::size_t)> whitenoiseMethod;
    
    std::size_t agentsNumber;
    std::size_t maxIterations;
    std::vector<LearningAgent> agents;
    std::vector<std::size_t>* leaders;
    akml::CSV_Saver<akml::DynamicMatrixSave<float>> LogManager;
    OLIGOPOLY_TYPE localSimulationType;
    
public:
    SimulationManager(OLIGOPOLY_TYPE type, std::size_t agentsNumber, std::size_t maxIterations=60000,
                      std::function<float(float)> localDemand = nullptr, std::function<float(float)> localInverseDemand = nullptr, std::function<float(float)> localCost = nullptr, std::function<float(std::size_t, std::size_t)> localWhitenoiseMethod = nullptr) :
    localSimulationType(type),
    agentsNumber(agentsNumber), maxIterations(maxIterations),
    demand(localDemand == nullptr ? &SimulationManager::STANDARD_LIN_DEMAND : localDemand),
    inverseDemand(localInverseDemand == nullptr ? &SimulationManager::STANDARD_LIN_INVDEMAND : localInverseDemand),
    cost(localCost == nullptr ? &SimulationManager::QUAD_COST : localCost),
    whitenoiseMethod(localWhitenoiseMethod == nullptr ? &SimulationManager::EXPDECAYED_WHITENOISE : localWhitenoiseMethod) {
        if (localSimulationType == STACKELBERG)
            leaders = new std::vector<std::size_t>;
        else
            leaders = nullptr;
        agents.reserve(agentsNumber);
        for (std::size_t agent_i(0); agent_i < agentsNumber; agent_i++){
            agents.emplace_back("Agent" + std::to_string(agent_i+1));
        }
        
        std::vector<std::string> parametersName;
        parametersName.reserve(2 + agentsNumber * 3);
        parametersName.push_back("round");
        parametersName.push_back("whitenoise");
        for (std::size_t agent_i(0); agent_i < agentsNumber; agent_i++){
            parametersName.push_back("agent" + std::to_string(agent_i+1) + "Action");
            parametersName.push_back("agent" + std::to_string(agent_i+1) + "Profit");
            parametersName.push_back("agent" + std::to_string(agent_i+1) + "EstimatedProfit");
        }
        
        akml::DynamicMatrixSave<float>::default_parameters_name = parametersName;
        
        LogManager.reserve(std::min((std::size_t)2001, maxIterations));
    };
    
    inline ~SimulationManager(){
        if (leaders != nullptr)
            delete leaders;
    }
    
    inline void setGamma(const float gamma){ LearningAgent::gamma = gamma; }
    inline void setDecayrate(const float dr){ LearningAgent::decayRate = dr; }
    inline void setWNMethod(const std::function<float(std::size_t, std::size_t)> method){ whitenoiseMethod = method; }
    
    inline float computeProfit(float p, float q){ return (float)( K * ( std::max(0.f, p) * q - cost(q)) + B  ); }
    
    void processSimulation();
};

#endif /* SimulationManager_hpp */
