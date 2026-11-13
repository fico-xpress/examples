#####################################
# This file is part of the          #
# Xpress-R interface examples       #
#                                   #
#   (c) 2022-2026 Fair Isaac Corporation #
#####################################
#' ---
#' title: "Production Of Drinking Glasses"
#' author: Y.Gu
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
#' This is a conversion of the Mosel example 'Production Of Drinking Glasses', <https://www.fico.com/fico-xpress-optimization/docs/latest/examples/mosel/ApplBook/C_ProdPlan/c2glass.mos>.
#' Brief introduction to this problem is given below, and to see the full
#' mathematical modeling of this problem you may refer to section 8.2, page 106 of the
#' book 'Applications of optimization with Xpress'.
#' 
#' A company produces 6 types of glasses and wishes to plan its production for the next
#' 12 weeks. For each glass type, the following information is provided: demands in each
#' week, initial and final stock levels, production and storage costs, required
#' working time for workers and machines and the required storage space. Besides, the
#' capacities of workers, machines and storage areas are also given.
#' 
#' We want to decide the quantities of each product to produce in each week to minimize
#' the objective: total cost of production and storage. Two sets of variables are defined:
#' produce(i,j) to represent the production of glass type i in time period j and store(i,j)
#' to represent the stock level of product i at the end of period j.
#' 
#' Three types of constraints should be satisfied, the first one is that at week 12 the
#' stock level of each product should not be less than the required final
#' stock level, and this constraint can actually be specified as the lower bounds for
#' variables store(i,j) with j=12.
#' 
#' The second set of constraints is about capacities. We need to guarantee that for
#' every week, the capacity limits on manpower, machine time and storage space are kept.
#' 
#' The last constraint is the stock balance constraint. It states that the quantity
#' store(i,j) of product i that is held in stock at the end of a time period j equals the
#' stock level store(i,j-1) at the end of the preceding period plus the production
#' produce(i,j) of the time period t minus the demand of this time period. For week 1,
#' we use initial stock of product i to represent store(i,0).
#' 
#' The mathematical formulations of these constraints are included in the guide book
#' 'Applications of optimization with Xpress'.
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
#' Create a new empty problem and give the problem a suitable name.
#' 
## ----Create The Problem-------------------------------------------------------
# firstly, create a new empty problem
prob <- createprob()

# set the problem name
setprobname(prob, "ProductionOfDrinkingGlasses")


#' 
#' 
#' Add the values we need for this example.
#' 
## ----Data---------------------------------------------------------------------
# information about production
Produce.df <- as.data.frame(
  matrix(
    c(#CPROD #CSTOCK #TIMEW #TIMEM #SPACE #ISTOCK #FSTOCK
      100,   25,     3,     2,     4,     50,     10,
      80,    28,     3,     1,     5,     20,     10,
      110,   25,     3,     4,     5,     0,      10,
      90,    27,     2,     8,     6,     15,     10,
      200,   10,     4,     11,    4,     0,      10,
      140,   20,     4,     9,     9,     10,     10
    ), byrow = T, ncol = 7
  ),
) %>% rename(
  CPROD = V1,
  CSTOCK = V2,
  TIMEW = V3,
  TIMEM = V4,
  SPACE = V5,
  ISTOCK = V6,
  FSTOCK = V7
)

Capacity <- data.frame(CAPW = 390, # capacities of workers
                       CAPM = 850, # capacities of machines
                       CAPS = 1000 # capacities of the storage area
                       )

# demand for each product in each week
Demand.df <- data.frame(PROD=rep(1:6,each=12),WEEKS=rep(1:12,6),
                          DEM=c(20, 22, 18, 35, 17, 19, 23, 20, 29, 30, 28, 32,
                                17, 19, 23, 20, 11, 10, 12, 34, 21, 23, 30, 12,
                                18, 35, 17, 10,  9, 21, 23, 15, 10,  0, 13, 17,
                                31, 45, 24, 38, 41, 20, 19, 37, 28, 12, 30, 37,
                                23, 20, 23, 15, 10, 22, 18, 30, 28,  7, 15, 10,
                                22, 18, 20, 19, 18, 35,  0, 28, 12, 30, 21, 23))

weeks <- unique(Demand.df$WEEKS)
products <- unique(Demand.df$PROD)


#' 
#' 
#' Add variables 'produce' and 'store', and change the lower bounds of some 'store' variables
#' to satisfy one requirement of this example.
#' 
## ----Add Columns--------------------------------------------------------------
# add decision variables:
# 1. produce(i,j), where i in products and j in weeks: production of glass type i in week j
#    create a vector 'produce' in 'Demand.df' to store their indices
Demand.df$produce <- Demand.df %>%
  apply(1, function(x)
    xprs_newcol(
      prob,
      lb = 0,
      ub = Inf,
      coltype = "C",
      name = pretty_name("produce_", x[c('PROD', 'WEEKS')]),
      objcoef = Produce.df$CPROD[x["PROD"]]
    ))


