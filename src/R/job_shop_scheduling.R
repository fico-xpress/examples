#####################################
# This file is part of the          #
# Xpress-R interface examples       #
#                                   #
#   (c) 2022-2026 Fair Isaac Corporation #
#####################################
#' ---
#' title: " Job Shop Scheduling "
#' author: Y.Gu
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
#' [This is a conversion of the Mosel example 'Job Shop Scheduling'](https://www.fico.com/fico-xpress-optimization/docs/latest/examples/mosel/ApplBook/B_Sched/b3jobshop2.mos).
#' A brief introduction to this problem is given below, and to see the full
#' mathematical modeling of this problem you may refer to section 7.3, page 90 of the
#' book 'Applications of optimization with Xpress'.
#' 
#' Three types of wallpapers are required to be produced and each of
#' them has a unique design of background and patterns. There are three machines and
#' each of them prints a different color. The wallpapers will be produced as a continuous
#' roll of paper passing through these machines and the intrinsic ordering in which the
#' papers are run through the machines depends on the design of the paper.
#' The table below shows the order of colors for each paper:
#' 
#' | Paper | Paper 1 | Paper 2 | Paper 3 |
#' |:-----:|:--------:|:---------:|:---------:|
#' |   Blue    | 1  |     2       |    2      |
#' |   Green   |    |     1       |    3      |
#' |   Yellow  | 2  |     3       |    1      |
#' 
#' For example, paper 1 has blue background with yellow patterns, so it should firstly pass
#' through the machine printing blue and then the machine printing yellow.
#' 
#' The following table shows the required time for each job, the
#' sequence numbers of jobs do not include ordering information and are just for labeling:
#' 
#' | Machine | Color | Paper 1 | Paper 2 | Paper 3 |
#' |:-------:|:-------:|:---------:|:---------:|:---------:|
#' |   1    |  Blue  | 45 (job1) | 20 (job2)   |   12 (job3)   |
#' |   2    |  Green |           | 10 (job4)   |   17 (job5)   |
#' |   3    | Yellow | 10 (job6) | 34 (job7)   |   28 (job8)   |
#' 
#' It is also known that every machine can only process one wallpaper at a time and
#' that a paper cannot be processed by several machines simultaneously.
#' 
#' The objective we want to minimize is simply the completion time of all jobs and we
#' denote this time as 'finish'. We use the sum of all processing time as a safe upper
#' bound for 'finish' and denote this upper bound as big-M ('BM'). The variables beside
#' 'finish' are starting times of each job.
#' 
#' The first constraint is obviously that each job should finish before 'finish' time.
#' Then, there are another two types of constraints between processing operations: the
#' conjunctive constraints represent the precedences between the operations for a single
#' paper type, and the disjunctive constraints express the fact that a machine can only
#' execute a single operation at a time. The mathematical formulations of these constraints
#' are included in the guide book 'Applications of optimization with Xpress'.
#' 
#' After solving this problem, we can represent the solution as a 'Gantt Chart' graph.
#' 
#' 
#' For this example, we need packages 'xpress', 'Matrix', 'dplyr', 'ggplot2' and
#' 'ggthemes'.
#' 
## ----Load The Packages--------------------------------------------------------
library(xpress)
library(Matrix)
library(dplyr)
library(ggplot2)
library(ggthemes)

#' 
#' 
#' Create a new empty problem and give the problem a suitable name.
#' 
## ----Create The Problem-------------------------------------------------------
# firstly, create a new empty problem
prob <- createprob()

# set the problem name
setprobname(prob, "JobShopScheduling")

#' 
#' 
#' Add the values we need for this example.
#' 
## ----Data---------------------------------------------------------------------
tasks.df <- as.data.frame(
  matrix(
    c(# JOB #ORDER #PAPER #DUR #MACHINE
      1,    1,     1,     45,   1,
      2,    2,     2,     20,   1,
      3,    2,     3,     12,   1,
      4,    1,     2,     10,   2,
      5,    3,     3,     17,   2,
      6,    2,     1,     10,   3,
      7,    3,     2,     34,   3,
      8,    1,     3,     28,   3
    ), byrow = T, ncol = 5
  ),
) %>% rename(
  Job = V1,
  Order = V2,
  Paper = V3,
  Dur = V4,
  Machine = V5
)

# big-M
BM <- sum(tasks.df[, "Dur"])

papers <- unique(tasks.df[, "Paper"])

#' 
#' 
#' Create variables 'finish' and 'start' for each job.
#' 
## ----Add Columns--------------------------------------------------------------
# add decision variables:

# 1.'finish',the completion time of all jobs
finish <-
  xprs_newcol(
    prob,
    lb = 0,
    ub = BM,
    coltype = 'I',
    name = "finish",
    objcoef = 1
  )

# 2. 'start' for each job in JOBS: start time of each job
#    create a vector 'start' in 'tasks.df' to store their indices
tasks.df$start <- tasks.df %>%
  apply(1, function(x)
    xprs_newcol(
      prob,
      lb = 0,
      ub = BM,
      coltype = "I",
      name = paste0("start_", paste(x["Job"], collapse = "_"))
    ))


