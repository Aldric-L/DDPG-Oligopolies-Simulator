
#import CSV Function
source("autoImportCSV.R")

#values computed for each oligopoly
source("utilsOligopolies.R")
agents_nb <- 4
folder_path <- "Outputs/Experimental/E57 (Cournot4)/"
#folder_path <- "Outputs/Cournot-2-60k-TRUNC_EXP-Gamma0/"
#folder_path <- "Outputs/Cournot-2-120k-LIN-Gamma0.99/WN0.15/"
#folder_path <- "Outputs/Cournot-4-100k-LIN-Gamma0/"

mode <- cournot4bis
model <- "COURNOT"

autoImport(folder_path = folder_path, agents_nb = agents_nb, lightmode = TRUE, mode=mode)

library(ggplot2)
## Total quantity plot
ggplot(simulsData[simulsData$whitenoise==0,], aes(x=round, y=totalQuantity, group=simulName, color=simulName)) +
  #geom_point(alpha = 0.01, aes(color = "Simulations")) +
  geom_line(aes(y = agents_nb * mode[["compQuantity"]], color = "Competition")) +
  geom_line(aes(y = agents_nb * mode[["cartelQuantity"]], color = "Cartel")) +
  geom_line(aes(y = mode[["modelTotalQuantity"]], color = "Expected Quantity")) +
  {if (model == "STACKELBERG")geom_line(aes(y = mode[["cournotTotalQuantity"]], color = "Cournot Quantity"))} +
  geom_smooth(se = FALSE, linewidth=0.1, aes(color = "Simulations")) +
  theme_minimal() + 
  labs(x = "Round", y = "Total Quantity", color = "Legend") +
  ylim(0,agents_nb)

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

## Individual profit plot
ggplot(simulsData[simulsData$whitenoise==0,], aes(x=round, group=simulName)) +
  geom_smooth(aes(y = agent1Profit, color="Simulations"), se = FALSE, linewidth=0.1) +
  geom_smooth(aes(y = agent2Profit, color="Simulations"), se = FALSE, linewidth=0.1) +
  {if (model == "COURNOT")geom_line(aes(y = mode[["cournotProfit"]], color = "Cournot")) } + 
  {if (model == "STACKELBERG")geom_line(aes(y = mode[["lowProfit"]])) } +
  {if (model == "STACKELBERG")geom_line(aes(y = mode[["highProfit"]]))} + 
  geom_line(aes(y = mode[["compProfit"]], color = "Competition")) +
  geom_line(aes(y = mode[["cartelProfit"]], color = "Cartel")) +
  geom_line(aes(y = mode[["compProfit"]], color = "Competition")) +
  geom_line(aes(y = mode[["cartelProfit"]], color = "Cartel")) +
  geom_line(aes(y = mode[["modelTotalProfit"]], color = "Expected Profit")) +
  theme_minimal() + 
  labs(x = "Round", y = "Total Profit", color = "Legend") +
  ylim(0,1)

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
s <- get(simuls[[2]])
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
  {if (model == "STACKELBERG")geom_smooth(aes(y = mode[["reactionFunction"]](agent1Action), color = "agent3Trend"), se = FALSE)} + 
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
  {if (model == "COURNOT")geom_smooth(aes(y = (agent1Profit+agent2Profit)/2, color = "meanTrend"), se = FALSE) } + 
  {if (model == "STACKELBERG")geom_line(aes(y = mode[["lowProfit"]])) } +
  {if (model == "STACKELBERG")geom_line(aes(y = mode[["highProfit"]]))} + 
  geom_line(aes(y = mode[["compProfit"]], color = "Competition")) +
  geom_line(aes(y = mode[["cartelProfit"]], color = "Cartel")) +
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
  geom_density(aes(x = followerActions), alpha = 0.5, fill = "darkblue") +
  geom_density(aes(x = leaderActions), alpha = 0.5, fill = "darkred") +
  geom_vline(xintercept=median(followerActions), color="red") + 
  geom_vline(xintercept=median(leaderActions), color="red") + 
  {if (model == "COURNOT")geom_vline(xintercept=median((leaderActions+followerActions)/2), color="darkred")} + 
  {if (model == "COURNOT")geom_vline(xintercept=mode[["cournotQuantity"]]) } + 
  {if (model == "STACKELBERG")geom_vline(xintercept=mode[["lowQuantity"]]) } +
  {if (model == "STACKELBERG")geom_vline(xintercept=mode[["highQuantity"]])} + 
  theme_minimal() + 
  xlim(0, 1) + 
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
  #geom_vline(xintercept=mode[["cournotTotalQuantity"]], color="yellow") + 
  theme_minimal() + 
  xlim(0.8, 1.8) + 
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
mean(tmp$totalActions)
sd(tmp$totalActions)

