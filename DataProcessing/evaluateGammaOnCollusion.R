#import CSV Function
source("autoImportCSV.R")
#values computed for each oligopoly
source("utilsOligopolies.R")
agents_nb <- 2
mode <- cournot2
model <- "COURNOT"

gammas <- c(-2, -1, 0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8)

results <- data.frame(convergenceTotalActions = numeric(), convergenceTotalProfits = numeric(),meanMaxDists = numeric(), 
                      sdMaxDists = numeric(), Delta = numeric(), gamma=numeric(),
                      a1ConvergenceAction = numeric(), a2ConvergenceAction = numeric(),
                      a1CriticMSE = numeric(), a1LocalCriticMSE = numeric(), a2CriticMSE = numeric(), a2LocalCriticMSE = numeric(),
                      a1CriticCollusiveMeanValue = numeric(), a2CriticCollusiveMeanValue = numeric())

distrib_results <- data.frame(gamma=numeric(), n=numeric(),
                              CI5share = numeric(), CI10share = numeric(), CI15share = numeric(),
                              Cartel_CI5share = numeric(), Cartel_CI10share = numeric(), Cartel_CI15share = numeric(),
                              localMean = numeric(), localSD = numeric(), localProfitMean =numeric(), shapiroPValue = numeric(),
                              localMeanMaxDist = numeric(), localSDMeanMaxDist = numeric(),
                              deltaMean = numeric(), profitDiff = numeric(), deltaDiff = numeric(), deltaAbsDiff = numeric(),
                              a1CriticCollusiveMeanValue = numeric(), a2CriticCollusiveMeanValue = numeric())

