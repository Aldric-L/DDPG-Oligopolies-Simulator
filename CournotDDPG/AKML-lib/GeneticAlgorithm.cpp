//
//  GeneticAlgorithm.cpp
//  AKML Project
//
//  Created by Aldric Labarthe on 13/09/2023.
//

#include "GeneticAlgorithm.hpp"

namespace akml {

static akml::GeneticAlgorithm::postactivation_process_type akml::GeneticAlgorithm::ACTIVATE_ROUND = [](DynamicMatrix <float> row_output) {
    for (std::size_t outputIncr(1); outputIncr <= row_output.getNRows(); outputIncr++){
        row_output(outputIncr, 1) = std::roundf(row_output(outputIncr, 1));
    }
    return row_output;
};

static akml::GeneticAlgorithm::merging_process_type akml::GeneticAlgorithm::DEFAULT_MERGING_INSTRUCTIONS = [](akml::NeuralNetwork* child, akml::NeuralNetwork* parent1, akml::NeuralNetwork* parent2) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<double> distribution(0.0,GeneticAlgorithm::sigma);
    
    for (std::size_t layer_id(1); layer_id < child->getLayerNb(); layer_id++){
        akml::NeuralLayer* parent1_layer = parent1->getLayer(layer_id);
        akml::NeuralLayer* parent2_layer = parent2->getLayer(layer_id);
        akml::NeuralLayer* child_layer = child->getLayer(layer_id);
        
        if (parent2_layer == parent1_layer){
            child_layer->setWeights(parent2_layer->getWeights());
            child_layer->setBiases(parent2_layer->getBiases());
            return;
        }
        
        for (std::size_t row(0); row < child_layer->getNeuronNumber(); row++){
            // Biases :
            if (row %2 == 0)
                child_layer->getBiasesAccess().operator()(row+1, 1) = parent1_layer->getBiasesAccess().operator()(row+1, 1);
            else
                child_layer->getBiasesAccess().operator()(row+1, 1) = parent2_layer->getBiasesAccess().operator()(row+1, 1);
            child_layer->getBiasesAccess().operator()(row+1, 1) = child_layer->getBiasesAccess().operator()(row+1, 1) * (1+distribution(gen));
            
            // weights :
            for (std::size_t col(0); col < child_layer->getPreviousNeuronNumber(); col++){
                if ((row+col) %2 == 0)
                    child_layer->getWeightsAccess().operator()(row+1, col+1) = parent1_layer->getWeightsAccess().operator()(row+1, col+1);
                else
                    child_layer->getWeightsAccess().operator()(row+1, col+1) = parent2_layer->getWeightsAccess().operator()(row+1, col+1);
                
                child_layer->getWeightsAccess().operator()(row+1, col+1) = child_layer->getWeightsAccess().operator()(row+1, col+1) * (1+distribution(gen));
            }
            
        }
    }
};


void akml::GeneticAlgorithm::generateNewGeneration(const std::vector< std::pair<float, akml::NeuralNetwork*> >& MSE, std::size_t iteration,
                                  const akml::GeneticAlgorithm::merging_process_type merging_instructions) {
    std::vector<akml::NeuralNetwork*> newNetworksPopulation;
    
    if (MSE.size() != pop_size)
        throw std::exception();
    
    float mean_MSE(0);
    for (std::size_t i(0); i < newcomers_bound; i++){
        NeuralNetwork* newnn = new NeuralNetwork(networksAllocationList, std::string("RANDOM_") + std::to_string(iteration));
        newNetworksPopulation.push_back(std::move(newnn));
        mean_MSE += MSE.at(i).first;
    }
    for (std::size_t i(newcomers_bound); i < reproduction_bound; i++){
        NeuralNetwork* newnn = new NeuralNetwork(networksAllocationList, std::string("CHILD_") + std::to_string(iteration));
        newNetworksPopulation.push_back(std::move(newnn));
        merging_instructions(newNetworksPopulation.at(i),
                             MSE.at(pop_size-i-1).second,
                             MSE.at(pop_size-i).second);
        mean_MSE += MSE.at(i).first;
    }
    for (std::size_t i(reproduction_bound); i < pop_size; i++){
        newNetworksPopulation.push_back(MSE.at(i).second);
        mean_MSE += MSE.at(i).first;
    }
    mean_MSE = mean_MSE/pop_size;
    std::cout << "Best MSE = " << MSE.at(pop_size-1).first << " - Mean=" << mean_MSE << " - Origin: " << newNetworksPopulation.at(pop_size-1)->getCustomOriginField() << std::endl;

    for (std::size_t i(0); i < pop_size; i++){
        if (i < reproduction_bound && MSE[i].second != nullptr)
            delete MSE[i].second;
        networksPopulation[i] = newNetworksPopulation[i];
    }
}

akml::NeuralNetwork* akml::GeneticAlgorithm::trainNetworks(const std::size_t iterations,
                                    const std::vector<akml::DynamicMatrix<float>>& inputs,
                                    const std::vector<akml::DynamicMatrix<float>>& outputs,
                                    const akml::ErrorFunction<float, akml::DynamicMatrix<float>>& evalfunc,
                                    const akml::GeneticAlgorithm::merging_process_type& merging_instructions) {
    if (inputs.size() != outputs.size())
        throw std::invalid_argument("Dimension error, training set irregular.");
    
    for (std::size_t iteration(1); iteration <= iterations; iteration++){
        std::vector<std::vector<akml::DynamicMatrix<float>>> localoutputs;
        std::vector<std::pair<float, NeuralNetwork*>> MSE;
        MSE.reserve(pop_size);
        for (std::size_t netid(0); netid < pop_size; netid++){
            float MSE_i = 0;
            std::vector<akml::DynamicMatrix<float>> local;
            local.reserve(pop_size);
            for (std::size_t inputid(0); inputid < inputs.size(); inputid++){
                local.emplace_back(std::move((this->postactivation_process != nullptr) ? this->postactivation_process->operator()(this->networksPopulation[netid]->process(inputs[inputid])) : this->networksPopulation[netid]->process(inputs[inputid])));
            }
            MSE_i = evalfunc.sumfunction(outputs, local);
            localoutputs.emplace_back(std::move(local));
            MSE.emplace_back(MSE_i, this->networksPopulation[netid]);
        }
        std::sort(MSE.begin(), MSE.end(), [](const std::pair<float, NeuralNetwork*> &x, const std::pair<float, NeuralNetwork*> &y){
            return x.first > y.first;
        });
        this->generateNewGeneration(MSE, iteration, merging_instructions);
        
        if (MSE[pop_size-1].first == 0.f){
            std::cout << "Best NN trained in " << iteration << " iterations (max allowed: " << iterations << ")\n";
            break;
        }
    }
    
    return this->networksPopulation[pop_size-1];
}

}
