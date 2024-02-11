//
//  main.cpp
//  CournotDDPG
//
//  Created by Aldric Labarthe on 26/01/2024.
//

#include <iostream>
#include <functional>
#include <thread>
#include "AKML-lib/AKML.hpp"
#include "AKML-lib/AgentBasedUtilities/CSV_Saver.hpp"
#include "LearningAgent.hpp"

#define C 1.f
#define D 3.f

//#define STUPID_DEMO 1
#define PSEUDO_STACKELBERG 1

int main(int argc, const char * argv[]) {
    // insert code here...
    std::cout << "Hello, terrible World!\n";

    #ifdef STUPID_DEMO
    typedef akml::Save<7, std::size_t, float, float, float, float, float, float> LogSaveType;
    akml::CSV_Saver<LogSaveType> LogManager;
    LogSaveType::default_parameters_name = {{ "round", "random_action", "action", "bestAction", "profit", "whitenoise", "estimated_profit" }};
    
    LearningAgent learningAgent;
    
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
    
    #ifdef PSEUDO_STACKELBERG
    typedef akml::Save<10, std::size_t, float, float, float, float, float, float, float, float, float> LogSaveType;
    akml::CSV_Saver<LogSaveType> LogManager;
    LogSaveType::default_parameters_name = {{ "round", "leaderAction", "followerAction", "leaderBestAction", "followerBestAction", "leaderProfit", "followerProfit", "whitenoise", "leaderEstimatedProfit", "followerEstimatedProfit" }};

    LearningAgent leader ("Leader");
    LearningAgent follower ("Follower");

    // We try with a naive stackelberg-like model, without memory.
    // Each firm chooses its quantity and is rewarded according to its profit
    // Firm 1 is the leader (i.e. it plays first, firm 2 the follower)

    // Cost and demand are linear
    auto demand = [](float p) { return (float)(D - p); };
    auto inverse_demand = [](float q) { return (float)(D - q); };
    auto cost = [](float q) { return (float)(C*std::pow(q,2)); };
    auto bestResponse = [](float q) { return (float)((D-q)/(2.f+2.f*C))/1.2; };
    
    float bestLeaderAction = (float)((D*(1.f+2.f*C))/(2.f*(1.f+4.f*C+std::pow(C,2))))/1.2;

    auto profitFunc = [cost](float p, float q) {
        return (float)(std::max(0.f, p) * q - cost(q))/1.2;
    };

    std::size_t maxiterations(11000);
    LogManager.reserve(2001);
    struct episode {
        float leaderAction;
        float followerAction;
        float leaderProfit;
        float followerProfit;
    };
    std::vector<episode> buffer; buffer.reserve(128);
    for (std::size_t iteration(0); iteration <= maxiterations/128; iteration++){
        buffer.clear();
        for (std::size_t inside_iteration(0); inside_iteration < 128; inside_iteration++){
            std::size_t it = inside_iteration + iteration*128;
            float whitenoise;
            if (maxiterations > 6000)
                whitenoise = ((it % 10 == 0) ? 0.f : 0.3 * std::max((1.f-(float)it/(float)(maxiterations-2000)), 0.f));
            else
                whitenoise = ((it % 10 == 0) ? 0.f : 0.3 * (1.f-(float)it/(float)maxiterations));
            
            float leaderAction = leader.play(0.f, whitenoise);
            float followerAction = follower.play(leaderAction, whitenoise);
            
            float price = inverse_demand(leaderAction+followerAction);
            float leaderProfit = profitFunc(price, leaderAction);
            float followerProfit = profitFunc(price, followerAction);
            float leaderEstimatedProfit = leader.askCritic(0.f, leaderAction);
            float followerEstimatedProfit = follower.askCritic(leaderAction, leaderAction);
            
            std::cout << "\nTurnament " << it << " (Wn=" << whitenoise << ")";
            std::cout << "\nActions: Leader=" << (float)leaderAction << " Follower=" << followerAction;
            std::cout << "\nBestResponses: Leader=" << bestLeaderAction << " Follower=" << bestResponse(leaderAction);
            std::cout << "\nMarket price set to " << price << " Profits : L=" << leaderProfit << " (e: " << leaderEstimatedProfit << ") F=" << followerProfit << " (e: " << followerEstimatedProfit << ")";

            if (it%2000 == 0)
                LogManager.bufferize();
            LogManager.addSave(it, leaderAction, followerAction, bestLeaderAction, bestResponse(leaderAction), leaderProfit, followerProfit, whitenoise, leaderEstimatedProfit, followerEstimatedProfit);
            std::cout << "\n";
            buffer.push_back({
                .leaderAction = leaderAction,
                .followerAction = followerAction,
                .leaderProfit = leaderProfit,
                .followerProfit = followerProfit
            });
        }
        for (std::size_t inside_iteration(0); inside_iteration < 127; inside_iteration++){
            leader.feedBack(0.f, buffer.at(inside_iteration).leaderAction, buffer.at(inside_iteration).leaderProfit, 0.f, true, false );
            follower.feedBack(buffer.at(inside_iteration).leaderAction, buffer.at(inside_iteration).followerAction,  buffer.at(inside_iteration).followerProfit, buffer.at(inside_iteration+1).leaderAction, true, false );
        }
        std::thread leaderThread(&LearningAgent::manualTrainingLaunch, &leader);
        std::thread followerThread(&LearningAgent::manualTrainingLaunch, &follower);
        leaderThread.join();
        followerThread.join();
    }

    long int t = static_cast<long int> (std::clock());
    std::string curt = std::to_string(t);
    LogManager.saveToCSV("DDPG-NaiveStackelberg-Output-" + curt + ".csv", false);
    #endif
    

    return 0;
}
