data <- read.csv("DDPG-TemporalCournot-2-5748710865.csv")

library(ggplot2)
data$round <- as.integer(data$round)
data$agent1Action <- as.numeric(data$agent1Action)  
data$agent2Action <- as.numeric(data$agent2Action)  
data$agent1Profit <- as.numeric(data$agent1Profit)  
data$agent2Profit <- as.numeric(data$agent2Profit)  
data$whitenoise <- as.numeric(data$whitenoise)  
data$agent1EstimatedProfit <- as.numeric(data$agent1EstimatedProfit)  
data$agent2EstimatedProfit <- as.numeric(data$agent2EstimatedProfit)  

ggplot(data[data$round>0,], aes(x = round)) +
  geom_point(aes(y = agent2Action, color = "agent2"), alpha = 0.1) +
  geom_point(aes(y = agent1Action, color = "agent1"), alpha = 0.1) +
  geom_smooth(aes(y = agent1Action, color = "agent1Trend"), se = FALSE) +
  geom_smooth(aes(y = agent2Action, color = "agent2Trend"), se = FALSE) +
  #geom_smooth(aes(y = agent1Action+agent2Action+agent3Action+agent4Action, color = "totalTrend"), se = FALSE) +
  geom_line(aes(y = 0.9166666666666667, color = "Competition")) +
  geom_line(aes(y = 0.6470588235294118, color = "Cournot")) +
  geom_line(aes(y = 0.5, color = "Cartel")) +
  # geom_line(aes(y = leaderBestAction, color = "Stackelberg")) +
  scale_color_manual(values = c("agent1" = "blue", 
                                "agent2" = "yellow", 
                                "agent3" = "grey", 
                                "agent4" = "red", 
                                "agent1Trend" = "darkblue", 
                                "agent2Trend" = "yellow",
                                "agent3Trend" = "darkgrey",
                                "agent4Trend" = "darkred",
                                "totalTrend" = "black",
                                "Competition" = "green", 
                                "Cournot" = "darkgreen", 
                                "Cartel"="purple")) +
  labs(x = "Round", y = "Quantity", color = "Legend") +
  theme_minimal()

ggplot(data[data$round>0&data$whitenoise==0,], aes(x = round)) +
  geom_point(aes(y = agent1Action+agent2Action, color = "total"), alpha = 0.1) +
  geom_smooth(aes(y = agent1Action+agent2Action, color = "totalTrend"), se = FALSE) +
  geom_line(aes(y = 2*0.9166666666666667, color = "Competition")) +
  geom_line(aes(y = 2*0.6470588235294118, color = "Cournot")) +
  geom_line(aes(y = 2*0.5, color = "Cartel")) +
  scale_color_manual(values = c("total" = "darkgrey",
                                "totalTrend" = "black",
                                "Competition" = "green", 
                                "Cournot" = "darkgreen", 
                                "Cartel"="purple")) +
  labs(x = "Round", y = "Quantity", color = "Legend") +
  theme_minimal()


ggplot(data[data$round>0&data$whitenoise==0,], aes(x = round)) +
  geom_smooth(aes(y = agent1EstimatedProfit, color = "agent1Estimated"), se = FALSE,linetype="dotted") +
  geom_smooth(aes(y = agent2EstimatedProfit, color = "agent2Estimated"), se = FALSE,linetype="dotted") +
  geom_smooth(aes(y = agent1Profit, color = "agent1"), se = FALSE) +
  geom_smooth(aes(y = agent2Profit, color = "agent2"), se = FALSE) +
  geom_line(aes(y = 0.49237370242214545, color = "Cournot")) +
  geom_line(aes(y = 0.16469444444444442, color = "Competition")) +
  geom_line(aes(y = 0.539, color = "Cartel")) +
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
                                "Cartel"="purple")) +
  labs(x = "Round", y = "Profit", color = "Actor") +
  theme_minimal()+  
  ggtitle("Profit evolution")  

# Should converge toward 0.6470588235294118
convergence_test(data[data$whitenoise==0,]$agent1Action, 0.98)
convergence_test(data[data$whitenoise==0,]$agent2Action, 0.98)
# Should converge toward 1.294118
convergence_test(data[data$whitenoise==0,]$agent1Action + data[data$whitenoise==0,]$agent2Action, 0.98)

#should converge toward 0.49237370242214545
convergence_test(data[data$whitenoise==0,]$agent1Profit + data[data$whitenoise==0,]$agent2Profit, 0.98)
