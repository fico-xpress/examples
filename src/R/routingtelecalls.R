#####################################
# This file is part of the          #
# Xpress-R interface examples       #
#                                   #
#   (c) 2022-2026 Fair Isaac Corporation #
#####################################
#' ---
#' title: "Routing Telephone Calls"
#' author: Y. Gu
#' date: Jun. 2021
#' ---
#' 
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
#' [This is a conversion of the Mosel example 'Routing Telephone Calls'](https://www.fico.com/fico-xpress-optimization/docs/latest/examples/mosel/ApplBook/G_Telecomm/g3routing.mos).
#' A brief introduction to this problem is given below, and to see the full
#' mathematical modeling of this problem you may refer to section 12.3, page 180 of the
#' book 'Applications of optimization with Xpress'.
#' 
#' A private telephone company exploits a network (shown below) among five cities and
#' the edge labels represent the capacities of the link in terms of circuits. The capacity
#' of an edge is consumed in increments of a bidirectional circuit and we do not consider
#' any directed flows. So, if 10 persons call from Nantes to Nice and 20 from Nice to
#' Nantes, then 30 circuits are used. The problem is that faced with demands for circuits,
#' the company wants to know whether it is possible to satisfy the demands entirely. If
#' it is not possible, the company wants to transmit as much as possible.
#' 
#' To solve this problem, we use arc-paths formulation. This formulation uses a single
#' integer variable 'flow_p' for the flow all along a path p, where this path corresponds
#' to a known pair of cities, and the flow conservation laws per node are implicitly fulfilled.
#' 
#' The objective we want to maximize is obviously the sum of 'flow' on all paths that
#' may be used. Two sets of constraints should be satisfied, the first one is that for
#' every arc, the sum of flows on the paths passing through it must not exceed its
#' capacity. The second one is that for every pair of cities, the sum of flows exchanged
#' between them over various paths must not exceed the demand for circuits.
#' 
#' More detailed explanations and full mathematical formulation about this example are
#' included in the book 'Applications of optimization with Xpress'.
#' 
#' 
#' For this example, we need packages 'xpress', 'dplyr' and 'igraph'.
#' 
## ----Load The Packages--------------------------------------------------------
library(xpress)
library(dplyr)
library(igraph)


#' 
#' 
#' Create a new empty problem, set the objective sense as maximization and give the
#' problem a suitable name.
#' 
## ----Create The Problem-------------------------------------------------------
# create a new problem
prob <- createprob()

# change this problem to a maximization problem
chgobjsense(prob,objsense = xpress:::OBJ_MAXIMIZE)

# set the problem name
setprobname(prob, "Routing")


#' 
#' 
#' Add the values we need for this example and plot the network.
#' 
## ----Data And Graph-----------------------------------------------------------
# 1. 'CALLS': the set of city pairs between which there are demands for calls
#    'DEM': demands between pairs of cities
demand.df <- data.frame("CALLS" = c("Nantes-Nice", "Nantes-Troyes", "Nantes-Valenciennes",
                                       "Nice-Valenciennes", "Paris-Troyes"),
                        "DEM" = c(100, 80, 75, 100, 70))

# 2. 'ARCS': the set of (undirected) arcs
#    'CAP': the capacity of each arc
capacity.df <- data.frame("ARCS" = c("Nantes-Paris", "Nantes-Nice", "Paris-Nice",
                                     "Paris-Valenciennes", "Troyes-Nice", "Valenciennes-Troyes"),
                          "CAP" = c(300, 120, 300, 200, 80, 70))

# 3. list of arcs composing the routes for demands
ROUTE.lst <- list(c("Nantes-Nice"),
                  c("Nantes-Paris", "Paris-Nice"),
                  c("Nantes-Paris","Paris-Valenciennes", "Valenciennes-Troyes", "Troyes-Nice"),
                  c("Nantes-Paris", "Paris-Valenciennes", "Valenciennes-Troyes"),
                  c("Nantes-Paris", "Paris-Nice", "Troyes-Nice"),
                  c("Nantes-Nice", "Troyes-Nice"),
                  c("Nantes-Nice", "Paris-Nice", "Paris-Valenciennes", "Valenciennes-Troyes"),
                  c("Nantes-Paris", "Paris-Valenciennes"),
                  c("Nantes-Nice", "Paris-Nice", "Paris-Valenciennes"),
                  c("Nantes-Paris", "Paris-Nice", "Troyes-Nice", "Valenciennes-Troyes"),
                  c("Nantes-Nice", "Troyes-Nice", "Valenciennes-Troyes"),
                  c("Nantes-Nice", "Nantes-Paris", "Paris-Valenciennes"),
                  c("Paris-Nice", "Paris-Valenciennes"),
                  c("Troyes-Nice", "Valenciennes-Troyes"),
                  c("Paris-Valenciennes", "Valenciennes-Troyes"),
                  c("Nantes-Paris", "Nantes-Nice", "Troyes-Nice"),
                  c("Paris-Nice", "Troyes-Nice"))

