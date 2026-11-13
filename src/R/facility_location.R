#####################################
# This file is part of the          #
# Xpress-R interface examples       #
#                                   #
#   (c) 2022-2026 Fair Isaac Corporation #
#####################################
#' ---
#' title: "Facility Location Problem"
#' author: Gregor Hendel
#' date: Dec. 2020
#' ---
#' 

#' 
## ----Loading Libraries--------------------------------------------------------
# load the Libraries
suppressMessages(library(xpress))

# for plotting the data
library(ggplot2)
library(magrittr)
library(reshape2)
library(dplyr)
library(RColorBrewer)

#' 
#' Facility Location is an optimization problem which aims at connecting customers
#' to facilities at minimum costs. Costs in this optimization problem comprise an
#' initial cost for operating a facility, and additionally connection costs between
#' facilities and customers.
#' 
#' The Facility Location Problem (FLP) is a classical application of mixed-integer
#' optimization and comes in many variants. We consider the capacitated facility
#' location problem, in which facilities can only serve customer demands up to a
#' given maximum capacity.
#' 
#' This example is the baseline for many of the quick examples, during which we
#' will read the base model as an LP file and impose various additional
#' constraints.
#' 
#' # Mathematical Formulation
#' 
#' Let's review the mathematical formulation of Capacitated Facility Location. Let
#' $m,n \in \mathbb{N}$ and denote a set of facilities by $F = \{F_1,\dots,F_m\}$
#' and a set of customers by $C = \{C_1,\dots,C_n\}$. Furthermore, we have:
#' 
#' - demands $d_j \geq 0$ for each customer $C_j$.
#' - capacities $u_i \geq 0$ for each facility $F_i$.
#' - opening costs $r_i \geq 0$ for each facility $F_i$,
#' - connection costs $s_{ij} \geq 0$ for connecting facility $F_i$ with customer $C_j$.
#' 
#' With these parameters, we are ready to formulate the FLP:
#' 
#' $$
#' \begin{align}
#'   &\min & \sum\limits_{i = 1}^m r_i x_i + \sum\limits_{i=1,j=1}^n s_{ij}y_{ij}\\
#'   & & \sum\limits_{i = 1}^n y_{ij} & =  d_j & j = 1,\dots,n\\
#'   & & \sum\limits_{j = 1}^n y_{ij} & \leq u_i x_i & i = 1,\dots,m\\
#'   & &                       y_{ij} & \geq 0 & i=1,\dots m, \ j = 1,\dots,n\\
#'   & &                       x_i & \in \{0,1\} & i=1,\dots m\\
#' \end{align}
#' $$
#' We use binary variables $x_i$ to model the yes/no decision
#' whether to open facility $F_i$. Nonnegative continuous variables $y_{ij}$ are
#' used to model how much service customer $C_j$ receives from facility $F_i$. The
#' objective is formulated to incorporate the opening and service costs together.
#' The constraints ensure that the demand of each customer is satisfied, and that
#' no capacity is exceeded.
#' 
#' Next, we turn this into an R model from standard data frames and solve it with
#' the FICO Xpress Optimizer.
#' 
#' # Problem Data Preparation
#' 
## ----Data Preparation---------------------------------------------------------

# six customers in a data frame
customer_df <- data.frame(
                  Demand=c(2,6,4,10,7,4)
               )

# 5 facilities
facility_df <- data.frame(
                  Capacity=c(14,11,6,16,15),
                  Opening=c(3000, 4000,3500,8000,5000)
                )

# some useful shorthand notation
m <- nrow(facility_df)
n <- nrow(customer_df)
mTimesN <- m * n

set.seed(811) # for reproducibility of the below random number generation
connection_df <- data.frame(
                Facility=rep(1:m, each=n),
                Customer=rep(1:n, m),
                Cost=runif(mTimesN, min=1000, max = 2000)
)

#' 
#' We visualize the generated data.
#' 
## ----Visualizing the Generated Data, echo=FALSE-------------------------------
  colors <- RColorBrewer::brewer.pal(10, "RdYlBu")[c(2,10)]
  connection_df %>%
    ggplot(aes(Customer, Facility, fill=Cost, label=format(Cost))) +
    geom_tile() +
    scale_fill_gradient(low = colors[2], high = colors[1]) +
    geom_text() + ggtitle("Heat Map of Connection Cost")

