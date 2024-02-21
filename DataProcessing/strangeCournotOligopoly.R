data <- read.csv("DDPG-StrangeCournot4Oligopoly-Output-3568542506(gamma=0.3).csv")

data$round <- as.integer(data$round)
data$agent1Action <- as.numeric(data$agent1Action)  
data$agent2Action <- as.numeric(data$agent2Action)  
data$agent3Action <- as.numeric(data$agent3Action)  
data$agent4Action <- as.numeric(data$agent4Action)  
data$agent1Profit <- as.numeric(data$agent1Profit)  
data$agent2Profit <- as.numeric(data$agent2Profit)  
data$agent3Profit <- as.numeric(data$agent3Profit)  
data$agent4Profit <- as.numeric(data$agent4Profit)  
data$whitenoise <- as.numeric(data$whitenoise)  
data$agent1EstimatedProfit <- as.numeric(data$agent1EstimatedProfit)  
data$agent2EstimatedProfit <- as.numeric(data$agent2EstimatedProfit)  
data$agent3EstimatedProfit <- as.numeric(data$agent3EstimatedProfit)  
data$agent4EstimatedProfit <- as.numeric(data$agent4EstimatedProfit)  

ggplot(data[data$round>0,], aes(x = round)) +
  geom_point(aes(y = agent4Action, color = "agent4"), alpha = 0.1) +
  geom_point(aes(y = agent1Action, color = "agent1"), alpha = 0.1) +
  geom_point(aes(y = agent2Action, color = "agent2"), alpha = 0.1) +
  geom_point(aes(y = agent3Action, color = "agent3"), alpha = 0.1) +
  geom_smooth(aes(y = agent1Action, color = "agent1Trend"), se = FALSE) +
  geom_smooth(aes(y = agent2Action, color = "agent2Trend"), se = FALSE) +
  geom_smooth(aes(y = agent3Action, color = "agent3Trend"), se = FALSE) +
  geom_smooth(aes(y = agent4Action, color = "agent4Trend"), se = FALSE) +
  geom_line(aes(y = 0.2, color = "Competition")) +
  geom_line(aes(y = 0.40740740740740744, color = "Cournot")) +
  # geom_line(aes(y = leaderBestAction, color = "Stackelberg")) +
  scale_color_manual(values = c("agent1" = "blue", 
                                "agent2" = "yellow", 
                                "agent3" = "grey", 
                                "agent4" = "red", 
                                "agent1Trend" = "darkblue", 
                                "agent2Trend" = "yellow",
                                "agent3Trend" = "darkgrey",
                                "agent4Trend" = "darkred",
                                "Competition" = "green", 
                                "Cournot" = "darkgreen", 
                                "Stackelberg"="purple")) +
  labs(x = "Round", y = "Quantity", color = "Legend") +
  theme_minimal()


ggplot(data[data$round>0,], aes(x = round)) +
  geom_smooth(aes(y = agent1EstimatedProfit, color = "agent1Estimated"), se = FALSE,linetype="dotted") +
  geom_smooth(aes(y = agent2EstimatedProfit, color = "agent2Estimated"), se = FALSE,linetype="dotted") +
  geom_smooth(aes(y = agent3EstimatedProfit, color = "agent3Estimated"), se = FALSE,linetype="dotted") +
  geom_smooth(aes(y = agent4EstimatedProfit, color = "agent4Estimated"), se = FALSE,linetype="dotted") +
  geom_smooth(aes(y = agent1Profit, color = "agent1"), se = FALSE) +
  geom_smooth(aes(y = agent2Profit, color = "agent2"), se = FALSE) +
  geom_smooth(aes(y = agent3Profit, color = "agent3"), se = FALSE) +
  geom_smooth(aes(y = agent4Profit, color = "agent4"), se = FALSE) +
  geom_line(aes(y = 0.1951934156378601, color = "Cournot")) +
  geom_line(aes(y = 0.26655999999999996, color = "Competition")) +
  #geom_line(aes(y = stackelbergProfit, color = "Stackelberg")) +
  scale_color_manual(values = c("agent1" = "blue", 
                                "agent2" = "yellow", 
                                "agent3" = "grey", 
                                "agent4" = "red", 
                                "agent1Estimated" = "darkblue", 
                                "agent2Estimated" = "yellow",
                                "agent3Estimated" = "darkgrey",
                                "agent4Estimated" = "darkred",
                                "Competition" = "green", 
                                "Cournot" = "darkgreen", 
                                "Stackelberg"="purple")) +
  labs(x = "Round", y = "Profit", color = "Actor") +
  theme_minimal()+  
  ggtitle("Profit evolution")  

# Should converge toward 0.40740740740740744
convergence_test(data[data$whitenoise==0,]$agent1Action, 0.9)
convergence_test(data[data$whitenoise==0,]$agent2Action, 0.9)
convergence_test(data[data$whitenoise==0,]$agent3Action, 0.9)
convergence_test(data[data$whitenoise==0,]$agent4Action, 0.9)
# Should converge toward 1.6296296296296298
convergence_test(data[data$whitenoise==0,]$agent1Action + data[data$whitenoise==0,]$agent2Action + 
                   data[data$whitenoise==0,]$agent3Action + data[data$whitenoise==0,]$agent4Action, 0.9)


