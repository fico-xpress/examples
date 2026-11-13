#####################################
# This file is part of the          #
# Xpress-R interface examples       #
#                                   #
#   (c) 2022-2026 Fair Isaac Corporation #
#####################################
#' ---
#' title: "Tuner Example"
#' author: "Gregor Hendel"
#' date: "12/10/2021"
#' output: html_document
#' ---
#' 
## ----setup, include=FALSE-----------------------------------------------------
knitr::opts_chunk$set(echo = TRUE)
library(xpress)

#' 
#' 
#' Create a file default-MIP.xtm with the tuner options that are tested by default by
#' the tuner. The file has the following format: a
#' section with controls that are applied to each tuner run
#' (`FIXED-CONTROLS`) and a set of controls that are tested
#' independently by the tuner (`TUNABLE-CONTROLS`). Tunable controls
#' have each one or more value that the tuner will test, first
#' independently and then in combination with others.
#' 
#' ```
#' FIXED-CONTROLS
#'  OUTPUTLOG            = 1
#'  XSLP_POSTSOLVE       = 1
#' TUNABLE-CONTROLS
#'  BRANCHDISJ           = 0
#'  CUTMAXROUNDS         = 0, 2, 20
#'  CUTFACTOR            = 0.5, 1, 5
#'  CUTFREQ              = 2
#'  ...
#' ```
#' and others. The file is written AFTER reading in the problem in order for the
#' optimizer to recognize that it is a MIP.
#' 
## ----Setup Tuner--------------------------------------------------------------
prob <- createprob()
setoutput(prob)
readprob(prob, "Data/miplib-rndinst.mps.gz")

tunerwritemethod(prob, "default-MIP.xtm")

setintcontrol(prob, xpress:::TUNERMAXTIME, 100)
setintcontrol(prob, xpress:::MAXTIME, -10)

#' 
#' Tune the optimizer on the problem
## -----------------------------------------------------------------------------
tune(prob)

#' 
#' Now try tuning on a different tuner method: the file mymethod.xtm contains slightly
#' different combination of controls and values, and the tuner will test different
#' combinations this way. Note that the output will show that the Optimizer finds old
#' tuner runs and will use the relative information.
#' 
## -----------------------------------------------------------------------------
tunerreadmethod(prob, 'Data/mymethod.xtm')
tune(prob)

#' 
#' For the final problem solve, we increase the time limit
## -----------------------------------------------------------------------------
setintcontrol(prob, xpress:::MAXTIME, -1000)
summary(xprs_optimize(prob))

#' 
#' 
