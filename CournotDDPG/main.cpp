//
//  main.cpp
//  CournotDDPG
//
//  Created by Aldric Labarthe on 26/01/2024.
//

#include <iostream>
#include <functional>
#include "AKML-lib/AKML.hpp"
#include "LearningAgent.hpp"

int main(int argc, const char * argv[]) {
    // insert code here...
    std::cout << "Hello, terrible World!\n";
    
    LearningAgent learningAgent;
    
    // We try with a "stupid" cournot-like model.
    // Each firm chooses its quantity and is rewarded according to its profit
    
    // Cost and demand are linear
    auto demand = [](float p) {
        return (float)(1.f - p);
    };
    auto inverse_demand = [](float q) {
        return (float)(1.f - q);
    };
    
    auto cost = [](float q) {
        return (float)(q/5.f);
    };
    
    std::random_device rd;
    std::mt19937 gen(rd());
    // first firm plays randomly (it decide a quantity between 1 and 100
    std::uniform_real_distribution distribution(0.f,1.f);
    
    // Each firm starts with a budget
    float cumulative_profit = 1;
    
    float random_firm_action = (float)distribution(gen);
    for (std::size_t it(0); it < 15000; it++){
        float our_firm_action = learningAgent.play(random_firm_action);
        float price = inverse_demand(random_firm_action);
        float true_profit = std::max(0.f, price) * our_firm_action - cost(our_firm_action);
        float profit = std::exp(std::max(0.f, price) * our_firm_action - cost(our_firm_action))/std::exp(1);
        
        std::cout << "\nTurnament " << it;
        std::cout << "\nOur learning agent is asked to respond to " << (float)random_firm_action;
        std::cout << "\nHe plays " << our_firm_action;
        std::cout << "\nMarket price set to " << price << " TrueProfit=" << true_profit << " Profit=" << profit << " Cumulative profit=" << cumulative_profit;
        
        cumulative_profit += profit;
        float oldaction = random_firm_action;
        random_firm_action=(float)distribution(gen);
        learningAgent.feedBack(oldaction, our_firm_action, profit, random_firm_action, (cumulative_profit <= 0) );
        if ((cumulative_profit <= 0))
            cumulative_profit = 1;
        std::cout << "\n";
    }
    
    
    
    
    return 0;
}
