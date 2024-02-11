//
//  main.cpp
//  CournotDDPG
//
//  Created by Aldric Labarthe on 26/01/2024.
//

#include <iostream>
#include <functional>
#include "AKML-lib/AKML.hpp"
#include "AKML-lib/AgentBasedUtilities/CSV_Saver.hpp"
#include "LearningAgent.hpp"

#define C 1
#define D 3

#define STUPID_DEMO 1

int main(int argc, const char * argv[]) {
    // insert code here...
    std::cout << "Hello, terrible World!\n";
    
    typedef akml::Save<7, std::size_t, float, float, float, float, float, float> LogSaveType;
    akml::CSV_Saver<LogSaveType> LogManager;
    LogSaveType::default_parameters_name = {{ "round", "random_action", "action", "bestAction", "profit", "whitenoise", "estimated_profit" }};
    
    LearningAgent learningAgent;
    
    
    #ifdef STUPID_DEMO
    // We try with a "stupid" cournot-like model.
    // Each firm chooses its quantity and is rewarded according to its profit
    
    // Cost and demand are linear
    auto demand = [](float p) {
        return (float)(D - p);
    };
    auto inverse_demand = [](float q) {
        return (float)(D - q);
    };
    
    auto cost = [](float q) {
        return (float)(C*std::pow(q,2));
    };
    
    auto bestResponse = [](float q) {
        return (float)((D-q)/(2.f+2.f*C))/1.2;
    };
    
    auto profitFunc = [cost](float p, float q) {
        return (float)(std::max(0.f, p) * q - cost(q))/1.2;
    };
    
    std::random_device rd;
    std::mt19937 gen(rd());
    // first firm plays randomly (it decide a quantity between 1 and 100
    std::uniform_real_distribution distribution(0.f,1.f);
    
    // Each firm starts with a budget
    float cumulative_profit = 1;
    
    float random_firm_action = (float)distribution(gen);
    std::size_t maxiterations(50000);
    LogManager.reserve(2001);
    for (std::size_t it(0); it <= maxiterations; it++){
        float our_firm_action = learningAgent.play(random_firm_action, ((it % 10 == 0) ? 0.f : 0.3 * (1.f-(float)it/(float)maxiterations)));
        float price = inverse_demand(random_firm_action+our_firm_action);
        float profit = profitFunc(price, our_firm_action);
        float estimatedProfit = learningAgent.askCritic(random_firm_action, our_firm_action);
        
        std::cout << "\nTurnament " << it;
        std::cout << "\nOur learning agent is asked to respond to " << (float)random_firm_action;
        std::cout << "\nHe plays " << our_firm_action << " (best response=" << bestResponse(random_firm_action) << ")";
        std::cout << "\nMarket price set to " << price << " EstimatedProfit=" << estimatedProfit << " Profit=" << profit << " Cumulative profit=" << cumulative_profit;
        cumulative_profit += profit;
        float oldaction = random_firm_action;
        random_firm_action=(float)distribution(gen);
        learningAgent.feedBack(oldaction, our_firm_action, profit, random_firm_action, /*(cumulative_profit <= 0)*/true );
        if ((cumulative_profit <= 0))
            cumulative_profit = 1;
        
        //LogSaveType* save = new LogSaveType(it, random_firm_action, our_firm_action, bestResponse(random_firm_action), profit, ((it % 10 == 0) ? 0.f : 0.3 * (1.f-(float)it/(float)maxiterations)), estimatedProfit );
        //LogManager.addSave(save);
        if (it%2000 == 0)
            LogManager.bufferize();
        LogManager.addSave(it, random_firm_action, our_firm_action, bestResponse(random_firm_action), profit, ((it % 10 == 0) ? 0.f : 0.3 * (1.f-(float)it/(float)maxiterations)), estimatedProfit);
        std::cout << "\n";
    }
    
    long int t = static_cast<long int> (std::clock());
    std::string curt = std::to_string(t);
    LogManager.saveToCSV("DDPG-Output-" + curt + ".csv", false);
    
    
    typedef akml::Save<5, float, float, float, float, float> CriticLogSaveType;
    akml::CSV_Saver<CriticLogSaveType> CriticLogManager;
    CriticLogSaveType::default_parameters_name = {{ "random_firm_action", "our_firm_action", "profit", "estimated_profit", "price" }};
    
    for (float randomA(0); randomA < 1; randomA += 0.01){
        for (float ourA(0); ourA < 1; ourA += 0.01){
            //CriticLogSaveType* save = new CriticLogSaveType(randomA, ourA, profitFunc(inverse_demand(randomA+ourA), ourA),  learningAgent.askCritic(randomA, ourA), inverse_demand(randomA+ourA));
            //CriticLogManager.addSave(save);
            CriticLogManager.addSave(randomA, ourA, profitFunc(inverse_demand(randomA+ourA), ourA),  learningAgent.askCritic(randomA, ourA), inverse_demand(randomA+ourA));
        }
    }
    CriticLogManager.saveToCSV("DDPG-Critic-" + curt + ".csv", false);
    #endif

    return 0;
}