#' 
#' 
#' Add the constraints mentioned in introduction.
#' 
## ----Add Rows, results='hide'-------------------------------------------------
# 1. every job should be done before the 'finish' time
apply(tasks.df, 1, function(x)
  xprs_newrow(
    prob,
    colind = c(x["start"], finish),
    rowcoef = c(1, -1),
    rowtype = "L",
    rhs = -x["Dur"],
    name = sprintf("finish_%d", x["start"])
  ))


# 2. precedence constraints
ARC_lst <-
  tasks.df %>% arrange(Order) %>% group_by(Paper) %>% group_map( ~ .x)

for (j in papers) {
  arclst <- ARC_lst[[j]]
  for (i in 1:(nrow(arclst) - 1)) {
    xprs_addrow(
      prob,
      colind = c(arclst$start[i], arclst$start[i + 1]),
      rowcoef = c(1, -1),
      rowtype = "L",
      rhs = -arclst$Dur[i],
      name = sprintf("precedence_%d_%d", j, i)
    )
  }
}


# 3. disjunctive constraints
DISJ <- data.frame(Job1 = c(), Job2 = c())
for (m in unique(tasks.df$Machine)) {
  jobs_on_machine_m <- tasks.df[tasks.df$Machine == m, "Job"]
  job_pairs <- utils::combn(jobs_on_machine_m, 2)
  job_pairs_transposed <- job_pairs %>% t()
  colnames(job_pairs_transposed) <- c("Job1", "Job2")
  DISJ <- rbind(DISJ, job_pairs_transposed)
}

colnames(DISJ) <- c("Job1", "Job2")

DISJ$label <- 1:nrow(DISJ)
DISJ$y <-
  DISJ %>% apply(1, function(x)
    xprs_newcol(
      prob,
      lb = 0,
      ub = 1,
      coltype = 'B',
      name =
        sprintf("y_%d", x["label"]),
      objcoef = NULL
    ))


for (i in 1:nrow(DISJ)) {
  job1 <- DISJ$Job1[i]
  job2 <- DISJ$Job2[i]
  
  # start_1 + Dur_1 <= start_2 + BM * y_i
  xprs_addrow(
    prob,
    rowtype = "L",
    rhs = -tasks.df$Dur[job1],
    name = sprintf("disjunctive1_%d_%d", job1, job2),
    colind = c(tasks.df$start[job1], tasks.df$start[job2], DISJ$y[i]),
    rowcoef = c(1, -1, -BM)
  )
  
  # start_2 + Dur_2 <= start_1 + BM * (1 - y_i)
  xprs_addrow(
    prob,
    rowtype = "L",
    rhs = BM - tasks.df$Dur[job2],
    name = sprintf("disjunctive2_%d_%d", job1, job2),
    colind = c(tasks.df$start[job2], tasks.df$start[job1], DISJ$y[i]),
    rowcoef = c(1, -1, BM)
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
# Note that the column indices are 0-based while the vector indices in R are 1-based,
# so, to get the corresponding solutions, we add '1' to column indices

# 1. optimal objective value
optimal.value <- getdblattrib(prob, xpress:::MIPOBJVAL)
print(paste("The optimal finish time is ", optimal.value))

# 2. start time of each job
tasks.df$solval <- getsolution(prob)$x[tasks.df$start + 1]
invisible(apply(tasks.df, 1, function(x)
  cat("The start time of job ", x["Job"], " is ", x["solval"], "\n")))

# 3. finish time of each job
tasks.df$finishsol <-
  getsolution(prob)$x[tasks.df$start + 1] + tasks.df$Dur
invisible(apply(tasks.df, 1, function(x)
  cat("The finish time of job ", x["Job"], " is ", x["finishsol"], "\n")))

#' 
#' 
#' We can represent the solutions of this problem as a bar chart, also called 'Gantt chart'.
#' 
## ----Visualize The Solutions--------------------------------------------------
tasks.df$Machine[which(tasks.df$Machine == 1)] = "Blue"
tasks.df$Machine[which(tasks.df$Machine == 2)] = "Green"
tasks.df$Machine[which(tasks.df$Machine == 3)] = "Yellow"

jobshop.solution <- data.frame(
  colour = as.factor(tasks.df$Machine),
  start = as.factor(tasks.df$solval),
  end = as.factor(tasks.df$finishsol),
  paper_type = as.factor(tasks.df$Paper)
)

ggplot(jobshop.solution,
       aes(
         x = start,
         xend = end,
         y = colour,
         yend = colour,
         color = paper_type
       )) +
  theme_bw() +
  geom_segment(size = 8) +
  labs(title = 'Job Shop Solution', x = 'Time', y = 'Machine') +
  scale_colour_manual(values = c('pink', 'purple', 'blue')) +
  theme_economist() +
  theme(axis.title = element_text())


#' 
#' 