for (gamma in gammas){
  if (gamma == -1)
    #folder_path <- "Outputs/Cournot-2-60k-LIN-Gamma0/WN0.3/"
    folder_path <- "Outputs/Cournot-2-90k-TRUNC_EXP/60kgamma0m1000"
  else if (gamma == -2)
    #folder_path <- "Outputs/Cournot-2-60k-TRUNC_EXP-Gamma0/"
    folder_path <- "Outputs/Cournot-2-90k-TRUNC_EXP/60kgamma0m500"
  else
    folder_path <- paste(sep = "", "Outputs/Cournot-2-90k-TRUNC_EXP/gamma", gamma, "/")
  autoImport(folder_path = folder_path, agents_nb = agents_nb, lightmode = TRUE, mode=mode,importCritics=T)
  
  for (i in 1:length(simuls)) {
    s <- get(simuls[[i]])
    if (i <= length(critics) && length(critics) > 0)
      c <- get(critics[[i]])
    s <- s[s$whitenoise==0,]
    meanMaxDist <- computeMeanMaxDistance(agents_nb, s[s$round>58000&s$whitenoise==0,])$meansDiff
    sdMaxDist <- computeMeanMaxDistance(agents_nb, s[s$round>58000&s$whitenoise==0,])$sdDiff
    convergenceTotalAction <- convergence_test(s$totalQuantity, 0.98)$mean
    a1ConvergenceAction <- convergence_test(s$agent1Action, 0.98)$mean
    a2ConvergenceAction <- convergence_test(s$agent2Action, 0.98)$mean
    convergenceTotalProfit <- convergence_test(s$totalProfit, 0.98)$mean
    delt <- (convergenceTotalProfit - mode[["modelTotalProfit"]])/((mode[["cartelProfit"]] * agents_nb) - mode[["modelTotalProfit"]])
    
    if (i <= length(critics) && length(critics) > 0){
      a1CriticMSE <- mean(c$agent1CriticError^2)
      a1LocalCriticMSE <- mean(c[c$prevState<mode[["cournotQuantity"]]*1.15&c$prevState>mode[["cartelQuantity"]]*0.85&c$action<mode[["cournotQuantity"]]*1.15&c$action>mode[["cartelQuantity"]]*0.85,]$agent1CriticError^2)
      a2CriticMSE <- mean(c$agent2CriticError^2)
      a2LocalCriticMSE <- mean(c[c$prevState<mode[["cournotQuantity"]]*1.15&c$prevState>mode[["cartelQuantity"]]*0.85&c$action<mode[["cournotQuantity"]]*1.15&c$action>mode[["cartelQuantity"]]*0.85,]$agent2CriticError^2)
      a1CriticCollusiveMeanValue <- mean(c[c$prevState<mode[["cartelQuantity"]]*1.15&c$prevState>mode[["cartelQuantity"]]*0.85&c$action<mode[["cartelQuantity"]]*1.15&c$action>mode[["cartelQuantity"]]*0.85,]$agent1EstimatedProfit)/
                                    mean(c[c$prevState<mode[["cournotQuantity"]]*1.15&c$prevState>mode[["cournotQuantity"]]*0.85&c$action<mode[["cournotQuantity"]]*1.15&c$action>mode[["cournotQuantity"]]*0.85,]$agent1EstimatedProfit)
      a2CriticCollusiveMeanValue <- mean(c[c$prevState<mode[["cartelQuantity"]]*1.15&c$prevState>mode[["cartelQuantity"]]*0.85&c$action<mode[["cartelQuantity"]]*1.15&c$action>mode[["cartelQuantity"]]*0.85,]$agent2EstimatedProfit)/
        mean(c[c$prevState<mode[["cournotQuantity"]]*1.15&c$prevState>mode[["cournotQuantity"]]*0.85&c$action<mode[["cournotQuantity"]]*1.15&c$action>mode[["cournotQuantity"]]*0.85,]$agent2EstimatedProfit)
    }else {
      a1CriticMSE <- NA
      a1LocalCriticMSE <- NA
      a2CriticMSE <- NA
      a2LocalCriticMSE <- NA
      a1CriticCollusiveMeanValue <- NA
      a2CriticCollusiveMeanValue <- NA
    }
    results[nrow(results) +1,] <- c(convergenceTotalAction, convergenceTotalProfit, meanMaxDist, 
                                    sdMaxDist, delt, gamma, a1ConvergenceAction, a2ConvergenceAction,
                                    a1CriticMSE, a1LocalCriticMSE, a2CriticMSE, a2LocalCriticMSE, a1CriticCollusiveMeanValue, a2CriticCollusiveMeanValue)
    rm(s)
  }
  rm(list = unlist(simuls))
  rm(list = "simulsData")
  rm(list = "simuls")
  if (i <= length(critics) && length(critics) > 0){
    rm(list = "criticsData")
    rm(list = unlist(critics))
    rm(list = "critics")
  }
}
results$minCriticMSE <- ifelse(results$a1CriticMSE<results$a2CriticMSE, results$a1CriticMSE, results$a2CriticMSE)
results$minLocalCriticMSE <- ifelse(results$a1LocalCriticMSE<results$a2LocalCriticMSE, results$a1LocalCriticMSE, results$a2LocalCriticMSE)
results$meanCriticCollusiveMeanValue <- (results$a2CriticCollusiveMeanValue + results$a1CriticCollusiveMeanValue)/2
results$minCriticCollusiveMeanValue <- ifelse(results$a1CriticCollusiveMeanValue<results$a2CriticCollusiveMeanValue, results$a1CriticCollusiveMeanValue, results$a2CriticCollusiveMeanValue)
results$maxCriticCollusiveMeanValue <- ifelse(results$a1CriticCollusiveMeanValue>results$a2CriticCollusiveMeanValue, results$a1CriticCollusiveMeanValue, results$a2CriticCollusiveMeanValue)

