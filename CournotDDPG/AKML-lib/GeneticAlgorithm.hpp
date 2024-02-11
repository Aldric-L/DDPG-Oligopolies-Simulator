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
    std::vector<akml::NeuralNetwork*> networksPopulation;
    std::vector<std::pair<std::size_t, const akml::ActivationFunction<float>*>> networksAllocationList;
    postactivation_process_type* postactivation_process = nullptr;
    
    double newcomers_rate = (double)1 / 12;
    std::size_t newcomers_bound;
    double reproduction_rate = (double)9 / 10 - newcomers_rate;
    std::size_t reproduction_bound;
    double conservation_rate = 1 - newcomers_rate - reproduction_rate;
    std::size_t conservation_bound = pop_size;
    
public:
    static postactivation_process_type ACTIVATE_ROUND;
    static merging_process_type DEFAULT_MERGING_INSTRUCTIONS;
    
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
    void generateNewGeneration(const std::vector< std::pair<float, NeuralNetwork*> >& MSE, std::size_t iteration,
                                      const merging_process_type merging_instructions = DEFAULT_MERGING_INSTRUCTIONS);
    
    
    /*
     Choices about generation of new population :
     Flop 1/12 is replaced by new networks generated randomly (introduction of immigration)
     Top (7/8-1/12) is selected and multiplied so that their offspring represents less than 2/3 of new generation
     The remainder is generated using former top players
     */
    akml::NeuralNetwork* trainNetworks(const std::size_t iterations,
                                        const std::vector<akml::DynamicMatrix<float>>& inputs,
                                        const std::vector<akml::DynamicMatrix<float>>& outputs,
                                        const akml::ErrorFunction<float, akml::DynamicMatrix<float>>& evalfunc = akml::ErrorFunctions::MSE,
                                              const merging_process_type& merging_instructions = DEFAULT_MERGING_INSTRUCTIONS);
    
    
};

}

#endif /* GeneticAlgorithm_hpp */
