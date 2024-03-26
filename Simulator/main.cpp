//
//     ___  ___  ___  _____  ____  ___                     ___
//    / _ \/ _ \/ _ \/ ___/ / __ \/ (_)__ ____  ___  ___  / (_)__ ___
//   / // / // / ___/ (_ / / /_/ / / / _ `/ _ \/ _ \/ _ \/ / / -_|_-<
//  /____/____/_/   \___/  \____/_/_/\_, /\___/ .__/\___/_/_/\__/___/
//                                  /___/    /_/
//  DDPG Oligopolies Simulator
//  Created by Aldric Labarthe 2023-2024
//  ENS Paris Saclay
//

#include <iostream>
#include <functional>
#include <thread>
#include <chrono>

#include "AKML-lib/AKML.hpp"
#include "AKML-lib/AgentBasedUtilities/CSV_Saver.hpp"

#include "LearningAgent.hpp"

#include "SimulationManager.hpp"

#define MAX_THREADS_USAGE 0.85

// Help of the CLI
void printProgramUsage(const std::string &programName) {
    std::cout << "Usage: " << programName << " [options]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -h, --help                  Show this help message and exit\n";
    std::cout << "  -o, --oligopolyModel=value  Choose the model (COURNOT|STACKELBERG|TEMPORAL_COURNOT) \n";
    std::cout << "  -N, --maxIterations=value   Set the number of iterations\n";
    std::cout << "  -n, --agentsNb=value        Set the number of agents\n";
    std::cout << "  -S, --simulationsNb=value   Set the number of simulations to process\n";
    std::cout << "  -T, --maxThreads=value      Set the number of simulations to process in parallel\n";
    std::cout << "  -g, --gamma=value           Set the gamma parameter\n";
    std::cout << "  -p, --profitNorm=value      Enable/Disable profit normalization\n";
    std::cout << "  -d, --decayRate=value       Edit the decayRate parameter\n";
    std::cout << "  -w, --wnDecay=value         Choose the Whitenoise decayMethod (LIN|EXP|SIG|TRUNC_EXP|TRUNC_EXP_RES)\n";
    std::cout << "  -s, --maxWhiteNoise=value   Edit the maximum std. deviation of the whitenoise process\n";
    std::cout << "  -a, --actorLR=value         Edit the actor learning rate\n";
    std::cout << "  -c, --criticLR=value        Edit the critic learning rate\n";
    std::cout << "  -C, --modC=value            Edit the marginal cost of the model\n";
    std::cout << "  -D, --modD=value            Edit the demand hyperparameter of the model\n";
    std::cout << "  -E, --exportCritics         Should we dump critics of agents at the end of simulation ?\n";
}

// Options that the user is allowed to edit in CLI
// DO NOT MODIFY OPTIONS HERE, first, do it in CLI and if you really want to change default, edit the main()
struct options {
    SimulationManager::OLIGOPOLY_TYPE type = SimulationManager::OLIGOPOLY_TYPE::TEMPORAL_COURNOT;
    std::size_t agentsNb = 2;
    std::size_t maxIteration = 120000;
    std::size_t simulationsNb = 12;
    unsigned int maxThreads = (std::thread::hardware_concurrency()*MAX_THREADS_USAGE > 1) ? std::thread::hardware_concurrency()*MAX_THREADS_USAGE : 1;
    float gamma = LearningAgent::gamma;
    bool profitNormalization = true;
    float decayRate = LearningAgent::decayRate;
    float learningRateActor = 0.01;
    float learningRateCritic = 0.1;
    SimulationManager::WN_DECAY_METHOD wnDecayMethod = SimulationManager::WN_DECAY_METHOD::EXP;
    float maxWhiteNoise = SimulationManager::maxWhiteNoise;
    float modC = SimulationManager::C;
    float modD = SimulationManager::D;
    bool exportCritics = true;
};

// CLI Options with parsed values
void printOptionsValues(const options &localoptions) {
    std::cout << "Options:" << std::endl;
    std::cout << "--oligopolyModel="<< SimulationManager::getOligopolyName(localoptions.type) << "\n";
    std::cout << "--maxIterations=" << localoptions.maxIteration << "\n";
    std::cout << "--agentsNb=" << localoptions.agentsNb << "\n";
    std::cout << "--simulationsNb=" << localoptions.simulationsNb << "\n";
    std::cout << "--maxThreads=" << localoptions.maxThreads << "\n";
    std::cout << "--gamma=" << localoptions.gamma << "\n";
    std::cout << "--profitNorm=" << localoptions.profitNormalization << "\n";
    std::cout << "--decayRate=" << localoptions.decayRate << "\n";
    std::cout << "--wnDecay=" << SimulationManager::getWNDecayMethodName(localoptions.wnDecayMethod) << "\n";
    std::cout << "--actorLR=" << localoptions.learningRateActor << "\n";
    std::cout << "--criticLR=" << localoptions.learningRateCritic << "\n";
    std::cout << "--maxWhiteNoise=" << localoptions.maxWhiteNoise << "\n";
    std::cout << "--modC=" << localoptions.modC << "\n";
    std::cout << "--modD=" << localoptions.modD << "\n";
    std::cout << "--exportCritics=" << localoptions.exportCritics << "\n";
}

