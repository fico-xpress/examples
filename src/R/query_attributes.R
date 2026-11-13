#####################################
# This file is part of the          #
# Xpress-R interface examples       #
#                                   #
#   (c) 2022-2026 Fair Isaac Corporation #
#####################################
#' ---
#' title: "Query Attributes"
#' author: Gregor Hendel
#' date: Dec. 2020
#' ---
#' 

#' 
#' This example shows how to query attributes of the FICO Xpress Optimizer. A list
#' of all public attributes can be found in the [Attributes
#' Reference](reference_attributes.html).
#' 
#' One important thing first: You can access all controls and attributes with a
#' special syntax in R: Use `xpress:::ROWS` (yes, 3 colons) to access the integer
#' ID of the rows attribute of the optimizer. We provide the integer IDs for
#' controls and attributes in this way such that they do not pollute the namespace
#' upon loading the `xpress` package.
#' 
#' 
#' If you haven't done so already, please familiarize yourself with the Facility
#' Location Example, which we use throughout our quick examples.
#' 
#' We read the Facility location problem from the introductory example, and print
#' it to verify its dimensions.
#' 
## ----Reading in the Facility Location Problem---------------------------------
suppressMessages(library(xpress))
p <- createprob()
print(readprob(p, "flp.lp"))

#' 
#' # Query Basic Problem Attributes
#' 
#' Attributes can be accessed via the functions `getintattrib`, `getdblattrib`, and
#' `getstringattrib` from the xpress-package. All three functions receive an
#' integer ID as input.
#' 
#' When we use the print()-function on our XPRSprob object, R internally calls
#' `print.XPRSprob` from the package. This functions uses the attribute querying
#' mechanism to inspect the currently loaded optimization problem.
#' 
#' Let's have a look.
## ----Inspecting the Print Function of XPRSprob--------------------------------
print(xpress:::print.XPRSprob)

#' 
#' # Query Attributes After a Solve
#' 
#' Many attributes are only meaningful after the problem was solved, for example
#' search tree and solution information, or solving time. Let's optimize the loaded
#' facility location problem and print such information.
#' 
## ----Query Attributes after The Solve-----------------------------------------
xprs_optimize(p)

# query some attributes
print(paste("Number of solutions: ", getintattrib(p, xpress:::MIPSOLS)))
print(paste("Number of B%B nodes: ", getintattrib(p, xpress:::NODES)))
print(paste("Solving Time: ", getdblattrib(p, xpress:::TIME)))

