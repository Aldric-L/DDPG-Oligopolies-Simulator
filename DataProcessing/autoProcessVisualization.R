# library(usethis) 
# usethis::edit_r_environ(scope="project")
# R_MAX_VSIZE=100Gb 

agents_nb <- 2
folder_path <- "Outputs/Cournot-2-TRUNCRESEXPDECAYED_WHITENOISE-120000-Gamma0-HDecay/norenorm/"

cournot2 <- c("compProfit" = 0.16469444444444442, "compQuantity" = 0.9166666666666667,
              "cournotProfit" = 0.49237370242214545, "cournotQuantity" = 0.6470588235294118,
              "cartelProfit" = 0.539, "cartelQuantity" = 0.5)
#cournot4 <- c("profit" = , "cournotQuantity" = , "compQuantity" = )

lightmode <- TRUE

mode <- cournot2

## Import files
library(data.table)
csv_files <- list.files(path = folder_path, pattern = "\\.csv$", full.names = TRUE)
simuls <- list()
for (i in 1:length(csv_files)) {
  file_name <- tools::file_path_sans_ext(basename(csv_files[i]))
  last_number <- as.numeric(sub('.*-(\\d+)$', '\\1', file_name))
  
  data <- read.csv(csv_files[i])
  if (lightmode)
    data <- data[data$whitenoise==0,]
  data <- subset(data, select=-c(iteration))
  data$totalQuantity <- data$agent1Action
  data$totalProfit <- data$agent1Profit
  for (agent in 2:agents_nb){
    data$totalQuantity <- data$totalQuantity + data[, paste("agent", agent, "Action", sep="")]
    data$totalProfit <- data$totalProfit + data[, paste("agent", agent, "Profit", sep="")]
  }
  assign(paste("sim_", toString(last_number), sep=""), data)
  simuls <- append(simuls, paste("sim_", toString(last_number), sep=""))
  
  # if (i > 1 && nrow(get(simuls[[i]])) != nrow(get(simuls[[i-1]]))){
  #   library(R.methodsS3)
  #   throw("ALERT: All simulations are not equally sized")
  # }
}

rounds <- data$round
rm(data)

s <- get(simuls[[1]])
s$simulName <- simuls[[1]]
simulsData <- s
for (i in 2:length(simuls)) {
  s <- get(simuls[[i]])
  s$simulName <- simuls[[i]]
  
  simulsData <- rbind(simulsData, s)
}
rm(s)

## Convergence test function
convergence_test <- function(actionVector, threshold_percent) {
  threshold <- round(length(actionVector)*threshold_percent)
  
  test_stat <- 0
  local_mean <- mean(actionVector[c(threshold:length(actionVector))])
  
  for (i in threshold:length(actionVector))
    test_stat <- (actionVector[i] - local_mean)^2
  test_stat <- sqrt(test_stat/(length(actionVector)-threshold))
  return(list("mean" = local_mean, "estimatedSigma" = test_stat, "threshold" = threshold, "obs" = length(actionVector)-threshold))
}


library(ggplot2)
## Total quantity plot
ggplot(simulsData[simulsData$whitenoise==0,], aes(x=round, y=totalQuantity, group=simulName, color=simulName)) +
  geom_point(alpha = 0.01) +
  geom_smooth(se = FALSE) +
  geom_line(aes(y = agents_nb * mode[["compQuantity"]], color = "Competition")) +
  geom_line(aes(y = agents_nb * mode[["cournotQuantity"]], color = "Cournot")) +
  geom_line(aes(y = agents_nb * mode[["cartelQuantity"]], color = "Cartel")) +
  labs(x = "Round", y = "Total Quantity", color = "Legend") +
  theme_minimal()

