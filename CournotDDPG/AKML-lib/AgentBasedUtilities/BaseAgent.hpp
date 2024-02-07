//
//  BaseAgent.hpp
//  AKML Project
//
//  Created by Aldric Labarthe on 05/11/2023.
//

#ifndef AKML_BaseAgent_hpp
#define AKML_BaseAgent_hpp

#include <stdio.h>

#include "../AKML.hpp"

namespace akml {

    class BaseAgent {
    protected:
        akml::NeuralNetwork<>** brainNN;
        
    public:
        const unsigned int akml_agent_id;

        BaseAgent(unsigned int id, akml::NeuralNetwork<>* NN) : akml_agent_id(id) {
            brainNN = new akml::NeuralNetwork<>* (NN);
        };
        
        void editNNPointer(akml::NeuralNetwork<>* NN);
        akml::NeuralNetwork<>* getNNAccess();
    
    };
}

#endif /* BaseAgent_hpp */
