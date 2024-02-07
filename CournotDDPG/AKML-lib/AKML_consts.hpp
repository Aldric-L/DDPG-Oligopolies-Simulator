//
//  AKML_consts.hpp
//  AKML Project
//
//  Created by Aldric Labarthe on 15/10/2023.
//

#ifndef AKML_consts_hpp
#define AKML_consts_hpp

#ifndef AKML_NEURAL_LAYER_NB
    #define AKML_NEURAL_LAYER_NB 1
#endif

#ifndef AKML_NN_STRUCTURE
    #define AKML_NN_STRUCTURE { 1 }
#endif
 
 namespace akml {
    const int NEURAL_LAYER_NB = AKML_NEURAL_LAYER_NB;
    constexpr int NN_STRUCTURE[NEURAL_LAYER_NB] = AKML_NN_STRUCTURE;
    constexpr int NEURAL_INPUT_NB = NN_STRUCTURE[0];
    constexpr int NEURAL_OUTPUT_NB = NN_STRUCTURE[AKML_NEURAL_LAYER_NB-1];
 };
#endif /* AKML_consts_hpp */

