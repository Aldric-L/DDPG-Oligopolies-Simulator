data1 <- read.csv("DDPG-TemporalCournot-2-5748710865.csv")
data2 <- read.csv("DDPG-TemporalCournot-2-5760298939.csv")
data3 <- read.csv("DDPG-TemporalCournot-2-5767274812.csv")
data4 <- read.csv("DDPG-TemporalCournot-2-5775338260.csv")
data5 <- read.csv("DDPG-TemporalCournot-2-5750257113.csv")
data6 <- read.csv("DDPG-TemporalCournot-2-5758143529.csv")
data7 <- read.csv("DDPG-TemporalCournot-2-5793131542.csv")
data8 <- read.csv("DDPG-TemporalCournot-2-5799786991.csv")

library(ggplot2)
ggplot() +
  geom_smooth(aes(x = data1[data1$round>0&data1$whitenoise==0,]$round, y = data1[data1$round>0&data1$whitenoise==0,]$agent1Action+data1[data1$round>0&data1$whitenoise==0,]$agent2Action, color = "d1"), se = FALSE) +
  geom_smooth(aes(x = data2[data2$round>0&data2$whitenoise==0,]$round, y = data2[data2$round>0&data2$whitenoise==0,]$agent1Action+data2[data2$round>0&data2$whitenoise==0,]$agent2Action, color = "d2"), se = FALSE) +
  geom_smooth(aes(x = data3[data3$round>0&data3$whitenoise==0,]$round, y = data3[data3$round>0&data3$whitenoise==0,]$agent1Action+data3[data3$round>0&data3$whitenoise==0,]$agent2Action, color = "d3"), se = FALSE) +
  geom_smooth(aes(x = data4[data4$round>0&data4$whitenoise==0,]$round, y = data4[data4$round>0&data4$whitenoise==0,]$agent1Action+data4[data4$round>0&data4$whitenoise==0,]$agent2Action, color = "d4"), se = FALSE) +
  geom_smooth(aes(x = data5[data5$round>0&data5$whitenoise==0,]$round, y = data5[data5$round>0&data5$whitenoise==0,]$agent1Action+data5[data5$round>0&data5$whitenoise==0,]$agent2Action, color = "d5"), se = FALSE) +
  geom_smooth(aes(x = data6[data6$round>0&data6$whitenoise==0,]$round, y = data6[data6$round>0&data6$whitenoise==0,]$agent1Action+data6[data6$round>0&data6$whitenoise==0,]$agent2Action, color = "d6"), se = FALSE) +
  geom_smooth(aes(x = data7[data7$round>0&data7$whitenoise==0,]$round, y = data7[data7$round>0&data7$whitenoise==0,]$agent1Action+data7[data7$round>0&data7$whitenoise==0,]$agent2Action, color = "d7"), se = FALSE) +
  geom_smooth(aes(x = data8[data8$round>0&data8$whitenoise==0,]$round, y = data8[data8$round>0&data8$whitenoise==0,]$agent1Action+data8[data8$round>0&data8$whitenoise==0,]$agent2Action, color = "d8"), se = FALSE) +
  geom_line(aes(x = data1[data1$round>0&data1$whitenoise==0,]$round, y = 2*0.9166666666666667, color = "Competition")) +
  geom_line(aes(x = data1[data1$round>0&data1$whitenoise==0,]$round, y = 2*0.6470588235294118, color = "Cournot")) +
  geom_line(aes(x = data1[data1$round>0&data1$whitenoise==0,]$round, y = 2*0.5, color = "Cartel")) +
  labs(x = "Round", y = "Quantity", color = "Legend") +
  theme_minimal()
