# library(usethis) 
# usethis::edit_r_environ(scope="project")
# R_MAX_VSIZE=100Gb 

agents_nb <- 4
#folder_path <- "Outputs/Cournot-2-60k-LIN-Gamma0/WN0.15/"
#folder_path <- "Outputs/Cournot-2-120k-LIN-Gamma0.99/WN0.15/"
folder_path <- "Outputs/Cournot-4-100k-LIN-Gamma0/"
cournot4 <- c("compProfit" = 0, "compQuantity" = 0.423,
              "cournotProfit" = 0.403, "cournotQuantity" = 0.355,
              "cartelProfit" = 0.526, "cartelQuantity" = 0.239,
              "modelTotalProfit" = 1.612,
              "modelTotalQuantity" = 1.419,
              "reactionFunction" = function(Q) {
                return((2.2-Q)/(2*(1+0.2)))
              })

cournot2 <- c("compProfit" = 0.16469444444444442, "compQuantity" = 0.9166666666666667,
              "cournotProfit" = 0.49237370242214545, "cournotQuantity" = 0.6470588235294118,
              "cartelProfit" = 0.539, "cartelQuantity" = 0.5,
              "modelTotalProfit" = 2 * 0.49237370242214545,
              "modelTotalQuantity" = 2 * 0.6470588235294118,
              "reactionFunction" = function(Q) {
                return((2.2-Q)/(2*(1+0.2)))
              })

stackelberg2bis <- c(
              "compProfit" = 0.111, "compQuantity" = 0.870,
              "lowProfit" = 0.306, "lowQuantity" = 0.521,
              "highProfit" = 0.438, "highQuantity" = 0.801,
              "cartelProfit" = 0.456, "cartelQuantity" = 0.465,
              "cournotProfit" = 0.414, "cournotQuantity" = 0.606,
              "modelTotalProfit" = 0.306+0.438, 
              "modelTotalQuantity" = 0.521 + 0.801,
              "reactionFunction" = function(Q) {
                return((2.2-Q)/(2*(1+0.2)))
              })

stackelberg2 <- c(
  "compProfit" = 0.16469444444444442, "compQuantity" = 0.9166666666666667,
  "lowProfit" = 0.37931870274102086, "lowQuantity" = 0.5679347826086958,
  "highProfit" = 0.5148647211720228, "highQuantity" = 0.8369565217391304,
  "cartelProfit" = 0.539, "cartelQuantity" = 0.5,
  "modelTotalProfit" = 0.37931870274102086+0.5148647211720228, 
  "modelTotalQuantity" = 0.5679347826086958 + 0.8369565217391304,
  "reactionFunction" = function(Q) {
    return((2.2-Q)/(2*(1+0.2)))
  })

#cournot4 <- c("profit" = , "cournotQuantity" = , "compQuantity" = )

lightmode <- TRUE

mode <- cournot4
model <- "COURNOT"

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
  temp <- data$agent1Profit
  temp <- ifelse(temp==0, 0.01, temp)
  data$agent1CriticError <- (data$agent1EstimatedProfit - data$agent1Profit)/temp
  rm(temp)
  data$criticError <- data$agent1CriticError
  for (agent in 2:agents_nb){
    data$totalQuantity <- data$totalQuantity + data[, paste("agent", agent, "Action", sep="")]
    data$totalProfit <- data$totalProfit + data[, paste("agent", agent, "Profit", sep="")]
    temp <- data[, paste("agent", agent, "Profit", sep="")]
    temp <- ifelse(temp==0, 0.01, temp)
    data[, paste("agent", agent, "CriticError", sep="")] <- (data[, paste("agent", agent, "EstimatedProfit", sep="")] - data[, paste("agent", agent, "Profit", sep="")])/temp
    rm(temp)
    data$criticError <- data$criticError + data[, paste("agent", agent, "CriticError", sep="")] 
  }
  for (agent in 1:agents_nb){
    data[, paste("agent", agent, "ActionError", sep="")] <- mode[["reactionFunction"]](data$totalQuantity-data[, paste("agent", agent, "Action", sep="")])-data[, paste("agent", agent, "Action", sep="")]
  }
  data$criticError <- data$criticError / agents_nb
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
  #geom_point(alpha = 0.01, aes(color = "Simulations")) +
  geom_line(aes(y = agents_nb * mode[["compQuantity"]], color = "Competition")) +
  geom_line(aes(y = agents_nb * mode[["cartelQuantity"]], color = "Cartel")) +
  geom_line(aes(y = mode[["modelTotalQuantity"]], color = "Expected Quantity")) +
  geom_smooth(se = FALSE, linewidth=0.1, aes(color = "Simulations")) +
  theme_minimal() + 
  labs(x = "Round", y = "Total Quantity", color = "Legend") +
  ylim(0,2)

