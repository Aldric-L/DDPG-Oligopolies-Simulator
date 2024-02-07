//
//  NeuralFunctions.cpp
//  XORNeural network
//
//  Created by Aldric Labarthe on 27/01/2024.
//

#include "NeuralFunctions.hpp"

namespace akml {
    namespace ActivationFunctions {
        const struct ActivationFunction<float> SIGMOID = {
            .function = [](const float x) {return 1/(1+std::exp(-x));},
            .derivative = [](const float x) { return std::exp(-x)/std::pow(1+std::exp(-x),2); }
        };

        const struct ActivationFunction<float> RELU = {
            .function = [](const float x) {return std::max(0.f, x); },
            .derivative = [](const float x) { return (x > 0) ? 1.f : 0.f; }
        };

        const struct ActivationFunction<float> NO_ACTION = {
            .function = [](const float x) {return x;},
            .derivative = [](const float x) { return 1; }
        };
    }

    namespace ErrorFunctions {
        const struct ErrorFunction<float, akml::DynamicMatrix<float>> MSE = {
            .function = [](const akml::DynamicMatrix<float>& a, const akml::DynamicMatrix<float>& b) {
                return akml::inner_product(akml::transform<akml::DynamicMatrix<float>, float>(a-b, [](float x) { return (float)x*x; }));
            },
            .sumfunction = [](const std::vector<akml::DynamicMatrix<float>>& a, const std::vector<akml::DynamicMatrix<float>>& b) {
                if (a.size() != b.size())
                    throw std::invalid_argument("Dimension error");
                float MSE (0);
                
                for (std::size_t i(0); i < a.size(); i++){
                    MSE += akml::inner_product(akml::transform<akml::DynamicMatrix<float>, float>(a[i]-b[i], [](float x) { return (float)x*x; }));
                }
                return MSE/(float)(a.size());
            },
            .derivative = nullptr,
            .local_derivative = [](akml::DynamicMatrix<float> val, const akml::DynamicMatrix<float> expected) {
                return 2 * (val-expected);
            }
        };
    
        const struct ErrorFunction<float, akml::DynamicMatrix<float>> ERRORS_COUNT = {
            .function = [](const akml::DynamicMatrix<float>& a, const akml::DynamicMatrix<float>& b) {
                return (a == b);
            },
            .sumfunction = [](const std::vector<akml::DynamicMatrix<float>>& a, const std::vector<akml::DynamicMatrix<float>>& b) {
                if (a.size() != b.size())
                    throw std::invalid_argument("Dimension error");
                float count (0);
                for (std::size_t i(0); i < a.size(); i++){
                    count += (a[i] == b[i]);
                }
                return count;
            },
            .derivative = nullptr
        };

    }
}
