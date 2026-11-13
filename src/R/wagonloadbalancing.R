#####################################
# This file is part of the          #
# Xpress-R interface examples       #
#                                   #
#   (c) 2022-2026 Fair Isaac Corporation #
#####################################
#' ---
#' title: "Wagon Load Balancing"
#' author: Y. Gu
#' date: Jun.2021
#' ---
#' 
#' 
## ----setup, include=FALSE-----------------------------------------------------
knitr::opts_chunk$set(echo = TRUE)
knitr::opts_chunk$set(results = "hold")
knitr::opts_chunk$set(warning = FALSE,message = FALSE)


#' 
#' 
#' ## Brief Introduction To The Problem
#' 
#' [This is a conversion of the Mosel example 'Wagon Load Balancing'](https://www.fico.com/fico-xpress-optimization/docs/latest/examples/mosel/ApplBook/D_LoadCut/d1wagon2.mos).
#' A brief introduction to this problem is given below, and to see the full
#' mathematical modeling of this problem you may refer to section 9.1, page 122 of the
#' book 'Applications of optimization with Xpress'.
#' 
#' In this example, 16 boxes are required to be loaded into 3 railway wagons each with
#' capacity of 100 quintals. This example is similar to the bin packing problem, but
#' instead of minimizing the number of bins used, here we would like to minimize the
#' heaviest wagon load.
#' 
#' We
#' formulate this example as a MIP problem and solve it using Xpress optimizer.
#' 
#' The objective of the MIP problem is the maximum weight over all the wagon loads which
#' we want to minimize. We define it as an integer variable 'maxweight'. Since the ideal
#' case is that all wagons take the same load and the box weights are all integers, so
#' we round the average of total box weights to the next larger integer and set this
#' number as the lower bound of 'maxweight'. Without this lower bound, the optimum
#' solution will be found easily but it will take a long time to prove its optimality.
#' We also define binary variables for all possible pairs of boxes and wagons, and value
#' '1' indicates that the box is assigned to the wagon and value '0' means no assignment.
#' 
#' The constraints are clear, firstly, boxes are not fragmented and thus each box can
#' only be assigned to one wagon. Then, we set 'maxweight' as upper bound of the weight
#' of the boxes loaded into each wagon. By proceeding this way, in the optimal solution
#' the minimization will force 'maxweight' to take the value corresponding to the weight
#' of the heaviest wagon load. The mathematical formulations of these constraints are
#' included in the guide book 'Applications of optimization with Xpress'.
#' 
#' Before we solve the MIP, we create an initial solution using the following
#' simple heuristic:
#' In each iteration, we greedily choose the heaviest unloaded box and assign it
#' to the wagon with the least load until all boxes are assigned.
#' This does not yield a feasible solution for the data of the example, but can
#' still be added to the MIP as an infeasible starting point to be repaired.
#' 
#' 
#' For this example, we need packages 'xpress' and 'dplyr'. Besides, we use the
#' function `pretty_name` to give the variables and constraints concise names.
#' 
## ----Load The Packages And The Function To Give Names-------------------------
library(xpress)
library(dplyr)

pretty_name <- function(prefix, y) {
  "%s_%s" %>% sprintf(prefix,
                      paste(lapply(names(y), function(name)
                        paste(name, y[name], sep = "_")), collapse = "_"))

}


#' 
#' 
#' Add the values we need for this example.
#' 
## ----Data---------------------------------------------------------------------
# concrete values
weight.df <- data.frame(
  Box = 1:16,
  Weight = c(34, 6, 8, 17, 16, 5, 13, 21, 25, 31, 14, 13, 33, 9, 25, 25)
)

# carrying capacity of each wagon
WMAX <- 100

# wagons
Wagon <- 1:3


#' 
#' 
#' Create a function to get the heuristic solution.
#' 
## ----Heuristic----------------------------------------------------------------
solve_heur <- function(weight, wagon) {
  # sort the weight in descending order
  sorted <-
    sort(
      weight,
      decreasing = TRUE,
      method = "shell",
      index.return = TRUE
    )
  weight.sorted <- sorted[[1]]
  box.sorted <- sorted[[2]]

  n = length(wagon)
  CurWeight <- rep(0, n) # current weight of wagon loads
  CurNumber <- rep(0, n) # current number of boxes per wagon
  Load <- vector("list", n) # boxes loaded onto the wagons

  l = length(weight.sorted)
  for (i in 1:l) {
    v = which.min(CurWeight)
    CurNumber[v] <- CurNumber[v] + 1
    Load[[v]] = c(Load[[v]], box.sorted[i])
    CurWeight[v] <- CurWeight[v] + weight.sorted[i]
  }

  results <- list(max(CurWeight), Load)
  names(results) <- c("maximum_weight", "Load")
  return(results)

}


