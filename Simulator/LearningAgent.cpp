//
//  LearningAgent.cpp
//  DDPG Oligopolies Simulator
//
//  Created by Aldric Labarthe on 04/02/2024.
//

#include "LearningAgent.hpp"

void LearningAgent::train(bool mute) {
    auto renorm = [](float val) {
        if (!renormReward)
            return val;
        float l = 5;
        float inter = 2.f * l /3.f;
        return 1.f / (1.f + std::exp(-1.f * l * val + inter));
    };
    
    struct innerBatchInputs {
        akml::DynamicMatrix<float>* input;
        akml::DynamicMatrix<float>* input_actor;
        akml::DynamicMatrix<float>* input_critic;
        akml::DynamicMatrix<float>* output_expected;
    };
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<std::size_t> distribution(0,replayBuffer.size()-1);
    
    std::vector<std::size_t> batch_elems_id;
    std::vector<innerBatchInputs> batch_elems;
    std::size_t batchsNb = std::min(replayBuffer.size()/batchSize, batchNumber);
    batch_elems.reserve(batchsNb*batchSize);
    batch_elems_id.reserve(batchsNb*batchSize);
    
    // These vectors are about the QNet network (the critic network)
    std::vector<akml::DynamicMatrix<float>> temp_outputs; temp_outputs.reserve(batchSize);
    std::vector<akml::DynamicMatrix<float>> temp_outputs_expected; temp_outputs_expected.reserve(batchSize);
    std::vector<akml::DynamicMatrix<float>> batch_critic_grads; batch_critic_grads.reserve(batchSize);
    
    // This vector is about the Actor network optimization phase
    std::vector<akml::DynamicMatrix<float>> batch_actor_grads; batch_actor_grads.reserve(batchSize);
    auto noErrorGradInput = akml::make_dynamic_vector<float>(1.f);
    for (std::size_t epoch(0); epoch < maxEpochsAtLearningTime; epoch++){
        // At each epoch, we create a sample of minibatches
        batch_elems.clear(); batch_elems_id.clear();
        for (std::size_t i(0); i < batchsNb*batchSize; i++){
            std::size_t target(0);
            do {
                target = distribution(gen);
            } while (std::find(batch_elems_id.begin(), batch_elems_id.end(), target) != batch_elems_id.end());
            batch_elems_id.push_back(target);
            
            batch_elems.push_back({
                .input = &rBufferInputs.at(target),
                .input_actor = &rBufferInputsStates.at(target),
                .input_critic = new akml::DynamicMatrix<float> (akml::make_dynamic_vector<float>( replayBuffer.at(target).prevState, policyNet.process(akml::make_dynamic_vector<float>( replayBuffer.at(target).prevState)).read(1,1))),
                .output_expected = new akml::DynamicMatrix<float> (akml::make_dynamic_vector<float>(
                            renorm(replayBuffer.at(target).reward +
                            gamma * (1 - replayBuffer.at(target).isTerminated ) * target_QNet.process(
                                akml::make_dynamic_vector<float>(replayBuffer.at(target).nextState,
                                target_policyNet.process(akml::make_dynamic_vector<float>(replayBuffer.at(target).nextState)).read(1,1))).read(1,1))))
            });
        }
        
        // For each batch
        for (std::size_t batch_i(0); batch_i < batchsNb; batch_i++){
            // Used for MSE computation : temp_outputs.clear(); temp_outputs_expected.clear();
            batch_actor_grads.clear(); batch_critic_grads.clear();
            
            for (std::size_t input_id(0); input_id < batchSize; input_id++){
                std::size_t preid = batch_i * batchSize;
                // Critic updating
                //akml::DynamicMatrix<float> local_output = QNet.process(*batch_elems.at(input_id + preid).input);
                //akml::DynamicMatrix<float> errorGrad = akml::ErrorFunctions::MSE.local_derivative(local_output, *batch_elems.at(input_id + preid).output_expected);
                // Used for MSE computation : temp_outputs.emplace_back(std::move(local_output));
                // Used for MSE computation : temp_outputs_expected.emplace_back(*batch_elems.at(input_id + preid).output_expected);
                
                //batch_critic_grads.emplace_back(QNet.computeErrorGradient(errorGrad));
                // We try a more "concise" way with rvalues to avoid needless matrix copies
                batch_critic_grads.emplace_back(QNet.computeErrorGradient(akml::ErrorFunctions::MSE.local_derivative(QNet.process(*batch_elems.at(input_id + preid).input), *batch_elems.at(input_id + preid).output_expected)));
                
                // Actor updating
                QNet.process(*batch_elems.at(input_id + preid).input_critic);
                policyNet.process(*batch_elems.at(input_id + preid).input_actor);
                //std::cout << QNet.computeGradient();
                batch_actor_grads.emplace_back(QNet.computeGradient().read(2,1) *
                                               policyNet.computeErrorGradient(noErrorGradInput));
            }
            
            /*std::cout << "\nEPOCH ";
            if (name != "")
                std::cout << "(" << name << ") ";
            std::cout << learningEpochsCompleted << " Batch " << batch_i+1 << " / " << batchsNb << " : MSE=" << akml::ErrorFunctions::MSE.sumfunction(temp_outputs, temp_outputs_expected) << " LRc=" << learningRateCritic;*/
            
            akml::DynamicMatrix<float> final_actor_grad = akml::mean(batch_actor_grads);
            akml::DynamicMatrix<float> final_critic_grad = akml::mean(batch_critic_grads);
            final_actor_grad = akml::NeuralNetwork::GRADIENT_METHODS::GRADIENT_ASCENT * learningRateActor * final_actor_grad;
            final_critic_grad = akml::NeuralNetwork::GRADIENT_METHODS::GRADIENT_DESCENT * learningRateCritic * final_critic_grad;
            if (std::min(learningRateActor, learningRateCritic) < gradientTolerance){
                std::cout << "\n Insufficient learning rate. End training."; epoch=maxEpochsAtLearningTime-1;
                goto deleter;
            }
            policyNet.mooveCoefficients(std::move(final_actor_grad), gradientTolerance);
            QNet.mooveCoefficients(std::move(final_critic_grad), gradientTolerance);
        }
        learningEpochsCompleted++;
        learningRateActor = learningRateActor * decayRate;
        learningRateCritic = learningRateCritic * decayRate;
        deleter:
        for (std::size_t i(0); i < batch_elems.size(); i++){
            delete batch_elems.at(i).input_critic;
            delete batch_elems.at(i).output_expected;
        }
    }
    target_policyNet.polyakMerge(policyNet, polyakCoef);
    target_QNet.polyakMerge(QNet, polyakCoef);
    
    if (learningEpochsCompleted % 1000 && !mute){
        std::cout << "\nEPOCH ";
        if (name != "")
            std::cout << "(" << name << ") ";
        std::cout << learningEpochsCompleted /*<< " MSE=" << akml::ErrorFunctions::MSE.sumfunction(temp_outputs, temp_outputs_expected)*/ << " LRc=" << learningRateCritic << " LRa=" << learningRateActor;
    }
    
    
    
}
