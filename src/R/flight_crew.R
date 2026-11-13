#####################################
# This file is part of the          #
# Xpress-R interface examples       #
#                                   #
#   (c) 2022-2026 Fair Isaac Corporation #
#####################################
#' ---
#' title: "Flight Crews"
#' author: Y. Gu
#' date: Jun. 2021
#' ---
#' 
#' 
#' 
## ----setup, include=FALSE-----------------------------------------------------
knitr::opts_chunk$set(echo = TRUE)
knitr::opts_chunk$set(results = "hold")
knitr::opts_chunk$set(warning = FALSE,message = FALSE)

#' 
#' 
#' ## Brief Introduction To The Problem
#' 
#' [This is a conversion of the Mosel example 'Composing Flight Crews'](https://www.fico.com/fico-xpress-optimization/docs/latest/examples/mosel/ApplBook/F_TransAir/f2crew.mos).
#' Brief introduction to this problem is given below, and to see the full
#' mathematical modeling of this problem you may refer to section 11.2, page 159 of the
#' book 'Applications of optimization with Xpress'.
#' 
#' There are 8 pilots and each of them has different levels (mark from 0 to 20) of
#' language skills and experience with different aircraft. We would like to form valid
#' flight crews that each consists of two pilots and both of the pilots have at least
#' 10/20 for the same language and 10/20 on the same aircraft type.
#' 
#' Two questions are raised, the first one asks whether all pilots can fly,and the second
#' one wants to find the set of crews with maximum total score. The score of each valid
#' crew is defined as the maximum of the sum of its pilot scores over all aircraft types
#' for which both pilots are rated at least 10/20.
#' 
#' To begin with, we find all the valid crews, and then we can think of this problem
#' as an undirected compatibility graph with 8 nodes representing 8 pilots and
#' arcs between each pair of valid crew pilots(nodes). We will visualize the solutions of
#' both questions in network graphs to check the solutions more intuitively.
#' 
#' For the first question, we actually want to find the maximum number of valid crews.
#' After we find all the compatible pairs of pilots, we set binary variables 'fly' for
#' each valid crew, and 'fly' indicates whether this crew is used or not. The objective
#' we want to maximize is simply the sum of 'fly' variables. The only constraint here is
#' that each pilot can only be contained in exactly one of the used crews. With this
#' constraint, we are actually looking for maximum cardinality matching for this question,
#' where matching is a set of arcs such that any two among them have no node in common.
#' 
#' For the second question, firstly we need to calculate the score of each valid crew.
#' Then, we can solve this question by just changing the objective coefficients of the
#' first problem from 1 to the scores and keeping everything else the same. In this
#' question, we are looking for a matching with maximum total weight.
#' 
#' The full mathematical formulation and detailed explanations are included in the book
#' 'Applications of optimization with Xpress'.
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
  "%s_%s" %>% sprintf(prefix,
                      paste(lapply(names(y), function(name)
                        paste(name, y[name], sep = "_")), collapse = "_"))
}

#' 
#' 
#' ## Question 1
#' 
#' Firstly, we create a new empty problem, set the objective sense as maximization and
#' give the problem a suitable name.
#' 
## ----Create The Problem-------------------------------------------------------
# create a new problem
prob = createprob()

# change this problem to a maximization problem
chgobjsense(prob, objsense = xpress:::OBJ_MAXIMIZE)

# set the problem name
setprobname(prob, "FlightCrew")

#' 
#' 
#' Add the values we need for this example.
#' 
## ----Data---------------------------------------------------------------------
# 1. pilots
PILOT <- data.frame("pilot" = 1:8)
cname <-
  apply(PILOT, 1, function(x)
    name = sprintf("P_%d", x["pilot"]))

# 2. language scores of all the pilots
LANG <- as.data.frame(matrix(c(20, 14, 0,  13, 0,  0,  8,  8,   # English
                               12, 0,  0,  10, 15, 20, 8,  9,   # French
                               0,  20, 12, 0,  8,  11, 14, 12,  # Dutch
                               0,  0,  0,  0,  17, 0,  0,  16), # Norwegian
                             byrow = TRUE, nrow = 4, ncol = 8))
rownames(LANG) <- c("English", "French", "Dutch", "Norwegian")
colnames(LANG) <- cname

# 3. scores of different plane types of all the pilots
PTYPE <- as.data.frame(matrix(c(18, 12, 15, 0,  0,  0,  8,  0,   # Reconnaissance
                                10, 0,  9,  14, 15, 8,  12, 13,  # Transport
                                0,  17, 0,  11, 13, 10, 0,  0,   # Bomber
                                0,  0,  14, 0,  0,  12, 16, 0,   # Fighter-bomber
                                0,  0,  0,  0,  12, 18, 0,  18), # Supply plane
                              byrow = TRUE, nrow = 5, ncol = 8))
rownames(PTYPE) <-
  c("Reconnaissance",
    "Transport",
    "Bomber",
    "Fighter-bomber",
    "Supply plane")
colnames(PTYPE) <- cname


#' 
#' 
#' We need to find all compatible crews.
#' 
## ----Compatible Crews---------------------------------------------------------
CREW <- data.frame()

for (i in 1:(nrow(PILOT) - 1)) {
  for (j in (i + 1):nrow(PILOT)) {
    for (l in 1:nrow(LANG)) {
      if ((LANG[l, i] >= 10 && LANG[l, j] >= 10)) {
        for (p in 1:nrow(PTYPE)) {
          if (PTYPE[p, i] >= 10 && PTYPE[p, j] >= 10) {
            compatible <- c(i, j)
            CREW <- rbind(CREW, compatible)
            break
          }
        }
      }
      if (i == CREW[nrow(CREW), 1] & j == CREW[nrow(CREW), 2]) {
        break
      }
    }
  }
}

