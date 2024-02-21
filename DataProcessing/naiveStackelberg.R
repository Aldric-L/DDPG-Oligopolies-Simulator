#data <- read.csv("DDPG-NaiveStackelberg-Output-1977062708.csv")
#data <- read.csv("DDPG-NaiveStackelberg-Output-1552360841.csv")
data <- read.csv("DDPG-NaiveCournotWithMem-Output-2631691123.csv")

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

# ggplot(data, aes(x = round)) +
#   geom_point(aes(y = leaderSquaredError, color = "Leader" )) +  
#   geom_point(aes(y = followerSquaredError, color = "Follower" )) +  
#   geom_smooth(aes(y = leaderSquaredError, color = "LeaderTrend" )) +  
#   geom_smooth(aes(y = followerSquaredError, color = "FollowerTrend" )) +  
#   scale_color_manual(values = c("Leader" = "blue", "Follower" = "red", "LeaderTrend" = "darkblue", "FollowerTrend" = "darkred")) +
#   labs(x = "Round", y = "Squared Error") +  
#   ggtitle("Squared Error vs Round with whitenoise")  

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


convergence_test(data[data$whitenoise==0,]$leaderAction, 0.9)
convergence_test(data[data$whitenoise==0,]$followerAction, 0.9)


dataCritic <- read.csv("DDPG-NaiveStackelberg-Critic-1552360841.csv")
dataCritic$followerError <- abs(dataCritic$followerProfit - dataCritic$followerEstimatedProfit)
dataCritic$leaderError <- abs(dataCritic$leaderProfit - dataCritic$leaderEstimatedProfit)

library(gridExtra)
library(hrbrthemes)
library(viridis)

followerEstimated <- ggplot(dataCritic, aes(x = leaderAction, y = followerAction, fill = followerEstimatedProfit)) +
  geom_tile() +
  scale_fill_gradient(low = "white", high = "red") +  # Adjust color gradient as needed
  labs(x = "leaderAction", y = "followerAction", title = "Estimated profit") +
  theme_minimal()

followerTrueProfit <- ggplot(dataCritic, aes(x = leaderAction, y = followerAction, fill = followerProfit)) +
  geom_tile() +
  scale_fill_gradient(low = "white", high = "red") +  # Adjust color gradient as needed
  labs(x = "leaderAction", y = "followerAction", title = "True profit") +
  theme_minimal()

grid.arrange(followerEstimated, followerTrueProfit, nrow = 2)

followerError <- ggplot(dataCritic, aes(x = leaderAction, y = followerAction, fill = followerError)) +
  geom_tile() +
  scale_fill_viridis(option="magma") +  
  labs(x = "leaderAction", y = "followerAction", title = "Follower Error profit") +
  theme_minimal()

leaderError <- ggplot(dataCritic, aes(x = leaderAction, y = followerAction, fill = leaderError)) +
  geom_tile() +
  scale_fill_viridis(option="magma") +  
  labs(x = "leaderAction", y = "followerAction", title = "Leader Error profit") +
  theme_minimal()
grid.arrange(followerError, leaderError, nrow = 2)

ggplot(dataCritic, aes(x = leaderAction, y = followerAction, fill = leaderProfit)) +
  geom_tile() +
  scale_fill_viridis(option="magma") +  
  labs(x = "leaderAction", y = "followerAction", title = "Leader true profit") +
  theme_minimal()

LeaderCriticMSE <- mean((dataCritic$leaderProfit - dataCritic$leaderEstimatedProfit)^2)
FollowerCriticMSE <- mean((dataCritic$followerProfit - dataCritic$followerEstimatedProfit)^2)

print("Leader Critic MSE: ", LeaderCriticMSE)
print("Follower Critic MSE: ", FollowerCriticMSE)

ggplot(dataCritic, aes(x = leaderAction, y = followerAction, fill = price)) +
  geom_tile() +
  scale_fill_viridis(option="magma") +  
  labs(x = "leaderAction", y = "followerAction", title = "Price") +
  theme_minimal()

