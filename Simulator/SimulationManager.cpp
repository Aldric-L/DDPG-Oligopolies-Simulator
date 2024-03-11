//
//  SimulationManager.cpp
//  CournotDDPG
//
//  Created by Aldric Labarthe on 28/02/2024.
//

#include "SimulationManager.hpp"

void SimulationManager::processSimulation(bool mute) {
    float prevprice = 0.f;
    std::vector<std::vector<float>> buffer; buffer.reserve(128);
    std::thread logBufferize;
    std::vector<std::thread> agentThreads;
    agentThreads.reserve(agentsNumber);
    for (std::size_t iteration(0); iteration <= maxIterations/128; iteration++){
        buffer.clear();
        for (std::size_t inside_iteration(0); inside_iteration < 128; inside_iteration++){
            std::vector<float> iterationBuffer; iterationBuffer.reserve(2*agentsNumber+2);
            akml::DynamicMatrix<float> iterationLogSynthesis(2 + agentsNumber * 3, 1);
            std::size_t it = inside_iteration + iteration*128;
            float whitenoise = ((it % 10 == 0) ? 0.f : 0.3 * (whitenoiseMethod(it, maxIterations)));
            iterationLogSynthesis[{0, 0}] = (float)it;
            iterationLogSynthesis[{1, 0}] = whitenoise;
            
            float totalQuantity = 0.f;
            float leadersQuantity = 0.f;
            
            switch (localSimulationType) {
                case STACKELBERG:
                    for (std::size_t agent_i(0); agent_i < agentsNumber; agent_i++){
                        if (std::find(leaders->begin(), leaders->end(), agent_i) == leaders->end())
                            continue;
                        iterationLogSynthesis[{2+3*agent_i, 0}] = agents[agent_i].play(0, whitenoise);
                        iterationBuffer.push_back(iterationLogSynthesis[{2+3*agent_i, 0}]);
                        totalQuantity += iterationLogSynthesis[{2+3*agent_i, 0}];
                    }
                    leadersQuantity = totalQuantity;
                    for (std::size_t agent_i(0); agent_i < agentsNumber; agent_i++){
                        if (std::find(leaders->begin(), leaders->end(), agent_i) != leaders->end())
                            continue;
                        iterationLogSynthesis[{2+3*agent_i, 0}] = agents[agent_i].play(leadersQuantity, whitenoise);
                        iterationBuffer.push_back(iterationLogSynthesis[{2+3*agent_i, 0}]);
                        totalQuantity += iterationLogSynthesis[{2+3*agent_i, 0}];
                    }
                    break;
                    
                case COURNOT:
                    for (std::size_t agent_i(0); agent_i < agentsNumber; agent_i++){
                        float totalOtherActions = 0.f;
                        if (buffer.size() >0){
                            for (std::size_t ai(0); ai < agentsNumber; ai++){
                                if (ai != agent_i)
                                    totalOtherActions += buffer.front().at(ai);
                            }
                        }
                        iterationLogSynthesis[{2+3*agent_i, 0}] = agents[agent_i].play(totalOtherActions, whitenoise);
                        iterationBuffer.push_back(iterationLogSynthesis[{2+3*agent_i, 0}]);
                        totalQuantity += iterationLogSynthesis[{2+3*agent_i, 0}];
                    }
                    break;
                    
                case TEMPORAL_COURNOT:
                    for (std::size_t agent_i(0); agent_i < agentsNumber; agent_i++){
                        iterationLogSynthesis[{2+3*agent_i, 0}] = agents[agent_i].play(prevprice, whitenoise);
                        iterationBuffer.push_back(iterationLogSynthesis[{2+3*agent_i, 0}]);
                        totalQuantity += iterationLogSynthesis[{2+3*agent_i, 0}];
                    }
                    break;
            }
            float price = inverseDemand(totalQuantity);

            for (std::size_t agent_i(0); agent_i < agentsNumber; agent_i++){
                iterationLogSynthesis[{3+3*agent_i, 0}] = computeProfit(price, iterationLogSynthesis[{2+3*agent_i, 0}]);
                iterationBuffer.push_back(iterationLogSynthesis[{3+3*agent_i, 0}]);
                iterationLogSynthesis[{4+3*agent_i, 0}] = agents[agent_i].askCritic(prevprice, iterationLogSynthesis[{2+3*agent_i, 0}]);
            }
            iterationBuffer.push_back(prevprice);
            iterationBuffer.push_back(price);
            LogManager.addSave(akml::DynamicMatrixSave<float>(2 + agentsNumber * 3, std::move(iterationLogSynthesis)));
            
            if (it%2000 == 0)
                logBufferize = std::thread([](akml::CSV_Saver<akml::DynamicMatrixSave<float>>* saver) { saver->bufferize(); }, &LogManager);
        
            prevprice = price;
            buffer.emplace_back(std::move(iterationBuffer));
        }
        for (std::size_t inside_iteration(0); inside_iteration < 127; inside_iteration++){
            for (std::size_t agent_i(0); agent_i < agentsNumber; agent_i++){
                agents[agent_i].feedBack(A * buffer.at(inside_iteration).at(agentsNumber*2), buffer.at(inside_iteration).at(agent_i), buffer.at(inside_iteration).at(agentsNumber+agent_i), buffer.at(inside_iteration).back(), inside_iteration == 126, false );
            }
        }
        for (std::size_t agent_i(0); agent_i < agentsNumber; agent_i++){
            agentThreads.emplace_back(&LearningAgent::manualTrainingLaunch, &agents[agent_i], mute);
        }
        for (std::size_t agent_i(0); agent_i < agentsNumber; agent_i++){
            if (agentThreads.at(agent_i).joinable())
                agentThreads.at(agent_i).join();
        }
        if (logBufferize.joinable())
            logBufferize.join();
        agentThreads.clear();
    }
    std::string filename = "DDPG-";
    switch (localSimulationType) {
        case STACKELBERG:
            filename += "Stackelberg-";
            break;
            
        case COURNOT:
            filename += "Cournot-";
            break;
            
        case TEMPORAL_COURNOT:
            filename += "TemporalCournot-";
            break;
    }
    filename += std::to_string(agentsNumber);
    long int t = static_cast<long int> (std::clock());
    std::string curt = std::to_string(t);
    if (logBufferize.joinable())
        logBufferize.join();
    logBufferize = std::thread([curt, filename](akml::CSV_Saver<akml::DynamicMatrixSave<float>>* saver) { saver->saveToCSV(filename + "-" + curt + ".csv", false); }, &LogManager);

    if (logBufferize.joinable())
        logBufferize.join();
}
