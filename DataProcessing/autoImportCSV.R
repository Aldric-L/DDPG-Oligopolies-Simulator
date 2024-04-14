
autoImport <- function (folder_path, lightmode, agents_nb, mode, importCritics = TRUE){
  library(data.table)
  csv_files <- list.files(path = folder_path, pattern = "\\.csv$", full.names = TRUE)
  simuls <- list()
  critics <- list()
  
  for (i in 1:length(csv_files)) {
    file_name <- tools::file_path_sans_ext(basename(csv_files[i]))
    last_number <- as.numeric(sub('.*-(\\d+)$', '\\1', file_name))
    data <- read.csv(csv_files[i])
    if (grepl("Critics", file_name) && importCritics){
      for (agent in 1:agents_nb){
        data[, paste("agent", agent, "ActorError", sep="")] <- data[, paste("agent", agent, "Actor", sep="")] - mode[["reactionFunction"]](data$prevState)
        data[, paste("agent", agent, "CriticError", sep="")] <- data[, paste("agent", agent, "EstimatedProfit", sep="")] - data$agentProfit
        
        data[, paste("agent", agent, "CriticErrorNormed", sep="")] <- (data[, paste("agent", agent, "EstimatedProfit", sep="")]-mean(data[, paste("agent", agent, "EstimatedProfit", sep="")], na.rm=T))/sd(data[, paste("agent", agent, "EstimatedProfit", sep="")], na.rm=T) 
        - (data$agentProfit-mean(data$agentProfit, na.rm=T))/sd(data$agentProfit, na.rm=T)
      }
      
      assign(paste("critic_", toString(last_number), sep=""), data, envir = .GlobalEnv)
      critics <- append(critics, paste("critic_", toString(last_number), sep=""))
    }else if(!grepl("Critics", file_name)) {
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
        data[, paste("agent", agent, "ActionError", sep="")] <- data[, paste("agent", agent, "Action", sep="")]-mode[["reactionFunction"]](data$totalQuantity-data[, paste("agent", agent, "Action", sep="")])
      }
      data$criticError <- data$criticError / agents_nb
      assign(paste("sim_", toString(last_number), sep=""), data, envir = .GlobalEnv)
      simuls <- append(simuls, paste("sim_", toString(last_number), sep=""))
      # if (i > 1 && nrow(get(simuls[[i]])) != nrow(get(simuls[[i-1]]))){
      #   library(R.methodsS3)
      #   throw("ALERT: All simulations are not equally sized")
      # }
    }
    
  }
  rm(data)
  assign("simuls", simuls, envir = .GlobalEnv)
  if (importCritics)
    assign("critics", critics, envir = .GlobalEnv)
  
  s <- get(simuls[[1]])
  s$simulName <- simuls[[1]]
  simulsData <- s
  for (i in 2:length(simuls)) {
    s <- get(simuls[[i]])
    s$simulName <- simuls[[i]]
    
    simulsData <- rbind(simulsData, s)
  }
  rm(s)
  assign("simulsData", simulsData, envir = .GlobalEnv)
  
  if (importCritics && length(critics) > 0){
    c <- get(critics[[1]])
    c$simulName <- critics[[1]]
    criticsData <- c
    for (i in 2:length(critics)) {
      c <- get(critics[[i]])
      c$simulName <- critics[[i]]
      
      criticsData <- rbind(criticsData, c)
    }
    rm(c)
    assign("criticsData", criticsData, envir = .GlobalEnv)
  }
  
}

cleanSimulationsFromRam <- function(simuls, critics){
  rm(list = unlist(simuls), envir = .GlobalEnv)
  rm(list = "simulsData", envir = .GlobalEnv)
  rm(list = "simuls", envir = .GlobalEnv)
  if (i <= length(critics) && length(critics) > 0){
    rm(list = "criticsData", envir = .GlobalEnv)
    rm(list = unlist(critics), envir = .GlobalEnv)
    rm(list = "critics", envir = .GlobalEnv)
  }
}

# library(usethis) 
# usethis::edit_r_environ(scope="project")
# R_MAX_VSIZE=100Gb 