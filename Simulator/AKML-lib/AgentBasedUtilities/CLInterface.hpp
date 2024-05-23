//
//  CLInterface.hpp
//  AKML_AgentBasedUtilities
//
//  Created by Aldric Labarthe on 11/05/2024.
//

#ifndef CLInterface_hpp
#define CLInterface_hpp

#include "../StaticLoops.hpp"

#include <stdio.h>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <tuple>

namespace akml {

class AbstractCLOption{};

template<typename T>
class CLOptionInterface : public AbstractCLOption{
protected:
    T* value;
    std::string name, longName, description;
    
    static std::string to_string(const std::string elem){ return elem; }
    template <typename Z>
    static std::string to_string(const Z elem){ return std::to_string(elem); }
    
public:
    inline CLOptionInterface(T* value, const std::string name, const std::string longName, const std::string description) : value(value), name(name), longName(longName), description(description) {
        if (value == nullptr)
            throw std::invalid_argument("Value pointer points towards nullptr.");
    };
    
    inline ~CLOptionInterface() {
    }
    
    virtual bool tryToSetValue(std::string input, const bool printDebug=true) = 0;
    virtual const std::string getStringifiedValue() const = 0;
    
    inline const T* getValue() const { return value; }
    inline const std::string& getName() const { return name; }
    inline const std::string& getLongName() const { return longName; }
    inline const std::string& getDescription() const { return description; }
};

template<typename T>
class CLOption : public CLOptionInterface<T> {
    std::vector<T> authorizedValues;
    
public:
    inline CLOption(T* value, const std::string name, const std::string longName, const std::string description)  : CLOptionInterface<T>(value, name, longName, description){
    };
    
    inline CLOption(T* value, const std::string name, const std::string longName, const std::string description, const T defaultValue, const std::vector<T>& authorizedValues)  : CLOptionInterface<T>(value, name, longName, description), authorizedValues(authorizedValues){
        *(this->value) = defaultValue;
    };
    
    inline CLOption(T* value, const std::string name, const std::string longName, const std::string description, const T defaultValue) : CLOptionInterface<T>(value, name, longName, description){
        *(this->value) = defaultValue;
    };
    
    inline CLOption(T* value, const std::string name, const std::string longName, const std::string description, const std::vector<T>& authorizedValues) : CLOptionInterface<T>(value, name, longName, description), authorizedValues(authorizedValues){};
    
    inline CLOption(T* value, const std::string name, const std::string longName, const std::vector<T>& authorizedValues) : CLOptionInterface<T>(value, name, longName, ""), authorizedValues(authorizedValues){};
    
    bool tryToSetValue(std::string input, const bool printDebug=true);
    
    inline const std::string getStringifiedValue() const { return CLOptionInterface<T>::to_string(*(this->getValue())); }
};

template<typename T>
class CLSelectOption : public CLOptionInterface<T> {
    std::map<std::string, T> authorizedValues;
    
public:
    inline CLSelectOption(T* value, const std::string name, const std::string longName, const std::map<std::string, T>& authorizedValues, const T defaultValue) : CLOptionInterface<T>(value, name, longName, ""), authorizedValues(authorizedValues){
        *(this->value) = defaultValue;
    };
    inline CLSelectOption(T* value, const std::string name, const std::string longName, const std::map<std::string, T>& authorizedValues) : authorizedValues(authorizedValues), CLOptionInterface<T>(value, name, longName, ""){};
    
    inline CLSelectOption(T* value, const std::string name, const std::string longName, const std::string& description, const std::map<std::string, T>& authorizedValues, const T defaultValue) : CLOptionInterface<T>(value, name, longName, description), authorizedValues(authorizedValues){
        *(this->value) = defaultValue;
    };
    
    inline CLSelectOption(T* value, const std::string name, const std::string longName, const std::string& description, const std::map<std::string, T>& authorizedValues) : CLOptionInterface<T>(value, name, longName, description), authorizedValues(authorizedValues){};
    