# 4. 'Cindex': pair of cities at the ends of every path
CINDEX <- data.frame(
  Routeidx = 1:length(ROUTE.lst),
  Cindex = c(
    rep("Nantes-Nice", 3),
    rep("Nantes-Troyes", 4),
    rep("Nantes-Valenciennes", 4),
    rep("Nice-Valenciennes", 3),
    rep("Paris-Troyes", 3)
  )
)

# 5. the network
elist <- data.frame(tails=c("Nantes", "Nantes", "Paris", "Paris", "Troyes", "Valenciennes"),
                    heads=c("Paris", "Nice", "Nice", "Valenciennes", "Nice", "Troyes"))
graph <- graph_from_data_frame(d = elist, directed = F)

set.seed(66)
plot(
  graph,
  vertex.color = "gold",
  vertex.size = 25,
  vertex.frame.color = "gray",
  vertex.label.color = "black",
  edge.color = "gray",
  edge.label = capacity.df$CAP,
  layout = layout.auto(graph)
)


#' 
#' 
#' Create variables 'flow' and store their indices in a newly created vector 'flow' in
#' data frame 'CINDEX'.
#' 
## ----Add Columns--------------------------------------------------------------
# integer variable 'flow' representing flows on each path
CINDEX$flow <-
  CINDEX %>% apply(1, function(x)
    xprs_newcol(
      prob,
      lb = 0,
      ub = Inf,
      coltype = "I",
      name = sprintf("flow_%s", x["Routeidx"]),
      objcoef = 1
    ))


#' 
#' 
#' Add the constraints mentioned in introduction.
#' 
## ----Add Rows, results='hide'-------------------------------------------------
# 1. for every pair of cities in 'CALLS', the sum of flows exchanged between them over
#    the various paths must not exceed the demand for circuits
apply(demand.df, 1, function(x)
  xprs_addrow(
    prob,
    colind = (CINDEX %>% filter(Cindex == x["CALLS"]) %>% select(flow))$flow,
    rowcoef = rep(1, sum(CINDEX$Cindex == x["CALLS"])),
    rowtype = "L",
    rhs = as.numeric(x["DEM"]),
    name = sprintf("flow_%s", x["CALLS"])
  ))

# 2. for every arc in 'ARCS', the sum of flows on the paths passing through it must
#    not exceed its capacity
for (i in capacity.df$ARCS) {
  colidx <- c()
  for (j in 1:length(ROUTE.lst)) {
    if (i %in% ROUTE.lst[[j]]) {
      colidx <- c(colidx, CINDEX$flow[j])
    }
  }
  xprs_addrow(
    prob,
    colind = colidx,
    rowcoef = rep(1, length(colidx)),
    rowtype = "L",
    rhs = capacity.df$CAP[which(capacity.df$ARCS == i)],
    name = sprintf("arc_capacity_%s", i)
  )

}


#' 
#' 
#' Now we can solve this problem.
#' 
## ----Solve The Problem--------------------------------------------------------
setoutput(prob)
summary(xprs_optimize(prob))

#' 
#' 
#' Display the solutions here.
#' 
## ----Solutions And Graph------------------------------------------------------
CINDEX$flow.sol <- getsolution(prob)$x

print(paste0(
  "The optimum total flow is: ",
  getdblattrib(prob, xpress:::MIPOBJVAL)
))
for (d in 1:nrow(demand.df)) {
  cat(
    demand.df$CALLS[d],
    "(demand: ",
    demand.df$DEM[d],
    ", routed calls: ",
    CINDEX %>% filter(Cindex == demand.df$CALLS[d]) %>% select(flow.sol) %>%
      sum(),
    ")\n"
  )

  p.route <-
    CINDEX %>% filter(Cindex == demand.df$CALLS[d] & flow.sol > 0) %>%
      select(Routeidx)

  apply(p.route, 1, function(x)
    cat(" ", CINDEX$flow.sol[x], ":", ROUTE.lst[[x]], "\n"))

}


print("Unused capacity:")
capacity.df$used <- rep(0, nrow(capacity.df))

for (i in capacity.df$ARCS) {
  used <- 0

  for (j in 1:length(ROUTE.lst)) {
    if (i %in% ROUTE.lst[[j]]) {
      used <- used + CINDEX$flow.sol[j]
    }
  }
  capacity.df$used[which(capacity.df$ARCS == i)] <- used
  unused <- capacity.df$CAP[which(capacity.df$ARCS == i)] - used
  if (unused > 0) {
    print(paste0(" ", i, ": ", unused))
  }
}

# the graph
elabel <-
  apply(capacity.df, 1, function(x)
    paste0(x["used"], "/", x["CAP"]))

set.seed(66)
plot(
  graph,
  vertex.color = "gold",
  vertex.size = 20,
  vertex.frame.color = "gray",
  vertex.label.color = "black",
  edge.color = "gray",
  edge.label = elabel,
  edge.arrow.size = 0.5,
  layout = layout.auto(graph)
)


#' 
#' 
