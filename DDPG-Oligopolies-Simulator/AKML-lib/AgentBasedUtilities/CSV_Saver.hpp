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
        std::vector<std::pair<std::size_t, SaveClass*>> memory;
        std::string buffer = "";
        
    public:
        inline CSV_Saver(std::size_t bufferMaxSize=0){
            if (bufferMaxSize != 0)
                memory.reserve(bufferMaxSize);
        }
        
        inline void reserve(std::size_t bufferMaxSize){
            memory.reserve(bufferMaxSize);
        }
        
        inline void addSave(SaveClass* iteration) {
            memory.push_back(std::make_pair((memory.size() == 0) ? 1 : memory.back().first+1, iteration));
        }
        
        inline void addSave(const SaveClass iteration) {
            SaveClass* itpoint = new SaveClass();
            *itpoint = std::move(iteration);
            memory.push_back(std::make_pair((memory.size() == 0) ? 1 : memory.back().first+1, itpoint));
        }
        
        template<typename... types>
        inline void addSave(const types... parameters) {
            SaveClass* itpoint = new SaveClass(parameters...);
            memory.push_back(std::make_pair((memory.size() == 0) ? 1 : memory.back().first+1, itpoint));
        }
        
        inline void bufferize(bool iteration=true){
            if (memory.size() > 0){
                if (buffer == ""){
                    if (iteration)
                        buffer += "iteration,";
                    buffer += memory[0].second->printTitleAsCSV() + "\n";
                }
                
                for (int it(0); it < memory.size(); it++){
                    if (iteration)
                        buffer += std::to_string(it) + ",";
                    buffer += memory[it].second->printAsCSV() + "\n";
                    delete memory[it].second;
                }
                memory.clear();
            }
            
        }
        
        inline void saveToCSV(const std::string filepathandname="data.csv", bool iteration=true) {
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
                    file << memory[0].second->printTitleAsCSV() <<"\n";
                }
                for (int it(0); it < memory.size(); it++){
                    if (iteration)
                        file << it << ",";
                    file << memory[it].second->printAsCSV() <<"\n";
                }
            }
            file.close();
        }
        
        inline ~CSV_Saver() {
            for (int it(0); it < memory.size(); it++){
                delete memory[it].second;
            }
        }
        
    };

}

#endif /* CSV_Saver_hpp */