#' 
#' 
#' Create a function to formulate this example as a MIP problem and solve it.
#' 
## ----Define And Solve the MIP Problem-----------------------------------------
solve_MIP <- function(weight.df,Wagon){
  # firstly, create a new problem
  prob <- createprob()
  # set the problem name
  setprobname(prob, "WagonLoadBalancing")

  # add columns
  # 1. variable 'maxweight'
  maxweight <-
    xprs_newcol(
      prob,
      lb = ceiling(sum(weight.df$Weight) / 3),
      ub = Inf,
      coltype = "I",
      name = "maxweight",
      objcoef = 1
    )

  # 2. binary variables 'load'
  # create a data frame to store the 'load' indices
  loads_index <- data.frame(
    idx = 1:(length(Wagon) * nrow(weight.df)),
    Wagon = rep(Wagon, each = nrow(weight.df)),
    Box = rep(weight.df$Box, length(Wagon)),
    Weight = rep(weight.df$Weight, length(Wagon))
  )

  loads_index$loads <- loads_index %>% apply(1, function(x)
    xprs_newcol(
      prob,
      lb = 0,
      ub = 1,
      coltype = "B",
      name = pretty_name("Load_", x["idx"])
    ))




  # add rows
  # 1. each box can only be assigned to one wagon
  apply(weight.df, 1, function(x)
    xprs_newrow(
      prob,
      colind = (loads_index %>% filter(Box == x["Box"]))$loads,
      rowcoef = rep(1, length(Wagon)),
      rowtype = "E",
      rhs = 1,
      name = pretty_name("Box_Wagon", x["Box"])
    ))

  # 2. set 'maxweight' as the upper bound of weight loaded into each wagon
  loads_index %>% group_by(Wagon) %>%
    group_map( ~ xprs_newrow(
      prob,
      colind = c(.x$loads, maxweight),
      rowcoef = c(.x$Weight, -1),
      rowtype = 'L',
      rhs = 0,
      name = pretty_name("Wagon_Capacity", .y)
    ))


  # solve the heuristic and get the solution
  heuristic <- solve_heur(weight.df$Weight, Wagon)

  HeurMaxWeight <-
    heuristic$maximum_weight # the max weight of the heuristic solution
  HeurLoad <- heuristic$Load # list of boxes loaded to each wagon

  # store the heuristic solution in a data frame
  Heur.df <-
    cbind(data.frame(c(
      rep(1, length(HeurLoad[[1]])), rep(2, length(HeurLoad[[2]])),
      rep(3, length(HeurLoad[[3]]))
    )),
    do.call(rbind, lapply(HeurLoad, data.frame)))
  names(Heur.df) = c("Wagon", "Box")

  # represent the heuristic solution in binary form
  loads_index$HeurSol <- rep(0, nrow(loads_index))
  for (i in 1:nrow(Heur.df)) {
    loads_index$HeurSol[which(loads_index$Wagon == Heur.df$Wagon[i] &
                                loads_index$Box == Heur.df$Box[i])] <- 1
  }

  # we create the heuristic solution in this order since we first add variable 'maxweight',
  # and then the 'loads' variables
  Heuristic <- c(HeurMaxWeight, loads_index$HeurSol)


  # add the heuristic solution
  addmipsol(
    prob,
    solval = Heuristic,
    colind = c(maxweight, loads_index$loads),
    name = "Heuristic_solution"
  )

  # solve the MIP problem
  setoutput(prob)
  summary(xprs_optimize(prob))
  solution <- getsolution(prob)$x

  # optimum objective value
  objsolval <-  getdblattrib(prob, xpress:::MIPOBJVAL)

  # load solutions:
  # Note that the column indices are 0-based while the vector indices in R are 1-based,
  # so, to get the corresponding solutions, we add '1' to column indices
  loads_index$MIPSol <- solution[loads_index$loads + 1]
  wagon_loaded <-
    loads_index %>% filter(MIPSol == 1) %>% select(Wagon, Box, Weight)

  # the total weight each wagon loads:
  sum_weight <- c()
  for (i in Wagon) {
    sum_weight <-
      c(sum_weight, sum(wagon_loaded$Weight[which(wagon_loaded$Wagon == Wagon[i])]))
  }

  weight_loaded <- data.frame(wagon = Wagon, weight = sum_weight)

  results <-
    list(
      prob = prob,
      Heuristic_max_weight = HeurMaxWeight,
      Heuristic_solution = Heur.df,
      MIP_maximum_weight = objsolval,
      MIP_wagon_loaded = wagon_loaded,
      MIP_weight_loaded = weight_loaded
    )

  return(results)

}


#' 
#' 
#' Now we can solve this problem. We formulate the problem as a MIP problem,
#' add a heuristic solution to the MIP formulation and then solve it.
#' 
## ----Solve The Problem--------------------------------------------------------
MIP_solution <- solve_MIP(weight.df, Wagon)
print(MIP_solution)
if (MIP_solution$Heuristic_max_weight <= WMAX) {
  print(
    paste(
      "Heuristic solution is feasible with weight ",
      MIP_solution$Heuristic_max_weight
    )
  )
} else {
  print(
    paste(
      "Heuristic solution weight exceeds maximum allowed weight of ",
      WMAX,
      " by ",
      MIP_solution$Heuristic_max_weight - WMAX
    )
  )
}


#' 
#' 
