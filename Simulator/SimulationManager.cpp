//
//  SimulationManager.cpp
//  DDPG Oligopolies Simulator
//
//  Created by Aldric Labarthe on 28/02/2024.
//

#include "SimulationManager.hpp"

std::string SimulationManager::processSimulation(bool mute) {
    struct agentLog { float prevState, action, profit; };
    float prevprice = 0.f;
    std::vector<std::vector<agentLog>> buffer (128); //buffer.reserve(128);
    std::thread logBufferize;
    std::vector<std::thread> agentThreads;
    agentThreads.reserve(agentsNumber);
    for (std::size_t iteration(0); iteration <= maxIterations/128; iteration++){
        for (std::size_t inside_iteration(0); inside_iteration < 128; inside_iteration++){
            buffer.at(inside_iteration).resize(agentsNumber);
            akml::DynamicMatrix<float> iterationLogSynthesis(2 + agentsNumber * 3, 1);
            // IterationLogSynthesis: (akml::DynamicMatrix (3*agentsNumber+2)x1)
            // 0: iteration
            // 1: whitenoise
            // for i in agents
            // 2+3i: action_i
            // 3+3i: profit_i
            // 4+3i: estilatedProfit_i
            // endfor
            
            std::size_t it = inside_iteration + iteration*128;
            float whitenoise = ((it % 10 == 0) ? 0.f : maxWhiteNoise * (whitenoiseMethod(it, maxIterations)));
            iterationLogSynthesis[{0, 0}] = (float)it;
            iterationLogSynthesis[{1, 0}] = whitenoise;
            
            float totalQuantity = 0.f;
            float leadersQuantity = 0.f;
            float price = 0.f;
            
            switch (localSimulationType) {
                case STACKELBERG:
                    // First we process all leaders with qty input of 0
                    for (std::size_t agent_i(0); agent_i < agentsNumber; agent_i++){
                        // If agent_i is not leader... we skip
                        if (std::find(leaders.begin(), leaders.end(), agent_i) == leaders.end())
                            continue;
                        buffer.at(inside_iteration).at(agent_i).action = agents[agent_i]->play(0.f, whitenoise);
                        iterationLogSynthesis[{2+3*agent_i, 0}] = buffer.at(inside_iteration).at(agent_i).action;
                        buffer.at(inside_iteration).at(agent_i).prevState = 0.f;
                        totalQuantity += buffer.at(inside_iteration).at(agent_i).action;
                    }
                    leadersQuantity = totalQuantity;
                    for (std::size_t agent_i(0); agent_i < agentsNumber; agent_i++){
                        // If agent_i is leader... we skip
                        if (std::find(leaders.begin(), leaders.end(), agent_i) != leaders.end())
                            continue;
                        buffer.at(inside_iteration).at(agent_i).action = agents[agent_i]->play(leadersQuantity, whitenoise);
                        iterationLogSynthesis[{2+3*agent_i, 0}] = buffer.at(inside_iteration).at(agent_i).action;
                        buffer.at(inside_iteration).at(agent_i).prevState = leadersQuantity;
                        totalQuantity += buffer.at(inside_iteration).at(agent_i).action;
                    }
                    break;
                    
                case COURNOT:
                    for (std::size_t agent_i(0); agent_i < agentsNumber; agent_i++){
                        float totalOtherActions = 0.f;
                        if (buffer.size() >0){
                            for (std::size_t ai(0); ai < agentsNumber; ai++){
                                if (ai != agent_i)
                                    totalOtherActions += buffer.front().at(ai).action;
                            }
                        }
                        buffer.at(inside_iteration).at(agent_i).prevState = totalOtherActions;
                        buffer.at(inside_iteration).at(agent_i).action = agents[agent_i]->play(totalOtherActions, whitenoise);
                        iterationLogSynthesis[{2+3*agent_i, 0}] = buffer.at(inside_iteration).at(agent_i).action;
                        totalQuantity += buffer.at(inside_iteration).at(agent_i).action;
                    }
                    break;
                    
                case TEMPORAL_COURNOT:
                    for (std::size_t agent_i(0); agent_i < agentsNumber; agent_i++){
                        buffer.at(inside_iteration).at(agent_i).action = agents[agent_i]->play(prevprice, whitenoise);
                        iterationLogSynthesis[{2+3*agent_i, 0}] = buffer.at(inside_iteration).at(agent_i).action;
                        buffer.at(inside_iteration).at(agent_i).action = iterationLogSynthesis[{2+3*agent_i, 0}];
                        buffer.at(inside_iteration).at(agent_i).prevState = prevprice;
                        totalQuantity += iterationLogSynthesis[{2+3*agent_i, 0}];
                    }
                    break;
            }
            price = inverseDemand(totalQuantity);
            for (std::size_t agent_i(0); agent_i < agentsNumber; agent_i++){
                buffer.at(inside_iteration).at(agent_i).profit = computeProfit(price, buffer.at(inside_iteration).at(agent_i).action);
                iterationLogSynthesis[{3+3*agent_i, 0}] = buffer.at(inside_iteration).at(agent_i).profit;
                iterationLogSynthesis[{4+3*agent_i, 0}] = agents[agent_i]->askCritic(buffer.at(inside_iteration).at(agent_i).prevState, buffer.at(inside_iteration).at(agent_i).action);
            }
            prevprice = price;
            LogManager.addSave(akml::DynamicMatrixSave<float>(2 + agentsNumber * 3, std::move(iterationLogSynthesis)));
        }
        
        if (iteration%15 == 0)
            logBufferize = std::thread([](akml::CSV_Saver<akml::DynamicMatrixSave<float>>* saver) { saver->bufferize(); }, &LogManager);
        
        for (std::size_t inside_iteration(0); inside_iteration < 127; inside_iteration++){
            for (std::size_t agent_i(0); agent_i < agentsNumber; agent_i++){
                agents[agent_i]->feedBack(buffer.at(inside_iteration).at(agent_i).prevState, buffer.at(inside_iteration).at(agent_i).action, buffer.at(inside_iteration).at(agent_i).profit, buffer.at(inside_iteration+1).at(agent_i).prevState, inside_iteration == 126, false );
            }
        }
        for (std::size_t agent_i(0); agent_i < agentsNumber; agent_i++){
            agentThreads.emplace_back(&LearningAgent::manualTrainingLaunch, agents[agent_i], mute);
        }
        for (std::size_t agent_i(0); agent_i < agentsNumber; agent_i++){
            if (agentThreads.at(agent_i).joinable())
                agentThreads.at(agent_i).join();
        }
        if (logBufferize.joinable())
            logBufferize.join();
        agentThreads.clear();
        buffer.clear();
        buffer.resize(128);
    }
    std::string filename = "DDPG-" + getOligopolyName(localSimulationType) + "-" + std::to_string(agentsNumber);
    long int t = static_cast<long int> (std::clock());
    std::string curt = std::to_string(t);
    if (logBufferize.joinable())
        logBufferize.join();
    logBufferize = std::thread([curt, filename](akml::CSV_Saver<akml::DynamicMatrixSave<float>>* saver) { saver->saveToCSV(filename + "-" + curt + ".csv", false); }, &LogManager);

    if (logBufferize.joinable())
        logBufferize.join();
    
    return curt;
}