## Single agent tests
# Follower
CI5shareF <- nrow(tmp[tmp$followerActions>=mode[["lowQuantity"]]*0.95&tmp$followerActions<=mode[["lowQuantity"]]*1.05,])/nrow(tmp)
print(CI5shareF)
CI10shareF <- nrow(tmp[tmp$followerActions>=mode[["lowQuantity"]]*0.90&tmp$followerActions<=mode[["lowQuantity"]]*1.10,])/nrow(tmp)
print(CI10shareF)
CI15shareF <- nrow(tmp[tmp$followerActions>=mode[["lowQuantity"]]*0.85&tmp$followerActions<=mode[["lowQuantity"]]*1.15,])/nrow(tmp)
print(CI15shareF)
shapiro.test(tmp$followerActions)
mean(tmp$followerActions)
sd(tmp$followerActions)

# Leader
CI5shareL <- nrow(tmp[tmp$leaderActions>=mode[["highQuantity"]]*0.95&tmp$leaderActions<=mode[["highQuantity"]]*1.05,])/nrow(tmp)
print(CI5shareL)
CI10shareL <- nrow(tmp[tmp$leaderActions>=mode[["highQuantity"]]*0.90&tmp$leaderActions<=mode[["highQuantity"]]*1.10,])/nrow(tmp)
print(CI10shareL)
CI15shareL <- nrow(tmp[tmp$leaderActions>=mode[["highQuantity"]]*0.85&tmp$leaderActions<=mode[["highQuantity"]]*1.15,])/nrow(tmp)
print(CI15shareL)
shapiro.test(tmp$leaderActions)
mean(tmp$leaderActions)
sd(tmp$leaderActions)

rm(tmp)

###############################################################################
######## CRITICS ANALYSIS

localcritic <- get(critics[[1]])
computeAverageCritic(agents_nb, localcritic)

library(gridExtra)
library(hrbrthemes)
library(viridis)

criticsHeatMaps <- list()


for (agent in 1:agents_nb){
  criticsHeatMaps[[length(criticsHeatMaps) + 1]] <- 
    ggplot(localcritic, aes(x = prevState, y = action, fill = localcritic[, paste("agent", agent, "CriticError", sep="")])) +
    geom_tile() +
    geom_vline(xintercept=mode[["cournotQuantity"]]) + 
    geom_hline(yintercept=mode[["cournotQuantity"]]) + 
    geom_vline(xintercept=mode[["cartelQuantity"]], color="red") + 
    geom_hline(yintercept=mode[["cartelQuantity"]], color="red") + 
    #scale_fill_gradient(low = "white", high = "red") +  # Adjust color gradient as needed
    scale_fill_viridis(option="inferno") +  
    labs(x = "Previous State", y = "Action", title = "Estimated profit error", fill = "Legend") +
    theme_minimal()
}

grid.arrange(grobs = criticsHeatMaps, ncol = 2)


globalErrorhm <- ggplot(averageCritic, aes(x = prevState, y = action, fill = averageCritic[, paste("agentsCriticError", sep="")])) +
  geom_tile() +
  geom_vline(xintercept=mode[["cournotQuantity"]]) + 
  geom_hline(yintercept=mode[["cournotQuantity"]]) + 
  geom_vline(xintercept=mode[["cartelQuantity"]], color="red") + 
  geom_hline(yintercept=mode[["cartelQuantity"]], color="red") + 
  #scale_fill_gradient(low = "white", high = "red") +  # Adjust color gradient as needed
  scale_fill_viridis(option="inferno") +  
  labs(x = "Previous State", y = "Action", fill = "Error") +
  theme_minimal()