    inline bool tryToSetValue(std::string input, const bool printDebug=true){
        while (!input.empty() && std::isspace(input.back()))
            input.pop_back();
        
        if (auto search = authorizedValues.find(input); search != authorizedValues.end()){
            *(this->value) = search->second;
            return true;
        }
        return false;
    }
    
    
    inline const std::string getStringifiedValue() const {
        auto findResult = std::find_if(std::begin(authorizedValues), std::end(authorizedValues), [&](const std::pair<std::string, T> &pair){ return pair.second == *(this->value);});

        if (findResult != std::end(authorizedValues))
            return findResult->first;
        throw std::runtime_error("Unable to find key for the CLISelect value selected.");
    }
    
};


template<>
inline bool CLOption<int>::tryToSetValue(std::string input, const bool printDebug){
    int tmp;
    try {
        tmp = stoi(input);
    }catch (const std::invalid_argument& e) {
        std::cerr << "Invalid argument: " << e.what() << std::endl;
        return false;
    }catch (const std::out_of_range& e) {
        std::cerr << "Out of range argument: " << e.what() << std::endl;
        return false;
    }
    if (this->authorizedValues.size() > 0 && std::find(authorizedValues.begin(), authorizedValues.end(), tmp) == authorizedValues.end())
        return false;
    *(this->value) = tmp;
    return true;
}

template<>
inline bool CLOption<long int>::tryToSetValue(std::string input, const bool printDebug){
    long int tmp;
    try {
        tmp = stol(input);
    }catch (const std::invalid_argument& e) {
        std::cerr << "Invalid argument: " << e.what() << std::endl;
        return false;
    }catch (const std::out_of_range& e) {
        std::cerr << "Out of range argument: " << e.what() << std::endl;
        return false;
    }
    if (this->authorizedValues.size() > 0 && std::find(authorizedValues.begin(), authorizedValues.end(), tmp) == authorizedValues.end())
        return false;
    *(this->value) = tmp;
    return true;
}

template<>
inline bool CLOption<long long int>::tryToSetValue(std::string input, const bool printDebug){
    long long int tmp;
    try {
        tmp = stoll(input);
    }catch (const std::invalid_argument& e) {
        std::cerr << "Invalid argument: " << e.what() << std::endl;
        return false;
    }catch (const std::out_of_range& e) {
        std::cerr << "Out of range argument: " << e.what() << std::endl;
        return false;
    }
    if (this->authorizedValues.size() > 0 && std::find(authorizedValues.begin(), authorizedValues.end(), tmp) == authorizedValues.end())
        return false;
    *(this->value) = tmp;
    return true;
}

template<>
inline bool CLOption<unsigned int>::tryToSetValue(std::string input, const bool printDebug){
    unsigned int tmp;
    try {
        std::istringstream iss(input);
        iss >> tmp;
    }catch (const std::invalid_argument& e) {
        std::cerr << "Invalid argument: " << e.what() << std::endl;
        return false;
    }catch (const std::out_of_range& e) {
        std::cerr << "Out of range argument: " << e.what() << std::endl;
        return false;
    }
    if (this->authorizedValues.size() > 0 && std::find(authorizedValues.begin(), authorizedValues.end(), tmp) == authorizedValues.end())
        return false;
    *(this->value) = tmp;
    return true;
}

template<>
inline bool CLOption<unsigned short int>::tryToSetValue(std::string input, const bool printDebug){
    unsigned short int tmp;
    try {
        std::istringstream iss(input);
        iss >> tmp;
    }catch (const std::invalid_argument& e) {
        std::cerr << "Invalid argument: " << e.what() << std::endl;
        return false;
    }catch (const std::out_of_range& e) {
        std::cerr << "Out of range argument: " << e.what() << std::endl;
        return false;
    }
    if (this->authorizedValues.size() > 0 && std::find(authorizedValues.begin(), authorizedValues.end(), tmp) == authorizedValues.end())
        return false;
    *(this->value) = tmp;
    return true;
}

template<>
inline bool CLOption<unsigned long int>::tryToSetValue(std::string input, const bool printDebug){
    unsigned long int tmp;
    try {
        tmp = stoul(input);
    }catch (const std::invalid_argument& e) {
        std::cerr << "Invalid argument: " << e.what() << std::endl;
        return false;
    }catch (const std::out_of_range& e) {
        std::cerr << "Out of range argument: " << e.what() << std::endl;
        return false;
    }
    if (this->authorizedValues.size() > 0 && std::find(authorizedValues.begin(), authorizedValues.end(), tmp) == authorizedValues.end())
        return false;
    *(this->value) = tmp;
    return true;
}

template<>
inline bool CLOption<unsigned long long int>::tryToSetValue(std::string input, const bool printDebug){
    unsigned long long int tmp;
    try {
        tmp = stoull(input);
    }catch (const std::invalid_argument& e) {
        std::cerr << "Invalid argument: " << e.what() << std::endl;
        return false;
    }catch (const std::out_of_range& e) {
        std::cerr << "Out of range argument: " << e.what() << std::endl;
        return false;
    }
    if (this->authorizedValues.size() > 0 && std::find(authorizedValues.begin(), authorizedValues.end(), tmp) == authorizedValues.end())
        return false;
    *(this->value) = tmp;
    return true;
}

template<>
inline bool CLOption<float>::tryToSetValue(std::string input, const bool printDebug){
    float tmp;
    try {
        tmp = stof(input);
    }catch (const std::invalid_argument& e) {
        std::cerr << "Invalid argument: " << e.what() << std::endl;
        return false;
    }catch (const std::out_of_range& e) {
        std::cerr << "Out of range argument: " << e.what() << std::endl;
        return false;
    }
    if (this->authorizedValues.size() > 0 && std::find(authorizedValues.begin(), authorizedValues.end(), tmp) == authorizedValues.end())
        return false;
    *(this->value) = tmp;
    return true;
}

template<>
inline bool CLOption<double>::tryToSetValue(std::string input, const bool printDebug){
    double tmp;
    try {
        tmp = stod(input);
    }catch (const std::invalid_argument& e) {
        std::cerr << "Invalid argument: " << e.what() << std::endl;
        return false;
    }catch (const std::out_of_range& e) {
        std::cerr << "Out of range argument: " << e.what() << std::endl;
        return false;
    }
    if (this->authorizedValues.size() > 0 && std::find(authorizedValues.begin(), authorizedValues.end(), tmp) == authorizedValues.end())
        return false;
    *(this->value) = tmp;
    return true;
}

template<>
inline bool CLOption<long double>::tryToSetValue(std::string input, const bool printDebug){
    long double tmp;
    try {
        tmp = stold(input);
    }catch (const std::invalid_argument& e) {
        std::cerr << "Invalid argument: " << e.what() << std::endl;
        return false;
    }catch (const std::out_of_range& e) {
        std::cerr << "Out of range argument: " << e.what() << std::endl;
        return false;
    }
    if (this->authorizedValues.size() > 0 && std::find(authorizedValues.begin(), authorizedValues.end(), tmp) == authorizedValues.end())
        return false;
    *(this->value) = tmp;
    return true;
}

template<>
inline bool CLOption<std::string>::tryToSetValue(std::string input, const bool printDebug){
    while (!input.empty() && std::isspace(input.back()))
        input.pop_back();
    
    if (this->authorizedValues.size() > 0 && std::find(authorizedValues.begin(), authorizedValues.end(), input) == authorizedValues.end())
        return false;
    
    *(this->value) = input;
    return true;
}

template<>
inline bool CLOption<bool>::tryToSetValue(std::string input, const bool printDebug){
    while (!input.empty() && std::isspace(input.back()))
     input.pop_back();
    
    *(this->value) = (input == "true" || input == "T" || input == "1" || input == "True" || input == "TRUE" || input == "yes" || input == "Y" || input == "y");
    return true;
}

template <class T>
concept CLArgumentConcept = std::is_base_of<AbstractCLOption, T>::value;

class CLManager {
    template <std::size_t i>
    struct printOneValue {
        
