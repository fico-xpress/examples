#####################################
# This file is part of the          #
# Xpress-R interface examples       #
#                                   #
#   (c) 2022-2026 Fair Isaac Corporation #
#####################################
#' ---
#' title: "Finding a Shortest Tour"
#' subtitle: "The Travelling Salesperson Problem"
#' author: Gregor Hendel
#' date: Dec. 2020
#' ---
#' 
## ----setup, include=FALSE-----------------------------------------------------
knitr::opts_chunk$set(echo = TRUE)
knitr::opts_chunk$set(results = "hold")

#' 
## ----Load libraries-----------------------------------------------------------
suppressMessages(library(xpress))

# load necessary libraries for plotting
suppressMessages(library(reshape2))
suppressMessages(library(magrittr))
suppressMessages(library(ggplot2))
suppressMessages(library(dplyr))
suppressMessages(library(tibble))

#' 
#' In this example, we show how to use the FICO Xpress R-interface to model and
#' solve the famous **Travelling Salesperson Problem (TSP)**. The TSP is
#' the classic combinatorial optimization problem to find the shortest so-called
#' tour through a complete graph.
#' 
#' This longer example discusses different approaches for solving TSPs with Xpress.
#' First we solve the problem without any subtour elimination constraints, which
#' will give an invalid solution. Then we solve the problem initialized with all
#' subtour elimination constraints, which does not perform well. Finally we show
#' how to use callbacks for adding subtour elimination constraints
#' dynamically during the search.
#' 
#' We dive right into the mathematical formulation.
#' 
#' # Mathematical formulation
#' 
#' There are different variants how to formulate the TSP. We use the classical
#' formulation using binary variables and exponentially many constraints to
#' eliminate all so-called subtours.
#' 
#' ## Graphs and Tours
#' 
#' We assume we are given a complete Graph $K_n$ on $n$ vertices $V$. A graph is
#' called complete if it contains all $\binom{n}{2}$ possible edges, one between
#' each pair of nodes $v \neq w \in V \times V$. Denote the set of edges by $E$. We
#' further have a function that assigns a cost to each edge,
#' 
#' $$
#'   d: E \rightarrow \mathbb{Q}
#' $$
#' 
#' A path in $K_n$ is a sequence of adjacent edges $(e_1,e_2,\dots,e_k)$ such that
#' consecutive edges have a common node, and no node is visited twice (except,
#' possibly, for the endpoints). A tour is a path of length $n$, which starts and
#' ends at the same node and in between visits each node exactly once. The cost of
#' a tour is the sum of the costs of its edges. The TSP demands to find a tour of
#' minimum cost.
#' 
#' For the mathematical formulation, we introduce $n(n-1)/2$ binary variables
#' $x_{e}$ for each edge $e \in E$. Of course, $x_{e} = 1$ corresponds to $e$ being
#' part of the tour represented by a solution to our program.
#' 
#' $$
#'   \begin{align}
#'     && \min \sum\limits_{e \in E} d(e) x_{e}\\
#'     & \text{s.t.}     &\sum\limits_{e \in \delta(v)} x_{e} &= 2 & \forall v\in V\\
#'     &                 &\sum\limits_{e \in \delta(S)} x_e & \geq 2 & \forall \emptyset \neq S \subsetneq V\\
#'     &                 & x_e &\in \{0,1\} & \forall e \in E
#'   \end{align}
#' $$
#' 
#' The first set of $n$ constraints requires that each node should have exactly two
#' adjacent edges in a solution. This set of constraints is often referred to as
#' "node-degree" constraints. The second set of constraints formulate that each
#' nonempty strict subset of nodes be entered at least once and left at least once.
#' We refer to this type of constraints as "subtour-elimination" constraints.
#' 
#' It is easy to verify that a tour, encoded as a characteristic vector of its
#' edges, is indeed a feasible solution for the above program. The reverse is also
#' true.
#' 
#' It is obvious that the above formulation is computationally critical for larger
#' graphs due to its sheer size. The number of subtour elimination constraints
#' grows exponentially with the number $n$ of nodes in the graph, which becomes
#' intractable even for moderate graph sizes.
#' 
#' # Creating the Formulation Explicitly
#' 
#' We generate $n = 10$  points $(x,y)$. The locations are drawn uniformly at
#' random from [-3,3]. We create a distance data frame that holds the Euclidean
#' distance for each pair of nodes `Vi`, `Vj`, $i < j$
#' Each row of this data frame will correspond to 1 binary variable of our TSP.
#' 
#' ## Data Generation
## ----Data Generation----------------------------------------------------------
# the number of nodes
n <- 10