for (gamma in gammas){
  CI5share <- nrow(results[results$gamma==gamma&results$convergenceTotalAction>=mode[["modelTotalQuantity"]]*0.95&results$convergenceTotalAction<=mode[["modelTotalQuantity"]]*1.05,])/nrow(results[results$gamma==gamma,])
  CI10share <- nrow(results[results$gamma==gamma&results$convergenceTotalAction>=mode[["modelTotalQuantity"]]*0.90&results$convergenceTotalAction<=mode[["modelTotalQuantity"]]*1.1,])/nrow(results[results$gamma==gamma,])
  CI15share <- nrow(results[results$gamma==gamma&results$convergenceTotalAction>=mode[["modelTotalQuantity"]]*0.85&results$convergenceTotalAction<=mode[["modelTotalQuantity"]]*1.15,])/nrow(results[results$gamma==gamma,])
  
  Cartel_CI5share <- nrow(results[results$gamma==gamma&results$convergenceTotalAction>=(mode[["cartelQuantity"]]*agents_nb)*0.95&results$convergenceTotalAction<=(mode[["cartelQuantity"]]*agents_nb)*1.05,])/nrow(results[results$gamma==gamma,])
  Cartel_CI10share <- nrow(results[results$gamma==gamma&results$convergenceTotalAction>=(mode[["cartelQuantity"]]*agents_nb)*0.90&results$convergenceTotalAction<=(mode[["cartelQuantity"]]*agents_nb)*1.1,])/nrow(results[results$gamma==gamma,])
  Cartel_CI15share <- nrow(results[results$gamma==gamma&results$convergenceTotalAction>=(mode[["cartelQuantity"]]*agents_nb)*0.85&results$convergenceTotalAction<=(mode[["cartelQuantity"]]*agents_nb)*1.15,])/nrow(results[results$gamma==gamma,])
  
  localMean <- mean(results[results$gamma==gamma, ]$convergenceTotalAction, na.rm=T)
  localSD <- sd(results[results$gamma==gamma, ]$convergenceTotalAction, na.rm=T)
  localProfitMean <- mean(results[results$gamma==gamma, ]$convergenceTotalProfit, na.rm=T)
  shapiroPValue <- shapiro.test(results[results$gamma==gamma, ]$convergenceTotalAction)$p.value
  localMeanMaxDist <- mean(results[results$gamma==gamma, ]$meanMaxDists, na.rm=T)
  localSDMeanMaxDist <- sd(results[results$gamma==gamma, ]$meanMaxDists, na.rm=T)
  deltaMean <- mean(results[results$gamma==gamma, ]$Delta, na.rm=T)
  profitDiff <- t.test(results[results$gamma==0,]$convergenceTotalProfit, results[results$gamma==gamma,]$convergenceTotalProfit, alternative = "greater")$p.value
  deltaDiff <- t.test(results[results$gamma==gamma,]$Delta, results[results$gamma==0,]$Delta, alternative = "greater")$p.value
  deltaAbsDiff <- t.test(results[results$gamma==gamma,]$Delta, mu = 0)$p.value
  a1CriticCollusiveMeanValue <- mean(results[results$gamma==gamma,]$a1CriticCollusiveMeanValue, na.rm = T)
  a2CriticCollusiveMeanValue <- mean(results[results$gamma==gamma,]$a2CriticCollusiveMeanValue, na.rm = T)
  distrib_results[nrow(distrib_results) +1,] <- c(gamma, nrow(results[results$gamma==gamma, ]), CI5share, CI10share, CI15share,
                                Cartel_CI5share, Cartel_CI10share, Cartel_CI15share,
                                localMean, localSD, localProfitMean, shapiroPValue, localMeanMaxDist, localSDMeanMaxDist,
                                deltaMean, profitDiff, deltaDiff, deltaAbsDiff,
                                a1CriticCollusiveMeanValue, a2CriticCollusiveMeanValue)
  
}

library(ggplot2)
ggplot(results[results$gamma>=0,], aes(x=gamma, y=Delta)) +
  geom_point(size=1) +
  geom_smooth(se = FALSE) +
  scale_color_manual(values = c("Simulations (Values)" = "lightblue", 
                                "Simulations (Trend)" = "darkblue")) +
  theme_minimal() + 
  labs(x = "gamma", y = "Delta (Collusion measure)", color = "Legend")

ggplot(results[results$gamma>=0,], aes(x=gamma, y=Delta)) +
  #geom_point(size=1) +
  geom_smooth(se = FALSE) +
  theme_minimal() + 
  labs(x = "gamma", y = "Delta (Collusion measure)", color = "Legend")

ggplot(results[results$gamma>=0,], aes(x=factor(gamma), y=Delta))  + 
  geom_boxplot(notch=FALSE)  + 
  #geom_dotplot(binaxis='y', stackdir = "center", dotsize=0.5) + 
  geom_smooth(se=TRUE, color="darkred", aes(group=1)) + 
  theme_minimal() + 
  labs(x = "gamma", y = "Delta (Collusion measure)", color = "Legend") +
  ylim(-1, 1)


ggplot(results[results$gamma>=0,], aes(x=factor(gamma), y=((a1CriticCollusiveMeanValue + a2CriticCollusiveMeanValue)/2)))  + 
  geom_boxplot(notch=FALSE)  + 
  #geom_dotplot(binaxis='y', stackdir = "center", dotsize=0.5) + 
  geom_smooth(method=lm, se=TRUE, color="darkred", aes(group=1)) + 
  theme_minimal() + 
  labs(x = "gamma", y = "Collusive Q-Value / Nash Q-Value", color = "Legend") +
  ylim(0.8, 1.06)