ggplot(simulsData[simulsData$whitenoise==0,], aes(x=round, group=simulName, color=simulName)) +
  #geom_point(alpha = 0.01, aes(color = "Simulations")) +
  geom_line(aes(y = mode[["compQuantity"]], color = "Competition")) +
  geom_line(aes(y = mode[["cartelQuantity"]], color = "Cartel")) +
  {if (model == "COURNOT")geom_line(aes(y = mode[["cournotQuantity"]], color = "Cournot")) } + 
  {if (model == "STACKELBERG")geom_line(aes(y = mode[["lowQuantity"]], color="Stackelberg Leader")) } +
  {if (model == "STACKELBERG")geom_line(aes(y = mode[["highQuantity"]], color="Stackelberg Follower"))} + 
  geom_smooth(se = FALSE, linewidth=0.1, aes(y=agent1Action, color = "Simulations")) +
  geom_smooth(se = FALSE, linewidth=0.1, aes(y=agent2Action, color = "Simulations")) +
  theme_minimal() + 
  labs(x = "Round", y = "Quantity", color = "Legend") +
  ylim(0,1)

## Total profit plot
ggplot(simulsData[simulsData$whitenoise==0,], aes(x=round, y=totalProfit, group=simulName, color=simulName)) +
  #geom_point(alpha = 0.01, aes(color = "Simulations")) +
  geom_line(aes(y = agents_nb * mode[["compProfit"]], color = "Competition")) +
  geom_line(aes(y = agents_nb * mode[["cartelProfit"]], color = "Cartel")) +
  geom_line(aes(y = mode[["modelTotalProfit"]], color = "Expected Profit")) +
  geom_smooth(se = FALSE, linewidth=0.1, aes(color = "Simulations")) +
  theme_minimal() + 
  labs(x = "Round", y = "Total Profit", color = "Legend") +
  ylim(0,2)


## Critic error
ggplot(simulsData[simulsData$whitenoise==0&simulsData$round>40000,], aes(x=round, y=criticError, group=simulName, color=simulName)) +
  geom_point(alpha = 0.5, size=0.1, aes(color = "Simulations Values")) +
  geom_smooth(se = FALSE, linewidth=0.1, aes(color = "Simulations")) +
  scale_color_manual(values = c("Simulations (Values)" = "lightblue", 
                                "Simulations (Trend)" = "darkblue")) +
  theme_minimal() + 
  labs(x = "Round", y = "Critic estimation error", color = "Legend")

## Action Error
ggplot(simulsData[simulsData$whitenoise==0,], aes(x=round, group=simulName, color=simulName)) +
  geom_smooth(se = FALSE, linewidth=0.1, aes(y=agent1ActionError, color = "Simulations")) +
  {if (agents_nb>=2)geom_smooth(se = FALSE, linewidth=0.1, aes(y=agent2ActionError, color = "Simulations")) } + 
  {if (agents_nb>=3)geom_smooth(se = FALSE, linewidth=0.1, aes(y=agent3ActionError, color = "Simulations")) } + 
  {if (agents_nb>=4)geom_smooth(se = FALSE, linewidth=0.1, aes(y=agent4ActionError, color = "Simulations")) } + 
  theme_minimal() + 
  labs(x = "Round", y = "Deviation from (static) best response function", color = "Legend")

## One simul plot
s <- get(simuls[[4]])
ggplot(s[s$whitenoise==0,], aes(x = round)) +
  geom_point(aes(y = agent2Action, color = "agent2"), alpha = 0.1) +
  geom_point(aes(y = agent1Action, color = "agent1"), alpha = 0.1) +
  geom_smooth(aes(y = agent1Action, color = "agent1Trend"), se = FALSE) +
  geom_smooth(aes(y = agent2Action, color = "agent2Trend"), se = FALSE) +
  {if (agents_nb>=3)geom_point(aes(y = agent3Action, color = "agent3"), alpha = 0.1) } + 
  {if (agents_nb>=3)geom_smooth(aes(y = agent3Action, color = "agent3Trend"), se = FALSE) } + 
  {if (agents_nb>=4)geom_point(aes(y = agent4Action, color = "agent4"), alpha = 0.1) } + 
  {if (agents_nb>=4)geom_smooth(aes(y = agent4Action, color = "agent4Trend"), se = FALSE) } +
  {if (model == "COURNOT")geom_line(aes(y = mode[["cournotQuantity"]], color = "Cournot")) } + 
  {if (model == "COURNOT")geom_smooth(aes(y = (totalQuantity)/agents_nb, color = "meanTrend"), se = FALSE) } + 
  {if (model == "STACKELBERG")geom_line(aes(y = mode[["lowQuantity"]])) } +
  {if (model == "STACKELBERG")geom_line(aes(y = mode[["highQuantity"]]))} + 
  {if (model == "STACKELBERG")geom_smooth(aes(y = (2.2-agent1Action)/(2*(1+0.2)), color = "agent3Trend"), se = FALSE)} + 
  geom_line(aes(y = mode[["compQuantity"]], color = "Competition")) +
  geom_line(aes(y = mode[["cournotQuantity"]], color = "Cournot")) +
  geom_line(aes(y = mode[["cartelQuantity"]], color = "Cartel")) +
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
  theme_minimal() +
  labs(x = "Round", y = "Quantity", color = "Legend") + 
  ylim(0,1)
  