# random seed for reproducibility
set.seed(3701)
loc.matrix <- matrix(runif(n * 2, -3, 3), ncol = 2)

#
# each row of dist.df will correspond to 1 binary variable of our TSP
dist.df <- dist(loc.matrix) %>%
  as.matrix() %>%
  melt(varnames=c("Vi","Vj"), value.name="Dist") %>%
  dplyr::filter(Vi < Vj)


tibble::glimpse(dist.df)

# plot the n nodes in the plane and label them
ggplot(data.frame(loc.matrix),
  aes(x=loc.matrix[,1], y=loc.matrix[,2], label=1:n)
  ) +
  geom_point() +
  geom_label(nudge_y=0.3) +
  labs(x="X", y="Y")

#' 
#' ## Loading Node Degree Constraints
#' 
#' We declare a function `load_tsp` that receives the above distance data frame as
#' input and creates a new Xpress Problem with only the node degree constraints
#' loaded. We align the columns we create with the `dist.df` data frame.
#' 
#' 
## ----Load TSP with Node Degree Constraints------------------------------------

# create a TSP problem with node degree constraints
load_tsp <- function(dist.df) {
  p <- createprob()
  nedges <-  nrow(dist.df)
  problemdata <- list()

  # column information
  problemdata$lb <- rep(0,nedges)
  problemdata$ub <- rep(1,nedges)

  # column names
  problemdata$colname <- sprintf("x(%d,%d)",dist.df$Vi, dist.df$Vj)

  # objective coefficients
  problemdata$objcoef <- dist.df$Dist
  problemdata$columntypes <- rep("B", nedges)

  # right hand side and row type
  problemdata$rowtype <- rep("E", n)
  problemdata$rhs <- rep(2, n)

  # we input the node degree constraints as sparse matrix in column major format
  # directly using the corresponding problemdata names,
  # see the documentation of
  # xprs_loadproblemdata for more information

  # each edge appears in exactly two node degree rows, once for each endpoint
  # the number of elements in each column
  problemdata$collen <- rep(2,nedges) # 1 additional marker for the end
  # the start index of the coefficients of each column. we need to pass a 0-based index into Xpress!
  problemdata$start <- 2 * (0:(nedges - 1)) # 0-based index!

  # matrix coefficients, all 1's in the node degree constraints
  problemdata$rowcoef <- rep(1, 2 * nedges)

  # row indices of each coefficient. We first create an array with 0's
  problemdata$rowind <- rep(0, 2 * nedges)

  # The first endpoint Vi corresponds to the first row index of each edge
  problemdata$rowind[2 * (1:nedges) - 1] <- dist.df$Vi - 1 # 0-based index!

  # The second endpoint Vj corresponds to the second row index of each edge
  problemdata$rowind[2 * (1:nedges)] <- dist.df$Vj - 1 # 0-based index!

  # assign row names to identify the constraints easily in MPS or LP format
  problemdata$probname <- "TSP"
  problemdata$rowname <- sprintf("NodeDeg_%d", 1:n)

  # this invisibly returns the XPRSprob p from the function
  return(xprs_loadproblemdata(p, problemdata))
}

#' 
#' ## Solving the Node Degree TSP Without Subtour Elimination
#' 
#' We solve this initial formulation using Xpress and plot the corresponding
#' solution.
#' 
## ----Load and Solve with Node Degree Constraints------------------------------
p <- load_tsp(dist.df)
xprs_optimize(p)

#' 
#' Let's plot the TSP solution. For this, we translate
#' the vertex locations $(x,y)$ into
#' edges (segments in ggplot2 lingo) with start and endpoints.
#' 
## ----Plot the Solution--------------------------------------------------------
plot_tsp_solution <- function(p) {

  # convert (x,y) locations of the nodes into a data frame.
  loc.df <- as.data.frame(loc.matrix)
  colnames(loc.df) <- c("X", "Y")
  loc.df$V <- 1:n

  solution <- getsolution(p)$x
  # copy dist.df. For plotting, we must store the (x,y)-coordinates
  # of both endpoints
  plot.df <- dist.df[solution == 1,]
  plot.df <- plot.df %>% dplyr::mutate(
    Xi = loc.df$X[Vi],
    Yi = loc.df$Y[Vi],
    Xj = loc.df$X[Vj],
    Yj = loc.df$Y[Vj]
  )

  plot.df %>% ggplot(aes(x=Xi, xend=Xj, y=Yi, yend=Yj, label=Vi)) +
    geom_segment() +
    geom_point() + geom_label(nudge_y = 0.3) +
    geom_label(aes(x=Xj, y=Yj, label=Vj), nudge_y = 0.3) +
    geom_point(aes(x=Xj, y=Yj))
}

