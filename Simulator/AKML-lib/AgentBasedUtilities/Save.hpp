//
//  Save.hpp
//  AKML Project
//
//  Created by Aldric Labarthe on 05/11/2023.
//

#ifndef AKML_Save_hpp
#define AKML_Save_hpp

#include <tuple>
#include "../StaticLoops.hpp"
#include "../Matrices.hpp"

namespace akml {
    class AbstractSave {
        protected:
            std::size_t parameters_nb;
        public:
            inline const std::size_t getParametersNb() const { return parameters_nb; }
            inline virtual std::string printAsCSV() = 0;
    };

    template <std::size_t PARAMETERS_NB, typename... types>
    class Save : public AbstractSave{
        private:
            template<std::size_t i>
            struct exportLayer_functor {
                static std::string to_string(const std::string elem){ return elem; }
                template <typename T>
                static std::string to_string(const T elem){ return std::to_string(elem); }
                
                static void run(std::string& rtrn_to_edit, std::tuple<types...>& parameters) {
                    rtrn_to_edit += to_string(std::get<i>(parameters));
                    if (i != PARAMETERS_NB-1)
                        rtrn_to_edit += ", ";
                }
            };
        
        protected:
            std::tuple<types...> parameters;
        
        public:
            inline Save(const std::tuple<types...>& param) : parameters(param){
                parameters_nb = PARAMETERS_NB;
            };
        
            inline Save(const types... params){
                parameters = std::make_tuple(params...);
                parameters_nb = PARAMETERS_NB;
            };
        
            inline Save(){
                parameters_nb = PARAMETERS_NB;
            };
        
            template <int param_id, typename type>
            inline void editParameter(type& param_val){
                if (param_id < parameters_nb)
                    std::get<param_id>(parameters) = param_val;
                else
                    throw std::invalid_argument("Dimension error in attempting to edit a save object.");
            }
        
            inline void editParameters(types... params){
                if (sizeof...(params) == parameters_nb)
                    parameters = std::make_tuple(params...);
                else
                    throw std::invalid_argument("Dimension error in attempting to edit a save object.");
            }
        
            template <int param_id, typename type>
            inline type& getParameter(){
                return std::get<param_id>(parameters);
            }
            
            inline std::string printAsCSV() {
                std::string rtrn ("");
                akml::for_<0, PARAMETERS_NB>::template run<exportLayer_functor,std::string&, std::tuple<types...>&>(rtrn, parameters);
                return rtrn;
            }
    };

    template <std::size_t PARAMETERS_NB, akml::Matrixable MATRIX_INNER_TYPE>
    class MatrixSave : public AbstractSave {
        protected:
            akml::Matrix<MATRIX_INNER_TYPE, PARAMETERS_NB, 1> parameters;
        
        public:
            inline MatrixSave(const akml::Matrix<MATRIX_INNER_TYPE, PARAMETERS_NB, 1>& param) : parameters(param){
                parameters_nb = PARAMETERS_NB;
            };
        
            template<typename... types>
            inline MatrixSave(const types... params) {
                parameters = akml::make_vector<MATRIX_INNER_TYPE>(params...);
                parameters_nb = PARAMETERS_NB;
            };
        
            inline MatrixSave() {};
        
            template <std::size_t param_id, typename type>
            inline void editParameter(type& param_val){
                if (param_id < parameters_nb)
                    parameters[{param_id, 0}] = param_val;
                else
                    throw std::invalid_argument("Dimension error in attempting to edit a save object.");
            }
        
            template<typename... types>
            inline void editParameters(types... params){
                if (sizeof...(params) == parameters_nb)
                    parameters = std::move(akml::make_vector<MATRIX_INNER_TYPE>(params...));
                else
                    throw std::invalid_argument("Dimension error in attempting to edit a save object.");
            }
        
            template <std::size_t param_id, typename type>
            inline type& getParameter(){
                if (param_id < parameters_nb)
                    return parameters[{param_id, 0}];
                else
                    throw std::invalid_argument("Dimension error in attempting to edit a save object.");
            }
            
