#####################################
# This file is part of the          #
# Xpress-R interface examples       #
#                                   #
#   (c) 2022-2026 Fair Isaac Corporation #
#####################################
#' ---
#' title: "Chess2"
#' author: Y. Gu
#' date: Jul.2021
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
#' This is an introductory example in the book 'Applications of optimization with Xpress'. <https://www.fico.com/fico-xpress-optimization/docs/latest/examples/mosel/ApplBook/Intro/chess2.mos>
#' 
#' This is a very small example, where the manufacture only produces two types of chess
#' sets: a small set and large set. The time and materials per set required are known and
#' within the limited lathe-hours and boxwood, we would like to maximize the profit by
#' deciding how many small sets and large sets should be produced per week.
#' 
#' The purpose of this example is to give readers experience in practical modeling and
#' have a general sense of what modeling is. Instead of theoretical overview, a practical
#' example is easier to get started with. In Chapter 1 of the book, full mathematical
#' formulations of this example are provided, as well as further discussion of the results
#' and explanations of the benefits of modeling.
#' 
#' 
#' For this example, we need the package 'xpress'.
#' 
## ----Load The Package---------------------------------------------------------
library(xpress)


#' 
#' 
#' Create a new empty problem, set the objective sense as maximization and give the problem
#' a suitable name.
#' 
## ----Create The Problem-------------------------------------------------------
# create a new problem
prob = createprob()

# change this problem to a maximization problem
chgobjsense(prob, objsense = xpress:::OBJ_MAXIMIZE)

# set the problem name
setprobname(prob, "Chess2")


#' 
#' 
#' Add the values we need for this example.
#' 
## ----Data---------------------------------------------------------------------
info.df <-
  data.frame(
    # time required on a lathe to make small and large sets
    lathe = c(3, 2),
    # amount of boxwood required for small and large sets
    boxwood = c(1, 3),
    # profit of small and large sets
    profit = c(5, 20)
  )
rownames(info.df) <- c("small", "large")

# capacity of lathe-hours per week
Lathe <- 160

# capacity of boxwood per week
Boxwood <- 200


#' 
#' 
#' Create integer variables 'produce' for each product to represent the number of sets
#' to make.
#' 
## ----Add Columns--------------------------------------------------------------
# create a vector 'produce' in 'info.df' to store the column indices.
info.df$produce <-
  mapply(
    function(profit, name)
      xprs_newcol(
        prob,
        lb = 0,
        ub = Inf,
        coltype = "I",
        objcoef = profit,
        name = paste0("produce_", name)
      ),
    info.df$profit,
    rownames(info.df)
  )


#' 
#' 
#' Add the capacity constraints of this example.
#' 
## ----Add Rows, results='hide'-------------------------------------------------
# cannot plan to use more of the lathe-hours than we have available
xprs_addrow(
  prob,
  colind = info.df$produce,
  rowcoef = info.df$lathe,
  rowtype = "L",
  rhs = Lathe,
  name = "Lathe"
)

# cannot plan to use more of the boxwood than we have available
xprs_addrow(
  prob,
  colind = info.df$produce,
  rowcoef = info.df$boxwood,
  rowtype = "L",
  rhs = Boxwood,
  name = "Boxwood"
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
#' Display the solutions here.
#' 
## ----The Solutions------------------------------------------------------------
info.df$solution <- getsolution(prob)$x

cat("The optimum profit is:",
    getdblattrib(prob, xpress:::MIPOBJVAL),
    "\n")
for (i in 1:nrow(info.df)) {
  cat("Produce", info.df$solution[i], rownames(info.df[i, ]), "sets\n")
}


#' 
#' 
