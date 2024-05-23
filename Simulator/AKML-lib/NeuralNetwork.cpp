//
//  NeuralNetwork.cpp
//  AKML Project
//
//  Created by Aldric Labarthe on 11/02/2024.
//

#include "NeuralNetwork.hpp"

namespace akml {
void akml::NeuralNetwork::stochGradientTraining(const std::vector<akml::DynamicMatrix<float>> inputs_set,
                                  const std::vector<akml::DynamicMatrix<float>> outputs_set,
                                  const std::size_t batch_size,
                                  float learning_rate,
                                  const std::size_t max_epochs,
                                  const double tolerance,
                                  const akml::ErrorFunction<float, DynamicMatrix<float>>* errorFunc,
                                  const GRADIENT_METHODS method,
                                  const double decayFactor){
    
    if (inputs_set.size() != outputs_set.size())
        throw std::invalid_argument("Training set size irregular.");
    
    if (inputs_set.size() < batch_size)
        throw std::invalid_argument("Batch_size is too big.");
    
    std::size_t epochs(0);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<std::size_t> distribution(0,inputs_set.size()-batch_size);
    do {
        std::size_t start = distribution(gen);
        std::vector<akml::DynamicMatrix<float>> batch_grads;
        batch_grads.reserve(batch_size);
        
        std::vector<akml::DynamicMatrix<float>> temp_outputs;
        std::vector<akml::DynamicMatrix<float>> temp_outputs_expected;
        temp_outputs.reserve(batch_size);
        temp_outputs_expected.reserve(batch_size);
        for (std::size_t start_i(start); start_i < std::min(start+batch_size+1, inputs_set.size()-batch_size+1); start_i++){
            for (std::size_t input_id(start_i); input_id < start_i + batch_size; input_id++){
                akml::DynamicMatrix<float> r = this->process(inputs_set.at(input_id));
                akml::DynamicMatrix<float> errorGrad = errorFunc->local_derivative(r, outputs_set.at(input_id));
                temp_outputs.emplace_back(std::move(r));
                temp_outputs_expected.emplace_back(outputs_set.at(input_id));
                batch_grads.emplace_back(this->computeErrorGradient(errorGrad));
            }
            std::cout << "\nEPOCH " << epochs << " / " << max_epochs << ": MSE=" << errorFunc->sumfunction(temp_outputs, temp_outputs_expected) << " LR=" << learning_rate;
            temp_outputs.clear();
            temp_outputs_expected.clear();
            akml::DynamicMatrix<float> final_grad = akml::mean(batch_grads);
            final_grad = method * learning_rate * final_grad;
            if (learning_rate < tolerance){
                std::cout << "\n Inactivity detected. End training.";
                break;
            }
            this->mooveCoefficients(std::move(final_grad), tolerance);
            batch_grads.clear();
        }
        learning_rate *= decayFactor;
        epochs++;
    }while (epochs < max_epochs);
    
}

void akml::NeuralNetwork::adamGradientTraining(const std::vector<akml::DynamicMatrix<float>> inputs_set,
                                  const std::vector<akml::DynamicMatrix<float>> outputs_set,
                                  const std::size_t batch_size,
                                  const std::size_t max_epochs,
                                  double lr_moment1,
                                  double lr_moment2,
                                  double step_size,
                                  const double tolerance,
                                  const akml::ErrorFunction<float, DynamicMatrix<float>>* errorFunc,
                                  const GRADIENT_METHODS method,
                                  const double epsilon){
    
    if (inputs_set.size() != outputs_set.size() || inputs_set.size() < 2)
        throw std::invalid_argument("Training set size irregular.");
    
    if (inputs_set.size() < batch_size)
        throw std::invalid_argument("Batch_size is too big.");
    
    std::size_t epochs(0);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<std::size_t> distribution(0,inputs_set.size()-batch_size);
    
    akml::DynamicMatrix<float> firstMoment (this->computeErrorGradient(errorFunc->local_derivative(this->process(inputs_set.at(0)), outputs_set.at(0))));
    for (std::size_t i(0); i < firstMoment.getNRows(); i++)
        firstMoment[{i, 0}] = 0.f;
    akml::DynamicMatrix<float> secondMoment(firstMoment);
    akml::DynamicMatrix<float> correctedFirstMoment(firstMoment);
    akml::DynamicMatrix<float> correctedSecondMoment(firstMoment);
                                                                            
    do {
        std::size_t start = distribution(gen);
        std::vector<akml::DynamicMatrix<float>> batch_grads;
        batch_grads.reserve(batch_size);
        
        std::vector<akml::DynamicMatrix<float>> temp_outputs;
        std::vector<akml::DynamicMatrix<float>> temp_outputs_expected;
        temp_outputs.reserve(batch_size);
        temp_outputs_expected.reserve(batch_size);
        for (std::size_t start_i(start); start_i < std::min(start+batch_size+1, inputs_set.size()-batch_size+1); start_i++){
            for (std::size_t input_id(start_i); input_id < start_i + batch_size; input_id++){
                akml::DynamicMatrix<float> r = this->process(inputs_set.at(input_id));
                akml::DynamicMatrix<float> errorGrad = errorFunc->local_derivative(r, outputs_set.at(input_id));
                temp_outputs.emplace_back(std::move(r));
                temp_outputs_expected.emplace_back(outputs_set.at(input_id));
                batch_grads.emplace_back(this->computeErrorGradient(errorGrad));
            }
            std::cout << "\nEPOCH " << epochs << " / " << max_epochs << ": MSE=" << errorFunc->sumfunction(temp_outputs, temp_outputs_expected) << " LR1=" << lr_moment1 << " LR2=" << lr_moment2;
            temp_outputs.clear();
            temp_outputs_expected.clear();
            akml::DynamicMatrix<float> final_grad = akml::mean(batch_grads);
            
            firstMoment = lr_moment1 * firstMoment + (1.f - lr_moment1)*final_grad;
            secondMoment = lr_moment2 * secondMoment + (1.f - lr_moment2)*akml::hadamard_product(final_grad, final_grad);
            
            correctedFirstMoment = firstMoment * (1/(1-std::pow(lr_moment1,epochs)));
            correctedSecondMoment = secondMoment * (1/(1-std::pow(lr_moment2,epochs)));
            
            final_grad = method * step_size * akml::hadamard_division(correctedFirstMoment,
                                                                      akml::transform(correctedSecondMoment, (std::function<float(float)>) ([epsilon](float x) {return std::sqrt(x) + epsilon; })));
            
            this->mooveCoefficients(std::move(final_grad), tolerance);
            batch_grads.clear();
        }
        epochs++;
    }while (epochs < max_epochs);
}

akml::DynamicMatrix<float> akml::NeuralNetwork::computeErrorGradient(akml::DynamicMatrix<float> errorGrad){
    std::vector<float> coefs;
    for (std::size_t layer_id(1); layer_id < layers.size(); layer_id++){
        DynamicMatrix<float> temp = layers.at(layer_id)->computeLayerError(errorGrad);
        for (std::size_t row(0); row < layers.at(layer_id)->getWeightsAccess().getNRows(); row++){
            for (std::size_t col(0); col < layers.at(layer_id)->getWeightsAccess().getNColumns(); col++){
                coefs.push_back(layers.at(layer_id-1)->getActivationLayer()[{col, 0}] * temp[{row, 0}]);
            }
        }
        for (std::size_t row(0); row < temp.getNRows(); row++){
            coefs.push_back(temp[{row, 0}]);
        }
    }
    DynamicMatrix<float> result (coefs.size(), 1);
    result.forceByteCopy(&coefs[0]);
    return result;
}

akml::DynamicMatrix<float> akml::NeuralNetwork::computeGradient(){
    akml::DynamicMatrix<float> gradient (akml::make_identity<akml::DynamicMatrix<float>>(layers.back()->getNeuronNumber()));

    for (std::size_t lay(1); lay < layers.size(); lay++){
        if (layers.at(layers.size()-lay)->getPreActivationLayer() == nullptr)
            throw std::invalid_argument("The neuralNetwork need to have been asked with an input, before asking to compute gradient of the input.");
        
        if (layers.at(layers.size()-lay)->getActivationFunction() == nullptr)
            throw std::invalid_argument("The neuralNetwork has an hidden layer without activation function.");
            
        /*akml::DynamicMatrix<float> jacobian (akml::make_diagonal(akml::transform(*(layers.at(layers.size()-lay)->getPreActivationLayer()), layers.at(layers.size()-lay)->getActivationFunction()->derivative)));
        akml::DynamicMatrix<float> w (layers.at(layers.size()-lay)->getWeights());
        akml::DynamicMatrix<float> r (akml::matrix_product<float>(gradient, akml::matrix_product<float>(jacobian, w)));
        gradient.forceAssignement(r);*/
        
        gradient.forceAssignement(akml::matrix_product<float>(gradient, akml::matrix_product<float>(
        // Jacobian computation
        akml::make_diagonal(akml::transform(*(layers.at(layers.size()-lay)->getPreActivationLayer()), layers.at(layers.size()-lay)->getActivationFunction()->derivative)),
            layers.at(layers.size()-lay)->getWeights())));
        
    }
    return akml::transpose(gradient);
}

void akml::NeuralNetwork::mooveCoefficients(akml::DynamicMatrix<float>&& coefsdiff, const double tolerance){
    std::size_t incr(0);
    for (std::size_t layer_id(1); layer_id < layers.size(); layer_id++){
        for (std::size_t row(0); row < layers.at(layer_id)->getWeightsAccess().getNRows(); row++){
            for (std::size_t col(0); col < layers.at(layer_id)->getWeightsAccess().getNColumns(); col++){
                if (std::abs(coefsdiff[{incr, 0}]) > tolerance)
                    layers.at(layer_id)->getWeightsAccess()[{row, col}] += coefsdiff[{incr, 0}];
                incr++;
            }
        }
        for (std::size_t row(0); row < layers.at(layer_id)->getBiasesAccess().getNRows(); row++){
            if (std::abs(coefsdiff[{incr, 0}]) > tolerance)
                layers.at(layer_id)->getBiasesAccess()[{row, 0}] += coefsdiff[{incr, 0}];
            incr++;
        }
    }
}


void akml::NeuralNetwork::polyakMerge(const NeuralNetwork& othernet, float polyak_coef){
    if (othernet.getLayers().size() != this->getLayers().size())
        throw std::invalid_argument("NeuralNetworks can only be merged between similar networks.");
    
    if (polyak_coef > 1 || polyak_coef < 0)
        throw std::invalid_argument("Polyak coefficient out of range.");
    
    for (std::size_t layer_id(0); layer_id < othernet.getLayers().size(); layer_id++){
        for (std::size_t row(0); row < othernet.getLayer(layer_id)->getWeights().getNRows(); row++){
            for (std::size_t col(0); col < othernet.getLayer(layer_id)->getWeights().getNColumns(); col++){
                this->getLayer(layer_id)->getWeightsAccess()[{row, col}] = polyak_coef * othernet.getLayer(layer_id)->getWeights()[{row, col}] + (1 - polyak_coef) * this->getLayer(layer_id)->getWeights()[{row, col}];
            }
        }
        
        for (std::size_t row(0); row < othernet.getLayer(layer_id)->getBiases().getNRows(); row++){
            this->getLayer(layer_id)->getBiasesAccess()[{row, 0}] = polyak_coef * othernet.getLayer(layer_id)->getBiases()[{row, 0}] + (1 - polyak_coef) * this->getLayer(layer_id)->getBiases()[{row, 0}];
        }
    }
}
}
