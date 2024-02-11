data <- read.csv("DDPG-NaiveStackelberg-Output-1977062708.csv")
data$round <- as.integer(data$round)
data$leaderAction <- as.numeric(data$leaderAction)  
data$followerAction <- as.numeric(data$followerAction)  
data$leaderBestAction <- as.numeric(data$leaderBestAction)  
data$followerBestAction <- as.numeric(data$followerBestAction)  
data$leaderProfit <- as.numeric(data$leaderProfit)  
data$followerProfit <- as.numeric(data$followerProfit)  
data$whitenoise <- as.numeric(data$whitenoise)  
data$leaderEstimatedProfit <- as.numeric(data$leaderEstimatedProfit)  
data$followerEstimatedProfit <- as.numeric(data$followerEstimatedProfit) 


#data$leaderBestAction <- data$leaderBestAction/1.2

data$leaderError <- data$leaderAction - data$leaderBestAction
data$leaderAbsError <- abs(data$leaderError)
data$leaderSquaredError <- (data$leaderError)^2

data$followerError <- data$followerAction - data$followerBestAction
data$followerAbsError <- abs(data$followerError)
data$followerSquaredError <- (data$followerError)^2

ggplot(data[data$whitenoise==0,], aes(x = round)) +
  geom_point(aes(y = leaderSquaredError, color = "Leader" )) +  
  geom_point(aes(y = followerSquaredError, color = "Follower" )) +  
  geom_smooth(aes(y = leaderSquaredError, color = "LeaderTrend" )) +  
  geom_smooth(aes(y = followerSquaredError, color = "FollowerTrend" )) +  
  scale_color_manual(values = c("Leader" = "blue", "Follower" = "red", "LeaderTrend" = "darkblue", "FollowerTrend" = "darkred")) +
  labs(x = "Round", y = "Squared Error") +  
  ggtitle("Squared Error vs Round without whitenoise")  

ggplot(data, aes(x = round)) +
  geom_point(aes(y = leaderSquaredError, color = "Leader" )) +  
  geom_point(aes(y = followerSquaredError, color = "Follower" )) +  
  geom_smooth(aes(y = leaderSquaredError, color = "LeaderTrend" )) +  
  geom_smooth(aes(y = followerSquaredError, color = "FollowerTrend" )) +  
  scale_color_manual(values = c("Leader" = "blue", "Follower" = "red", "LeaderTrend" = "darkblue", "FollowerTrend" = "darkred")) +
  labs(x = "Round", y = "Squared Error") +  
  ggtitle("Squared Error vs Round with whitenoise")  

transparency <- 1 - 0.5
ggplot(data[data$round>0,], aes(x = round)) +
  geom_point(aes(y = leaderAction, color = "Leader"), alpha = transparency) +
  geom_point(aes(y = followerAction, color = "Follower"), alpha = transparency) +
  geom_smooth(aes(y = leaderAction, color = "LeaderTrend"), se = FALSE) +
  geom_smooth(aes(y = followerAction, color = "FollowerTrend"), se = FALSE) +
  geom_smooth(aes(y = followerBestAction, color = "BestFollowerTrend"), se = FALSE) +
  geom_line(aes(y = leaderBestAction, color="BestLeaderTrend")) +
  scale_color_manual(values = c("Leader" = "blue", "Follower" = "red", "LeaderTrend" = "darkblue", "FollowerTrend" = "darkred", "BestFollowerTrend" = "lightpink", "BestLeaderTrend" = "lightblue")) +
  labs(x = "Round", y = "Action", color = "Actor") +
  theme_minimal()

ggplot(data[data$round>0,], aes(x = round)) +
  geom_point(aes(y = leaderProfit, color = "Leader"), alpha = transparency) +
  geom_point(aes(y = followerProfit, color = "Follower"), alpha = transparency) +
  geom_smooth(aes(y = leaderEstimatedProfit, color = "EstimatedLeaderTrend"), se = FALSE) +
  geom_smooth(aes(y = followerEstimatedProfit, color = "EstimatedFollowerTrend"), se = FALSE) +
  geom_smooth(aes(y = leaderProfit, color = "LeaderTrend"), se = FALSE) +
  geom_smooth(aes(y = followerProfit, color = "FollowerTrend"), se = FALSE) +
  scale_color_manual(values = c("Leader" = "blue", "Follower" = "red", "LeaderTrend" = "darkblue", "FollowerTrend" = "darkred", "EstimatedFollowerTrend" = "lightpink", "EstimatedLeaderTrend" = "lightblue")) +
  labs(x = "Round", y = "Profit", color = "Actor") +
  theme_minimal()+  
  ggtitle("Profit evolution")  
