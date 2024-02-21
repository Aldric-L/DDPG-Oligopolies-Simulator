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

// Model parameters, for calibration see the python notebook
#define C 0.2
#define D 2.2
#define K 0.98
#define B 0
#define A 0.45

//#define STUPID_DEMO 1
//#define PSEUDO_STACKELBERG 1
//#define PSEUDO_COURNOT 1
#define PSEUDO_COURNOT_OLIGOPOLY 1
//#define STRANGE_COURNOT 1

int main(int argc, const char * argv[]) {
    auto demand = [](float p) { return (float)(D - p); };
    auto inverse_demand = [](float q) { return (float)(D - q); };
    
    auto cost = [](float q) { return (float)(C*std::pow(q,2)); };
    
    auto profitFunc = [cost](float p, float q) {
        return (float)( K * ( std::max(0.f, p) * q - cost(q)) + B  );
    };
    
    float bestLeaderAction = (float)( ( D*(1.f+2.f*C) ) / (2.f* (1.f+ 4.f*C + std::pow(C,2)) ) );
    float bestCournotAction = (float)( ( D*(1.f+2.f*C) ) / (3.f+ 8.f*C + 4.f * std::pow(C,2)) );
    auto bestResponseFollower = [](float q) { return (float)((D-q)/(2.f+2.f*C)); };
    float bestResponseCompetition = (float)( D/(2.f+2.f*C) );

    #ifdef STUPID_DEMO
    typedef akml::Save<7, std::size_t, float, float, float, float, float, float> LogSaveType;
    akml::CSV_Saver<LogSaveType> LogManager;
    LogSaveType::default_parameters_name = {{ "round", "random_action", "action", "bestAction", "profit", "whitenoise", "estimated_profit" }};
    
    LearningAgent learningAgent;
    
    // We try with a "stupid" cournot-like model.
    // Each firm chooses its quantity and is rewarded according to its profit
    
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
    std::size_t maxiterations(60000);
    LogManager.reserve(2001);
    struct episode {
        float leaderAction;
        float followerAction;
        float leaderProfit;
        float followerProfit;
    };
    std::vector<episode> buffer; buffer.reserve(128);
    std::thread logBufferize;
    for (std::size_t iteration(0); iteration <= maxiterations/128; iteration++){
        buffer.clear();
        for (std::size_t inside_iteration(0); inside_iteration < 128; inside_iteration++){
            std::size_t it = inside_iteration + iteration*128;
            float whitenoise;
            if (/*maxiterations > 6000*/ false)
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
            std::cout << "\nBestResponses: Leader=" << bestLeaderAction << " Follower=" << bestResponseFollower(leaderAction);
            std::cout << "\nMarket price set to " << price << " Profits : L=" << leaderProfit << " (e: " << leaderEstimatedProfit << ") F=" << followerProfit << " (e: " << followerEstimatedProfit << ")";

            if (it%2000 == 0)
                LogManager.bufferize();
                //logBufferize = std::thread([](akml::CSV_Saver<LogSaveType>* saver) { saver->bufferize(); }, &LogManager);

            LogManager.addSave(it, leaderAction, followerAction, bestLeaderAction, bestResponseFollower(leaderAction), leaderProfit, followerProfit, whitenoise, leaderEstimatedProfit, followerEstimatedProfit);
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
    if (logBufferize.joinable())
        logBufferize.join();
    logBufferize = std::thread([curt](akml::CSV_Saver<LogSaveType>* saver) { saver->saveToCSV("DDPG-NaiveStackelberg-Output-" + curt + ".csv", false); }, &LogManager);
    //logBufferize = std::thread(&akml::CSV_Saver<LogSaveType>::saveToCSV, &LogManager, "DDPG-NaiveStackelberg-Output-" + curt + ".csv", false);
    //LogManager.saveToCSV("DDPG-NaiveStackelberg-Output-" + curt + ".csv", false);
    
    typedef akml::Save<7, float, float, float, float, float, float, float> CriticLogSaveType;
    akml::CSV_Saver<CriticLogSaveType> CriticLogManager;
    CriticLogSaveType::default_parameters_name = {{ "leaderAction", "followerAction", "leaderProfit", "followerProfit", "leaderEstimatedProfit", "followerEstimatedProfit", "price" }};
    
    for (float leaderAction(0); leaderAction < 1; leaderAction += 0.01){
        for (float followerAction(0); followerAction < 1; followerAction += 0.01){
            float price = inverse_demand(leaderAction+followerAction);
            CriticLogManager.addSave(leaderAction, followerAction, profitFunc(price, leaderAction), profitFunc(price, followerAction), leader.askCritic(0.f, leaderAction), follower.askCritic(leaderAction, leaderAction), price);
        }
    }
    CriticLogManager.saveToCSV("DDPG-NaiveStackelberg-Critic-" + curt + ".csv", false);
    if (logBufferize.joinable())
        logBufferize.join();
    #endif
    
    
    #ifdef PSEUDO_COURNOT
    typedef akml::Save<10, std::size_t, float, float, float, float, float, float, float, float, float> LogSaveType;
    akml::CSV_Saver<LogSaveType> LogManager;
    LogSaveType::default_parameters_name = {{ "round", "leaderAction", "followerAction", "leaderBestAction", "followerBestAction", "leaderProfit", "followerProfit", "whitenoise", "leaderEstimatedProfit", "followerEstimatedProfit" }};

    LearningAgent leader ("Leader");
    LearningAgent follower ("Follower");

    // We try with a naive cournot-like model, without memory.
    // Each firm chooses its quantity and is rewarded according to its profit
    // We keep the leader/follower names only for compatibility. Both firms are equal
    // They only see what have been made in the two previous turns

    std::size_t maxiterations(100000);
    LogManager.reserve(2001);
    struct episode {
        float leaderAction;
        float followerAction;
        float leaderProfit;
        float followerProfit;
        float previousLeaderAction;
        float previousFollowerAction;
    };
    std::vector<episode> buffer; buffer.reserve(128);
    std::thread logBufferize;
    float previousLeaderAction = 0.f;
    float previousFollowerAction = 0.f;
    for (std::size_t iteration(0); iteration <= maxiterations/128; iteration++){
        buffer.clear();
        for (std::size_t inside_iteration(0); inside_iteration < 128; inside_iteration++){
            std::size_t it = inside_iteration + iteration*128;
            float whitenoise;
            if (/*maxiterations > 6000*/ false)
                whitenoise = ((it % 10 == 0) ? 0.f : 0.3 * std::max((1.f-(float)it/(float)(maxiterations-2000)), 0.f));
            else
                whitenoise = ((it % 10 == 0) ? 0.f : 0.3 * (1.f-(float)it/(float)maxiterations));
            
            float leaderAction = leader.play(previousFollowerAction, whitenoise);
            float followerAction = follower.play(previousLeaderAction, whitenoise);
            
            float price = inverse_demand(leaderAction+followerAction);
            float leaderProfit = profitFunc(price, leaderAction);
            float followerProfit = profitFunc(price, followerAction);
            float leaderEstimatedProfit = leader.askCritic(0.f, leaderAction);
            float followerEstimatedProfit = follower.askCritic(leaderAction, leaderAction);
            
            std::cout << "\nTurnament " << it << " (Wn=" << whitenoise << ")";
            std::cout << "\nActions: Leader=" << (float)leaderAction << " Follower=" << followerAction;
            std::cout << "\nBestResponses: Leader=" << bestCournotAction << " Follower=" << bestCournotAction;
            std::cout << "\nMarket price set to " << price << " Profits : L=" << leaderProfit << " (e: " << leaderEstimatedProfit << ") F=" << followerProfit << " (e: " << followerEstimatedProfit << ")";

            if (it%2000 == 0)
                LogManager.bufferize();
                //logBufferize = std::thread([](akml::CSV_Saver<LogSaveType>* saver) { saver->bufferize(); }, &LogManager);

            LogManager.addSave(it, leaderAction, followerAction, bestCournotAction, bestCournotAction, leaderProfit, followerProfit, whitenoise, leaderEstimatedProfit, followerEstimatedProfit);
            std::cout << "\n";
            buffer.push_back({
                .leaderAction = leaderAction,
                .followerAction = followerAction,
                .leaderProfit = leaderProfit,
                .followerProfit = followerProfit,
                .previousLeaderAction = previousLeaderAction,
                .previousFollowerAction = previousFollowerAction
            });
            previousLeaderAction = leaderAction;
            previousFollowerAction = previousFollowerAction;
        }
        for (std::size_t inside_iteration(0); inside_iteration < 127; inside_iteration++){
            leader.feedBack(buffer.at(inside_iteration).previousFollowerAction, buffer.at(inside_iteration).leaderAction, buffer.at(inside_iteration).leaderProfit, 0.f, inside_iteration == 126, false );
            follower.feedBack(buffer.at(inside_iteration).previousLeaderAction, buffer.at(inside_iteration).followerAction,  buffer.at(inside_iteration).followerProfit, buffer.at(inside_iteration+1).leaderAction, inside_iteration == 126, false );
        }
        std::thread leaderThread(&LearningAgent::manualTrainingLaunch, &leader);
        std::thread followerThread(&LearningAgent::manualTrainingLaunch, &follower);
        leaderThread.join();
        followerThread.join();
    }

    long int t = static_cast<long int> (std::clock());
    std::string curt = std::to_string(t);
    if (logBufferize.joinable())
        logBufferize.join();
    logBufferize = std::thread([curt](akml::CSV_Saver<LogSaveType>* saver) { saver->saveToCSV("DDPG-NaiveCournot-Output-" + curt + ".csv", false); }, &LogManager);

    if (logBufferize.joinable())
        logBufferize.join();
    #endif
    
    #ifdef STRANGE_COURNOT
    typedef akml::Save<15, std::size_t, float, float, float, float, float, float, float, float, float,float, float, float, float, float> LogSaveType;
    akml::CSV_Saver<LogSaveType> LogManager;
    LogSaveType::default_parameters_name = {{ "round", "leaderAction", "followerAction", "leaderBestAction", "followerBestAction", "competitiveAction", "cournotAction", "leaderProfit", "followerProfit", "whitenoise", "leaderEstimatedProfit", "followerEstimatedProfit", "stackelbergProfit", "cournotProfit", "competitiveProfit" }};

    LearningAgent leader ("Leader");
    LearningAgent follower ("Follower");

    // We try with a strange cournot-like model, without memory.
    // Each firm chooses its quantity and is rewarded according to its profit
    // We keep the leader/follower names only for compatibility. Both firms are equal
    // We delete the assumption that firms can observe other firm's moove
    // Hence, each firm only sees the price at last period
    
    // Currently, we test the adjunction of memory
    LearningAgent::gamma = 0.1;

    std::size_t maxiterations(60000);
    LogManager.reserve(2001);
    struct episode {
        float leaderAction;
        float followerAction;
        float leaderProfit;
        float followerProfit;
        float prevPrice;
        float newPrice;
    };
    float prevprice = 0.f;
    std::vector<episode> buffer; buffer.reserve(128);
    std::thread logBufferize;
    for (std::size_t iteration(0); iteration <= maxiterations/128; iteration++){
        buffer.clear();
        for (std::size_t inside_iteration(0); inside_iteration < 128; inside_iteration++){
            std::size_t it = inside_iteration + iteration*128;
            float whitenoise;
            if (/*maxiterations > 6000*/ false)
                whitenoise = ((it % 10 == 0) ? 0.f : 0.3 * std::max((1.f-(float)it/(float)(maxiterations-2000)), 0.f));
            else
                whitenoise = ((it % 10 == 0) ? 0.f : 0.3 * (1.f-(float)it/(float)maxiterations));
            
            float leaderAction = leader.play(prevprice, whitenoise);
            float followerAction = follower.play(prevprice, whitenoise);
            
            float price = inverse_demand(leaderAction+followerAction);
            float leaderProfit = profitFunc(price, leaderAction);
            float followerProfit = profitFunc(price, followerAction);
            float leaderEstimatedProfit = leader.askCritic(prevprice, leaderAction);
            float followerEstimatedProfit = follower.askCritic(prevprice, followerAction);
            
            std::cout << "\nTurnament " << it << " (Wn=" << whitenoise << ")";
            std::cout << "\nActions: Leader=" << (float)leaderAction << " Follower=" << followerAction;
            //std::cout << "\nBestResponses: Leader=" << bestLeaderAction << " Follower=" << bestResponse(leaderAction);
            std::cout << "\nMarket price set to " << price << " Profits : L=" << leaderProfit << " (e: " << leaderEstimatedProfit << ") F=" << followerProfit << " (e: " << followerEstimatedProfit << ")";

            if (it%2000 == 0)
                LogManager.bufferize();
                //logBufferize = std::thread([](akml::CSV_Saver<LogSaveType>* saver) { saver->bufferize(); }, &LogManager);
            
            LogManager.addSave(it, leaderAction, followerAction, bestLeaderAction, bestResponseFollower(leaderAction), bestResponseCompetition, bestCournotAction, leaderProfit, followerProfit, whitenoise, leaderEstimatedProfit, followerEstimatedProfit, profitFunc(inverse_demand(bestLeaderAction+ bestLeaderAction), bestLeaderAction), profitFunc(inverse_demand(bestCournotAction+ bestCournotAction), bestCournotAction), profitFunc(inverse_demand(bestResponseCompetition+ bestResponseCompetition), bestResponseCompetition) );
            std::cout << "\n";
            buffer.push_back({
                .leaderAction = leaderAction,
                .followerAction = followerAction,
                .leaderProfit = leaderProfit,
                .followerProfit = followerProfit,
                .prevPrice = prevprice,
                .newPrice = price
            });
            prevprice = price;
        }
        for (std::size_t inside_iteration(0); inside_iteration < 127; inside_iteration++){
            leader.feedBack(A * buffer.at(inside_iteration).prevPrice, buffer.at(inside_iteration).leaderAction, buffer.at(inside_iteration).leaderProfit, buffer.at(inside_iteration).newPrice, inside_iteration == 126, false );
            follower.feedBack(A * buffer.at(inside_iteration).prevPrice, buffer.at(inside_iteration).followerAction,  buffer.at(inside_iteration).followerProfit, buffer.at(inside_iteration).newPrice, inside_iteration == 126, false );
        }
        std::thread leaderThread(&LearningAgent::manualTrainingLaunch, &leader);
        std::thread followerThread(&LearningAgent::manualTrainingLaunch, &follower);
        leaderThread.join();
        followerThread.join();
    }

    long int t = static_cast<long int> (std::clock());
    std::string curt = std::to_string(t);
    if (logBufferize.joinable())
        logBufferize.join();
    logBufferize = std::thread([curt](akml::CSV_Saver<LogSaveType>* saver) { saver->saveToCSV("DDPG-StrangeCournot-Output-" + curt + ".csv", false); }, &LogManager);

    if (logBufferize.joinable())
        logBufferize.join();
    #endif
    
    #ifdef PSEUDO_COURNOT_OLIGOPOLY
    typedef akml::Save<14, std::size_t, float, float, float, float, float, float, float, float, float,float, float, float, float> LogSaveType;
    akml::CSV_Saver<LogSaveType> LogManager;
    LogSaveType::default_parameters_name = {{ "round", "agent1Action", "agent2Action", "agent3Action", "agent4Action", "agent1Profit", "agent2Profit", "agent3Profit", "agent4Profit","whitenoise", "agent1EstimatedProfit", "agent2EstimatedProfit", "agent3EstimatedProfit", "agent4EstimatedProfit" }};

    LearningAgent agent1 ("Agent1");
    LearningAgent agent2 ("Agent2");
    LearningAgent agent3 ("Agent3");
    LearningAgent agent4 ("Agent4");

    // We try with a strange cournot-like model, without memory.
    // Each firm chooses its quantity and is rewarded according to its profit
    // We keep the leader/follower names only for compatibility. Both firms are equal
    // We delete the assumption that firms can observe other firm's moove
    // Hence, each firm only sees the price at last period

    // Currently, we test the adjunction of memory
    LearningAgent::gamma = 0.3;

    std::size_t maxiterations(60000);
    LogManager.reserve(2001);
    struct episode {
        float agent1Action;
        float agent2Action;
        float agent3Action;
        float agent4Action;
        float agent1Profit;
        float agent2Profit;
        float agent3Profit;
        float agent4Profit;
        float prevPrice;
        float newPrice;
    };
    float prevprice = 0.f;
    std::vector<episode> buffer; buffer.reserve(128);
    std::thread logBufferize;
    for (std::size_t iteration(0); iteration <= maxiterations/128; iteration++){
        buffer.clear();
        for (std::size_t inside_iteration(0); inside_iteration < 128; inside_iteration++){
            std::size_t it = inside_iteration + iteration*128;
            float whitenoise;
            if (/*maxiterations > 6000*/ false)
                whitenoise = ((it % 10 == 0) ? 0.f : 0.3 * std::max((1.f-(float)it/(float)(maxiterations-2000)), 0.f));
            else
                whitenoise = ((it % 10 == 0) ? 0.f : 0.3 * (1.f-(float)it/(float)maxiterations));
            
            float agent1Action = agent1.play(prevprice, whitenoise);
            float agent2Action = agent2.play(prevprice, whitenoise);
            float agent3Action = agent3.play(prevprice, whitenoise);
            float agent4Action = agent4.play(prevprice, whitenoise);
            
            float price = inverse_demand(agent1Action+agent2Action+agent3Action+agent4Action);
            float agent1Profit = profitFunc(price, agent1Action);
            float agent2Profit = profitFunc(price, agent2Action);
            float agent3Profit = profitFunc(price, agent3Action);
            float agent4Profit = profitFunc(price, agent4Action);
            float agent1EstimatedProfit = agent1.askCritic(prevprice, agent1Action);
            float agent2EstimatedProfit = agent2.askCritic(prevprice, agent2Action);
            float agent3EstimatedProfit = agent3.askCritic(prevprice, agent3Action);
            float agent4EstimatedProfit = agent4.askCritic(prevprice, agent4Action);
            
            //if (it%2000 == 0)
                //LogManager.bufferize();
                //logBufferize = std::thread([](akml::CSV_Saver<LogSaveType>* saver) { saver->bufferize(); }, &LogManager);
            LogManager.addSave(it, agent1Action, agent2Action, agent3Action, agent4Action, agent1Profit, agent2Profit, agent3Profit, agent4Profit, whitenoise, agent1EstimatedProfit, agent2EstimatedProfit, agent3EstimatedProfit, agent4EstimatedProfit );
            
            if (it%2000 == 0)
                logBufferize = std::thread([](akml::CSV_Saver<LogSaveType>* saver) { saver->bufferize(); }, &LogManager);

            std::cout << "\n";
            buffer.push_back({
                .agent1Action = agent1Action,
                .agent2Action = agent2Action,
                .agent3Action = agent3Action,
                .agent4Action = agent4Action,
                .agent1Profit = agent1Profit,
                .agent2Profit = agent2Profit,
                .agent3Profit = agent3Profit,
                .agent4Profit = agent4Profit,
                .prevPrice = prevprice,
                .newPrice = price
            });
            prevprice = price;
        }
        for (std::size_t inside_iteration(0); inside_iteration < 127; inside_iteration++){
            agent1.feedBack(A * buffer.at(inside_iteration).prevPrice, buffer.at(inside_iteration).agent1Action, buffer.at(inside_iteration).agent1Profit, buffer.at(inside_iteration).newPrice, inside_iteration == 126, false );
            agent2.feedBack(A * buffer.at(inside_iteration).prevPrice, buffer.at(inside_iteration).agent2Action, buffer.at(inside_iteration).agent2Profit, buffer.at(inside_iteration).newPrice, inside_iteration == 126, false );
            agent3.feedBack(A * buffer.at(inside_iteration).prevPrice, buffer.at(inside_iteration).agent3Action, buffer.at(inside_iteration).agent3Profit, buffer.at(inside_iteration).newPrice, inside_iteration == 126, false );
            agent4.feedBack(A * buffer.at(inside_iteration).prevPrice, buffer.at(inside_iteration).agent4Action, buffer.at(inside_iteration).agent4Profit, buffer.at(inside_iteration).newPrice, inside_iteration == 126, false );
        }
        std::thread agent1Thread(&LearningAgent::manualTrainingLaunch, &agent1);
        std::thread agent2Thread(&LearningAgent::manualTrainingLaunch, &agent2);
        std::thread agent3Thread(&LearningAgent::manualTrainingLaunch, &agent3);
        std::thread agent4Thread(&LearningAgent::manualTrainingLaunch, &agent4);
        agent1Thread.join();
        agent2Thread.join();
        agent3Thread.join();
        agent4Thread.join();
        if (logBufferize.joinable())
            logBufferize.join();
    }

    long int t = static_cast<long int> (std::clock());
    std::string curt = std::to_string(t);
    if (logBufferize.joinable())
        logBufferize.join();
    logBufferize = std::thread([curt](akml::CSV_Saver<LogSaveType>* saver) { saver->saveToCSV("DDPG-StrangeCournot4Oligopoly-Output-" + curt + ".csv", false); }, &LogManager);

    if (logBufferize.joinable())
        logBufferize.join();
    #endif
    

    return 0;
}
