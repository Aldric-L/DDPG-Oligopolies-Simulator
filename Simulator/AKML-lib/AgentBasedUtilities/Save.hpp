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
    class AbstractSave {};

    template <std::size_t parameters_nb, typename... types>
    class Save : public AbstractSave{
        private:
            template<std::size_t i>
            struct exportLayer_functor {
                static std::string to_string(const std::string elem){ return elem; }
                template <typename T>
                static std::string to_string(const T elem){ return std::to_string(elem); }
                
                static void run(std::string& rtrn_to_edit, std::tuple<types...>& parameters) {
                    rtrn_to_edit += to_string(std::get<i>(parameters));
                    if (i != parameters_nb-1)
                        rtrn_to_edit += ", ";
                }
            };
        
        protected:
            std::array<std::string, parameters_nb> parameters_name;
            std::tuple<types...> parameters;
        
        public:
            static inline std::array<std::string, parameters_nb> default_parameters_name;
        
            inline Save(const std::tuple<types...>& param) : parameters(param), parameters_name(default_parameters_name) {};
        
            inline Save(const types... params) : parameters_name(default_parameters_name){
                parameters = std::make_tuple(params...);
            };
        
            inline Save() : parameters_name(default_parameters_name){};
        
            inline void setParameterNames(std::array<std::string, parameters_nb> pn){
                parameters_name = pn;
            }
        
            template <int param_id, typename type>
            inline void editParameter(type& param_val){
                std::get<param_id>(parameters) = param_val;
            }
        
            inline void editParameters(types... params){
                parameters = std::make_tuple(params...);
            }
        
            template <int param_id, typename type>
            inline type& getParameter(){
                return std::get<param_id>(parameters);
            }
            
            inline std::string printAsCSV() {
                std::string rtrn ("");
                akml::for_<0, parameters_nb>::template run<exportLayer_functor,std::string&, std::tuple<types...>&>(rtrn, parameters);
                return rtrn;
            }
        
            inline std::string printTitleAsCSV() {
                std::string rtrn ("");
                for (std::size_t i(0); i < parameters_nb; i++){
                    rtrn += parameters_name[i];
                    if (i != parameters_nb-1)
                        rtrn += ", ";
                }
                return rtrn;
            }
    };

    template <unsigned short int parameters_nb, akml::Matrixable MATRIX_INNER_TYPE>
    class MatrixSave : public AbstractSave {
        protected:
            std::array<std::string, parameters_nb> parameters_name;
            akml::Matrix<MATRIX_INNER_TYPE, parameters_nb, 1> parameters;
        
        public:
            static inline std::array<std::string, parameters_nb> default_parameters_name;
        
            inline MatrixSave(const akml::Matrix<MATRIX_INNER_TYPE, parameters_nb, 1>& param) : parameters(param), parameters_name(default_parameters_name) {};
        
            template<typename... types>
            inline MatrixSave(const types... params) : parameters_name(default_parameters_name){
                parameters = akml::make_vector<MATRIX_INNER_TYPE>(params...);
            };
        
            inline MatrixSave() : parameters_name(default_parameters_name){};
        
            inline void setParameterNames(std::array<std::string, parameters_nb> pn){
                parameters_name = pn;
            }
        
            template <int param_id, typename type>
            inline void editParameter(type& param_val){
                parameters[{param_id, 0}] = param_val;
            }
        
            template<typename... types>
            inline void editParameters(types... params){
                parameters = std::move(akml::make_vector<MATRIX_INNER_TYPE>(params...));
            }
        
            template <int param_id, typename type>
            inline type& getParameter(){
                return parameters[{param_id, 0}];
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
        
            inline std::string printTitleAsCSV() {
                std::string rtrn ("");
                for (std::size_t i(0); i < parameters_nb; i++){
                    rtrn += parameters_name[i];
                    if (i != parameters_nb-1)
                        rtrn += ", ";
                }
                return rtrn;
            }
    };

    template <akml::Matrixable MATRIX_INNER_TYPE>
    class DynamicMatrixSave : public AbstractSave {
        public:
            std::size_t parameters_nb;
            static inline std::vector<std::string> default_parameters_name;
        
        protected:
            std::vector<std::string> parameters_name;
            akml::DynamicMatrix<MATRIX_INNER_TYPE> parameters;
        
        public:
            inline DynamicMatrixSave(const std::size_t parameters_nb, const akml::DynamicMatrix<MATRIX_INNER_TYPE>& param) : parameters_nb(parameters_nb), parameters(param), parameters_name(default_parameters_name) {};
        
            template<typename... types>
            inline DynamicMatrixSave(const std::size_t parameters_nb, const types... params) : parameters_nb(parameters_nb), parameters_name(default_parameters_name), parameters(akml::make_dynamic_vector<MATRIX_INNER_TYPE>(params...)){
            };
        
            inline DynamicMatrixSave(const std::size_t parameters_nb) : parameters_nb(parameters_nb), parameters(parameters_nb, 1), parameters_name(default_parameters_name){};
        
            inline DynamicMatrixSave() : parameters(0, 0), parameters_name(default_parameters_name){};
        
            inline void operator=(DynamicMatrixSave other){
                parameters.forceAssignement(other.getParameters());
                parameters_nb = other.getParameters().getNRows();
            }
        
            inline void setParameterNames(std::vector<std::string> pn){
                if (pn.size() != parameters_nb)
                    throw std::invalid_argument("Invalid parameter list length");
                parameters_name = pn;
            }
        
            template <int param_id, typename type>
            inline void editParameter(type& param_val){
                parameters[{param_id, 0}] = param_val;
            }
        
            template<typename... types>
            inline void editParameters(types... params){
                parameters.forceAssignment(std::move(akml::make_dynamic_vector<MATRIX_INNER_TYPE>(params...)));
                parameters_nb = parameters.getNRows();
            }
        
            template <int param_id, typename type>
            inline type& getParameter(){
                return parameters[{param_id, 0}];
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
        
            inline std::string printTitleAsCSV() {
                std::string rtrn ("");
                for (std::size_t i(0); i < parameters_nb; i++){
                    rtrn += parameters_name[i];
                    if (i != parameters_nb-1)
                        rtrn += ", ";
                }
                return rtrn;
            }
    };

}

#endif /* AKML_Save_hpp */
