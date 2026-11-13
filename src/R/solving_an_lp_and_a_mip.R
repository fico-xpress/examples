#####################################
# This file is part of the          #
# Xpress-R interface examples       #
#                                   #
#   (c) 2022-2026 Fair Isaac Corporation #
#####################################
#' ---
#' title: "Solving an LP and a MIP Model"
#' author: Gregor Hendel
#' date: Dec. 2020
#' ---
#' 

#' 
#' This example shows how to solve LP and MIP models.
#' 
#' If you haven't done so already, please familiarize yourself with the Facility
#' Location Example example, which we use throughout our quick examples.
#' 
#' # Solving an LP
#' 
#' We solve a very small LP and retrieve both the solution and the primal slacks of
#' the solution.
#' 
## ----A simple 2 by 2 LP problem-----------------------------------------------
suppressMessages(library(xpress))

# max        x2
# s.t.  x1 + x2 >= 11
#       x1 + x2 <= 13
#     - x1 + x2 >= 2
#     - x1 + x2 <= 4
#         x1,x2 >= 0

# create a fresh problem data
problemdata <- list()

# objective
problemdata$objcoef <- c(0,1)

# row coefficients as a dense matrix object
problemdata$A <- matrix(c(1,1,
                          1,1,
                         -1,1,
                         -1,1), ncol = 2, byrow = T)
# right hand side
problemdata$rhs <- c(11,13, 2,4)

# constraint sense
problemdata$rowtype <- c("G", "L", "G", "L")

# lower bounds on the variables
problemdata$lb <- c(0,0)

# upper bounds on the variables
problemdata$ub <- c(Inf, Inf)

# problem name
problemdata$probname <- "2x2LP"

# load everything into a (newly created) XPRSprob object 'p'
p <- xprs_loadproblemdata(problemdata=problemdata)

# change to maximization
chgobjsense(p, -1L) # -1 stands for maximization, + 1 stands for minimization
print(p)

#' 
#' 
#' Let's solve this example LP and query the solution
## ----Solve This LP------------------------------------------------------------

# enable output to stdout
setoutput(p)

# summarize the optimization
summary(xprs_optimize(p))

# print the LP status and the LP Objective value. We access them by passing
# the attribute integer indices that are part of the Xpress package, but
# not exposed upon loading the library to avoid polluting the global namespace
print(getintattrib(p, xpress:::LPSTATUS))
print(getdblattrib(p, xpress:::LPOBJVAL))

# print the solution
print(getsolution(p)$x)

#' # Solving a MIP
#' 
#' We read the Facility location problem from the introductory example. This frees
#' the LP from before.
#' 
## ----Read Facility Location Problem-------------------------------------------
readprob(p, "flp.lp")

#' 
#' By printing the MIP entities attribute we can see that this is now a
#' mixed-integer or, more precisely, a mixed-binary program:
#' 
## ----Print MIP Entities-------------------------------------------------------
print(getintattrib(p, xpress:::MIPENTS))

#' 
#' We solve this problem using `xprs_optimize`.
#' 
#' The optional flags argument allows to specify
#' the LP root algorithm: 'b' for the barrier algorithm, 'd' for the dual simplex
#' algorithm, and 'p' for the primal simplex algorithm.
#' 
## -----------------------------------------------------------------------------
 # optionally: try mipoptimize(p, "d")
summary(xprs_optimize(p, "b"))

#' 
#' To retrieve the MIP solution, we call
## ----Get the solution---------------------------------------------------------
print(getsolution(p)$x)

#' which returns a named list with solution values 'x' and solution status 'status'.
