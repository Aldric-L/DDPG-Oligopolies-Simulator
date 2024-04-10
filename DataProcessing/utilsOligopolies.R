cournot4 <- c("compProfit" = 0, "compQuantity" = 0.423,
              "cournotProfit" = 0.403, "cournotQuantity" = 0.355,
              "cartelProfit" = 0.526, "cartelQuantity" = 0.239,
              "modelTotalProfit" = 1.612,
              "modelTotalQuantity" = 1.419,
              "cournotTotalQuantity" = 1.419,
              "reactionFunction" = function(Q) {
                return((2.2-Q)/(2*(1+0.2)))
              })


cournot4bis <- c("compProfit" = 0.289, "compQuantity" = 0.962,
                 "cournotProfit" = 0.410, "cournotQuantity" = 0.806,
                 "cartelProfit" = 0.489, "cartelQuantity" = 0.543,
                 "modelTotalProfit" = 4*0.410,
                 "modelTotalQuantity" = 4*0.806,
                 "cournotTotalQuantity" = 4*0.806,
                 "reactionFunction" = function(Q) {
                   return((2.2-Q)/(2*(1+0.2)))
                 })

cournot2 <- c("compProfit" = 0.16469444444444442, "compQuantity" = 0.9166666666666667,
              "cournotProfit" = 0.49237370242214545, "cournotQuantity" = 0.6470588235294118,
              "cartelProfit" = 0.539, "cartelQuantity" = 0.5,
              "modelTotalProfit" = 2 * 0.49237370242214545,
              "modelTotalQuantity" = 2 * 0.6470588235294118,
              "cournotTotalQuantity" = 2 * 0.6470588235294118,
              "reactionFunction" = function(Q) {
                return((2.2-Q)/(2*(1+0.2)))
              })

stackelberg2bis <- c(
  "compProfit" = 0.037, "compQuantity" = 0.857,
  "lowProfit" = 0.227, "lowQuantity" = 0.465,
  "highProfit" = 0.387, "highQuantity" = 0.823,
  "cartelProfit" = 0.395, "cartelQuantity" = 0.439,
  "cournotProfit" = 0.354, "cournotQuantity" = 0.581,
  "modelTotalProfit" = 0.387+0.227, 
  "modelTotalQuantity" = 0.465 + 0.823,
  "cournotTotalQuantity" = 0.581 + 0.581,
  "reactionFunction" = function(Q) {
    return((1.8-Q)/(2*(1+0.05)))
  })

stackelberg2 <- c(
  "compProfit" = 0.16469444444444442, "compQuantity" = 0.9166666666666667,
  "cournotProfit" = 0.49237370242214545, "cournotQuantity" = 0.6470588235294118,
  "lowProfit" = 0.37931870274102086, "lowQuantity" = 0.5679347826086958,
  "highProfit" = 0.5148647211720228, "highQuantity" = 0.8369565217391304,
  "cartelProfit" = 0.539, "cartelQuantity" = 0.5,
  "modelTotalProfit" = 0.37931870274102086+0.5148647211720228, 
  "modelTotalQuantity" = 0.5679347826086958 + 0.8369565217391304,
  "cournotTotalQuantity" = 2 * 0.6470588235294118,
  "reactionFunction" = function(Q) {
    return((2.2-Q)/(2*(1+0.2)))
  })

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

computeAverageCritic <- function (agents_nb, localCritic){
  averageCritic <- data.frame(prevState= localcritic$prevState, action = localcritic$action, price= localcritic$price,agentProfit=localcritic$agentProfit,
                              agent1EstimatedProfit=0, agent1CriticError=0,
                              agent2EstimatedProfit=0, agent2CriticError=0,
                              agent3EstimatedProfit=0, agent3CriticError=0,
                              agent4EstimatedProfit=0, agent4CriticError=0,
                              agentsEstimatedProfit=0, agentsCriticError=0,
                              tempEstimatedProfit=0, tempCriticError=0)
  
  for (critic in critics){
    c <- get(critic)
    averageCritic[, "tempEstimatedProfit"] <- c[, paste("agent1EstimatedProfit", sep="")]
    averageCritic[, "tempCriticError"] <- c[, paste("agent1CriticError", sep="")]
    for (i in 1:agents_nb){
      averageCritic[, "tempEstimatedProfit"] <- c[, paste("agent", i, "EstimatedProfit", sep="")]
      averageCritic[, "tempCriticError"] <- c[, paste("agent", i, "CriticError", sep="")]
      averageCritic[, paste("agent", i, "EstimatedProfit", sep="")] <- averageCritic[, paste("agent", i, "EstimatedProfit", sep="")] + c[, paste("agent", i, "EstimatedProfit", sep="")]
      averageCritic[, paste("agent", i, "CriticError", sep="")] <- averageCritic[, paste("agent", i, "CriticError", sep="")] + c[, paste("agent", i, "CriticError", sep="")]
    }
    averageCritic[, "agentsEstimatedProfit"] <- averageCritic[, "agentsEstimatedProfit"] + averageCritic[, "tempEstimatedProfit"]/agents_nb
    averageCritic[, "agentsCriticError"] <- averageCritic[, "agentsCriticError"] + averageCritic[, "tempCriticError"]/agents_nb
  }
  rm(c)
  for (i in 1:agents_nb){
    averageCritic[, paste("agent", i, "EstimatedProfit", sep="")] <- averageCritic[, paste("agent", i, "EstimatedProfit", sep="")]/length(critics)
    averageCritic[, paste("agent", i, "CriticError", sep="")] <- averageCritic[, paste("agent", i, "CriticError", sep="")]/length(critics)
  }
  assign("averageCritic", averageCritic, envir = .GlobalEnv)
}
