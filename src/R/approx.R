#####################################
# This file is part of the          #
# Xpress-R interface examples       #
#                                   #
#   (c) 2022-2026 Fair Isaac Corporation #
#####################################
#' ---
#' title: "Approx"
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
#' [This is an introductory example about Special Ordered Sets of type 2 (SOS2)](https://www.fico.com/fico-xpress-optimization/docs/latest/examples/mosel/ApplBook/Intro/approx.mos).
#' SOS2 can be used for modeling piecewise approximations of functions of
#' a single variable.
#' 
#' In this example, we want to express $f$ as a function of $x$ and minimize $f$.
#' There are 4 line segments and 5 break points ($R_i$,$F_i$), and 5 weight variables
#' $y_i$ associated with each break point. We express $x$ as the sum of $R_i*y_i$ and
#' $f$ as the sum of $F_i*y_i$, for i from 1 to 5. SOS2 requires that at most two adjacent
#' $y_i$ can be nonzero, and this means that we are always on the piecewise linear function.
#' The convexity constraint requires that the sum of all $y_i$ should be 1. To add an SOS2
#' restriction, we can use the function `addsets`.
#' 
#' An alternative way to formulate SOS2 restriction is to use binary variables. We set
#' 4 binary variables $b_i$ associated with each of the 4 intervals, where $b_i$ is 1 if
#' the value of x lies between $R_i$ and $R_(i+1)$. So, one necessary constraint is that
#' exactly one of the $b_i$ can be 1, i.e., the sum of $b_i$ should be 1. For a full
#' mathematical formulation of SOS2 using binary variables and graphical illustrations,
#' please refer to section 3.4.5 of the book 'Applications of optimization with Xpress'.
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
setprobname(prob, "Approx")


#' 
#' 
#' Add the values we need for this example.
#' 
## ----Data---------------------------------------------------------------------
# coordinates of break points
breaks.df <- data.frame(
  BREAK = 1:5,
  R = c(1, 2.2, 3.4, 4.8, 6.5),
  F = c(2, 3.2, 1.4, 2.5, 0.8)
)


#' 
#' 
#' Create weight variables, variable $x$ and variable $f$ as mentioned in the introduction.
#' 
## ----Add Columns--------------------------------------------------------------
# create a vector 'y' in 'breaks.df' to store the column indices of weight variables
breaks.df$y <-
  apply(breaks.df, 1, function(x)
    xprs_newcol(
      prob,
      lb = 0,
      ub = Inf,
      coltype = "C",
      name = paste0("y_", x["BREAK"]),
      objcoef = NULL
    ))

var.x <-
  xprs_newcol(
    prob,
    lb = breaks.df$R[1],
    ub = breaks.df$R[nrow(breaks.df)],
    coltype = "C",
    name = "x",
    objcoef = NULL
  )

var.f <-
  xprs_newcol(
    prob,
    lb = 0,
    ub = Inf,
    coltype = "C",
    name = "f",
    objcoef = 1
  )


#' 
#' 
#' Three constraints are required, the first one is that all $y_i$ should sum up to 1.
#' The other two respectively represent the relationship between $x$ and $y_i$, and $f$
#' and $y_i$.
#' 
## ----Add Rows, results='hide'-------------------------------------------------
# 1. convexity row
xprs_addrow(
  prob,
  colind = breaks.df$y,
  rowcoef = rep(1, nrow(breaks.df)),
  rowtype = "E",
  rhs = 1,
  name =  "convexity")

# 2. reference row
xprs_addrow(
  prob,
  colind = c(var.x, breaks.df$y),
  rowcoef = c(1, -breaks.df$R),
  rowtype = "E",
  rhs = 0,
  name = "x-row"
)

# 3. relationship between f and y
xprs_addrow(
  prob,
  colind = c(var.f, breaks.df$y),
  rowcoef = c(1, -breaks.df$F),
  rowtype = "E",
  rhs = 0,
  name = "f-row"
)


#' 
#' 
#' We can add the SOS2 restriction by function `addsets`, or the SOS2 restriction can be
#' formulated using binary variables.
#' 
## ----Add SOS2 Restriction, results='hide'-------------------------------------
# add SOS2 restriction using function `addset`
addsets(
  prob,
  settype = '2',
  start = 0,
  colind = breaks.df$y,
  refval = breaks.df$R
)


#' 
#' 
#' Now we can solve the problem.
#' 
## ----Solve The Problem--------------------------------------------------------
setoutput(prob)
summary(xprs_optimize(prob))


#' 
#' 
#' Display the solutions here. Although all variables are continuous, the presence of a
#' set (SOS2) constraint requires the branch-and-bound or "MIP" algorithm of the optimizer.
#' Accordingly, we have to query the MIP objective value.
#' 
## ----The Solutions------------------------------------------------------------
solution <- getsolution(prob)$x

# Note that the index vectors are 0-based, so we need to add 1 when requiring the
# solution value of x
print(paste0(
  "The optimum value is: ",
  getdblattrib(prob, xpress:::MIPOBJVAL),
  ", and the value of x is : ",
  solution[var.x + 1]
))


#' 
#' 
