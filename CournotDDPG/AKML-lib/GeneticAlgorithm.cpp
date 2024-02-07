//
//  GeneticAlgorithm.cpp
//  AKML Project
//
//  Created by Aldric Labarthe on 13/09/2023.
//

#include "GeneticAlgorithm.hpp"

namespace akml {
/*
template <size_t NBLAYERS, size_t INPUTNUMBER, size_t OUTPUTNUMBER, size_t TRAINING_LENGTH, size_t POP_SIZE>
std::function<float(std::array<StaticMatrix <float, OUTPUTNUMBER, 1>, TRAINING_LENGTH>&, std::array< StaticMatrix <float, OUTPUTNUMBER, 1> , TRAINING_LENGTH>&)> GeneticAlgorithm<NBLAYERS, INPUTNUMBER, OUTPUTNUMBER, TRAINING_LENGTH, POP_SIZE>::MSE = [] (std::array<StaticMatrix <float, OUTPUTNUMBER, 1>, TRAINING_LENGTH>& training_set, std::array< StaticMatrix <float, OUTPUTNUMBER, 1> , TRAINING_LENGTH>& real_outputs) {
    float MSE(0);
    for (std::size_t inputid(0); inputid < TRAINING_LENGTH; inputid++){
        for (std::size_t outputIncr(0); outputIncr < OUTPUTNUMBER; outputIncr++){
            MSE += std::pow(real_outputs[inputid].read(outputIncr+1, 1) - training_set[inputid].read(outputIncr+1, 1), 2);
        }
    }
    
    MSE = MSE/TRAINING_LENGTH;
    return MSE;
};

template <size_t NBLAYERS, size_t INPUTNUMBER, size_t OUTPUTNUMBER, size_t TRAINING_LENGTH, size_t POP_SIZE>
std::function<float(std::array<StaticMatrix <float, OUTPUTNUMBER, 1>, TRAINING_LENGTH>&, std::array< StaticMatrix <float, OUTPUTNUMBER, 1> , TRAINING_LENGTH>&)> GeneticAlgorithm<NBLAYERS, INPUTNUMBER, OUTPUTNUMBER, TRAINING_LENGTH, POP_SIZE>::ERRORS_COUNT = [] (std::array<StaticMatrix <float, OUTPUTNUMBER, 1>, TRAINING_LENGTH>& training_set, std::array< StaticMatrix <float, OUTPUTNUMBER, 1> , TRAINING_LENGTH>& real_outputs) {
    float errors(0);
    for (std::size_t inputid(0); inputid < TRAINING_LENGTH; inputid++){
        for (std::size_t outputIncr(0); outputIncr < OUTPUTNUMBER; outputIncr++){
            if (real_outputs[inputid].read(outputIncr+1, 1) != training_set[inputid].read(outputIncr+1, 1)){
                errors++;
                break;
            }
        }
    }
    return errors;
};

template <size_t NBLAYERS, size_t INPUTNUMBER, size_t OUTPUTNUMBER, size_t POP_SIZE>
std::function<StaticMatrix <float, OUTPUTNUMBER, 1>(StaticMatrix <float, OUTPUTNUMBER, 1>)> BaseGeneticAlgorithm<NBLAYERS, INPUTNUMBER, OUTPUTNUMBER, POP_SIZE>::ACTIVATE_ROUND = [](StaticMatrix <float, OUTPUTNUMBER, 1> row_output) {
    for (std::size_t outputIncr(1); outputIncr <= OUTPUTNUMBER; outputIncr++){
        row_output(outputIncr, 1) = std::roundf(row_output(outputIncr, 1));
    }
    return row_output;
};
*/
}
