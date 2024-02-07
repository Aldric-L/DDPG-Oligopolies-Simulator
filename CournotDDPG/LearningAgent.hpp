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
    static inline unsigned short int preLearningExpriences = 50;
    static inline std::size_t maxBufferSize = 1000;
    static inline std::size_t maxEpochsAtLearningTime = 50;
    static inline std::size_t batchSize = 15;
    static inline float learningRate = 0.1;
    static inline float gradientTolerance = 1e-05;
    static inline float gamma = 0.1;
    static inline float polyakCoef = 0.8;
    
    
protected:
    unsigned short int accumulatedExperiences;
    akml::NeuralNetwork QNet, target_QNet, policyNet, target_policyNet;
    std::deque <LearningAgent::replay> replayBuffer;
    std::size_t learningEpochsCompleted = 0;
    
    const akml::NeuralNetwork::initialize_list_type QNetInitList = {{ { 2, nullptr }, { 6, &akml::ActivationFunctions::RELU }, { 6, &akml::ActivationFunctions::RELU }, { 1, &akml::ActivationFunctions::SIGMOID } }};
    
    const akml::NeuralNetwork::initialize_list_type PolicyNetInitList = {{ { 1, nullptr }, { 6, &akml::ActivationFunctions::RELU }, { 6, &akml::ActivationFunctions::RELU }, { 1, &akml::ActivationFunctions::SIGMOID } }};
    
public:
    LearningAgent() : QNet(4), policyNet(4), accumulatedExperiences(0), target_QNet(4), target_policyNet(4) {
        QNet.construct(QNetInitList); policyNet.construct(PolicyNetInitList);
        target_QNet = QNet; target_policyNet = policyNet;
    }
    
    inline float play(float state, bool white_noise=true) {
        std::random_device rd;
        std::mt19937 gen(rd());
        // first firm plays randomly (it decide a quantity between 1 and 100
        std::normal_distribution<float> distribution(0,0.1);
        return std::max((float) std::min((float) policyNet.process(akml::make_dynamic_vector<float>(state)).read(1, 1) + (white_noise ? (float)distribution(gen) : 0), 1.f), 0.f); }
    
    inline void feedBack(float prevState, float agentAction, float reward, float nextState, bool isTerminated) {
        if (replayBuffer.size() > maxBufferSize){
            replayBuffer.pop_front();
        }
        replayBuffer.emplace_back(prevState, agentAction, reward, nextState, isTerminated);
        if (accumulatedExperiences >= preLearningExpriences){
            this->train();
            accumulatedExperiences = 0;
        }else {
            accumulatedExperiences++;
        }
    }
    
    void train();
    
};

#endif /* LearningAgent_hpp */