ggplot(results[results$gamma>=0&results$gamma<0.8,], aes(x=((a1CriticCollusiveMeanValue + a2CriticCollusiveMeanValue)/2), y=convergenceTotalProfits)) +
  geom_point(size=1) +
  geom_smooth(method=lm, se = T) +
  scale_color_manual(values = c("Simulations (Values)" = "lightblue", 
                                "Simulations (Trend)" = "darkblue")) +
  theme_minimal() + 
  labs(x = "CriticCollusiveMeanValue", y = "Total profit outcome", color = "Legend")

ggplot(results[results$gamma>=0&results$gamma<0.8,], aes(x=ifelse(a1CriticCollusiveMeanValue>a2CriticCollusiveMeanValue, a1CriticCollusiveMeanValue,a2CriticCollusiveMeanValue), y=convergenceTotalProfits)) +
  geom_point(size=1) +
  geom_smooth(method=lm, se = TRUE) +
  scale_color_manual(values = c("Simulations (Values)" = "lightblue", 
                                "Simulations (Trend)" = "darkblue")) +
  theme_minimal() + 
  labs(x = "CriticCollusiveMeanValue", y = "Total profit outcome", color = "Legend")

ggplot(results[results$gamma>=0&results$gamma<0.8,], aes(x=ifelse(a1CriticCollusiveMeanValue<a2CriticCollusiveMeanValue, a1CriticCollusiveMeanValue,a2CriticCollusiveMeanValue), y=convergenceTotalProfits)) +
  geom_point(size=1) +
  geom_smooth(method=lm, se = TRUE) +
  scale_color_manual(values = c("Simulations (Values)" = "lightblue", 
                                "Simulations (Trend)" = "darkblue")) +
  theme_minimal() + 
  labs(x = "CriticCollusiveMeanValue", y = "Total profit outcome", color = "Legend")

t.test(results[results$gamma==0,]$Delta, results[results$gamma==0.5,]$Delta, alternative = "greater")
summary(lm(data=results[results$gamma>=0&results$gamma<0.8,], convergenceTotalProfits ~ gamma))
summary(lm(data=results[results$gamma>=0,], I((a1CriticCollusiveMeanValue + a2CriticCollusiveMeanValue)/2) ~ gamma))

printDistribResults <- function(distrib_results) {
  for (i in 1:nrow(distrib_results)){
    print(paste(distrib_results[i,]$gamma, 1000, "$\num{9e4}$", round(distrib_results[i,]$CI5share, 2), round(distrib_results[i,]$CI10share,2),
          round(distrib_results[i,]$CI15share, 2), round(distrib_results[i,]$shapiroPValue,3), round(distrib_results[i,]$localMean, 3),
          round(distrib_results[i,]$localSD, 3), round(distrib_results[i,]$localMeanMaxDist, 3), round(distrib_results[i,]$localSDMeanMaxDist, 3),
          round(distrib_results[i,]$n, 3), "\\", sep=" & "))
  }
}

printDistribResultsCollusion <- function(distrib_results) {
  for (i in 1:nrow(distrib_results)){
    print(paste(distrib_results[i,]$gamma, 1000, "$\num{9e4}$", round(distrib_results[i,]$Cartel_CI5share, 2), round(distrib_results[i,]$Cartel_CI10share,2),
                round(distrib_results[i,]$Cartel_CI15share, 2), round(distrib_results[i,]$localProfitMean,3), round(mean(distrib_results[i,]$a1CriticCollusiveMeanValue, distrib_results[i,]$a2CriticCollusiveMeanValue), 3),
                round(distrib_results[i,]$deltaMean, 3), round(distrib_results[i,]$deltaDiff, 3), round(distrib_results[i,]$deltaAbsDiff, 3),
                round(distrib_results[i,]$n, 3), "\\", sep=" & "))
  }
}

summary(lm(data=results[results$gamma>=0&results$gamma<0.8,], minCriticCollusiveMeanValue ~ maxCriticCollusiveMeanValue + gamma))
summary(lm(data=results[results$gamma==-1,], minCriticCollusiveMeanValue ~ maxCriticCollusiveMeanValue))
summary(lm(data=results, minCriticCollusiveMeanValue ~ maxCriticCollusiveMeanValue + gamma))