colnames(CREW) <- c("Member1", "Member2")

#' 
#' 
#' Add binary variables 'fly' and store their indices in a newly created vector 'fly' in
#' data frame 'CREW'.
#' 
## ----Add Columns--------------------------------------------------------------
# binary variable 'fly' to indicate whether this crew will fly or not
CREW$fly <-
  CREW %>% apply(1, function(x)
    xprs_newcol(
      prob,
      lb = 0,
      ub = 1,
      coltype = "B",
      name = pretty_name("fly", c(x["Member1"], x["Member2"])),
      objcoef = 1
    ))

#' 
#' 
#' Add constraints that ensure every pilot is member of at most one single crew.
#' 
## ----Add Rows-----------------------------------------------------------------
for (i in PILOT$pilot) {
  crew <- CREW %>% filter(Member1 == i | Member2 == i)
  xprs_addrow(
    prob,
    colind = crew$fly,
    rowcoef = rep(1, nrow(crew)),
    rowtype = "L",
    rhs = 1,
    name = sprintf("Pilot_%d_in_1_crew", i)
  )
}

#' 
#' 
#' Now we can solve the first question.
#' 
## ----Solve The First Question-------------------------------------------------
setoutput(prob)
summary(xprs_optimize(prob))

#' 
#' 
#' Display solutions of the first question here.
#' 
## ----Solution Of The First Question-------------------------------------------
CREW$sol_Q1 <- getsolution(prob)$x
print(paste(
  "The maximum number of crews that can fly is:",
  getdblattrib(prob, xpress:::MIPOBJVAL)
))

if (2 * sum(CREW$sol_Q1) == nrow(PILOT)) {
  print("All pilots can fly.")
} else{
  print("Not all pilots can fly.")
}

print("The crews are:")
CREW %>% filter(sol_Q1 == 1) %>% apply(1, function(x)
  paste(x["Member1"], "-", x["Member2"]))

#' 
#' 

#' 
#' Visualize the solutions in a graph, where the red edges represent the crews that can
#' fly.
#' 
## ----Visualize The Solution Of Question 1-------------------------------------
# create the network object
elist <- data.frame(tails = CREW$Member1, heads = CREW$Member2)
graph <- graph_from_data_frame(d = elist, directed = F)

# set the solution edges(crews) in red color
g1.sol <- which(CREW$sol_Q1 == 1)

ecolor1 <- rep("gray", nrow(CREW))
ecolor1[g1.sol] <- rep("red", length(g1.sol))

# plot
plot(
  graph,
  vertex.color = "gold",
  vertex.size = 25,
  vertex.frame.color = "gray",
  vertex.label.color = "black",
  edge.color = ecolor1,
  layout = layout.circle(graph)
)

#' 
#' ## Question 2
#' 
#' To solve question 2, we firstly need to calculate the scores of valid crews.
#' 
## ----Scores-------------------------------------------------------------------
CREW$SCORE <- rep(0, nrow(CREW))

for (i in 1:nrow(CREW)) {
  c1 <- cname[CREW$Member1[i]]
  c2 <- cname[CREW$Member2[i]]

  crew_score <- PTYPE %>% select(all_of(c1), all_of(c2))
  crew_score$sum <- rep(0, nrow(crew_score))

  for (j in 1:nrow(crew_score)) {
    if (crew_score[j, c1] >= 10 && crew_score[j, c2] >= 10) {
      crew_score$sum[j] <- sum(crew_score[j, c1], crew_score[j, c2])
    }
  }

  CREW$SCORE[i] <- max(crew_score$sum)
}


#' 
#' Then based on the first problem, we change the objective coefficients to the scores.
#' 
## ----Change The Objective Coefficients----------------------------------------
chgobj(prob, colind = CREW$fly, objcoef = CREW$SCORE)

#' 
#' 
#' Now we can solve the second question.
#' 
## ----Solve The Second Question------------------------------------------------
setoutput(prob)
summary(xprs_optimize(prob))

#' 
#' 
#' Display the solutions Of the second question here.
#' 
## ----Solution Of The Second Question------------------------------------------
CREW$sol_Q2 <- getsolution(prob)$x
print(paste(
  "The maximum total score is:",
  getdblattrib(prob, xpress:::MIPOBJVAL)
))
print(paste("The set of crews with maximum total score is :"))
CREW %>% filter(sol_Q2 == 1) %>% select(Member1, Member2) %>% apply(1, function(x)
  paste(x["Member1"], "-", x["Member2"]))


#' 
#' Visualize the solutions in a graph, where the red edges represent the crews that can
#' fly and the width of each edge reflects the score of the crew it represents, an edge
#' connecting crew with larger score will be wider and vice versa.
#' 
## ----Visualize The Solution Of Question 2-------------------------------------
# set the solution edges(crews) in red color
g2.sol <- which(CREW$sol_Q2 == 1)
ecolor2 <- rep("gray", nrow(CREW))
ecolor2[g2.sol] <- rep("red", length(g2.sol))

# plot
plot(
  graph,
  vertex.color = "gold",
  vertex.size = 25,
  vertex.frame.color = "gray",
  vertex.label.color = "black",
  edge.width = CREW$SCORE / 10,
  edge.label = CREW$SCORE,
  edge.color = ecolor2,
  layout = layout.circle(graph)
)


#' 
#' 