        template <typename... Args>
        static void run(const bool withDescription, const std::tuple<Args...> &t) {
            std::string space = "                             ";
            if (std::get<i>(t).getValue() != nullptr)
                std::cout << "--"<< std::get<i>(t).getLongName() << "=" << std::get<i>(t).getStringifiedValue()
                << space.substr(0, space.size() - std::get<i>(t).getStringifiedValue().length() - std::get<i>(t).getLongName().length());
            else
                std::cout << "--"<< std::get<i>(t).getLongName() << "=?" << space.substr(0, space.size() - 1 - std::get<i>(t).getLongName().length());
            if (withDescription)
                std::cout << std::get<i>(t).getDescription();
            std::cout <<"\n";
        }
    };
    
    template <std::size_t i>
    struct printOneUsage {
        template <typename... Args>
        static void run(const std::tuple<Args...> &t) {
            std::string space = "                             ";
            std::cout << "  -" << std::get<i>(t).getName() << " --"<< std::get<i>(t).getLongName() << space.substr(0, space.size() - std::get<i>(t).getLongName().length()) << std::get<i>(t).getDescription() <<"\n";
        }
    };
    
    template <std::size_t i>
    struct processInput {
        template <typename... Args>
        static void run(std::string& flag, std::string& input, bool& treated, std::tuple<Args...> &t) {
            if (!treated && (flag == std::get<i>(t).getName() || flag == std::get<i>(t).getLongName())){
                treated = std::get<i>(t).tryToSetValue(input, true);
            }
        }
    };
    
public:
    
