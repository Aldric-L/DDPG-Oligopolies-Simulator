//
//  GeneticAlgorithm.hpp
//  AKML Project
//
//  Created by Aldric Labarthe on 13/09/2023.
//
#ifndef GeneticAlgorithm_hpp
#define GeneticAlgorithm_hpp

#include <iostream>
#include <stdio.h>
#include <cmath>
#include <functional>
#include <array>
#include <vector>
#include <algorithm>
#include <random>
#include <string>
#include <utility>

#include "AKML_consts.hpp"
#include "NeuralNetwork.hpp"
#include "NeuralLayer.hpp"
#include "Matrices.hpp"

namespace akml {

class GeneticAlgorithm {
public:
    typedef std::function<DynamicMatrix<float>(DynamicMatrix<float>)> postactivation_process_type;
    typedef std::function<void(NeuralNetwork*, NeuralNetwork*, NeuralNetwork*)> merging_process_type;
    static inline double sigma = 0.25;
    
protected:
    std::size_t pop_size;
    std::vector<NeuralNetwork*> networksPopulation;
    std::vector<std::pair<std::size_t, const akml::ActivationFunction<float>*>> networksAllocationList;
    postactivation_process_type* postactivation_process = nullptr;
    
    double newcomers_rate = (double)1 / 12;
    std::size_t newcomers_bound;
    //double reproduction_rate = (double)7 / 8 - newcomers_rate;
    double reproduction_rate = (double)9 / 10 - newcomers_rate;
    std::size_t reproduction_bound;
    double conservation_rate = 1 - newcomers_rate - reproduction_rate;
    std::size_t conservation_bound = pop_size;
    
public:
    static inline postactivation_process_type ACTIVATE_ROUND = [](DynamicMatrix <float> row_output) {
        for (std::size_t outputIncr(1); outputIncr <= row_output.getNRows(); outputIncr++){
            row_output(outputIncr, 1) = std::roundf(row_output(outputIncr, 1));
        }
        return row_output;
    };
    
    static inline merging_process_type DEFAULT_MERGING_INSTRUCTIONS = [](akml::NeuralNetwork* child, akml::NeuralNetwork* parent1, akml::NeuralNetwork* parent2) {
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
    
    inline GeneticAlgorithm(std::size_t pop_size, akml::NeuralNetwork::initialize_list_type instructions) : networksAllocationList(instructions), pop_size(pop_size), conservation_bound(pop_size), networksPopulation(pop_size) {
        for (std::size_t i(0); i < pop_size; i++){
            networksPopulation[i] = new NeuralNetwork (instructions, std::string("RANDOM_INIT"));
        }
        
        newcomers_bound = std::round(newcomers_rate * pop_size);
        reproduction_bound = std::round((reproduction_rate + newcomers_rate) * pop_size);
    }
    
    inline ~GeneticAlgorithm() {
        for (std::size_t i(0); i < pop_size; i++){
            if (networksPopulation[i] != nullptr)
                delete networksPopulation[i];
        }
    };
    
    inline void setNewGenerationRates(double newcomers_r, double reproduction_r){
        reproduction_rate = reproduction_r;
        newcomers_rate = newcomers_r;
        conservation_rate = 1 - newcomers_rate - reproduction_rate;
        newcomers_bound = std::max(std::round(newcomers_rate * pop_size), (double)1);
        reproduction_bound = std::round((reproduction_rate + newcomers_rate) * pop_size);
    }
    
    inline void setPostActivationProcess(postactivation_process_type* func){ postactivation_process = func; }
    
    inline std::vector<NeuralNetwork*> getNetworksPopulation() { return networksPopulation; }
    
    inline std::vector<DynamicMatrix<float>> evaluateNN(const DynamicMatrix<float>& input){
        std::vector<DynamicMatrix<float>> localoutputs;
        localoutputs.reserve(pop_size);
        for (std::size_t netid(0); netid < pop_size; netid++){
            localoutputs.emplace_back(std::move((this->postactivation_process != nullptr) ? postactivation_process->operator()(networksPopulation[netid]->process(input)) : networksPopulation[netid]->process(input)));
        }
        return localoutputs;
    };
    
    inline DynamicMatrix<float> evaluateOneNN(const std::size_t netid, const DynamicMatrix<float>& input){
        return (this->postactivation_process != nullptr) ? postactivation_process->operator()(networksPopulation[netid]->process(input)) : networksPopulation[netid]->process(input);
    };

    
    /*
     MSE should be sorted from worst to best.
     */
    inline void generateNewGeneration(const std::vector< std::pair<float, NeuralNetwork*> >& MSE, std::size_t iteration,
                                      const merging_process_type merging_instructions = DEFAULT_MERGING_INSTRUCTIONS) {
        std::vector<NeuralNetwork*> newNetworksPopulation;
        
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
        if (MSE.at(pop_size-1).first < 0)
            std::cout << "OUPS";
        for (std::size_t i(0); i < pop_size; i++){
            if (i < reproduction_bound && MSE[i].second != nullptr)
                delete MSE[i].second;
            networksPopulation[i] = newNetworksPopulation[i];
        }
    }
    
    
    /*
     Choices about generation of new population :
     Flop 1/12 is replaced by new networks generated randomly (introduction of immigration)
     Top (7/8-1/12) is selected and multiplied so that their offspring represents less than 2/3 of new generation
     The remainder is generated using former top players
     */
    
    inline NeuralNetwork* trainNetworks(const std::size_t iterations,
                                        const std::vector<akml::DynamicMatrix<float>>& inputs,
                                        const std::vector<akml::DynamicMatrix<float>>& outputs,
                                        const akml::ErrorFunction<float, akml::DynamicMatrix<float>>& evalfunc = akml::ErrorFunctions::MSE,
                                        const merging_process_type& merging_instructions = DEFAULT_MERGING_INSTRUCTIONS) {
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
                if (MSE_i < 0){
                    std::cout << "OUPS";
                    for (std::size_t inputid(0); inputid < inputs.size(); inputid++){
                        std::cout << local[inputid];
                    }
                    evalfunc.sumfunction(outputs, local);
                }
                    
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
    
    
};

}

#endif /* GeneticAlgorithm_hpp */
