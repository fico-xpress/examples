#####################################
# This file is part of the          #
# Xpress-R interface examples       #
#                                   #
#   (c) 2022-2026 Fair Isaac Corporation #
#####################################
#' ---
#' title: "Set Controls"
#' author: Gregor Hendel
#' date: Dec. 2020
#' ---
#' 

#' 
#' This example shows how to set controls of the FICO Xpress Optimizer. A list of
#' all public controls can be found in the [Controls Reference](reference_controls.html).
#' 
#' One important thing first: You can access
#' all controls and attributes with a special syntax in R: Use `xpress:::ROWS`
#' (yes, 3 colons) to access the integer ID of the rows attribute of the optimizer.
#' We provide the integer IDs for controls and attributes in this way such that
#' they do not pollute the namespace upon loading the `xpress` package.
#' 
#' If you haven't done so already, please familiarize yourself with the Facility
#' Location Example, which we use throughout our quick examples.
#' 
#' We read the Facility location problem from the introductory example, and print
#' it to verify its dimensions.
#' 
## ----Reading the Facility Location Problem------------------------------------
suppressMessages(library(xpress))
p <- createprob()
print(readprob(p, "flp.lp"))

#' 
#' # Setting Limits
#' 
#' Most often, users seek to limit the solution process, for example a time limit,
#' node limit, or LP iterations limit.
#' 
#' Here is how to set a limit on:
#' 
#' - the maximum allowed wallclock time of 10 seconds, or
#' - the number of allowed branch-and-bound nodes, or
#' - the number of solutions.
#' 
## ----Setting Limits on Time, Nodes, and Solutions-----------------------------
setintcontrol(p, xpress:::MAXTIME, -10L)
setintcontrol(p, xpress:::MAXNODE, 1L)
setintcontrol(p, xpress:::MAXMIPSOL, 1L)
dumpcontrols(p)

#' 
#' The alert reader will notice that we set a *negative* MAXTIME of -10 seconds.
#' This is the right approach to stop the Optimizer after 10 seconds *regardless of
#' whether a solution has been found or not*. A positive limit of 10 seconds stops
#' the optimizer after 10 seconds if a solution was found by then, or if not, as
#' soon as a solution is found.
#' 
#' 
## ----Starting the Optimization Process----------------------------------------
# enable logging to stdout
setoutput(p)

summary(mipoptimize(p)) # we could also call "xprs_optimize()"
print(getintattrib(p, xpress:::MIPSTATUS))

#' In this case, we hit the solution limit and interrupt the solution process.
#' The first solution has an objective value of
#' `r format(getdblattrib(p, xpress:::MIPOBJVAL))`,
#' which is
#' `r format(getdblattrib(p, xpress:::MIPOBJVAL) - getdblattrib(p, xpress:::BESTBOUND))`
#' larger than the
#' dual bound of `r format(getdblattrib(p, xpress:::BESTBOUND))`.
#' 
#' # Resetting Controls
#' 
#' We decide to set the solution and node limit back to their default values, and
#' continue the search:
#' 
## ----Setting Default Controls-------------------------------------------------
setdefaultcontrol(p, xpress:::MAXMIPSOL)
setdefaultcontrol(p, xpress:::MAXNODE)

## continue the solution process
summary(mipoptimize(p))

#' In order to set all controls to their default values, we invoke
## ----Resetting all controls---------------------------------------------------
setdefaults(p)

#' 
#' We verify that no controls are set via `dumpcontrols`
## ----Output User-set Controls to the Console----------------------------------
dumpcontrols(p)

#' 
#' # Change Solver Behavior
#' 
#' After we familiarized ourselves with limits on the solution process, we can change some algorithmic behavior.
#' 
#' - the [HEURSEARCHFREQ](https://www.fico.com/fico-xpress-optimization/docs/latest/solver/optimizer/HTML/HEURSEARCHFREQ.html)
#'   parameter specifies the frequency at which heuristics are used in the tree
#'   search
#' - the [CUTSTRATEGY](https://www.fico.com/fico-xpress-optimization/docs/latest/solver/optimizer/HTML/CUTSTRATEGY.html)
#'   parameter specifies the aggressiveness with which cutting planes are separated
#' - the [SBITERLIMIT](https://www.fico.com/fico-xpress-optimization/docs/latest/solver/optimizer/HTML/SBITERLIMIT.html)
#'   parameter specifies a user limit on the number of Simplex iterations in each
#'   Strong Branching LP
#' 
## ----Change Algorithmic Parameters--------------------------------------------

# the frequency at which heuristics are used in the tree search
setintcontrol(p, xpress:::HEURSEARCHFREQ, 2L)

# disable all cutting planes
setintcontrol(p, xpress:::CUTSTRATEGY, 0)

# use a user limit on the number of Simplex iterations in each Strong Branching LP
setintcontrol(p, xpress:::SBITERLIMIT, 200)
summary(mipoptimize(p))

#' 