## Total profit plot
ggplot(simulsData[simulsData$whitenoise==0,], aes(x=round, y=totalProfit, group=simulName, color=simulName)) +
  geom_point(alpha = 0.01) +
  geom_smooth(se = FALSE) +
  geom_line(aes(y = agents_nb * mode[["compProfit"]], color = "Competition")) +
  geom_line(aes(y = agents_nb * mode[["cournotProfit"]], color = "Cournot")) +
  geom_line(aes(y = agents_nb * mode[["cartelProfit"]], color = "Cartel")) +
  labs(x = "Round", y = "Total profit", color = "Legend") +
  theme_minimal()


## One simul plot
s <- get(simuls[[5]])
ggplot(s[s$whitenoise==0,], aes(x = round)) +
  geom_point(aes(y = agent2Action, color = "agent2"), alpha = 0.1) +
  geom_point(aes(y = agent1Action, color = "agent1"), alpha = 0.1) +
  geom_smooth(aes(y = agent1Action, color = "agent1Trend"), se = FALSE) +
  geom_smooth(aes(y = agent2Action, color = "agent2Trend"), se = FALSE) +
  geom_smooth(aes(y = (agent1Action+agent2Action)/2, color = "meanTrend"), se = FALSE) +
  geom_line(aes(y = 0.9166666666666667, color = "Competition")) +
  geom_line(aes(y = 0.6470588235294118, color = "Cournot")) +
  geom_line(aes(y = 0.5, color = "Cartel")) +
  # geom_line(aes(y = leaderBestAction, color = "Stackelberg")) +
  scale_color_manual(values = c("agent1" = "blue", 
                                "agent2" = "yellow", 
                                "agent3" = "grey", 
                                "agent4" = "red", 
                                "meanTrend" = "pink", 
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

# Density plots
convergenceTotalActions <- c()
followerActions <- c()
leaderActions <- c()
for (i in 1:length(simuls)) {
  s <- get(simuls[[i]])
  convergenceTotalActions <- append(convergenceTotalActions, convergence_test(s$totalQuantity, 0.95)$mean)
  a1 <- convergence_test(s$agent1Action, 0.95)$mean
  a2 <- convergence_test(s$agent2Action, 0.95)$mean
  followerActions <- append(followerActions, min(a1, a2))
  leaderActions <- append(leaderActions, max(a1, a2))
  rm(s)
}
tmp <- data.frame(totalActions = convergenceTotalActions, followerActions, leaderActions)

## Total density and cdf
library(gridExtra)
densotyplot <- ggplot(tmp, aes(x = totalActions)) +
  geom_density(alpha = 0.5, fill = "orange") +
  geom_vline(xintercept=agents_nb * mode[["cournotQuantity"]]) + 
  geom_vline(xintercept=median(tmp$totalActions), color="darkred") + 
  labs(x = "Total quantity",
       y = "Density", color = "Legend")

cdfplot <- ggplot(tmp, aes(x = totalActions)) +
  stat_ecdf(geom = "step", color = "blue") +  # cumulative distribution line
  geom_vline(xintercept=agents_nb * mode[["cournotQuantity"]], color="red") + 
  geom_vline(xintercept=mean(convergenceTotalActions), color="darkblue") + 
  geom_vline(xintercept=median(convergenceTotalActions), color="lightblue") + 
  labs(x = "Total quantity",
       y = "Cumulative distribution", color = "Legend")

grid.arrange(densotyplot, cdfplot, nrow = 2)


ggplot(tmp) +
  geom_density(aes(x = followerActions), alpha = 0.5, fill = "orange") +
  geom_density(aes(x = leaderActions), alpha = 0.5, fill = "orange") +
  geom_vline(xintercept=median(followerActions), color="red") + 
  geom_vline(xintercept=median(leaderActions), color="red") + 
  geom_vline(xintercept=median((leaderActions+followerActions)/2), color="darkred") + 
  geom_vline(xintercept=mode[["cournotQuantity"]]) + 
  labs(x = "Selected quantity",
       y = "Density", color = "Legend")

rm(tmp)


