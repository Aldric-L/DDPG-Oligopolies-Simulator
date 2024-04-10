#import CSV Function
source("autoImportCSV.R")
#values computed for each oligopoly
source("utilsOligopolies.R")
agents_nb <- 2
mode <- cournot2
model <- "COURNOT"

gammas <- c(0, 0.7, 0.8)

results <- data.frame(convergenceTotalActions = numeric(), convergenceTotalProfits = numeric(),meanMaxDists = numeric(), 
                      sdMaxDists = numeric(), Delta = numeric(), gamma=numeric())

distrib_results <- data.frame(gamma=numeric(), n=numeric(),
                              CI5share = numeric(), CI10share = numeric(), CI15share = numeric(),
                              Cartel_CI5share = numeric(), Cartel_CI10share = numeric(), Cartel_CI15share = numeric(),
                              localMean = numeric(), localSD = numeric(), shapiroPValue = numeric(),
                              localMeanMaxDist = numeric(), localSDMeanMaxDist = numeric())

for (gamma in gammas){
  folder_path <- paste(sep = "", "Outputs/Cournot-2-90k-TRUNC_EXP/gamma", gamma, "/")
  autoImport(folder_path = folder_path, agents_nb = agents_nb, lightmode = TRUE, mode=mode,importCritics=F)
  
  for (i in 1:length(simuls)) {
    s <- get(simuls[[i]])
    s <- s[s$whitenoise==0,]
    meanMaxDist <- computeMeanMaxDistance(agents_nb, s[s$round>58000&s$whitenoise==0,])$meansDiff
    sdMaxDist <- computeMeanMaxDistance(agents_nb, s[s$round>58000&s$whitenoise==0,])$sdDiff
    convergenceTotalAction <- convergence_test(s$totalQuantity, 0.98)$mean
    convergenceTotalProfit <- convergence_test(s$totalProfit, 0.98)$mean
    delt <- (convergenceTotalProfit - mode[["modelTotalProfit"]])/((mode[["cartelProfit"]] * agents_nb) - mode[["modelTotalProfit"]])
    results[nrow(results) +1,] <- c(convergenceTotalAction, convergenceTotalProfit, meanMaxDist, 
                                    sdMaxDist, delt, gamma)
    rm(s)
    rm(list = simuls[[i]])
  }
}

for (gamma in gammas){
  CI5share <- nrow(results[results$gamma==gamma&results$convergenceTotalAction>=mode[["modelTotalQuantity"]]*0.95&results$convergenceTotalAction<=mode[["modelTotalQuantity"]]*1.05,])/nrow(results[results$gamma==gamma,])
  CI10share <- nrow(results[results$gamma==gamma&results$convergenceTotalAction>=mode[["modelTotalQuantity"]]*0.90&results$convergenceTotalAction<=mode[["modelTotalQuantity"]]*1.1,])/nrow(results[results$gamma==gamma,])
  CI15share <- nrow(results[results$gamma==gamma&results$convergenceTotalAction>=mode[["modelTotalQuantity"]]*0.85&results$convergenceTotalAction<=mode[["modelTotalQuantity"]]*1.15,])/nrow(results[results$gamma==gamma,])
  
  Cartel_CI5share <- nrow(results[results$gamma==gamma&results$convergenceTotalAction>=(mode[["cartelQuantity"]]*agents_nb)*0.95&results$convergenceTotalAction<=(mode[["cartelQuantity"]]*agents_nb)*1.05,])/nrow(results[results$gamma==gamma,])
  Cartel_CI10share <- nrow(results[results$gamma==gamma&results$convergenceTotalAction>=(mode[["cartelQuantity"]]*agents_nb)*0.90&results$convergenceTotalAction<=(mode[["cartelQuantity"]]*agents_nb)*1.1,])/nrow(results[results$gamma==gamma,])
  Cartel_CI15share <- nrow(results[results$gamma==gamma&results$convergenceTotalAction>=(mode[["cartelQuantity"]]*agents_nb)*0.85&results$convergenceTotalAction<=(mode[["cartelQuantity"]]*agents_nb)*1.15,])/nrow(results[results$gamma==gamma,])
  
  localMean <- mean(results[results$gamma==gamma, ]$convergenceTotalAction, na.rm=T)
  localSD <- sd(results[results$gamma==gamma, ]$convergenceTotalAction, na.rm=T)
  shapiroPValue <- shapiro.test(results[results$gamma==gamma, ]$convergenceTotalAction)$p.value
  localMeanMaxDist <- mean(results[results$gamma==gamma, ]$meanMaxDists, na.rm=T)
  localSDMeanMaxDist <- sd(results[results$gamma==gamma, ]$meanMaxDists, na.rm=T)
  distrib_results[nrow(distrib_results) +1,] <- c(gamma, nrow(results[results$gamma==gamma, ]), CI5share, CI10share, CI15share,
                                Cartel_CI5share, Cartel_CI10share, Cartel_CI15share,
                                localMean, localSD, shapiroPValue, localMeanMaxDist, localSDMeanMaxDist)
  
}

library(ggplot2)
ggplot(results, aes(x=gamma, y=Delta)) +
  geom_point(size=1) +
  geom_smooth(se = FALSE) +
  scale_color_manual(values = c("Simulations (Values)" = "lightblue", 
                                "Simulations (Trend)" = "darkblue")) +
  theme_minimal() + 
  labs(x = "gamma", y = "Delta (Collusion measure)", color = "Legend")

ggplot(results, aes(x=as.factor(results$gamma), y=Delta))  + 
  geom_boxplot(notch=FALSE)  + geom_dotplot(binaxis='y', stackdir = "center", dotsize=0.5) + 
  geom_smooth(method=lm, se=TRUE, color="darkred") + 
  theme_minimal() + 
  labs(x = "gamma", y = "Delta (Collusion measure)", color = "Legend")
