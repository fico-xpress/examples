#####################################
# This file is part of the          #
# Xpress-R interface examples       #
#                                   #
#   (c) 2022-2026 Fair Isaac Corporation #
#####################################
#' ---
#' title: "Infeasibility and IIS Detection"
#' author: Gregor Hendel
#' date: Dec. 2021
#' ---
#' 
## ----setup, include=FALSE-----------------------------------------------------
knitr::opts_chunk$set(echo = TRUE)
library(xpress)

#' 
#' In this example, we model an infeasible linear programming problem.
#' For analyzing the infeasibility, we use IIS detection of the Xpress Optimizer.
#' 
#' In most cases, an infeasible model points at a mistake in the modeling process. 
#' Learn more about how Xpress supports the analysis of infeasible models
#' and the tracing of infeasibility reasons
#' [here](https://www.fico.com/fico-xpress-optimization/docs/latest/solver/optimizer/HTML/chapter3_sec_section3001.html).
#' 
#' First, we need to create an LP that is infeasible.
#' 
## ----Problem Formulation------------------------------------------------------
  p <- createprob()
  x0 <- xprs_newcol(p, 0, Inf, "C")
  x1 <- xprs_newcol(p, 0, Inf, "C")
  
  # x0 + 2 x1 >= 1
  c1 <- xprs_newrow(p, c(x0, x1), c(1,2), "G", 1, name="First Row")
  
  # 2 x0 + x1 >= 1
  c2 <- xprs_newrow(p, c(x0, x1), c(2,1), "G", 1, name="Second Row")
  
  # x0 + x1 <= 0.5
  c3 <- xprs_newrow(p, c(x0, x1), c(1,1), "L", 0.5, name="Third Row")

#' 
#' The problem is found to be infeasible by calling `xprs_optimize`.
#' 
## ----Solve the Problem--------------------------------------------------------
  setoutput(p)
  xprs_optimize(p)
  
  # a status of 2 means LP infeasible
  print(paste("Problem solved to status", getintattrib(p, xpress:::LPSTATUS)))

#' 
#' We initiate the IIS detection using `iisall` and query the status using `iisstatus`. 
#' 
#' On larger problems where IIS detection
#' consumes time, we might rather search for a first IIS using `iisfirst`, and 
#' then iterate consecutively through other IISs using `issnext`.
#' 
## ----Initiate IIS Detection---------------------------------------------------
  iisall(p)
  status <- iisstatus(p)
  print(status)
  iis <- getiisdata(p, 1)
  print("IIS Data:")
  print(iis)

## ----Determine Isolations-----------------------------------------------------
  iisisolations(p, 1)

