#####################################
# This file is part of the          #
# Xpress-R interface examples       #
#                                   #
#   (c) 2022-2026 Fair Isaac Corporation #
#####################################
#' 
#' ---
#' title: "Price Breaks 2"
#' author: Y. Gu
#' date: Jul.2021
#' ---
#' 
## ----setup, include=FALSE-----------------------------------------------------
knitr::opts_chunk$set(echo = TRUE)
knitr::opts_chunk$set(results = "hold")
knitr::opts_chunk$set(warning = FALSE, message = FALSE)

#' 
#' 
#' ## Brief Introduction To The Problem
#' 
#' [This example is from the book 'Applications of optimization with Xpress'](https://www.fico.com/fico-xpress-optimization/docs/latest/examples/mosel/ApplBook/Intro/pricebrinc2.mos).
#' 
#' This example simulates the situation where we are buying a required number of items
#' and we get discounts incrementally, and we want to minimize the total cost.
#' 
#' This is a problem suitable to be modeled with SOS2 since we can express the cost as a
#' piecewise linear function of the number of items bought. For SOS2 formulation,
#' please refer to the file 'approx.R' and page 44 of the book, in this file we will
#' model the incremental price breaks using binary variables.
#' 
#' There are 4 break points: $0$, $B_1$, $B_2$ and $B_3$, and 3 line segments, i.e. 3
#' different costs. We define binary variables $b_1$, $b_2$ and $b_3$, where $b_i$ is 1
#' if we have bought any item at a unit cost of $cost_i$. We also define decision
#' variables $x_1$, $x_2$ and $x_3$ for the number of items bought at each cost.
#' 
#' One important constraint here is that we cannot buy any item at the lower price without
#' having bought the maximum number of items at the higher price, because otherwise the
#' solution will tell us to buy all items at the lowest unit price. Besides, we need to
#' set bounds for $x_1$, $x_2$ and $x_3$ to model the incremental discounts. For mathematical
#' formulation of the constraints for these bounds, please refer to page 45 of the book
#' 'Applications of optimization with Xpress'.
#' 
#' 
#' For this example, we need the package 'xpress'.
#' 
## ----Load The Package---------------------------------------------------------
library(xpress)

#' 
#' 
#' Create a new empty problem and give the problem a suitable name.
#' 
## ----Create The Problem-------------------------------------------------------
# create a new problem
prob <- createprob()

# set the problem name
setprobname(prob, "PriceBrinc2")

#' 
#' Add the values we need for this example.
#' 
## ----Data---------------------------------------------------------------------
# demand
DEM <- 150

# break points of cost function
B <- c(0, 50, 120, 200)

# three different unit costs
cost.df <- data.frame(Break = 1:3,
                      Cost = c(0.8, 0.5, 0.3))


#' 
#' 
#' Create variables $x_i$ and binary variables $b_i$ as mentioned in the introduction.
#' 
## ----Add Columns--------------------------------------------------------------
# 1. create a vector 'x' in 'cost.df' to store the indices of x: the number of
#    items bought at price Cost1, Cost2 and Cost3
cost.df$x <-
  apply(cost.df, 1, function(x)
    xprs_newcol(
      prob,
      lb = 0,
      ub = Inf,
      coltype = "C",
      name = paste0("x_", x["Break"]),
      objcoef = x["Cost"]
    ))

# 2. create a vector 'b' in 'cost.df' to store the indices of binary variables b1,
#    b2 and b3, where bi is 1 if we have bought any items at a unit cost of Cost_i.
cost.df$b <-
  apply(cost.df, 1, function(x)
    xprs_newcol(
      prob,
      lb = 0,
      ub = 1,
      coltype = "B",
      name = paste0("b_", x["Break"]),
      objcoef = NULL
    ))


#' 
#' 
#' Add the four sets of constraints.
#' 
## ----Add Rows, results='hide'-------------------------------------------------
# 1. satisfy the demand
xprs_addrow(
  prob,
  colind = cost.df$x,
  rowcoef = rep(1, nrow(cost.df)),
  rowtype = "E",
  rhs = DEM ,
  name = "Meet the demand"
)

# 2. lower bounds on quantities
for (i in 1:(length(cost.df$Break) - 1)) {
  xprs_addrow(
    prob,
    colind = c(cost.df$x[i], cost.df$b[i + 1]),
    rowcoef = c(-1, (B[i + 1] - B[i])),
    rowtype = "L",
    rhs = 0,
    name = paste0("lb_", i)
  )
}

# 3. upper bounds on quantities
apply(cost.df, 1, function(x)
  xprs_addrow(
    prob,
    colind = c(x["x"], x["b"]),
    rowcoef = c(1, -(B[i + 1] - B[i])),
    rowtype = "L",
    rhs = 0,
    name = paste0("ub_", x["Break"])
  ))

# 4. sequence of price intervals, b1 >= b2 >= b3
for (i in 1:(length(cost.df$b) - 1)) {
  xprs_addrow(
    prob,
    colind = cost.df$b[i:(i + 1)],
    rowcoef = c(1, -1),
    rowtype = "G",
    rhs = 0,
    name = paste0("sequence_", i)
  )
}

#' 
#' 
#' Now we can solve the problem.
#' 
## ----Solve The Problem--------------------------------------------------------
setoutput(prob)
summary(xprs_optimize(prob))

#' 
#' 
#' Display the solutions here.
#' 
## ----The Solutions------------------------------------------------------------
solution <- getsolution(prob)$x

print(paste0(
  "The minimum total cost is: ",
  getdblattrib(prob, xpress:::MIPOBJVAL)
))
invisible(apply(cost.df, 1, function(x)
  cat("The number of items bought at unit cost", x["Cost"], "is:", solution[x["x"] + 1], "\n")))

#' 
#' 
