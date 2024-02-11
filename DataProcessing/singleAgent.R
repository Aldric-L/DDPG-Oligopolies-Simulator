data <- read.csv("DDPG-BigOutput-7525415758.csv")

# Convert columns to appropriate types
data$round <- as.integer(data$round)
data$random_action <- as.numeric(data$random_action)  
data$action <- as.numeric(data$action)            
data$bestAction <- as.numeric(data$bestAction)    
data$profit <- as.numeric(data$profit)              
data$whitenoise <- as.numeric(data$whitenoise)

data$bestAction <- as.numeric(data$bestAction) *(5/3)

data$squared_error <- (data$action - data$bestAction)^2
data$abserror <- abs(data$action - data$bestAction)
data$error <- data$action - data$bestAction

library(ggplot2)

# Create a ggplot object
ggplot(data[data$whitenoise==0,], aes(x = round, y = squared_error)) +
  geom_point() +  # Add points for each data point
  geom_smooth() +
  labs(x = "Round", y = "Squared Error") +  # Label the axes
  ggtitle("Squared Error vs Round")  # Add a title

ggplot(data[data$whitenoise==0&data$round>49000,], aes(x = random_action, y = squared_error)) +
  geom_point() +  # Add points for each data point
  geom_smooth() +
  labs(x = "Other Action", y = "Squared Error") +  # Label the axes
  ggtitle("Squared Error vs Round")  # Add a title

ggplot(data[data$whitenoise==0&data$round>49000,], aes(x = bestAction, y = error)) +
  geom_point() +  # Add points for each data point
  geom_smooth() +
  labs(x = "Expected action", y = "Absolute Error") +  # Label the axes
  ggtitle("Absolute Error vs Round")  # Add a title

max(data[data$whitenoise==0,]$bestAction)
ggplot(data[data$whitenoise==0,], aes(x = round, y = action)) +
  geom_point() +  # Add points for each data point
  geom_smooth() +
  labs(x = "Round", y = "Played") +  # Label the axes
  ggtitle("Played")  # Add a title

ggplot(data[data$whitenoise!=0,], aes(x = action)) +
  geom_density(fill = "skyblue", color = "blue", alpha = 0.5) +
  labs(x = "Action", y = "Density", title = "Density Plot of Actions")

ggplot(data[data$whitenoise==0&data$round>9000,], aes(x = action)) +
  geom_density(fill = "skyblue", color = "blue", alpha = 0.5) +
  labs(x = "Action", y = "Density", title = "Density Plot of Actions")+
  xlim(0, 1)

