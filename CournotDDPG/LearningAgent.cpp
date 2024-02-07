//
//  LearningAgent.cpp
//  CournotDDPG
//
//  Created by Aldric Labarthe on 04/02/2024.
//

#include "LearningAgent.hpp"

void LearningAgent::train() {
    std::vector<akml::DynamicMatrix<float>> inputs;
    std::vector<akml::DynamicMatrix<float>> inputs_actor;
    std::vector<akml::DynamicMatrix<float>> inputs_states;
    std::vector<akml::DynamicMatrix<float>> outputs_expected;
    inputs.reserve(replayBuffer.size()); outputs_expected.reserve(replayBuffer.size()); inputs_actor.reserve(replayBuffer.size()); inputs_states.reserve(replayBuffer.size());
    for (std::size_t replay_i(0); replay_i < replayBuffer.size(); replay_i++){
        inputs.emplace_back(akml::make_dynamic_vector<float>( replayBuffer.at(replay_i).prevState, replayBuffer.at(replay_i).agentAction ));
        inputs_actor.emplace_back(akml::make_dynamic_vector<float>( replayBuffer.at(replay_i).prevState, policyNet.process(akml::make_dynamic_vector<float>( replayBuffer.at(replay_i).prevState)).read(1,1) ));
        inputs_states.emplace_back(akml::make_dynamic_vector<float>( replayBuffer.at(replay_i).prevState ));
        outputs_expected.emplace_back(akml::make_dynamic_vector<float>(
            replayBuffer.at(replay_i).reward +
            gamma * (1 - replayBuffer.at(replay_i).isTerminated ) * target_QNet.process(
                akml::make_dynamic_vector<float>(
                    replayBuffer.at(replay_i).nextState,
                    target_policyNet.process(akml::make_dynamic_vector<float>(replayBuffer.at(replay_i).nextState)).read(1,1))).read(1,1)
        ));
    }
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<std::size_t> distribution(0,inputs.size()-1);
    std::vector<std::size_t> batch_elems; batch_elems.reserve(batchSize);
    
    // These vectors are about the QNet network (the critic network)
    std::vector<akml::DynamicMatrix<float>> temp_outputs; temp_outputs.reserve(batchSize);
    std::vector<akml::DynamicMatrix<float>> temp_outputs_expected; temp_outputs_expected.reserve(batchSize);
    std::vector<akml::DynamicMatrix<float>> batch_grads; batch_grads.reserve(batchSize);
    
    // This vector is about the Actor network optimization phase
    std::vector<akml::DynamicMatrix<float>> batch_critic_grads; batch_critic_grads.reserve(batchSize);
    for (std::size_t epoch(0); epoch < maxEpochsAtLearningTime; epoch++){
        for (std::size_t batch_i(0); batch_i < std::min(inputs.size() - batchSize, batchSize); batch_i++){
            batch_elems.clear();
            for (std::size_t i(0); i < batchSize; i++){
                std::size_t target(0);
                do {
                    target = distribution(gen);
                } while (std::find(batch_elems.begin(), batch_elems.end(), target) != batch_elems.end());
                batch_elems.push_back(target);
            }
            temp_outputs.clear(); temp_outputs_expected.clear(); batch_grads.clear(); batch_critic_grads.clear();
            
            for (std::size_t input_id(0); input_id < batchSize; input_id++){
                // Critic updating
                akml::DynamicMatrix<float> local_output = QNet.process(inputs.at(batch_elems.at(input_id)));
                akml::DynamicMatrix<float> errorGrad = akml::ErrorFunctions::MSE.local_derivative(local_output, outputs_expected.at(batch_elems.at(input_id)));
                temp_outputs.emplace_back(std::move(local_output));
                temp_outputs_expected.emplace_back(outputs_expected.at(batch_elems.at(input_id)));
                batch_grads.emplace_back(QNet.computeErrorGradient(errorGrad));
                
                // Actor updating
                QNet.process(inputs_actor.at(batch_elems.at(input_id)));
                batch_critic_grads.emplace_back(QNet.computeGradient().read(1,1) * inputs_states.at(batch_elems.at(input_id)));
            }
            
            // We got then 2 vectors of gradients, one for the actor (batch_critic_grads), one for the critic (batch_grads)
            std::cout << "\nEPOCH " << learningEpochsCompleted << " Batch " << batch_i << ": MSE=" << akml::ErrorFunctions::MSE.sumfunction(temp_outputs, temp_outputs_expected) << " LR=" << learningRate;
            
            akml::DynamicMatrix<float> final_actor_grad = akml::mean(batch_grads);
            akml::DynamicMatrix<float> final_critic_grad = akml::mean(batch_critic_grads);
            final_actor_grad = akml::NeuralNetwork::GRADIENT_METHODS::GRADIENT_DESCENT * learningRate * final_actor_grad;
            final_critic_grad = akml::NeuralNetwork::GRADIENT_METHODS::GRADIENT_ASCENT * learningRate * final_critic_grad;
            if (learningRate < gradientTolerance){
                std::cout << "\n Insufficient learning rate. End training."; break;
            }
            if (akml::arg_min(final_actor_grad, true) > gradientTolerance){
                QNet.mooveCoefficients(std::move(final_actor_grad), gradientTolerance);
            }
            if (akml::arg_min(final_critic_grad, true) > gradientTolerance){
                policyNet.mooveCoefficients(std::move(final_critic_grad), gradientTolerance);
            }
            
        }
        learningEpochsCompleted++;
    }
    target_policyNet.polyakMerge(policyNet, polyakCoef);
    target_QNet.polyakMerge(QNet, polyakCoef);
    
    
    
}