// This function is a parser for CLI arguments
// Returns false if there is an error
bool handleOptions(int argc, const char * argv[], options& localoptions) {
    std::vector<std::string> args(argv, argv + argc);
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
        }else {
            printProgramUsage(args[0]);
            return false;
        }
        
        //std::cout << "Debug: " << option << " " << value << "\n";
        try {
            if (option == "o" ||option == "oligopolyModel"){
                if (value == "STACKELBERG" ||value == "Stackelberg" ||value == "S"){
                    localoptions.type = SimulationManager::OLIGOPOLY_TYPE::STACKELBERG;
                }else if (value == "COURNOT" ||value == "Cournot" ||value == "C"){
                    localoptions.type = SimulationManager::OLIGOPOLY_TYPE::COURNOT;
                }else if (value == "TEMPORAL_COURNOT" ||value == "TEMPORALCOURNOT" ||value == "Temporal_Cournot" ||value == "TEMPORAL" ||value == "TC" ||value == "T"){
                    localoptions.type = SimulationManager::OLIGOPOLY_TYPE::TEMPORAL_COURNOT;
                }else {
                    std::cerr << "Alert! Unable to parse oligopoly model type.\n";
                    printProgramUsage(args[0]);
                    return false;
                }
            }else if (option == "w" ||option == "wnDecay"){
                if (value == "LIN"){
                    localoptions.wnDecayMethod = SimulationManager::WN_DECAY_METHOD::LIN;
                }else if (value == "EXP"){
                    localoptions.wnDecayMethod = SimulationManager::WN_DECAY_METHOD::EXP;
                }else if (value == "SIG"){
                    localoptions.wnDecayMethod = SimulationManager::WN_DECAY_METHOD::SIG;
                }else if (value == "TRUNC_EXP"){
                    localoptions.wnDecayMethod = SimulationManager::WN_DECAY_METHOD::TRUNC_EXP;
                }else if (value == "TRUNC_EXP_RES"){
                    localoptions.wnDecayMethod = SimulationManager::WN_DECAY_METHOD::TRUNC_EXP_RES;
                }else {
                    std::cerr << "Alert! Unable to parse whitenoise decay method.\n";
                    printProgramUsage(args[0]);
                    return false;
                }
            }else if (option == "N" ||option == "maxIterations"){
                localoptions.maxIteration = (std::size_t)std::stoi(value);
            }else if (option == "n" ||option == "agentsNb"){
                localoptions.agentsNb = (std::size_t)std::stoi(value);
            }else if (option == "S" ||option == "simulationsNb"){
                localoptions.simulationsNb = (std::size_t)std::stoi(value);
            }else if (option == "T" ||option == "maxThreads"){
                localoptions.maxThreads = (unsigned int)std::stoi(value);
                if (std::thread::hardware_concurrency() > 1 && std::thread::hardware_concurrency()*MAX_THREADS_USAGE < localoptions.maxThreads){
                    std::cerr << "Too many threads requested, maxThreads set to " << std::thread::hardware_concurrency()*MAX_THREADS_USAGE << "\n";
                    localoptions.maxThreads = std::thread::hardware_concurrency()*MAX_THREADS_USAGE;
                }
            }else if (option == "g" ||option == "gamma"){
                localoptions.gamma = std::stof(value);
            }else if (option == "p" ||option == "profitNorm"){
                localoptions.profitNormalization = (value == "true" || value == "T" || value == "1" ||value == "True" || value == "TRUE");
            }else if (option == "d" ||option == "decayRate"){
                localoptions.decayRate = std::stof(value);
            }else if (option == "a" ||option == "actorLR"){
                localoptions.learningRateActor = std::stof(value);
            }else if (option == "c" ||option == "criticLR"){
                localoptions.learningRateCritic = std::stof(value);
            }else if (option == "s" ||option == "maxWhiteNoise"){
                localoptions.maxWhiteNoise = std::stof(value);
            }else if (option == "C" ||option == "modC"){
                localoptions.modC = std::stof(value);
            }else if (option == "D" ||option == "modD"){
                localoptions.modD = std::stof(value);
            }else if (option == "E" ||option == "exportCritics"){
                localoptions.exportCritics = (value == "true" || value == "T" || value == "1" ||value == "True" || value == "TRUE");
            }else {
                printProgramUsage(args[0]);
                return false;
            }
        }catch (const std::invalid_argument& e) {
            std::cerr << "Invalid argument: " << e.what() << std::endl;
            return -1;
        }catch (const std::out_of_range& e) {
            std::cerr << "Out of range argument: " << e.what() << std::endl;
            return -1;
        }
    }
    if (localoptions.type == SimulationManager::OLIGOPOLY_TYPE::TEMPORAL_COURNOT && !localoptions.profitNormalization)
        std::cerr << "Alert! Profit normalization is disabled. Temporal Cournot should be implemented with profit normalization. \n";
    return true;
}

