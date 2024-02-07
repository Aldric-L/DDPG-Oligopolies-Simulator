//
//  NeuralLayer.hpp
//  AKML Project
//
//  Created by Aldric Labarthe on 09/09/2023.
//

#ifndef NeuralLayer_hpp
#define NeuralLayer_hpp

#include "Matrices.hpp"
#include "NeuralFunctions.cpp"

namespace akml {

class NeuralLayer {
protected:
    DynamicMatrix<float> ownActivationLayer;
    //DynamicMatrix<float> previousActivationLayer;
    DynamicMatrix<float> weights;
    DynamicMatrix<float> biases;
    DynamicMatrix<float> errorLayer;
    
    DynamicMatrix<float>* ownPreActivationLayer;
    
    NeuralLayer* previousLayer;
    NeuralLayer* nextLayer;
    const akml::ActivationFunction<float>* activationFunction;
    std::size_t neuronNumber, previousNeuronNumber, layerId;
    
public:
    inline NeuralLayer(const std::size_t layerId, const std::size_t neuronNumber, NeuralLayer* prevLayer=nullptr) : previousLayer(prevLayer), 
        layerId(layerId),
        neuronNumber(neuronNumber),
        ownActivationLayer(neuronNumber, 1),
        biases(neuronNumber, 1),
        previousNeuronNumber((prevLayer != nullptr) ? prevLayer->getNeuronNumber() : 1),
        weights(neuronNumber, (prevLayer != nullptr) ? prevLayer->getNeuronNumber() : 1),
        activationFunction(nullptr),
        //previousActivationLayer((prevLayer != nullptr) ? prevLayer->getNeuronNumber() : 1, 1),
        ownPreActivationLayer(nullptr),
        nextLayer(nullptr),
        errorLayer(neuronNumber, 1){
        weights.transform([](float x) {return 1;});
    }
    
    inline NeuralLayer(NeuralLayer& other) = default;
    
    inline ~NeuralLayer(){
        if (ownPreActivationLayer != nullptr)
            delete ownPreActivationLayer;
    }
    
    inline std::size_t getNeuronNumber(){ return neuronNumber; }
    inline std::size_t getPreviousNeuronNumber(){ return previousNeuronNumber; }
    inline void setActivationFunction(const akml::ActivationFunction<float>* actfunc){ activationFunction = actfunc; }
    inline const akml::ActivationFunction<float>* getActivationFunction(){ return activationFunction; }
    
    inline void setInput(const DynamicMatrix<float>& input) {
        if (previousLayer != nullptr)
            throw std::invalid_argument("You should not set an input to a layer that is not the first one.");
        
        ownActivationLayer = input;
    }
    
    inline void setBiases(const DynamicMatrix<float>& new_biases) { biases = new_biases; }
    inline void setWeights(const DynamicMatrix<float>& new_weights) { weights = new_weights; }
    //inline void setPreviousActivationLayer(const DynamicMatrix<float>& prev) { previousActivationLayer = prev; }
    inline void setNextLayer(NeuralLayer* next) { nextLayer = next; }

    inline DynamicMatrix<float>& getBiasesAccess(){ return biases; }
    inline DynamicMatrix<float> getBiases(){ return biases; }
    inline DynamicMatrix<float>& getWeightsAccess(){ return weights; }
    inline DynamicMatrix<float> getWeights(){ return weights; }
    inline const DynamicMatrix<float>* getPreActivationLayer(){ return ownPreActivationLayer; }
    //inline DynamicMatrix<float> getCachedPreviousActivationLayer (){ return previousActivationLayer; }
    
    inline DynamicMatrix<float> getActivationLayer(){
        if (previousLayer != nullptr){
            if (ownPreActivationLayer != nullptr)
                delete ownPreActivationLayer;
            
            ownPreActivationLayer = new akml::DynamicMatrix<float> (akml::matrix_product(weights, previousLayer->getActivationLayer()) + biases);
            
            if (activationFunction == nullptr)
                throw std::invalid_argument("No activation function provided");
            ownActivationLayer = akml::transform(*ownPreActivationLayer, activationFunction->function);
        }
        return ownActivationLayer;
    }
    
    inline DynamicMatrix<float> computeLayerError(const akml::DynamicMatrix<float>& errorGrad){
        if (nextLayer != nullptr && previousLayer != nullptr){
            // Intermediary layers
            errorLayer = akml::hadamard_product(akml::matrix_product(akml::transpose(nextLayer->getWeights()), nextLayer->computeLayerError(errorGrad)), akml::transform((ownPreActivationLayer != nullptr) ? *ownPreActivationLayer : akml::matrix_product(weights, previousLayer->getActivationLayer()) + biases, activationFunction->derivative));
        }else if (previousLayer == nullptr){
            // First layer
            errorLayer = akml::hadamard_product(akml::matrix_product(akml::transpose(nextLayer->getWeights()), nextLayer->computeLayerError(errorGrad)), (activationFunction == nullptr) ? ownActivationLayer : akml::transform(ownActivationLayer,  activationFunction->derivative));
        }else {
            // Last layer
            errorLayer = akml::hadamard_product(errorGrad, akml::transform((ownPreActivationLayer != nullptr) ? *ownPreActivationLayer : akml::matrix_product(weights, previousLayer->getActivationLayer()) + biases, activationFunction->derivative));
        }
        //std::cout << "\n Layer " << layerId << " \n";
        //std::cout << errorLayer;
        return errorLayer;
    }
};



}

#endif /* NeuralLayer_hpp */