ggplot(s[s$whitenoise==0,], aes(x = round)) +
  geom_point(aes(y = agent2Profit, color = "agent2"), alpha = 0.1) +
  geom_point(aes(y = agent1Profit, color = "agent1"), alpha = 0.1) +
  geom_smooth(aes(y = agent1Profit, color = "agent1Trend"), se = FALSE) +
  geom_smooth(aes(y = agent2Profit, color = "agent2Trend"), se = FALSE) +
  {if (model == "COURNOT")geom_line(aes(y = mode[["cournotProfit"]], color = "Cournot")) } + 
  {if (model == "COURNOT")geom_smooth(aes(y = (agent1Action+agent2Action)/2, color = "meanTrend"), se = FALSE) } + 
  {if (model == "STACKELBERG")geom_line(aes(y = mode[["lowProfit"]])) } +
  {if (model == "STACKELBERG")geom_line(aes(y = mode[["highProfit"]]))} + 
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
  labs(x = "Round", y = "Profit", color = "Legend") +
  theme_minimal()

ggplot(s[s$whitenoise==0,], aes(x = round)) +
  geom_point(aes(y = agent2CriticError, color = "agent2"), alpha = 0.1) +
  geom_point(aes(y = agent1CriticError, color = "agent1"), alpha = 0.1) +
  geom_smooth(aes(y = agent1CriticError, color = "agent1Trend"), se = FALSE) +
  geom_smooth(aes(y = agent2CriticError, color = "agent2Trend"), se = FALSE) +
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
  labs(x = "Round", y = "Profit estimation error", color = "Legend") +
  theme_minimal()

# One whitenoise
ggplot(data = s[s$whitenoise > 0,], aes(x=round, y=whitenoise)) + geom_point()


###############################################################################
######## FREQUENCY ANALYSIS
computeMeanMaxDistance <- function (agents, data) {
  meansDiff <- c()
  sdDiff <- c()
  for (i in 1:(agents-1)){
    for (j in (i+1):agents){
      meansDiff <- append(meansDiff, mean(abs(data[, paste("agent", i, "Action", sep="")]-data[, paste("agent", j, "Action", sep="")])))
      sdDiff <- append(sdDiff, sd(abs(data[, paste("agent", i, "Action", sep="")]-data[, paste("agent", j, "Action", sep="")])))
    }
  }
  return(list(meansDiff = max(meansDiff), sdDiff = max(sdDiff)))
}

convergenceTotalActions <- c()
followerActions <- c()
leaderActions <- c()
meanMaxDists <- c()
sdMaxDists <- c()
actionsDivergence <- c()

for (i in 1:length(simuls)) {
  s <- get(simuls[[i]])
  s <- s[s$whitenoise==0,]
  meanMaxDists <- append(meanMaxDists, computeMeanMaxDistance(agents_nb, s[s$round>58000&s$whitenoise==0,])$meansDiff)
  sdMaxDists <- append(sdMaxDists, computeMeanMaxDistance(agents_nb, s[s$round>58000&s$whitenoise==0,])$sdDiff)
  convergenceTotalActions <- append(convergenceTotalActions, convergence_test(s$totalQuantity, 0.98)$mean)
  a1 <- convergence_test(s$agent1Action, 0.98)$mean
  a2 <- convergence_test(s$agent2Action, 0.98)$mean
  followerActions <- append(followerActions, min(a1, a2))
  leaderActions <- append(leaderActions, max(a1, a2))
  for (agent in 2:agents_nb){
    actionsDivergence <- append(actionsDivergence, convergence_test(s[, paste("agent", agent, "ActionError", sep="")], 0.98)$mean)
  }
  rm(s)
}
tmp <- data.frame(totalActions = convergenceTotalActions, followerActions, leaderActions)

