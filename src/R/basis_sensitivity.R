#####################################
# This file is part of the          #
# Xpress-R interface examples       #
#                                   #
#   (c) 2022-2026 Fair Isaac Corporation #
#####################################
#' ---
#' title: "Basis And Stability"
#' author: Gregor Hendel
#' date: Dec 2021
#' ---
#' 
## ----setup, include=FALSE-----------------------------------------------------
knitr::opts_chunk$set(echo = TRUE)
library(xpress)

#' 
#' In this example, we solve a simple 2x2 LP problem to optimality,
#' which serves as a showcase for basis handling methods and 
#' sensitivity analysis.
#' 
## ----Problem Creation---------------------------------------------------------
# name the problem and add variables, objective function, constraints
p <-  createprob()
setprobname(p, "firstexample")
x1 <- xprs_newcol(p, 0, Inf, 'C', "x_1", 1)
x2 <- xprs_newcol(p, 0, Inf, 'C', "x_2", 1)
row1 <- xprs_newrow(p, c(x1, x2), c(5,1), "G", 7, name="Row1")
row2 <- xprs_newrow(p, c(x1, x2), c(1,4), "G", 9, name="Row2")

#' 
## ----Load a Basis-------------------------------------------------------------
# load a basis by specifying row and column basis status
# See the documentation of loadbasis for an explanation
# of the status codes.
# Below, the optimal basis status will be printed
loadbasis(p, c(2,2), c(1,1))

#' 
## ----Solve the Problem and Print the Solution---------------------------------
# solve the problem and print the solution
summary(xprs_optimize(p))
sol <- getsolution(p)$x
varnames <- getnamelist(p, 2, 0, 1)
cat(paste("The solution value for", trimws(varnames), " is ", sol, collapse = "\n"))

# write the solution to a file
writeslxsol(p, "firstexample.slx")

#' 
## ----Print the Row and Column Basis Status------------------------------------
# These should be equal to the row and column basis status we loaded above
basis <- getbasis(p)
print(paste("Row basis status:", paste(basis$rowstat, collapse=", ")))
print(paste("Column basis status:", paste(basis$colstat, collapse=", ")))

# write basis to a file
# a basis can then be read from a file using the readbasis function
writebasis(p, "basis")

#' 
#' From now on, we use sensitivity analysis methods `objsa`, `bndsa`, and `rhssa`
#' which can be used to determine ranges of objective coefficients, variable
#' lower and upper bounds, and right-hand sides within which the current basis 
#' stays optimal.
#' 
## ----Sensitivity Analysis of the Objective------------------------------------
print("Sensitivity Analysis of the objective:")
obj_sensitivity <- objsa(p, c(x1,x2))
for (i in c(1,2)) {
    print(
      paste("The objective coefficient of", 
            trimws(varnames[i]), 
            "can be varied between", 
            obj_sensitivity$lower[i], "and", obj_sensitivity$upper[i]))
}

#' 
## ----Sensitivity Analysis of the Lower and Upper Bounds-----------------------
print("Sensitivity Analysis of the lower and upper bounds:")
bnd_sensititivity <- bndsa(p, c(x1,x2))
for (i in c(1,2)) {
    print(
      paste(
        "The lower bounds of",  
        trimws(varnames[i]), 
        "can be varied between",
        bnd_sensititivity$lblower[i], "and", bnd_sensititivity$lbupper[i]))
    print(
      paste(
        "The upper bounds of",  
        trimws(varnames[i]), 
        "can be varied between", 
        bnd_sensititivity$ublower[i], "and", bnd_sensititivity$ubupper[i]))
}

#' 
## ----Sensitivity Analysis of Right-Hand Sides---------------------------------
print("Sensitivity Analysis of right-hand sides:")
rhs_sensitivity <- rhssa(p, c(row1,row2))
consnames <- getnamelist(p, 1, 0, 1)
for (i in c(1,2)) {
    print(
      paste(
        "The right-hand side of constraint", 
        trimws(consnames[i]), 
        "can be varied between", 
        rhs_sensitivity$lower[i], "and", rhs_sensitivity$upper[i]))
}