int main(int argc, const char * argv[]) {
    std::cout << "DDPG-Oligopolies-Simulator v0.1 - Welcome!\n";
    options localoptions = {
      // If we want to force default values:
        .type=SimulationManager::COURNOT,
        .maxIteration = 60000,
        .simulationsNb = 4,
        .gamma = 0.f,
        .profitNormalization = false,
        //.decayRate = 1.f, // or 0.9999
        .learningRateActor= 0.01,
        .learningRateCritic= 0.1,
        .wnDecayMethod = SimulationManager::WN_DECAY_METHOD::LIN,
        .maxWhiteNoise = 0.06
    };
    
    if (!handleOptions(argc, argv, localoptions))
        return -1;
    printOptionsValues(localoptions); std::cout << std::endl;
    
    unsigned int maxThreads = std::max((unsigned int)(localoptions.maxThreads/localoptions.agentsNb), (unsigned int)1);
    std::vector<SimulationManager*> managers;
    std::vector<std::thread> workers;
    
    // Input renormalization, in temporal Cournot, the price is the input, needing a higher scaling factor
    SimulationManager::A = (localoptions.type == SimulationManager::TEMPORAL_COURNOT) ? 0.45 : 1;
    SimulationManager::C = localoptions.modC;
    SimulationManager::D = localoptions.modD;
    SimulationManager::maxWhiteNoise = localoptions.maxWhiteNoise;
    
    managers.reserve(maxThreads); workers.reserve(maxThreads);
    for (std::size_t s(0); s < localoptions.simulationsNb;){
        for (unsigned int conc_s(0); s+conc_s < std::min(localoptions.simulationsNb, s+maxThreads); conc_s++){
            std::cout << "Initialization of simulation " << s+conc_s+1 << " / " << localoptions.simulationsNb << "\n";
            managers.push_back(new SimulationManager(localoptions.type, localoptions.agentsNb, localoptions.maxIteration));
            managers.back()->setGamma(localoptions.gamma);
            managers.back()->setDecayrate(localoptions.decayRate);
            switch (localoptions.wnDecayMethod) {
                case SimulationManager::LIN: managers.back()->setWNMethod(&SimulationManager::LINDECAYED_WHITENOISE); break;
                case SimulationManager::EXP: managers.back()->setWNMethod(&SimulationManager::EXPDECAYED_WHITENOISE); break;
                case SimulationManager::SIG: managers.back()->setWNMethod(&SimulationManager::SIGDECAYED_WHITENOISE); break;
                case SimulationManager::TRUNC_EXP: managers.back()->setWNMethod(&SimulationManager::TRUNCEXPDECAYED_WHITENOISE); break;
                case SimulationManager::TRUNC_EXP_RES: managers.back()->setWNMethod(&SimulationManager::TRUNCRESEXPDECAYED_WHITENOISE); break;
            }
            managers.back()->setLearningRates(localoptions.learningRateActor, localoptions.learningRateCritic);
            if (!localoptions.profitNormalization)
                managers.back()->disableProfitRenormalization();
        }
        
        for (unsigned int conc_s(0); conc_s < managers.size(); conc_s++){
            std::cout << "Processing simulation " << s+conc_s+1 << " / " << localoptions.simulationsNb << "\n";
            //workers.emplace_back(&SimulationManager::processSimulation, managers[conc_s], maxThreads>1&&localoptions.simulationsNb>1);
            workers.emplace_back([localoptions](SimulationManager* simul, bool mute, unsigned int simulId) {
                auto start = std::chrono::high_resolution_clock::now();
                std::string output = simul->processSimulation(mute);
                if (localoptions.exportCritics){
                    std::cout << "Critics dump of simulation " << simulId+1 << " - Log file: " << "DDPG-Critics-" << output << ".csv" << "\n";
                    simul->saveCritics(output);
                }
                auto end = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double> duration = end - start;
                
                std::ofstream cout(std::string("DDPG-SimulInfos-" + output +".txt"));
                std::ios_base::sync_with_stdio(false);
                auto *coutbuf = std::cout.rdbuf();
                std::cout.rdbuf(cout.rdbuf());
                printOptionsValues(localoptions);
                std::cout << "--executionTime=" << duration.count() << "\n";
                std::cout.rdbuf(coutbuf);
                std::cout << "End of simulation " << simulId+1 << " - Log file: " << "DDPG-" << SimulationManager::getOligopolyName(localoptions.type) << "-" << std::to_string(localoptions.agentsNb) << "-" << output << ".csv" << "\n";
            }, managers[conc_s], maxThreads>1&&localoptions.simulationsNb>1, s+conc_s);
            
        }
        for (unsigned int conc_s(0); conc_s < managers.size(); conc_s++){
            if (workers[conc_s].joinable())
                workers[conc_s].join();
            delete managers[conc_s];
        }
        s += managers.size();
        workers.clear(); managers.clear();
        
    }
    return 0;
}