summary(p)
plot_tsp_solution(p)

#' 
#' ## Adding Subtour Elimination Constraints
#' 
#' In a first shot, we add all subtour elimination constraints
#' explicitly to the model. We can do this because the
#' example is relatively small.
#' A row always enters the problem as last row (at position `xpress:::ROWS - 1`).
#' After adding a row, we declare it immediately as a delayed row.
#' 
#' 
## ----Adding Subtour Elimination Constraints-----------------------------------
# this function basically replicates a recursive subset enumeration approach
# without the need to copy and slice objects all the time

subsets <- function(r, base_set) {
  sorted_base_set <- sort(unique(base_set))
  n <- length(sorted_base_set)

  pointers <- 1:r
  curr_pointer <- r

  nsubsets <- factorial(n) / factorial(r) / factorial(n - r)

  result <- matrix(nrow = nsubsets, ncol = r)

  curr_row <- 1

  for (curr_row in 1:nsubsets) {
    # now snap all pointers back to the right of their previous
    while (curr_pointer < r) {
      pointers[curr_pointer + 1] <- pointers[curr_pointer] + 1
      curr_pointer <- curr_pointer + 1
    }

    # translate the current pointers into a matrix row
    for (p in 1:r) {
      result[curr_row, p] <- sorted_base_set[pointers[p]]
    }

    if (curr_row == nsubsets) break

    # check if the current pointer can advance.
    # Otherwise, bring the previous pointer into the right position
    while(curr_pointer > 0) {
      if (curr_pointer == r && pointers[curr_pointer] == n) {
        curr_pointer <- curr_pointer - 1;

      } else if (curr_pointer < r && pointers[curr_pointer] == pointers[curr_pointer + 1] - 1) {
        curr_pointer <- curr_pointer - 1;

      }else break
    }

    # advance the current pointer by 1
    pointers[curr_pointer] <- pointers[curr_pointer] + 1
  }

  return (result)
}


add_subtour_elim <- function(p, dist.df) {
  nedges <- nrow(dist.df)
  # we enumerate all subsets that contain the first node,
  # i.e, all subsets of v_2, ... ,v_n with at most n - 3 elements.
  for (r in 1:(n - 3)) {
    allsubsets <- subsets(r, 2:n)
    # each row in allsubsets represents 1 subset
    for (s in 1:nrow(allsubsets)) {
      viins <- (dist.df$Vi == 1) | (dist.df$Vi %in% allsubsets[s,])
      vjins <- (dist.df$Vj == 1) | (dist.df$Vj %in% allsubsets[s,])
      edgeins <- (viins != vjins) # exactly one endpoint in S

      # use addrows to add each row explicitly
      addrows(p, rowtype = "G", rhs = 2,
              start = c(0,(r+1) * (n - r - 1)),
              colind = (1:nedges)[edgeins] - 1,
              rowcoef = rep(1, (r+1) * (n - r - 1))
              )
      # declare this row as delayed row.
      loaddelayedrows(p, rowind=getintattrib(p, xpress:::ROWS) - 1)
    }
  }
  # return p again to the caller
  p
}

#' 
#' ## Solving the Full TSP
#' 
#' When we solve the full TSP with the added subtour elimination constraints, we
#' obtain a correct tour, albeit with higher cost than the subtour solution from
#' the previous run.
#' 
## -----------------------------------------------------------------------------
p <- load_tsp(dist.df) %>%
  add_subtour_elim(dist.df) %>%
  xprs_optimize()

print(p)
summary(p)
plot_tsp_solution(p)

