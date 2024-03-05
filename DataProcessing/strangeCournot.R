#data <- read.csv("DDPG-StrangeCournotWithMem-Output-1562059222.csv")
#Perfect cournot convergence : 
#data <- read.csv("DDPG-StrangeCournot-Output-1540358815.csv")
data <- read.csv("DDPG-StrangeCournot-Output-2083545200.csv")

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
  geom_smooth(aes(y = leaderAction, color = "LeaderTrend"), se = FALSE) +
  geom_smooth(aes(y = followerAction, color = "FollowerTrend"), se = FALSE) +
  geom_line(aes(y = competitiveAction, color = "Competition")) +
  geom_line(aes(y = cournotAction, color = "Cournot")) +
  geom_line(aes(y = leaderBestAction, color = "Stackelberg")) +
  scale_color_manual(values = c("Leader" = "blue", "Follower" = "red", "LeaderTrend" = "darkblue", "FollowerTrend" = "darkred", "BestFollowerTrend" = "lightpink", "BestLeaderTrend" = "lightblue", "Competition" = "green", "Cournot" = "darkgreen", "Stackelberg"="purple")) +
  labs(x = "Round", y = "Quantity", color = "Legend") +
  theme_minimal()


convergence_test <- function(actionVector, threshold_percent) {
  threshold <- round(length(actionVector)*threshold_percent)
  
  test_stat <- 0
  local_mean <- mean(actionVector[c(threshold:length(actionVector))])
  
  ggplot(data = NULL, aes(x = actionVector[c(threshold:length(actionVector))])) +
    geom_density(fill = "skyblue", color = "blue", alpha = 0.5) +
    ggtitle("Density Plot of Example Data") +
    xlab("Data") +
    ylab("Density")
  
  for (i in threshold:length(actionVector))
    test_stat <- (actionVector[i] - local_mean)^2
  test_stat <- sqrt(test_stat/(length(actionVector)-threshold))
  return(list("mean" = local_mean, "estimatedSigma" = test_stat, "threshold" = threshold, "obs" = length(actionVector)-threshold))
}

ggplot(data[data$round>0,], aes(x = round)) +
  geom_smooth(aes(y = leaderEstimatedProfit, color = "Leader"), se = FALSE) +
  geom_smooth(aes(y = followerEstimatedProfit, color = "Follower"), se = FALSE) +
  geom_smooth(aes(y = leaderProfit, color = "LeaderTrend"), se = FALSE) +
  geom_smooth(aes(y = followerProfit, color = "FollowerTrend"), se = FALSE) +
  geom_line(aes(y = cournotProfit, color = "Cournot")) +
  geom_line(aes(y = competitiveProfit, color = "Competition")) +
  geom_line(aes(y = stackelbergProfit, color = "Stackelberg")) +
  scale_color_manual(values = c("Leader" = "blue", "Follower" = "red", "LeaderTrend" = "darkblue", "FollowerTrend" = "darkred", "BestFollowerTrend" = "lightpink", "BestLeaderTrend" = "lightblue", "Competition" = "green", "Cournot" = "darkgreen", "Stackelberg"="purple")) +
  labs(x = "Round", y = "Profit", color = "Actor") +
  theme_minimal()+  
  ggtitle("Profit evolution")  

convergence_test(data[data$whitenoise==0,]$leaderAction, 0.9)
convergence_test(data[data$whitenoise==0,]$followerAction, 0.9)

