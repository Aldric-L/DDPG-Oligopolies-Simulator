//
//  CSV-Saver.hpp
//  AKML Project
//
//  Created by Aldric Labarthe on 05/11/2023.
//

#ifndef AKML_CSV_Saver_hpp
#define AKML_CSV_Saver_hpp

#include <stdio.h>
#include <vector>
#include <fstream>
#include <iomanip>

#include "Save.hpp"

namespace akml {

    template <class T>
    concept SaveClassConcept = std::is_base_of<akml::AbstractSave, T>::value;

    template <SaveClassConcept SaveClass>
    class CSV_Saver {
    protected:
        std::vector<std::string> parameters_name;
        std::vector<SaveClass*> memory;
        std::string buffer = "";
        
        inline std::string printTitleAsCSV() {
            std::string rtrn ("");
            if (parameters_name.size() == 0)
                return rtrn;
            
            if (memory.size() > 0 && parameters_name.size() != memory.front()->getParametersNb())
                throw std::invalid_argument("Unable to print save file title due to heterogenous dimension error in the parameters name.");
            
            for (std::size_t i(0); i < parameters_name.size(); i++){
                rtrn += parameters_name[i];
                if (i != parameters_name.size()-1)
                    rtrn += ", ";
            }
            rtrn += "\n";
            return rtrn;
        }
        
    public:
        inline CSV_Saver(std::size_t bufferMaxSize=0){
            if (bufferMaxSize != 0)
                memory.reserve(bufferMaxSize);
        }
        
        inline void setParameterNames(std::vector<std::string> pn){ parameters_name = pn; }
        
        inline void reserve(std::size_t bufferMaxSize){
            memory.reserve(bufferMaxSize);
        }
        
        inline void addSave(SaveClass* iteration) {
            if (memory.size() > 0 && iteration->getParametersNb() != memory.front()->getParametersNb())
                throw std::invalid_argument("Heterogenous dimensions accross an unique saves manager.");
            memory.push_back(iteration);
        }
        
        inline void addSave(const SaveClass iteration) {
            if (memory.size() > 0 && iteration.getParametersNb() != memory.front()->getParametersNb())
                throw std::invalid_argument("Heterogenous dimensions accross an unique saves manager.");
            SaveClass* itpoint = new SaveClass();
            *itpoint = std::move(iteration);
            memory.push_back(itpoint);
        }
        
        template<typename... types>
        inline void addSave(const types... parameters) {
            SaveClass* itpoint = new SaveClass(parameters...);
            if (memory.size() > 0 && itpoint->getParametersNb() != memory.front()->getParametersNb())
                throw std::invalid_argument("Heterogenous dimensions accross an unique saves manager.");
            memory.push_back(itpoint);
        }
        
        inline void bufferize(bool iteration=true, bool title=true){
            if (memory.size() > 0){
                if (buffer == ""){
                    if (iteration)
                        buffer += "iteration,";
                    if (title)
                        buffer += this->printTitleAsCSV();
                }
                
                for (int it(0); it < memory.size(); it++){
                    if (iteration)
                        buffer += std::to_string(it) + ",";
                    buffer += memory[it]->printAsCSV() + "\n";
                    delete memory[it];
                }
                memory.clear();
            }
            
        }
        
        inline void saveToCSV(const std::string filepathandname="data.csv", bool iteration=true, bool title=true) {
            if (memory.size() == 0 && buffer == "")
                throw std::runtime_error("Trying to print log but the memory is empty...");
            std::ofstream file;
            file.open (filepathandname);
            if (buffer != ""){
                file << buffer;
            }
            if (memory.size() > 0){
                if (buffer == ""){
                    if (iteration)
                        file << "iteration,";
                    if (title)
                        file << this->printTitleAsCSV();
                }
                for (int it(0); it < memory.size(); it++){
                    if (iteration)
                        file << it << ",";
                    file << memory[it]->printAsCSV() <<"\n";
                }
            }
            file.close();
        }
        
        inline ~CSV_Saver() {
            for (int it(0); it < memory.size(); it++){
                delete memory[it];
            }
        }
        
    };

}

#endif /* CSV_Saver_hpp */
