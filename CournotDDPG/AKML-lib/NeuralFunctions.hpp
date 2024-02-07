//
//  NeuralFunctions.hpp
//  XORNeural network
//
//  Created by Aldric Labarthe on 27/01/2024.
//

#ifndef NeuralFunctions_hpp
#define NeuralFunctions_hpp

#include "AKML_consts.hpp"
#include "Matrices.hpp"
#include <stdio.h>
#include <cmath>
#include <vector>
#include <functional>

namespace akml {
    template<akml::Arithmetic T>
    struct ActivationFunction {
        const std::function<T(const T)> function;
        const std::function<T(const T)> derivative;
    };

    template<akml::Arithmetic T, akml::MatrixInterfaceConcept<T> MATRIX_TYPE>
    struct ErrorFunction {
        const std::function<T(const MATRIX_TYPE&, const MATRIX_TYPE&)> function;
        const std::function<T(const std::vector<MATRIX_TYPE>&, const std::vector<MATRIX_TYPE>&)> sumfunction;
        const std::function<MATRIX_TYPE(const MATRIX_TYPE&, const MATRIX_TYPE&)> derivative;
        const std::function<MATRIX_TYPE(const MATRIX_TYPE, const MATRIX_TYPE)> local_derivative;
    };

}

#endif /* NeuralFunctions_hpp */