summary(lm(data=results[results$gamma>0&results$gamma<0.8,], a1ConvergenceAction ~ a2CriticCollusiveMeanValue + a1CriticCollusiveMeanValue))
summary(lm(data=results[results$gamma==-1,], a2ConvergenceAction ~ a2CriticCollusiveMeanValue + a1CriticCollusiveMeanValue))

summary(lm(data=results[results$gamma>0&results$gamma<0.8&results$Delta>0.2,], a1ConvergenceAction ~ a2CriticCollusiveMeanValue + a1CriticCollusiveMeanValue))
summary(lm(data=results[results$gamma==-1&results$Delta<0,], a1ConvergenceAction ~ a2CriticCollusiveMeanValue + a1CriticCollusiveMeanValue))

summary(lm(data=results[results$gamma==-1&results$Delta<0,], a1ConvergenceAction ~ I(a2CriticCollusiveMeanValue/max(results[results$gamma==-1&results$Delta<0,]$maxCriticCollusiveMeanValue)) + I(a1CriticCollusiveMeanValue/max(results[results$gamma==-1&results$Delta<0,]$maxCriticCollusiveMeanValue))))
summary(lm(data=results[results$gamma>0&results$gamma<0.8&results$Delta>=0.2,], a1ConvergenceAction ~ I(a2CriticCollusiveMeanValue/max(results[results$gamma>0&results$gamma<0.8&results$Delta>=0.2,]$maxCriticCollusiveMeanValue)) + I(a1CriticCollusiveMeanValue/max(results[results$gamma>0&results$gamma<0.8&results$Delta>=0.2,]$maxCriticCollusiveMeanValue))))

summary(lm(data=results[results$gamma==-1&results$Delta<0,], a2ConvergenceAction ~ I(a2CriticCollusiveMeanValue/maxCriticCollusiveMeanValue) + I(a1CriticCollusiveMeanValue/maxCriticCollusiveMeanValue)))
summary(lm(data=results[results$gamma>0&results$gamma<0.8&results$Delta>=0.2,], a2ConvergenceAction ~ I(a2CriticCollusiveMeanValue/maxCriticCollusiveMeanValue) + I(a1CriticCollusiveMeanValue/maxCriticCollusiveMeanValue)))


tmp1 <- results
tmp1$a_i <- tmp1$a1ConvergenceAction
tmp1$a_minus_i <- tmp1$a2ConvergenceAction
tmp1$delta_i <- tmp1$a1CriticCollusiveMeanValue
tmp1$delta_minus_i <- tmp1$a2CriticCollusiveMeanValue

tmp2 <- results
tmp2$a_i <- tmp2$a2ConvergenceAction
tmp2$a_minus_i <- tmp2$a1ConvergenceAction
tmp2$delta_i <- tmp2$a2CriticCollusiveMeanValue
tmp2$delta_minus_i <- tmp2$a1CriticCollusiveMeanValue

tmp <- rbind(subset(tmp1, select=-c(a1ConvergenceAction, a2ConvergenceAction, a1CriticCollusiveMeanValue, a2CriticCollusiveMeanValue)),
             subset(tmp2, select=-c(a1ConvergenceAction, a2ConvergenceAction, a1CriticCollusiveMeanValue, a2CriticCollusiveMeanValue)))


library(car)
model <- lm(data=results[results$gamma==-1&results$Delta<0,], a1ConvergenceAction ~ I(a2CriticCollusiveMeanValue/maxCriticCollusiveMeanValue) + I(a1CriticCollusiveMeanValue/maxCriticCollusiveMeanValue))
vif(model)

summary(lm(data=results[results$gamma>=0&results$gamma<0.8,], minCriticCollusiveMeanValue ~ maxCriticCollusiveMeanValue + gamma))
summary(lm(data=results[results$gamma==-1&results$Delta<0.3,], minCriticCollusiveMeanValue ~ maxCriticCollusiveMeanValue))

summary(lm(data=tmp[tmp$gamma>=0&tmp$gamma<0.8,], a_i ~ delta_i + delta_minus_i))
summary(lm(data=tmp[tmp$gamma==-1&results$Delta<0.3,], a_i ~ delta_i + delta_minus_i))




