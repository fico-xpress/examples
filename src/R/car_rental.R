#####################################
# This file is part of the          #
# Xpress-R interface examples       #
#                                   #
#   (c) 2022-2026 Fair Isaac Corporation #
#####################################
#' ---
#' title: "Fleet Management In Car Rental"
#' author: Y. Gu
#' date: Jun. 2021
#' ---
#' 
#' 
## ----setup, include=FALSE-----------------------------------------------------
knitr::opts_chunk$set(echo = TRUE)
knitr::opts_chunk$set(results = "hold")
knitr::opts_chunk$set(warning = FALSE, message = FALSE)


#' 
#' 
#' ## Brief Introduction To The Problem
#' 
#' [This is a conversion of the Mosel example 'Car Rental'](https://www.fico.com/fico-xpress-optimization/docs/latest/examples/mosel/ApplBook/E_TransGrd/e1carrent.mos).
#' A brief introduction to this problem is given below, and to see the full
#' mathematical modeling of this problem you may refer to section 10.1, page 140 of the
#' book 'Applications of optimization with Xpress'.
#' 
#' A car rental company has its 94 vehicles distributed among 10 agencies. For each
#' agency, the number of cars required for the next morning and the stocks in the preceding
#' evening are known. We would like to find a minimum cost plan for moving vehicles among
#' agencies to satisfy all the requirements.
#' 
#' Firstly, we test whether the number of required vehicles is equal to the number of
#' vehicles in stock. If this is not true, then we stop, otherwise we continue to define
#' two sets 'Excess' and 'Need' of agencies with an excess or deficit of cars. The road
#' distances between each pair of agencies taking one from 'Excess' and the other from 'Need'
#' are calculated.
#' 
#' Since we want to move cars from agencies in set 'Excess' to agencies in 'Need', we define
#' variables 'move_i_j' with i in set 'Excess' and j in set 'Need' to represent the number
#' of cars transported from agency i to j. The objective to minimize is the sum of total
#' costs for all the car movements.
#' 
#' As to the constraints, for each agency in 'Excess', they need to give their extra
#' vehicles to others that need cars. For agencies in 'Need', they should receive a
#' sufficient number of cars from 'Excess' agencies to satisfy the requirement.
#' 
#' The mathematical formulations of these constraints are included in the guide book
#' 'Applications of optimization with Xpress'.
#' 
#' 
#' For this example, we need packages 'xpress' and 'dplyr'.
#' 
## ----Load The Packages And The Function To Give Names-------------------------
library(xpress)
library(dplyr)

#' 
#' 
#' Add the values we need for this example.
#' 
## ----Data---------------------------------------------------------------------
data.df <- data.frame(
  Agency = 1:10,
  Req = c(10, 6, 8, 11, 9, 7, 15, 7, 9, 12), # REQUIREMENT
  Stock = c(8, 13, 4, 8, 12, 2, 14, 11, 15, 7), # STOCK
  X_axis = c(0, 20, 18, 30, 35, 33, 5, 5, 11, 2), # X coordinate
  Y_axis = c(0, 20, 10, 12, 0, 25, 27, 10, 0, 15) # Y coordinate
)

# cost for transporting a car per kilometer
Cost <- 0.5


#' 
#' 
#' If the total requirement is not equal to the total stock, then we stop, otherwise
#' we continue.
#' 
## ----Test Whether Stocks Equal Requirements-----------------------------------
# calculate whether sum(Req)=sum(Stock), if not, we stop
testthat::expect_equal(sum(data.df$Req), sum(data.df$Stock))

# the amount of stocks and requirements are the same, so we continue


#' 
#' 
#' Create a new empty problem and give the problem a suitable name.
#' 
## ----Create The Problem-------------------------------------------------------
# create a new problem
prob = createprob()

# set problem name
setprobname(prob, "CarRental")


#' 
#' 
#' Define sets 'Excess' and 'Need' to include agencies with excess or deficit of cars.
#' We also calculate the distances between agencies from these two sets and store the
#' distances in a data frame 'Dist'.
#' 
## ----Sets And Distances-------------------------------------------------------
# agencies with excess of cars
Excess <- data.df %>% filter(Stock > Req)

# agencies with deficit of cars
Need <- data.df %>% filter(Stock < Req)

# calculate the road distances (1.3 times the Euclidean distances) between agencies in Excess and Need
Dist <-
  as.data.frame(matrix(0, nrow = nrow(Excess), ncol = nrow(Need)))
colnames(Dist) <- Need$Agency
rownames(Dist) <- Excess$Agency

for (i in 1:nrow(Excess))
  for (j in 1:nrow(Need)) {
    Dist[i, j] <-
      1.3 * sqrt((Excess$X_axis[i] - Need$X_axis[j]) ^ 2 + (Excess$Y_axis[i] -
                                                              Need$Y_axis[j]) ^ 2)
  }


#' 
#' 
#' Create variables 'move' and store their indices in a data frame 'index.df'.
#' 
## ----Add Columns--------------------------------------------------------------
# variables 'move_i_j', where i in set 'Excess' and j in set 'Need'
index.df <-
  as.data.frame(matrix(0, nrow = nrow(Excess), ncol = nrow(Need)))
colnames(index.df) <- Need$Agency
rownames(index.df) <- Excess$Agency

for (i in 1:nrow(Excess))
  for (j in 1:nrow(Need)) {
    index.df[i, j] <- xprs_newcol(
      prob,
      lb = 0,
      ub = Inf,
      coltype = "I",
      name = sprintf("move_%d_%d", Excess$Agency[i], Need$Agency[j]),
      objcoef = Cost * Dist[i, j]
    )
  }


#' 
#' 
#' Add constraints that ensure the requirement of all agencies will be satisfied.
#' 
## ----Add Rows, results='hide'-------------------------------------------------
Excess <- cbind(Excess, index.df)
Need <- cbind(Need, t(index.df))

# agencies with excess availability
apply(Excess, 1, function(x)
  xprs_addrow(
    prob,
    colind = x[as.character(Need$Agency)],
    rowcoef = rep(1, nrow(Need)),
    rowtype = "E",
    rhs = x["Stock"] - x["Req"],
    name = paste0("Excess_Agency_", x["Agency"])
  ))

# agencies in need of cars
apply(Need, 1, function(x)
  xprs_addrow(
    prob,
    colind = x[as.character(Excess$Agency)],
    rowcoef = rep(1, nrow(Excess)),
    rowtype = "E",
    rhs = x["Req"] - x["Stock"],
    name = paste0("Need_Agency_", x["Agency"])
  ))


#' 
#' 
#' Now we can solve the problem.
#' 
## ----Solve The Problem--------------------------------------------------------
# solve the problem
setoutput(prob)
summary(xprs_optimize(prob))


#' 
#' 
#' Display the solutions here.
#' 
## ----The Solutions------------------------------------------------------------
# solutions
move_solution <-
  as.data.frame(matrix(
    getsolution(prob)$x,
    byrow = TRUE,
    nrow = nrow(Excess),
    ncol = nrow(Need)
  ))
colnames(move_solution) <- Need$Agency
rownames(move_solution) <- Excess$Agency

objval <- round(getdblattrib(prob, xpress:::MIPOBJVAL), 3)
print(paste("The optimum cost is:", objval))
print("The amount moved from 'Excess' agencies(column) to 'Need' agencies(row):")
move_solution


#' 
#' 