#' 
#' # The R Problem Data
#' 
#' In order to load the model into Xpress at once, we fill a list object `problemdata` which
#' we pass into `xprs_loadproblemdata`.
#' 
#' Each row of the `facility_df` data frame corresponds to one $x$ variable.
#' Each row of the `connection_df` data frame corresponds to one $y$ variable.
#' 
## ----Problem Data, collapse=TRUE----------------------------------------------
# create an empty model as list object
problemdata <- list()

# Objective: opening costs for the x variables, connection costs for the y_ij variables
problemdata$objcoef <- c(facility_df$Opening, connection_df$Cost)

# global entity types: binary for the x variables
#
# an alternative is to specify all column types
# problemdata$columntypes <- c(rep('B', m), rep('C', mTimesN))
problemdata$coltype <- c(rep('B', m))

# the coltype approach requires us to specify the indices
# of the binary variables explicitly. We have to use 0-based
# indexing for Xpress!
problemdata$entind <- 0:(m-1)

# demand constraints: 1 constraint for each customer
demand_matrix_transposed <- sapply(1:n, function(x) c(
  rep(0,m),
  connection_df$Customer == x))

# capacity constraint on each facility
capacity_matrix_transposed <- sapply(1:m, function(x)
  c(ifelse(
    (1:m)==x, -facility_df$Capacity[x], 0
    ),
    connection_df$Facility==x)
  )

# we transpose and glue both matrices together into the final A
problemdata$A <- rbind(
  t(demand_matrix_transposed),
  t(capacity_matrix_transposed)
)

# right hand side declaration: customer demand for customer or 0
problemdata$rhs <- c(
  customer_df$Demand,
  rep(0, m)
  )

# lower and upper bounds on the variables
problemdata$lb=rep(0,m+mTimesN)
problemdata$ub=c(rep(1,m), rep(1e+20, mTimesN))

# sense of the rows:
#'E'quations,
#'L'ess/'G'reater than (<=/>=),
#'R'ange rows, 'N'onbinding (neutral) rows
problemdata$rowtype <- c(
  rep("E", n), # n equalities to satisfy demands
  rep("L", m)  # m inequalities not to exceed capacity
)

# declare column names
problemdata$colname <- c(
  sprintf("x_%d", 1:m),
  sprintf("y_%d_%d", connection_df$Facility, connection_df$Customer)
)

# declare row names
problemdata$rowname <- c(
  sprintf("Demand_%d", 1:n),
  sprintf("Capacity_%d", 1:m)
)

#' 
#' Let's have a graphical look at the matrix to verify that we made no errors. For
#' a better display, we have flipped columns onto the vertical axis. It appears
#' that we translated the matrix correctly. Remember that variable $y_{35}$ models
#' the amount of service between facility $F_3$ and customer $C_5$ As expected
#' $y_{35}$ appears in 2 constraints with a coefficient of 1, namely the capacity
#' constraint of $F_3$, and the demand constraint of $C_5$.
#' 
## ----echo=FALSE---------------------------------------------------------------
a_slim <- reshape2::melt(problemdata$A)
a_slim$rowname <- problemdata$rowname[a_slim$Var1]
a_slim$colname <- problemdata$colname[a_slim$Var2]
a_slim %>% dplyr::filter(value != 0) %>%
  ggplot(aes(rowname, colname, fill=value, label=value)) +
  geom_raster() + geom_text() +
  scale_fill_gradient2(midpoint = 0, mid="white") +
  theme_bw() +
  theme(axis.text.x = element_text(size = 5, angle = 30),
        axis.text.y = element_text(size = 6))

#' 
#' # Solving the Model with FICO Xpress
#' 
#' It is now very easy to input and solve this model using FICO Xpress.
#' 
## ----Solving The Problem------------------------------------------------------
problemdata$probname <- "FacilityLocation"

# load the problem into a newly created problem
p <- xprs_loadproblemdata(problemdata=problemdata)

# enable output to console
setoutput(p)

# save the problem in an LP file
writeprob(p, "flp.lp", "l")

# call optimize
summary(xprs_optimize(p))

#' 
## ----Displaying the Solution--------------------------------------------------
# get solution and split it into facility and connection
sol <- getsolution(p)$x
facility_sol <- sol[1:m]
connection_sol <- sol[m+(1:mTimesN)]

# print the open facilities:
print("Open facilities:")
print(sprintf("F_%d", which(facility_sol > 0)))

# print the connections:
print("Connections:")
connection_df$TotalCost <- connection_df$Cost * connection_sol

# use a slight numeric tolerance to filter the actual connections
print(connection_df[connection_sol > 1e-9,])