# 2. store(i,j), where i in products and j in weeks: stock level of every product i at the end of week j
#    create a vector 'store' in 'Demand.df' to store their indices
Demand.df$store <- Demand.df %>%
  apply(1, function(x)
    xprs_newcol(
      prob,
      lb = 0,
      ub = Inf,
      coltype = "C",
      name = pretty_name("store_", x[c('PROD', 'WEEKS')]),
      objcoef = Produce.df$CSTOCK[x["PROD"]]
    ))

# change the lower bounds of stock levels of each product at week12:
s.week12 <- Demand.df %>% filter(WEEKS == 12)
chgbounds(
  prob,
  colind = s.week12$store,
  bndtype = rep('L', 6),
  bndval = Produce.df$FSTOCK
)


#' 
#' 
#' Add capacity constraints and the constraints that guarantee stock balances.
#' 
## ----Add Rows, results='hide'-------------------------------------------------
# 1. capacity constraints
# 1.1 workers capacity
Demand.df %>% group_by(WEEKS) %>%
  group_map(
    ~ xprs_newrow(
      prob,
      colind = .x$produce,
      rowcoef = Produce.df$TIMEW,
      rowtype = 'L',
      rhs = Capacity$CAPW,
      name = pretty_name("WorkerCapacity", .y)
    )
  )

# 1.2 machine capacity
Demand.df %>% group_by(WEEKS) %>%
  group_map(
    ~ xprs_newrow(
      prob,
      colind = .x$produce,
      rowcoef = Produce.df$TIMEM,
      rowtype = 'L',
      rhs = Capacity$CAPM,
      name = pretty_name("MachineCapacity", .y)
    )
  )

# 1.3 space capacity
Demand.df %>% group_by(WEEKS) %>%
  group_map(
    ~ xprs_newrow(
      prob,
      colind = .x$store,
      rowcoef = Produce.df$SPACE,
      rowtype = 'L',
      rhs = Capacity$CAPS,
      name = pretty_name("SpaceCapacity", .y)
    )
  )



# 2. stock balances
# store1 represents the index of previous week's store variable, and for week1 we set it as 0
Demand.df$store1 <- lag(Demand.df$store)
WEEK_lst <-
  Demand.df %>%  mutate_if(is.double, as.integer) %>% group_by(WEEKS) %>% group_map( ~ .x)

# week 1
apply(WEEK_lst[[1]], 1, function(x)
  xprs_newrow(
    prob,
    colind = c(x["produce"], x["store"]),
    rowcoef = c(-1, 1),
    rowtype = "E",
    rhs = Produce.df$ISTOCK[x["PROD"]] - x["DEM"],
    name = sprintf("Stock_balance_%d_%d", 1, x["PROD"])
  ))

# weeks 2-12
WEEK_lst <- WEEK_lst[-1]
for (i in weeks[-last(weeks)]) {
  apply(WEEK_lst[[i]], 1, function(x)
    xprs_newrow(
      prob,
      colind = c(x["produce"], x["store1"], x["store"]),
      rowcoef = c(1, 1, -1),
      rowtype = "E",
      rhs = x["DEM"],
      name = sprintf("Stock_balance_%d_%d", (i + 1), x["PROD"])
    ))
}


#' 
#' 
#' Now we can solve the problem.
#' 
## ----Solve The Problem--------------------------------------------------------
print(prob)
setoutput(prob)
summary(xprs_optimize(prob))


#' 
#' 
#' 
#' Display the solutions here.
#' 
## ----The Solutions------------------------------------------------------------
# 1. optimum value
print(paste("The optimum cost is:", getdblattrib(prob, xpress:::LPOBJVAL)))

# 2. produce and store solutions:
Demand.df$Producesol <- getsolution(prob)$x[Demand.df$produce + 1]
Demand.df$Storesol <- getsolution(prob)$x[Demand.df$store + 1]

produce.solution <- matrix(Demand.df$Producesol, 6, 12, byrow = TRUE)
store.solution <- matrix(Demand.df$Storesol, 6, 12, byrow = TRUE)

print("Quantities to produce of every glass type:")
produce.solution
print("Quantities to store of every glass type:")
store.solution

# 3. capacity solutions
workerCAP <- t(Produce.df$TIMEW %*% produce.solution)
machineCAP <- t(Produce.df$TIMEM %*% produce.solution)
spaceCAP <- t(Produce.df$SPACE %*% store.solution)

capacity.solution <- cbind(workerCAP, machineCAP, spaceCAP)
colnames(capacity.solution) <- c("HumanPower", "Machine", "Storage")
print("Capacities used:")
capacity.solution

#' 
#' 
