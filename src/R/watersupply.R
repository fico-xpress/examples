#####################################
# This file is part of the          #
# Xpress-R interface examples       #
#                                   #
#   (c) 2022-2026 Fair Isaac Corporation #
#####################################
#' ---
#' title: "Water Supply Management"
#' author: Y. Gu
#' date: Jul. 2021
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
#' [This is a conversion of the Mosel example 'Water Supply Management'](https://www.fico.com/fico-xpress-optimization/docs/latest/examples/mosel/ApplBook/J_Service/j1water.mos).
#' A brief introduction to this problem is given below, and to see the full
#' mathematical modeling of this problem you may refer to section 15.1, page 229 of the
#' book 'Applications of optimization with Xpress'.
#' 
#' This example is a classical maximum flow problem. There are 2 reservoirs, 3 cities
#' and 5 pumping stations. The three cities are supplied from the two reservoirs.
#' All the 10 nodes are connected by a network of pipes, and we want to determine the
#' maximum flow in this network to see whether the demands can be satisfied entirely.
#' 
#' As a typical maximum flow problem, firstly we create two artificial nodes SOURCE and
#' SINK, where SOURCE is connected to the two reservoirs by two arcs with capacities
#' corresponding to the availability of water from the two reservoirs, and SINK is the
#' node to which the three cities are connected by three arcs with capacities corresponding
#' to the cities' requirement for water.
#' 
#' Then, we define variables 'flow' for each arc and the 'flow' of each arc should not
#' exceed its capacity. Note that we set the variable type as continuous, because it is
#' possible to let non-integer flows pass through the arcs. But since all capacities here
#' are integer, the simplex algorithm will automatically find integer solution values in
#' the optimal solution to the linear problem.
#' 
#' Our objective is to maximize the total flow, which is the sum of flows into SINK or
#' the total flows out of SOURCE. An important constraint needed to be satisfied in
#' network flow problems is the so-called flow conservation for all nodes except for
#' SINK and SOURCE. That is, for the intermediate nodes, the the total flow arriving at
#' any node also has to leave this node.
#' 
#' After solving this problem, we can see the optimum total flow is 52, which is 1 less
#' that the total demand from the three cities. So, the existing network will not be able
#' to satisfy the demands. The method used here may be applied to other liquids, and also
#' to road or telecommunications networks.
#' 
#' 
#' For this example, we need packages 'xpress', 'dplyr' and 'igraph'. Besides, we use the
#' function `pretty_name` to give the variables and constraints concise names.
#' 
## ----Load The Packages And The Function To Give Names-------------------------
library(xpress)
library(dplyr)
library(igraph)

pretty_name <- function(prefix, y) {
  "%s_%s" %>% sprintf(
    prefix,
    paste(lapply(names(y), function(name) paste(name, y[name], sep="_")), collapse = "_")
  )
}


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
setprobname(prob, "WaterSupply")


#' 
#' 
#' Add the values we need for this example.
#' 
## ----Data---------------------------------------------------------------------
# nodes, where nodes 1 and 2 represent 2 reservoirs, nodes 8, 9 and 10 represent 3 cities,
# nodes 3-7 represent pumping stations and nodes 11 and 12 are SOURCE and SINK
NODES <- 1:12
SOURCE <- 11; SINK <- 12

# arcs & capacities
ARCS.df <- data.frame(index = 1:20,
                      head = c(1, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 5, 6, 7, 7, 8, 9, 10, 11, 11),
                      tail = c(3, 5, 6, 5, 6, 4, 5, 8, 9, 8, 9, 10, 7, 9, 10, 12, 12, 12, 1, 2),
                      capacity = c(20, 15, 12, 6, 22, 15, 10, 7, 10, 10, 15, 15, 22, 10, 10, 18, 15, 20, 35, 25))


#' 
#' 
#' Create variables 'flow' for each arc and set objective as mentioned in introduction.
#' 
## ----Add Columns--------------------------------------------------------------
# variables 'flow' for each arc
ARCS.df$flow <-
  apply(ARCS.df, 1, function(x)
    xprs_newcol(
      prob,
      lb = 0,
      ub = x["capacity"],
      coltype = "C",
      name = pretty_name("flow", x["index"]),
      objcoef = NULL
    ))

# change objective coefficients according to the definition of total flow
chgobj(prob,
       colind = (ARCS.df %>% filter(tail == SINK))$flow,
       objcoef = rep(1, sum(ARCS.df$tail == SINK)))


#' 
#' 
#' Add the flow conservation constraints.
#' 
## ----Add Rows-----------------------------------------------------------------
# the total flow arriving at any node also has to leave this node (with the exception of the source and sink nodes)
for (v in NODES[-c(SINK, SOURCE)]) {
  xprs_addrow(
    prob,
    colind = c((ARCS.df %>% filter(head == v))$flow,
               (ARCS.df %>% filter(tail == v))$flow),
    rowcoef = c(rep(1, sum(ARCS.df$head == v)), rep(-1, sum(ARCS.df$tail == v))),
    rowtype = "E",
    rhs = 0,
    name = paste0("balance_", v)
  )
}


#' 
#' 
#' Now we can solve the problem.
#' 
## ----Solve The Problem--------------------------------------------------------
setoutput(prob)
summary(xprs_optimize(prob))


#' 
#' 
#' Display the solutions here.
#' 
## ----The Solutions------------------------------------------------------------
paste("The optimum total flow is: ",
      getdblattrib(prob, xpress:::LPOBJVAL))

ARCS.df$solution <- getsolution(prob)$x
invisible(apply(ARCS.df, 1, function(x)
  cat(x["head"], "->", x["tail"], ":", x["solution"], "/", x["capacity"], fill = TRUE)))

# visualize the solution in network graph
elist <- data.frame(tails = ARCS.df$head, heads = ARCS.df$tail)
graph <- graph_from_data_frame(d = elist, directed = T)

vlabel <-
  c(
    "1 reservoir",
    "2 reservoir",
    3:7,
    "8 Gotham City",
    "9 Metropolis",
    "10 Spider Ville",
    "11 SOURCE",
    "12 SINK"
  )
elabel <-
  apply(ARCS.df, 1, function(x)
    paste0(x["solution"], "/", x["capacity"]))

set.seed(66)
plot(
  graph,
  vertex.color = "gold",
  vertex.size = 20,
  vertex.frame.color = "gray",
  vertex.label = vlabel,
  vertex.label.color = "black",
  vertex.label.cex = 0.8,
  layout = layout.auto(graph),
  edge.color = "gray",
  edge.label = elabel,
  edge.label.color = "red",
  edge.arrow.size = 0.5,
  edge.label.cex = 0.8
)


#' 
#' 
