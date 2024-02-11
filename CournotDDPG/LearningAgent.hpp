//
//  LearningAgent.hpp
//  CournotDDPG
//
//  Created by Aldric Labarthe on 04/02/2024.
//

#ifndef LearningAgent_hpp
#define LearningAgent_hpp

#include <stdio.h>
#include <deque>
#include <algorithm>
#include <tuple>
#include "AKML-lib/NeuralNetwork.hpp"
#include "AKML-lib/NeuralFunctions.hpp"

class LearningAgent {
public:
    struct replay {float prevState; float agentAction; float reward; float nextState; bool isTerminated;
        replay(float prevState, float agentAction, float reward, float nextState, bool isTerminated) : prevState(prevState), agentAction(agentAction), reward(reward), nextState(nextState), isTerminated(isTerminated) {}; };
    static inline unsigned short int preLearningExpriences = 126;
    static inline std::size_t maxBufferSize = 2000;
    static inline std::size_t maxEpochsAtLearningTime = 25;
    //static inline std::size_t batchSize = 15;
    static inline std::size_t batchSize = 64;
    static inline std::size_t batchNumber = 32;
    static inline float learningRateActor = 0.015;
    static inline float learningRateCritic = 0.1;
    static inline float gradientTolerance = 1e-08;
    static inline float gamma = 0.1;
    static inline float polyakCoef = 0.95;
    
    
protected:
    std::string name;
    unsigned short int accumulatedExperiences;
    akml::NeuralNetwork QNet, target_QNet, policyNet, target_policyNet;
    std::deque <LearningAgent::replay> replayBuffer;
    // We create some buffers to prepare inputs for our NNs
    std::deque <akml::DynamicMatrix<float>> rBufferInputs, rBufferInputsStates;
    std::size_t learningEpochsCompleted = 0;
    
    const akml::NeuralNetwork::initialize_list_type QNetInitList = {{ { 2, nullptr }, { 8, &akml::ActivationFunctions::RELU }, { 8, &akml::ActivationFunctions::RELU }, { 1, &akml::ActivationFunctions::SIGMOID } }};
    
    const akml::NeuralNetwork::initialize_list_type PolicyNetInitList = {{ { 1, nullptr }, { 8, &akml::ActivationFunctions::RELU }, { 8, &akml::ActivationFunctions::RELU }, { 1, &akml::ActivationFunctions::SIGMOID } }};
    
    void train();
    
public:
    LearningAgent(std::string name="") : QNet(4), policyNet(4), accumulatedExperiences(0), target_QNet(4), target_policyNet(4), name(std::move(name)) {
        QNet.construct(QNetInitList); policyNet.construct(PolicyNetInitList);
        target_QNet = QNet; target_policyNet = policyNet;
    }
    
    inline float play(float state, float white_noise=0.f) {
        if (white_noise != 0){
            std::random_device rd;
            std::mt19937 gen(rd());
            std::normal_distribution<float> distribution(0,white_noise);
            return std::max((float) std::min((float) policyNet.process(akml::make_dynamic_vector<float>(state)).read(1, 1) + (float)distribution(gen), 1.f), 0.f);
        }
        return policyNet.process(akml::make_dynamic_vector<float>(state)).read(1, 1);
    }
    
    inline void feedBack(float prevState, float agentAction, float reward, float nextState, bool isTerminated, bool autoTrain=true) {
        if (replayBuffer.size() > maxBufferSize){
            replayBuffer.pop_front(); rBufferInputs.pop_front(); rBufferInputsStates.pop_front();
        }
        replayBuffer.emplace_back(prevState, agentAction, reward, nextState, isTerminated);
        rBufferInputs.emplace_back(akml::make_dynamic_vector<float>( prevState, agentAction ));
        rBufferInputsStates.emplace_back(akml::make_dynamic_vector<float>( prevState ));
        if (autoTrain) {
            if (accumulatedExperiences >= preLearningExpriences){
                this->train();
                accumulatedExperiences = 0;
            }else {
                accumulatedExperiences++;
            }
        }
        
    }
    
    void manualTrainingLaunch(){
        if (accumulatedExperiences >= preLearningExpriences)
            accumulatedExperiences = 0;
        this->train();
    }
    
    // Debug functions
    float askCritic(float state, float action){
        return QNet.process(akml::make_dynamic_vector<float>(state, action)).read(1,1);
    }
    
};

#endif /* LearningAgent_hpp */
