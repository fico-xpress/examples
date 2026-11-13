#####################################
# This file is part of the          #
# Xpress-R interface examples       #
#                                   #
#   (c) 2022-2026 Fair Isaac Corporation #
#####################################
#' ---
#' title: "Quadratic Optimization Problems"
#' author: Gregor Hendel
#' date: Dec. 2020
#' ---
#' 
#' We formulate a quadratic optimization problem in Xpress.
#' 
## ----Solving Quadratic Optimization Problems----------------------------------
#
# Minimize
# [ x^2 ] / 2
#
# Subject To
# Row1: 4 x + y <= 4
# Row2: x + y + [ z^2 ] <= 5
# Row3: [ x^2 + 2 x * y + 3 * y^2 + 4 y * z + z^2 ] <= 10
# Row4: x + 2 y >= 8
# Row5: [ 3 y^2 ] <= 20
#
# Bounds
#
# End
library(xpress)

# create a problem data object, as usual
problemdata <- list()
problemdata$Qobj <- diag(c(1,0,0), ncol = 3) # quadratic objective terms
problemdata$objcoef <- rep(0,3)  # linear objective terms

# formulate a dense linear coefficient matrix
problemdata$A <- matrix(c(4,1,0,
                          1,1,0,
                          0,0,0, # only zeros in row3
                          1,2,0,
                          0,0,0), nrow = 5, byrow = T)
problemdata$rhs <- c(4,5,10,8,20) # right hand side
problemdata$rowtype <- c("L", "L", "L", "G", "L") # row sense

# the quadratic terms for the constraints, given as a list of rows.
problemdata$Qrowlist <- list()

# no problemdata$Qrowlist[[1]] since row 4 is linear
problemdata$Qrowlist[[2]] <- diag(c(0,0,1))
problemdata$Qrowlist[[3]] <- matrix(c(1,1,0,
                                      1,5,2,
                                      0,2,1), ncol=3, byrow = T)
# no problemdata$Qrowlist[[4]] since row 4 is linear
problemdata$Qrowlist[[5]] <- diag(c(0,3,0))
problemdata$colname <- c("x", "y", "z")
problemdata$rowname <- sprintf("Row%d", 1:nrow(problemdata$A))

# problem name
problemdata$probname <- "SmallQCQP"

problemdata$lb <- rep(0, 3) # lower bounds
problemdata$ub <- rep(Inf, 3) # upper bounds

# column types. Continuous 'C' is also the default, if not present
problemdata$columntypes <- rep("C", 3)

# create a problem and make it print to stdout
p <- createprob()
setoutput(p)

# load the specified QCQP into Xpress
p <- xprs_loadproblemdata(p, problemdata)

# save it as an LP file
writeprob(p, "smallqp.lp", "l")

#' 
## ----Verify that our QCQP Looks as Expected-----------------------------------
cat(readLines("smallqp.lp"), sep = "\n")

#' 
## ----Solve the QCQP with the Optimizer----------------------------------------
print(p)
summary(xprs_optimize(p))

#' 