## Total density and cdf
library(gridExtra)
densityplot <- ggplot(tmp, aes(x = totalActions)) +
  geom_density(alpha = 0.5, fill = "orange") +
  geom_vline(xintercept=mode[["modelTotalQuantity"]]) + 
  geom_vline(xintercept=median(tmp$totalActions), color="darkred") + 
  labs(x = "Total quantity",
       y = "Density", color = "Legend")

cdfplot <- ggplot(tmp, aes(x = totalActions)) +
  stat_ecdf(geom = "step", color = "blue") +  # cumulative distribution line
  geom_vline(xintercept=mode[["modelTotalQuantity"]], color="red") + 
  geom_vline(xintercept=mean(convergenceTotalActions), color="darkblue") + 
  geom_vline(xintercept=median(convergenceTotalActions), color="lightblue") + 
  labs(x = "Total quantity",
       y = "Cumulative distribution", color = "Legend")

grid.arrange(densityplot, cdfplot, nrow = 2)


ggplot(tmp) +
  geom_density(aes(x = followerActions), alpha = 0.5, fill = "orange") +
  geom_density(aes(x = leaderActions), alpha = 0.5, fill = "orange") +
  geom_vline(xintercept=median(followerActions), color="red") + 
  geom_vline(xintercept=median(leaderActions), color="red") + 
  geom_vline(xintercept=median((leaderActions+followerActions)/2), color="darkred") + 
  {if (model == "COURNOT")geom_vline(xintercept=mode[["cournotQuantity"]]) } + 
  {if (model == "STACKELBERG")geom_vline(xintercept=mode[["lowQuantity"]]) } +
  {if (model == "STACKELBERG")geom_vline(xintercept=mode[["highQuantity"]])} + 
  labs(x = "Selected quantity",
       y = "Density", color = "Legend")

ggplot(tmp) +
  geom_density(aes(x = convergenceTotalActions), alpha = 0.5, fill = "orange") +
  stat_function(fun = dnorm, n = 101, args = list(mean = mean(tmp$totalActions), sd = sd(tmp$totalActions))) + 
  geom_vline(xintercept=median(convergenceTotalActions), color="red") + 
  geom_vline(xintercept=mode[["modelTotalQuantity"]])  + 
  geom_vline(xintercept=mode[["modelTotalQuantity"]]*0.95, color="blue", linetype="dotted") + 
  geom_vline(xintercept=mode[["modelTotalQuantity"]]*1.05, color="blue", linetype="dotted") + 
  geom_vline(xintercept=mode[["modelTotalQuantity"]]*0.85, color="darkgreen", linetype="dotted") + 
  geom_vline(xintercept=mode[["modelTotalQuantity"]]*1.15, color="darkgreen", linetype="dotted") + 
  labs(x = "Selected quantity",
       y = "Density", color = "Legend")

ggplot() +
  geom_density(aes(x = actionsDivergence), alpha = 0.5, fill = "orange") +
  stat_function(fun = dnorm, n = 101, args = list(mean = mean(actionsDivergence), sd = sd(actionsDivergence))) + 
  labs(x = "Average asymptotic diff. with static best response",
       y = "Density", color = "Legend")

summary(actionsDivergence)
shapiro.test(actionsDivergence)

# Share of correct values in the CI interval of modelQuantity +/- 5%
CI5share <- nrow(tmp[tmp$totalActions>=mode[["modelTotalQuantity"]]*0.95&tmp$totalActions<=mode[["modelTotalQuantity"]]*1.05,])/nrow(tmp)
print(CI5share)
CI10share <- nrow(tmp[tmp$totalActions>=mode[["modelTotalQuantity"]]*0.90&tmp$totalActions<=mode[["modelTotalQuantity"]]*1.10,])/nrow(tmp)
print(CI10share)
CI15share <- nrow(tmp[tmp$totalActions>=mode[["modelTotalQuantity"]]*0.85&tmp$totalActions<=mode[["modelTotalQuantity"]]*1.15,])/nrow(tmp)
print(CI15share)

# Distance btw agents
summary(meanMaxDists)

shapiro.test(tmp$totalActions)


rm(tmp)
## ggplot() + geom_density(aes(x = notes), alpha = 0.5, fill = "orange") + xlim(0, 20) +   stat_function(fun = dnorm, n = 101, args = list(mean = 10, sd = 3)) 