    template<CLArgumentConcept... Args>
    inline CLManager(const int argc, const char * argv[], std::tuple<Args...> arguments, bool endsByPrintingValues=true) {
        
        std::vector<std::string> args(argv, argv + argc);
        //std::string programName = args[0];
        for (int i = 1; i < argc; ++i) {
            const std::string &arg = args[i];
            
            std::size_t pos = arg.find('=');
            std::string option, value;
            
            if (arg.substr(0, 2) == "--" && pos > 2 && pos != std::string::npos) {
                option = arg.substr(2, pos - 2);
                value = arg.substr(pos + 1);
            }else if (arg.substr(0, 1) == "-" && pos != std::string::npos) {
                option = arg.substr(1, pos - 1);
                value = arg.substr(pos + 1);
            }else if (arg.substr(0, 2) == "-h" || arg.substr(0, 2) == "-H" || arg.substr(0, 3) == "--h") {
                printProgramUsage(args[0], arguments);
                throw 2;
            }else {
                std::cerr << "Unable to parse sequence. Usage: --flag=value OR -f=value" << std::endl;
                printProgramUsage(args[0], arguments);
                throw 0;
            }
            
            
            bool treated = false;
            akml::for_<0, std::tuple_size<decltype(arguments)>::value>::template run<processInput>(option, value, std::ref(treated), arguments);
            
            if (!treated){
                std::cerr << "Unable to parse the flag '" << option << "'" << std::endl;
                printProgramUsage(args[0], arguments);
                throw 1;
            }
                
        }
        
        if (endsByPrintingValues)
            printOptionsValues(arguments);
    }
    
    template<CLArgumentConcept... Args>
    inline static void printProgramUsage(const std::string &programName, std::tuple<Args...> arguments) {
        std::cout << "Usage: " << programName << " [options]" << std::endl;
        std::cout << "Options:" << std::endl;
        akml::for_<0, std::tuple_size<decltype(arguments)>::value>::template run<printOneUsage>(arguments);
    }
    
    template<CLArgumentConcept... Args>
    inline static void printOptionsValues(bool withDescription, std::tuple<Args...> arguments) {
        std::cout << "Options:" << std::endl;
        akml::for_<0, std::tuple_size<decltype(arguments)>::value>::template run<printOneValue>(withDescription, arguments);
    }
    
    template<CLArgumentConcept... Args>
    inline static void printOptionsValues(std::tuple<Args...> arguments) {
        std::cout << "Options:" << std::endl;
        akml::for_<0, std::tuple_size<decltype(arguments)>::value>::template run<printOneValue>(true, arguments);
    }
    
    
    
};


}


#endif /* CLInterface_hpp */
