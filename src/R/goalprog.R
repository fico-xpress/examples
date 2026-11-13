#####################################
# This file is part of the          #
# Xpress-R interface examples       #
#                                   #
#   (c) 2022-2026 Fair Isaac Corporation #
#####################################
#' ---
#' title: "Goal programming"
#' author: "Chris Brown"
#' date: "25/05/2022"
#' output: html_document
#' ---
#' 
## ----setup, include=FALSE-----------------------------------------------------
knitr::opts_chunk$set(echo = TRUE)
library(xpress)

#' 
#' An example of lexicographic goal programming using the Xpress
#' multi-objective API.
#' 
#' A company produces two electrical products, A and B. Both require
#' two stages of production: wiring and assembly. The production plan
#' must meet several goals:
#' 
#'   1. A profit of $200
#'   2. A contractual requirement of 40 units of product B
#'   3. To fully utilize the available wiring department hours
#'   4. To avoid overtime in the assembly department
#' 
#' Set up the problem and define its dimensions
## ----Setup--------------------------------------------------------------------
prob <- createprob()
setoutput(prob)

COLS     = 8    # Number of columns
ROWS     = 4    # Number of rows
ENTITIES = 2    # Number of entities

#' 
#' Define the columns
## ----Columns------------------------------------------------------------------
col_produce_a       = xprs_newcol(prob, lb=0, ub=Inf, coltype="I")
col_produce_b       = xprs_newcol(prob, lb=0, ub=Inf, coltype="I")
col_surplus_wiring  = xprs_newcol(prob, lb=0, ub=Inf, coltype="C")
col_deficit_wiring  = xprs_newcol(prob, lb=0, ub=Inf, coltype="C")
col_surplus_assem   = xprs_newcol(prob, lb=0, ub=Inf, coltype="C")
col_deficit_assem   = xprs_newcol(prob, lb=0, ub=Inf, coltype="C")
col_deficit_profit  = xprs_newcol(prob, lb=0, ub=Inf, coltype="C")
col_deficit_prod_b  = xprs_newcol(prob, lb=0, ub=Inf, coltype="C")

#' 
#' Add one row per goal
## ----Rows---------------------------------------------------------------------
# Goal 1: profit must meet or exceed $200
xprs_addrow(
  prob,
  colind = c(col_produce_a, col_produce_b, col_deficit_profit),
  rowcoef = c(7, 6, 1),
  rowtype = "G",
  rhs = 200
)
# Goal 2: production of product B must meet or exceed 40 units
xprs_addrow(
  prob,
  colind = c(col_produce_b, col_deficit_prod_b),
  rowcoef = c(1, 1),
  rowtype = 'G',
  rhs = 40
)
# Goal 3: hours of wiring should be close to the available 120 hours
xprs_addrow(
  prob,
  colind = c(col_produce_a, col_produce_b, col_surplus_wiring, col_deficit_wiring),
  rowcoef = c(2, 3, -1, 1),
  rowtype = 'E',
  rhs = 120
)
# Goal 4: hours of assembly should be close to the available 300 hours
xprs_addrow(
  prob,
  colind = c(col_produce_a, col_produce_b, col_surplus_assem, col_deficit_assem),
  rowcoef = c(6, 5, -1, 1),
  rowtype = 'E',
  rhs = 300
)

#' 
#' Define objectives to minimize deviations, in priority order
## ----Objectives---------------------------------------------------------------
# Goal 1: minimize profit deficit
chgobj(
  prob,
  colind = col_deficit_profit,
  objcoef = 1
)
# Goal 2: minimize production deficit
chgobjn(
  prob,
  objidx = 1,
  colind = c(col_deficit_prod_b),
  objcoef = c(1)
)
# Goal 3: minimize deviation from wiring hours target
chgobjn(
  prob,
  objidx = 2,
  colind = c(col_surplus_wiring, col_deficit_wiring),
  objcoef = c(1, 1)
)
# Goal 4: minimize deviation from assembly hours target
chgobjn(
  prob,
  objidx = 3,
  colind = c(col_surplus_assem, col_deficit_assem),
  objcoef = c(1, 1)
)

#' 
#' Set up objective priorities and tolerances
## ----Objective configuration--------------------------------------------------
for (i in 0:3) {
  setobjintcontrol(prob, i, xpress:::OBJECTIVE_PRIORITY, 4 - i)
  setobjdblcontrol(prob, i, xpress:::OBJECTIVE_ABSTOL, 0)
  setobjdblcontrol(prob, i, xpress:::OBJECTIVE_RELTOL, 0)
}

#' 
#' Solve the problem
## ----Solve--------------------------------------------------------------------
summary(xprs_optimize(prob))

#' 
#' Print the result
## ----Results------------------------------------------------------------------
if (getintattrib(prob, xpress:::SOLVESTATUS) == xpress:::SOLVESTATUS_COMPLETED &&
    getintattrib(prob, xpress:::SOLSTATUS) == xpress:::SOLSTATUS_OPTIMAL) {
    sol = getmipsol(prob)$x

    cat("Production plan:\n")
    cat("Product A: ", sol[col_produce_a], " units\n", sep="")
    cat("Product B: ", sol[col_produce_b], " units\n", sep="")
    cat("Profit: $", 7 * sol[col_produce_a] + 6 * sol[col_produce_b], "\n", sep="")
    if (sol[col_deficit_profit] > 0) {
        cat("Profit goal missed by $", sol[col_deficit_profit], "\n", sep="")
    }
    if (sol[col_deficit_prod_b] > 0) {
        cat("Contractual goal for product B missed by ", sol[col_deficit_prod_b], " units\n", sep="")
    }
    if (sol[col_surplus_wiring] > 0) {
        cat("Unused wiring department hours: ", sol[col_surplus_wiring], "\n", sep="")
    }
    if (sol[col_deficit_wiring] > 0) {
        cat("Wiring department overtime: ", sol[col_deficit_wiring], "\n", sep="")
    }
    if (sol[col_surplus_assem] > 0) {
        cat("Unused assembly department hours: ", sol[col_surplus_assem], "\n", sep="")
    }
    if (sol[col_deficit_assem] > 0) {
        cat("Assembly department overtime: ", sol[col_deficit_assem], "\n", sep="")
    }
} else {
    cat("Problem could not be solved\n")
}