hmTrueProfit <- ggplot(averageCritic, aes(x = prevState, y = action, fill = agentProfit)) +
  geom_tile() +
  scale_fill_viridis(option="inferno") +  
  geom_vline(xintercept=mode[["cournotQuantity"]]) + 
  geom_hline(yintercept=mode[["cournotQuantity"]]) + 
  geom_vline(xintercept=mode[["cartelQuantity"]], color="red") + 
  geom_hline(yintercept=mode[["cartelQuantity"]], color="red") + 
  labs(x = "Previous State", y = "Action", fill = "Profit", limits = c(0, 1)) +
  theme_minimal()


grid.arrange(globalErrorhm, hmTrueProfit, ncol = 2)

hmEstimated <- ggplot(averageCritic, aes(x = prevState, y = action, fill = agent1EstimatedProfit)) +
  geom_tile() +
  scale_fill_viridis(option="inferno", limits = c(0, 1)) +  
  geom_vline(xintercept=mode[["cournotQuantity"]]) + 
  geom_hline(yintercept=mode[["cournotQuantity"]]) + 
  labs(x = "Previous State", y = "Action", title = "Estimated profit", fill = "") +
  theme_minimal()

grid.arrange(hmEstimated, hmTrueProfit, ncol = 2)

hmEstimatedCorr <- ggplot(localcritic, aes(x = prevState, y = action, fill = (agentEstimatedProfit-mean(agent1EstimatedProfit))/sd(agent1EstimatedProfit))) +
  geom_tile() +
  scale_fill_viridis(option="inferno", limits = c(-2.5, 2.5)) +  
  geom_vline(xintercept=mode[["cournotQuantity"]]) + 
  geom_hline(yintercept=mode[["cournotQuantity"]]) + 
  labs(x = "Previous State", y = "Action", title = "Estimated profit", fill="") +
  theme_minimal()

hmTrueProfitCorr <- ggplot(localcritic, aes(x = prevState, y = action, fill = (agentProfit-mean(agentProfit))/sd(agentProfit))) +
  geom_tile() +
  scale_fill_viridis(option="inferno",limits = c(-2.5, 2.5)) +  
  geom_vline(xintercept=mode[["cournotQuantity"]]) + 
  geom_hline(yintercept=mode[["cournotQuantity"]]) + 
  labs(x = "Previous State", y = "Action", title = "True profit", fill="") +
  theme_minimal()

grid.arrange(hmEstimatedCorr, hmTrueProfitCorr, ncol = 2)

ggplot(criticsData, aes(x=agentProfit, group=simulName, color=simulName)) +
  geom_smooth(se = FALSE, linewidth=0.1, aes(y=agent1CriticErrorNormed, color = "Simulations")) +
  geom_smooth(se = FALSE, linewidth=0.1, aes(y=agent2CriticErrorNormed, color = "Simulations")) +
  {if (agents_nb>=3)geom_smooth(se = FALSE, linewidth=0.1, aes(y=agent3CriticError, color = "Simulations")) } + 
  {if (agents_nb>=4)geom_smooth(se = FALSE, linewidth=0.1, aes(y=agent4CriticError, color = "Simulations")) } +
  geom_smooth(se = FALSE, linewidth=1, aes(group=1, y=(agent1CriticError+agent2CriticError)/2, color = "Simulations (Trend)")) +
  geom_vline(aes(xintercept=mode[["cournotProfit"]], color="Cournot profit")) + 
  scale_color_manual(values = c("Simulations (Trend)" = "lightblue", 
                                "Simulations" = "darkblue",
                                "Cournot profit" = "blue")) +
  theme_minimal() + 
  labs(x = "Agent Profit", y = "Critic estimation error", color = "Legend")


