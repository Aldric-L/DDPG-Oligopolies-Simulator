//
//  BaseAgent.cpp
//  AKML Project
//
//  Created by Aldric Labarthe on 05/11/2023.
//

#include "BaseAgent.hpp"

namespace akml {

    void BaseAgent::editNNPointer(akml::NeuralNetwork* NN) {
        brainNN = new akml::NeuralNetwork* (NN);
    }

    akml::NeuralNetwork* BaseAgent::getNNAccess() { return *brainNN; }

}
