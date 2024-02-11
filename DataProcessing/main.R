data <- read.csv("DDPG-Output-1061851587.csv")

# Convert columns to appropriate types
data$round <- as.integer(data$round)
data$random_action <- as.numeric(data$random_action)  # Assuming float
data$action <- as.numeric(data$action)              # Assuming float
data$bestAction <- as.numeric(data$bestAction)      # Assuming float
data$profit <- as.numeric(data$profit)              # Assuming float
data$whitenoise <- as.numeric(data$whitenoise)

data$squared_error <- (data$action - data$bestAction)^2
data$error <- (data$action - data$bestAction)^2

library(ggplot2)

# Create a ggplot object
ggplot(data[data$whitenoise==0,], aes(x = round, y = squared_error)) +
  geom_point() +  # Add points for each data point
  geom_smooth() +
  labs(x = "Round", y = "Squared Error") +  # Label the axes
  ggtitle("Squared Error vs Round")  # Add a title


max(data[data$whitenoise!=0,]$action)
ggplot(data[data$whitenoise==0,], aes(x = round, y = action)) +
  geom_smooth() +
  geom_point() +  # Add points for each data point
  labs(x = "Round", y = "Played") +  # Label the axes
  ggtitle("Played")  # Add a title

ggplot(data[data$whitenoise!=0,], aes(x = action)) +
  geom_density(fill = "skyblue", color = "blue", alpha = 0.5) +
  labs(x = "Action", y = "Density", title = "Density Plot of Actions")

ggplot(data[data$whitenoise==0,], aes(x = action)) +
  geom_density(fill = "skyblue", color = "blue", alpha = 0.5) +
  labs(x = "Action", y = "Density", title = "Density Plot of Actions")+
  xlim(0, 1)