            inline std::string printAsCSV() {
                std::string rtrn ("");
                for (std::size_t i(0); i < parameters_nb; i++){
                    rtrn += std::to_string(parameters[{i, 0}]);
                    if (i != parameters_nb-1)
                        rtrn += ", ";
                }
                return rtrn;
            }
    };

    template <akml::Matrixable MATRIX_INNER_TYPE>
    class DynamicMatrixSave : public AbstractSave {
        protected:
            akml::DynamicMatrix<MATRIX_INNER_TYPE> parameters;
        
        public:
            inline DynamicMatrixSave(const std::size_t param_nb, const akml::DynamicMatrix<MATRIX_INNER_TYPE>& param) : parameters(param){
                parameters_nb = param_nb;
            };
        
            template<typename... types>
            inline DynamicMatrixSave(const std::size_t param_nb, const types... params) : parameters(akml::make_dynamic_vector<MATRIX_INNER_TYPE>(params...)){
                parameters_nb = param_nb;
            };
        
            inline DynamicMatrixSave(const std::size_t param_nb) : parameters(parameters_nb, 1) {
                parameters_nb = param_nb;
            };
        
            inline DynamicMatrixSave() : parameters(0, 0){};
        
            inline void operator=(const DynamicMatrixSave& other){
                parameters.forceAssignement(other.getParameters());
                parameters_nb = other.getParameters().getNRows();
            }
        
            template <std::size_t param_id, typename type>
            inline void editParameter(type& param_val){
                if (param_id < parameters_nb)
                    parameters[{param_id, 0}] = param_val;
                else
                    throw std::invalid_argument("Dimension error in attempting to edit a save object.");
            }
        
            template<typename... types>
            inline void editParameters(types... params){
                parameters.forceAssignment(std::move(akml::make_dynamic_vector<MATRIX_INNER_TYPE>(params...)));
                parameters_nb = parameters.getNRows();
            }
        
            template <std::size_t param_id, typename type>
            inline type& getParameter(){
                if (param_id < parameters_nb)
                    return parameters[{param_id, 0}];
                else
                    throw std::invalid_argument("Dimension error in attempting to edit a save object.");
            }
        
            inline akml::DynamicMatrix<MATRIX_INNER_TYPE> getParameters(){
                return parameters;
            }
            
            inline std::string printAsCSV() {
                std::string rtrn ("");
                for (std::size_t i(0); i < parameters_nb; i++){
                    rtrn += std::to_string(parameters[{i, 0}]);
                    if (i != parameters_nb-1)
                        rtrn += ", ";
                }
                return rtrn;
            }
    };

    template <akml::MatrixConcept MATRIX_TYPE>
    class FullMatrixSave : public AbstractSave {
        protected:
            MATRIX_TYPE parameters;
        
        public:
            inline FullMatrixSave(const MATRIX_TYPE& param) : parameters(param) {
                parameters_nb = param.getNRows();
            };
        
            template<typename... types>
            inline FullMatrixSave(const types... params) = delete;
            inline FullMatrixSave() = delete;
        
            template <std::size_t param_id_1, std::size_t param_id_2, typename type>
            inline void editParameter(type& param_val){
                if (param_id_1 < parameters_nb)
                    parameters[{param_id_1, param_id_2}] = param_val;
                else
                    throw std::invalid_argument("Dimension error in attempting to edit a save object.");
            }
        
            template<typename... types>
            inline void editParameters(types... params) = delete;
        
            template <std::size_t param_id_1, std::size_t param_id_2, typename type>
            inline type& getParameter(){
                if (param_id_1 < parameters_nb)
                    return parameters[{param_id_1, param_id_2}];
                else
                    throw std::invalid_argument("Dimension error in attempting to edit a save object.");
            }
            
            inline std::string printAsCSV() {
                std::string rtrn ("");
                for (std::size_t row(0); row < parameters.getNRows(); row++){
                    for (std::size_t col(0); col < parameters.getNColumns(); col++){
                        rtrn += std::to_string(parameters[{row, col}]);
                        if (col != parameters.getNColumns()-1)
                            rtrn += ", ";
                        if (row != parameters.getNRows()-1 && col == parameters.getNColumns()-1)
                            rtrn += "\n";
                    }
                }
                return rtrn;
            }
        };

}

#endif /* AKML_Save_hpp */