ggplot(criticsData, aes(x=agentProfit, group=simulName, color=simulName)) +
  geom_smooth(se = FALSE, linewidth=0.1, aes(y=agent1CriticError, color = "Simulations")) +
  geom_smooth(se = FALSE, linewidth=0.1, aes(y=agent2CriticError, color = "Simulations")) +
  {if (agents_nb>=3)geom_smooth(se = FALSE, linewidth=0.1, aes(y=agent3CriticError, color = "Simulations")) } + 
  {if (agents_nb>=4)geom_smooth(se = FALSE, linewidth=0.1, aes(y=agent4CriticError, color = "Simulations")) } +
  geom_smooth(se = FALSE, linewidth=1, aes(group=1, y=(agent1CriticError+agent2CriticError)/2, color = "Simulations (Trend)")) +
  geom_vline(aes(xintercept=mode[["cournotProfit"]], color="Cournot profit")) + 
  scale_color_manual(values = c("Simulations (Trend)" = "lightblue", 
                                "Simulations" = "darkblue",
                                "Cournot profit" = "blue")) +
  theme_minimal() + 
  labs(x = "Agent Profit", y = "Critic estimation error", color = "Legend")

ggplot(criticsData[criticsData$agent1Actor!=-1,], aes(x=prevState, group=simulName, color=simulName)) +
  geom_smooth(se = FALSE, linewidth=0.1, aes(y=agent1ActorError, color = "Simulations")) +
  geom_smooth(se = FALSE, linewidth=0.1, aes(y=agent2ActorError, color = "Simulations")) +
  {if (agents_nb>=3)geom_smooth(se = FALSE, linewidth=0.1, aes(y=agent3ActorError, color = "Simulations")) } + 
  {if (agents_nb>=4)geom_smooth(se = FALSE, linewidth=0.1, aes(y=agent4ActorError, color = "Simulations")) } +
  geom_smooth(se = FALSE, linewidth=1, aes(group=1, y=(agent1ActorError+agent2ActorError)/2, color = "Simulations (Trend)")) +
  geom_vline(aes(xintercept=mode[["cournotQuantity"]], color="Cournot quantity")) + 
  scale_color_manual(values = c("Simulations (Trend)" = "lightblue", 
                                "Simulations" = "darkblue",
                                "Cournot quantity" = "blue")) +
  theme_minimal() + 
  labs(x = "Previous State", y = "Actor (v. best response) error", color = "Legend")

ggplot(criticsData[criticsData$agent1Actor!=-1,], aes(x=prevState, group=simulName, color=simulName)) +
  geom_smooth(se = FALSE, linewidth=0.1, aes(y=agent1Actor, color = "Simulations")) +
  geom_smooth(se = FALSE, linewidth=0.1, aes(y=agent2Actor, color = "Simulations")) +
  {if (agents_nb>=3)geom_smooth(se = FALSE, linewidth=0.1, aes(y=agent3Actor, color = "Simulations")) } + 
  {if (agents_nb>=4)geom_smooth(se = FALSE, linewidth=0.1, aes(y=agent4Actor, color = "Simulations")) } +
  geom_smooth(se = FALSE, linewidth=1, aes(group=1, y=(agent1Actor+agent2Actor)/2, color = "Simulations (Trend)")) +
  geom_line(aes(y=mode[["reactionFunction"]](criticsData[criticsData$agent1Actor!=-1,]$prevState), color="Best response")) +
  geom_vline(aes(xintercept=mode[["cournotQuantity"]], color="Cournot quantity")) + 
  scale_color_manual(values = c("Simulations (Trend)" = "lightblue", 
                                "Simulations" = "darkblue",
                                "Cournot quantity" = "blue",
                                "Best response" = "darkgrey")) + 
  theme_minimal() + 
  labs(x = "Previous State", y = "Actor decision", color = "Legend") + 
  ylim(0,1)

## ggplot() + geom_density(aes(x = notes), alpha = 0.5, fill = "orange") + xlim(0, 20) +   stat_function(fun = dnorm, n = 101, args = list(mean = 10, sd = 3)) 

