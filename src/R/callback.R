#####################################
# This file is part of the          #
# Xpress-R interface examples       #
#                                   #
#   (c) 2022-2026 Fair Isaac Corporation #
#####################################
#' ---
#' title: "Using Callbacks"
#' author: Gregor Hendel
#' date: Dec. 2020
#' ---
#' 

#' 
## ----Drop Existing Log File, include=FALSE------------------------------------
# clear the log file output
if (file.exists("optnodecallbacklog.txt"))
  file.remove("optnodecallbacklog.txt")

#' 
## ----Libraries----------------------------------------------------------------
suppressMessages(library(xpress))

# load these libraries for plotting the primal and dual bound
library(ggplot2)
library(magrittr)

#' 
#'  This example shows how to customize the solution process using callbacks.
#' 
#'  In general, callbacks are functions that allow the user to monitor progress
#'  or control certain aspects of the search. Callbacks are invoked at certain
#'  stages of the solution produces and are given a chance to perform certain
#'  actions and/or query the current state of the search.
#' 
#' # Differences between C and R callbacks
#' 
#'  In C callbacks can have
#' 
#'  - a return value,
#'  - input arguments,
#'  - output arguments.
#' 
#'  Callbacks in R work pretty similar. The only exception is that they don't
#'  take any output arguments. Instead any output is supposed to be given by
#'  the return value. To this end, a callback function in R is supposed to
#'  return one of the following:
#' 
#'  - An integer. If the C API allows the callback to return an integer then
#'    this integer is returned to the optimizer (a non-zero value usually
#'    asks for interruption of the solve). See the C documentation for more
#'    details about that.
#'  - A list. The values in the list are used as output from the callback
#'    (for callbacks that return data). The list is also checked for an
#'    element called `ret`. If present, this must be an integer and will be
#'    returned to the solver (in case the C callback has a return value).
#' 
#'  Another difference between the C implementation and R is that in R there is
#'  no user data argument (called `vContext` in C). If the callback requires any
#'  data that is not directly available in the callback, then it is assumed the
#'  user makes this available in the callback's environment, for example by
#'  means of lexical scoping.
#' 
#' # Limitations
#' 
#'  Note that the Xpress R library does not increment the reference count
#'  of the callback function that is passed to the various `addcb*` functions.
#'  So it is up to the user to make sure that the reference count of these
#'  functions does not drop to zero while the optimizer might still call them.
#' 
#'  Finally, due to the fact that R is inherently single-threaded, all callbacks
#'  must be executed from the thread that runs R. To this end, the `createprob()`
#'  function automatically sets the `xpress:::CALLBACKFROMMAINTHREAD` control
#'  to 1. Don't change this if you are using any kind of callback.
#' 
#' 
#' # Reading the Example
#' 
#' We read the facility location problem from the introductory example.
#' By printing the number of MIP entities, we verify
#' that this is a problem with (some) integer variables, so MIP algorithms and
#' callbacks will be applied to solve it.
#' 
## ----Read The Example---------------------------------------------------------
# this also creates a problem
p <- readprob(createprob(), "flp.lp")
print(p)

print(getintattrib(p, xpress:::MIPENTS))

#' 
#' 
#' # Callback Definition
#' 
#' ## User Data
#' We define a data frame object `progress.df` in the main scope of R.
#' This is our "data" that we want to use inside the callback.
#' Thanks to R scoping rules, we can access `progress.df` from inside the callback.
## ----Setup User Data----------------------------------------------------------
  # initialize an empty progress data frame
  # this is input into the callback
  progress.df <- data.frame(
    "Node"=integer(),
    "MIPBestObjVal"=numeric(),
    "BestBound"=numeric()
  )

#' 
#' ## Callback Definition
#' 
#' We solve this problem using the default algorithm but with two callbacks that
#' are invoked at each node for which the LP relaxation has been solved to
#' optimality or nodes that were infeasible.
#' 
#' We report progress by printing the best integer solution found so far
#' and the best solution possible.
#' We append the data; Note the use of the  `<<-` operator,
#' which modifies the global progress data frame
#' instead of creating a local copy
#' 
## ----Callback Definition------------------------------------------------------
callback <- function(prob, header) {
  cat(paste(header, "dual bound:", getdblattrib(prob, xpress:::BESTBOUND),
            "primal bound:", getdblattrib(prob, xpress:::MIPBESTOBJVAL),
			"\n"),
			file = "optnodecallbacklog.txt", append = T)


  ### Caution: This approach is for illustrative purposes, but super ineffective for larger search trees,
  ###          because the entire data frame is copied at each call.
  progress.df <<- rbind(
    progress.df,
    data.frame(
      "Node"=getintattrib(prob, xpress:::NODES),
      "MIPBestObjVal"=getdblattrib(prob, xpress:::MIPBESTOBJVAL),
      "BestBound"=getdblattrib(prob, xpress:::BESTBOUND)
    )
  )

  return(0)
}

#' 
#' ## Callback Registration
#' 
#' We register our function as two callbacks into the loaded problem. The output of
#' the infnode callback will enrich our data set. We register the same function for
#' both 'opt'imal and 'inf'easible nodes, but with a different header.
#' 
## ----Callback Registration----------------------------------------------------
headeropt <- "OPTCALLBACK:"
headerinf <- "INFCALLBACK:"
addcboptnode(p, function(prob) {callback(prob, headeropt)})
addcbinfnode(p, function(prob) {callback(prob, headerinf)})

#' 
#' ## Invoking the Solution Process
#' 
#' We redirect the output into a log file, invoke the optimization process, and print the solution.
#' For easier interpretation of the results, we solve the problem single-threaded.
#' 
## ----Invoke The Solution Process----------------------------------------------
setlogfile(p, "optnodecallbacklog.txt")

# prints to stdout. This does not affect the logfile
setoutput(p)

# use only one thread for easier interpretation of the data frame
setintcontrol(p, xpress:::THREADS,1)
xprs_optimize(p)

# We retrieve the solution print the solution values for the x variables
sol <- getsolution(p)$x
# note that this print does not show in the log file, only in this code chunk.
print(sol[1:5])

#' 
## ----Show The Data Frame------------------------------------------------------
print(progress.df)

#' 
## ----Plot The Progress using ggplot2------------------------------------------

progress.df %>%
  ggplot(aes(1:nrow(progress.df))) +
  geom_line(mapping = aes(y=MIPBestObjVal, color="MIPBestObjVal")) +
  geom_line(mapping = aes(y=BestBound, color="BestBound")) +
  labs(
    y="Objective",
    x="Call",
    color=NULL
) + theme_dark()

#' 
#' ## The Output
#' 
#' The log file output shows that our callback has been successfully applied.
#' 
## ----echo=FALSE---------------------------------------------------------------
cat(readLines("optnodecallbacklog.txt"), sep = "\n")

