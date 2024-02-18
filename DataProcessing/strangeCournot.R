data <- read.csv("DDPG-StrangeCournotWithMem-Output-1562059222.csv")
#Perfect cournot convergence : 
#ata <- read.csv("DDPG-StrangeCournot-Output-1540358815.csv")


data$round <- as.integer(data$round)
data$leaderAction <- as.numeric(data$leaderAction)  
data$followerAction <- as.numeric(data$followerAction)  
data$competitiveAction <- as.numeric(data$competitiveAction)  
data$cournotAction <- as.numeric(data$cournotAction)  
data$leaderBestAction <- as.numeric(data$leaderBestAction)  
data$followerBestAction <- as.numeric(data$followerBestAction)  
data$leaderProfit <- as.numeric(data$leaderProfit)  
data$followerProfit <- as.numeric(data$followerProfit)  
data$stackelbergProfit <- as.numeric(data$stackelbergProfit)  
data$cournotProfit <- as.numeric(data$cournotProfit)  
data$competitiveProfit <- as.numeric(data$competitiveProfit)  
data$leaderEstimatedProfit <- as.numeric(data$leaderEstimatedProfit)  
data$followerEstimatedProfit <- as.numeric(data$followerEstimatedProfit)

transparency <- 1 - 0.7
ggplot(data[data$round>0,], aes(x = round)) +
  geom_point(aes(y = leaderAction, color = "Leader"), alpha = transparency) +
  geom_point(aes(y = followerAction, color = "Follower"), alpha = transparency) +
  geom_line(aes(y = competitiveAction, color = "Competition")) +
  geom_line(aes(y = cournotAction, color = "Cournot")) +
  geom_line(aes(y = leaderBestAction, color = "Stackelberg")) +
  geom_smooth(aes(y = leaderAction, color = "LeaderTrend"), se = FALSE) +
  geom_smooth(aes(y = followerAction, color = "FollowerTrend"), se = FALSE) +
  scale_color_manual(values = c("Leader" = "blue", "Follower" = "red", "LeaderTrend" = "darkblue", "FollowerTrend" = "darkred", "BestFollowerTrend" = "lightpink", "BestLeaderTrend" = "lightblue", "Competition" = "green", "Cournot" = "darkgreen", "Stackelberg"="purple")) +
  labs(x = "Round", y = "Quantity", color = "Legend") +
  theme_minimal()


ggplot(data[data$round>0,], aes(x = round)) +
  geom_smooth(aes(y = leaderEstimatedProfit, color = "Leader", style="dotted"), se = FALSE) +
  geom_smooth(aes(y = followerEstimatedProfit, color = "Follower", style="dotted"), se = FALSE) +
  geom_smooth(aes(y = leaderProfit, color = "LeaderTrend"), se = FALSE) +
  geom_smooth(aes(y = followerProfit, color = "FollowerTrend"), se = FALSE) +
  geom_line(aes(y = cournotProfit, color = "Cournot")) +
  geom_line(aes(y = competitiveProfit, color = "Competition")) +
  geom_line(aes(y = stackelbergProfit, color = "Stackelberg")) +
  scale_color_manual(values = c("Leader" = "blue", "Follower" = "red", "LeaderTrend" = "darkblue", "FollowerTrend" = "darkred", "BestFollowerTrend" = "lightpink", "BestLeaderTrend" = "lightblue", "Competition" = "green", "Cournot" = "darkgreen", "Stackelberg"="purple")) +
  labs(x = "Round", y = "Profit", color = "Actor") +
  theme_minimal()+  
  ggtitle("Profit evolution")  