#' 
#' # Separate Violated Subtour Elimination Constraints.
#' 
#' It is much more efficient to separate subtour elimination constraints
#' dynamically during the search.
#' We initially omit all subtour elimination constraints.
#' If we encounter an integer feasible LP solution, we verify if
#' the solution is indeed a tour with full length. If not,
#' we determine the subtour in which the $V1$ lies.
#' This subtour cannot reach all other nodes in the graph.
#' 
#' We define a function `get_subtour`. This will be used
#' for checking the feasibility of an otherwise
#' integer feasible solution and the
#' separation of violated subtour elimination constraints.
#' 
#' ## Callback Definition
#' 
## ----Helper function to find subtours-----------------------------------------
get_subtour <- function(prob, sol) {
  # subset the distance data frame to the currently used edges
  dist.df.sol <- dist.df[sol >= 1 - 1e-6,]

  subtourindices <- NULL

  # initialize a logical to indicate which nodes we have visited.
  visited <- logical(n)

  # edge rep maps each node to its 2 adjacent edges in the solution
  edgerep <- matrix(0, ncol = 2, nrow=n)
  for (i in 1:nrow(dist.df.sol)) {
    vi <- dist.df.sol$Vi[i]
    vj <- dist.df.sol$Vj[i]
    edgerep[vi, 1 + (edgerep[vi, 1] != 0)] <- i
    edgerep[vj, 1 + (edgerep[vj, 1] != 0)] <- i
  }

  # iterate through the edges of the solution.
  curredge <- 1
  pathlength <- 0
  currnode <- dist.df.sol$Vi[curredge]
  while (!visited[currnode] && pathlength <= n) {
    # label node as visited
    visited[currnode] <- TRUE
    # go to the other node of the current edge
    currnode <- ifelse(dist.df.sol$Vi[curredge] == currnode, dist.df.sol$Vj[curredge], dist.df.sol$Vi[curredge])

    #find the other edge to walk to
    edges <- edgerep[currnode,]
    curredge <- ifelse(edges[1] == curredge, edges[2], edges[1])
    pathlength <- pathlength + 1
  }

  # if the path length was smaller than n, we found a subtour that is not connected
  # to the rest of the graph. The subset is part of the visited array
  if (pathlength < n) {
    subtourindices <- which(visited) # indices of the corresponding nodes.
  }

  return(subtourindices)
}

#' 
## ----Preintsol callback definition--------------------------------------------
preintsolcallback <- function(prob, soltype, cutoff) {
  # use getcallbacksolution to access the current solution
  sol <- getcallbacksolution(prob)$x

  subtourindices <- get_subtour(prob, sol)
  reject <- !is.null(subtourindices)

  if (reject && soltype == 0) {
    # If soltype is zero, the infeasible solution was obtained from an integral
    # node. In this case we can generate and inject a cut that cuts
    # off this solution so that we don't find it again.
    viins <- (dist.df$Vi %in% subtourindices)
    vjins <- (dist.df$Vj %in% subtourindices)
    edgeins <- (viins != vjins) # exactly one endpoint in S
    subtourlength <- length(subtourindices)

    presrow <- presolverow(p, rowtype = "G",
                            origcolind = which(edgeins) - 1, #
                            origrowcoef = rep(1, subtourlength * (n - subtourlength)),
                            origrhs = 2)

    if (presrow$status == 0) {
      addcuts(p, cuttype=1, rowtype = "G",
              rhs = presrow$rhs,
              start = c(0, presrow$ncoefs),
              colind = presrow$colind,
              cutcoef = presrow$rowcoef)

      cat(paste("Added cut for subtour", paste(subtourindices, collapse = ", "), "\n"))
    }

    # set reject to FALSE to continue solving the node LP relaxation
    # with the new cut obtained.
    reject <- FALSE
  }

  # we can only reject solutions from heuristics. If the solution is a relaxation solution
  # we must not reject it in order to continue solving the node LP relaxation with the added
  # cut.
  return (list(reject=reject))
}


#' 
#' 
#' ## Callback registration
#' 
#' We need to register the above callback.
#' We load the initial TSP only with node degree constraints.
#' We display the optimal solution to verify that
#' the callback worked correctly.
#' 
## -----------------------------------------------------------------------------
p <- load_tsp(dist.df) # do not preload the subtour elimination constraints!
p <- p %>%
  setintcontrol(xpress:::MIPDUALREDUCTIONS, 0) %>%
  addcbpreintsol(preintsolcallback) %>%
  setoutput()

xprs_optimize(p)
summary(p)

#' 
#' The resulting solution represents a tour!
#' 
## -----------------------------------------------------------------------------
plot_tsp_solution(p)

#' 
#' 
#' 
#' 
#' 
#' 
#' 
#' 
