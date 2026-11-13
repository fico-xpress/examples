#####################################
# This file is part of the          #
# Xpress-R interface examples       #
#                                   #
#   (c) 2022-2026 Fair Isaac Corporation #
#####################################
#' ---
#' title: "Solving Sudoku Puzzles"
#' ---
#' 
## ----setup, include=FALSE-----------------------------------------------------
knitr::opts_chunk$set(echo = TRUE)
knitr::opts_chunk$set(results = "hold")

#' 
#' We formulate and solve Sudoku puzzles as binary optimization problems. This
#' example is inspired by the [Sudoku
#' example](http://roi.r-forge.r-project.org/use_case_sudoku.html) of the ROI
#' package in R, but also by the Sudoku enthusiasm of the author.
#' 
#' First, we will create a binary formulation inside a function,
#' `load_sudoku_model`, where we first load the general model. We fix the initial
#' assignment by modifying the lower bounds of variables for which we are given the
#' solution.
#' 
#' Secondly, we will extend the classical Sudoku by further constraints
#' that forbid the same element along diagonals.
#' 
#' Thirdly, we will extend the model further to incorporate a magic square and
#' knight's move constraints to solve [this
#' puzzle]((https://www.youtube.com/watch?v=hAyZ9K2EBF0)) with the FICO Xpress
#' Optimizer.
#' 
#' For all this, we use the `sudokuAlt` package.
#' 
## ----Load packages------------------------------------------------------------
# load the xpress library
suppressMessages(library(xpress))

# load the Sudoku package
library(sudokuAlt)

# for data transformations
library(dplyr)
set.seed(123)
game <- makeGame()
plot(game)

#' 
#' # Mathematical Formulation
#' 
#' A classical Sudoku puzzle is formulated in a 9x9 grid of cells,
#' where the cells are subdivided into 3x3 subcells, the so-called "squares".
#' The goal is to fill all cells with the numbers 1 through 9, such
#' that each number appears only once in
#' 
#' - each cell
#' - each row
#' - each square
#' 
#' Let $R = \{1,\dots, 9\}$,
#' $C = \{1,\dots, 9\}$, and $S = \{S_{11}, S_{12}, \dots, S_{33}\}$,
#' where each $S_{ij}$ denotes a 9-element subset of $R\times C$,
#' e.g., $S_{13} = \{1,2,3\} \times \{7,8,9\}$ is the top right square of the Sudoku.
#' 
#' In general
#' $$
#'   S_{ij} := \{3i - 2, 3i - 1, 3i\} \times \{3j - 2, 3j - 1, 3j\}  \quad \forall i,j \in \{1,2,3\}
#' $$
#' Let $L = \{1,\dots, 9\}$ denote the allowed symbols.
#' Now we are ready to formulate the binary program that encodes
#' Sudoku using 9 binary variables per cell,
#' 1 for each symbol. If $x_{rcl} = 1$, the symbol $l$ should
#' be placed in cell $(r,c)$ of the Sudoku board.
#' 
#' Let $\emptyset \neq V_0 \subseteq R\times C \times L$
#' denote the given set of nonconflicting assignments.
#' 
#' $$
#' \begin{align}
#'   &\min & 0\\
#'   & & \sum\limits_{l = 1}^9 x_{rcl} & =  1 & \forall r\in R, c \in C\\
#'   & & \sum\limits_{r = 1}^9 x_{rcl} & =  1 & \forall c \in C, l \in L\\
#'   & & \sum\limits_{c = 1}^9 x_{rcl} & =  1 & \forall r \in R, l \in L\\
#'   & & \sum\limits_{(r,c) \in S_{ij}} x_{rcl} & =  1 & \forall i,j\in \{1,2,3\}, l \in L\\
#'   & &                       x_{rcl} & = 1 & \forall (r,c,l) \in V_0\\
#'   & &                       x_{rcl} & \in \{0,1\} & \forall r \in R, c \in C, l \in L\\
#' \end{align}
#' $$
#' 
#' In this case, we have no objective function. There are five sets of constraints. The first constraint group ensures that each cell contains exactly 1 symbol. The next three model the restriction that each symbol should appear exactly once
#' in each column, row, and square, respectively.
#' The last set of constraints fixes the initial assignment. Note that with an additional assignment, there are further variables that could be fixed to 0.
#' We do not add those restrictions to 0 to the model itself, because these additional fixings can be inferred immediately from the
#' initial assignments during presolving by the Optimizer.
#' 
#' 
#' # The Load Sudoku Function
#' 
#' We declare a function `load_sudoku_problem` that takes as input an
#' XPRSprob object (which can be NULL, in which case we create a new one),
#' and a Sudoku game.
#' 
## -----------------------------------------------------------------------------
#' loads a sudoku problem into the FICO Xpress Optimizer
#'
#' @param prob an existing XPRSprob or NULL, in which case we create a new problem
#' @param sudoku_game  a Sudoku Game created via sudokuAlt::makeGame()
#'
#' @return list object into which the Sudoku Game formulation was loaded.
#'        This list has two named attributes:
#'          -  \code{prob} is the same \code{prob} if \code{prob} was non-NULL,
#'            or a newly created XPRSprob object
#'          - \code{index.df} The index data frame that contains the initial assignment
#'            and variable indices for each row/column/label triplet r,c,l.
#'
load_sudoku_problem <- function(prob=NULL, sudoku_game) {

  # create a new prob object if none was specified
  if (is.null(prob)) {prob <- createprob()}

  problemdata <- list()

  # create some sets to match the notation above
  R <- 1:9
  C <- 1:9
  L <- 1:9

  # create a convenient data frame that contains all combinations
  # of R,C,and L to simplify indexing
  index.df <- expand.grid(R,C,L)
  names(index.df) <- c("R", "C", "L")


  # infer the subcell for each row/column combination.
  index.df$Si <- (index.df$R - 1) %/% 3 + 1
  index.df$Sj <- (index.df$C - 1) %/% 3 + 1

  # we introduce var index to label each of the 9 * 9 * 9 = 729 binary variables
  n <- nrow(index.df)
  index.df$VarIndex <- 1:n

  # We use the designGame function to query a data frame representation of the Sudoku.
  game.df <- sudokuAlt::designGame(sudoku_game)
  # head of typical game.df. Unassigned symbols show as LNA.
  #     Row Col Square Symbol
  # 1  R3  C3    S11     L9
  # 2  R1  C6    S12     L6
  # 3  R2  C4    S12     L4
  # 4  R1  C7    S13     L5
  # 5  R1  C8    S13     L7
  # 6  R3  C8    S13     L2
  # We convert the game.df and merge the symbols
  game.df <- game.df[game.df$Symbol != "LNA",]
  game.df$R <- as.integer(gsub("R", "", game.df$Row))
  game.df$C <- as.integer(gsub("C", "", game.df$Col))
  game.df$Si <- as.integer(gsub("S(.).", "\\1", game.df$Square))
  game.df$Sj <- as.integer(gsub("S.(.)", "\\1", game.df$Square))
  index.df <- merge(index.df, game.df, all.x = T)
  index.df <- index.df[order(index.df$VarIndex),]

  #the number of constraints: 9 * 9 * 4 (324)
  nconss <- 9 * 9 * 4

  problemdata$objcoef <- rep(0, n)

  # We formulate the constraints as 4 sparse column matrices
  # Each constraint group arranges the variables differently,
  # namely across a major and a minor index.
  to_sparse_matrix <- function(major, minor) {
    considx <- 9 * (major - 1) + minor
    sparse_matrix <- Matrix::Matrix(0, 9*9, n, sparse = T)
    idxmatrix <- matrix(c(considx,1:n), ncol = 2)
    sparse_matrix[idxmatrix] <- 1
    sparse_matrix
  }

  # 1 symbol in each cell: Constraint index determined by row and column
  cell.cons.mat <- to_sparse_matrix(index.df$R, index.df$C)

  # each symbol only once per column: Constraint index determined by column and Label
  col.cons.mat <- to_sparse_matrix(index.df$C, index.df$L)

  # each symbol only once per row: Constraint index determined by row and Label
  row.cons.mat <- to_sparse_matrix(index.df$R, index.df$L)
  #
  # each symbol only once in each square:
  # Constraint index determined by label and index of square
  # use the square indices Si and Sj to compute the minor indices ranging from 1:9
  square.cons.mat <- to_sparse_matrix(index.df$L, 3*(index.df$Si - 1) + index.df$Sj)

  problemdata$A <- rbind(
    cell.cons.mat,
    col.cons.mat,
    row.cons.mat,
    square.cons.mat
  )

  # right hand sides. we only treat the cardinality constraint here.
  # Initial assignment is realized later via bounds on variables.
  problemdata$rhs <- rep(1, nconss)
  # declare all constraints as equations
  problemdata$rowtype <- rep("E", nconss)

  # all upper bounds are 1
  problemdata$ub <- rep(1, n)

  # fix the initial assignment by setting variable lower bounds
  initial.assignment <- which(sprintf("L%d", index.df$L) == index.df$Symbol)

  problemdata$lb <- rep(0, n)
  problemdata$lb[initial.assignment] <- 1

  # all columns are binary
  problemdata$columntypes <- rep("B", n)

  # assign column names to identify the variables when writing the problem to a file
  problemdata$colname <- sprintf("x_R%dC%dL%d", index.df$R, index.df$C, index.df$L)

  problemdata$probname <- "Sudoku"
  # this call also returns prob from the function
  list(prob=xpress::xprs_loadproblemdata(prob, problemdata=problemdata),
       index.df=index.df
  )
}

#' 
#' 
#' # Solving a Sudoku with Xpress
#' 
#' We call `load_sudoku_problem` on the game that we created at the beginning
#' of this tutorial.
#' 
#' The problem is ready to be solved. We enable console output and
#' solve the Sudoku using `mipoptimize`.
#' 
## ----solve--------------------------------------------------------------------
problist <- load_sudoku_problem(sudoku_game = game)
prob <- problist$prob

print(prob)
setoutput(prob)
mipoptimize(prob)

#' 
#' # Plot the Result
#' 
#' We want to convert the binary solution into a 9-by-9 matrix containing the
#' entries 1--9.
#' 
#' Therefore, we use the index data frame. Recall that each row $i$ of this data
#' frame corresponds to the $i$'th column of the Sudoku model.
#' We filter those index rows for which the corresponding binary variable
#' has a solution value of 1.
#' 
#' We use the dplyr function arrange to sort the filtered entries by row and column.
#' The corresponding symbols are then used to fill a matrix representation
#' of the binary solution.
#' 
#' Finally, we use the plot functionality of the `sudokuAlt` package
#' to display the solution.
#' 
## ----plottheresult------------------------------------------------------------
# translate the MIP solution into a solution of the game:
solution <- getmipsol(prob)$x

# nice trick to enable Sudoku style plotting from ROI example:
to_sudoku_solution <- function(solution, index.df) {
  rowbyrowsol <- index.df %>%
    dplyr::filter(near(solution,1)==TRUE) %>%
    dplyr::arrange(R, C) %>%
    dplyr::select(L)
  matrix_solution <- matrix(rowbyrowsol$L, 9,9, byrow = T)
  sudoku_solution <- structure(matrix_solution, class = c("sudoku", "matrix"))
  sudoku_solution
}

sudoku_solution <- to_sudoku_solution(solution, problist$index.df)
par(mfrow = c(1,2))
plot(game)
plot(sudoku_solution)

#' 
#' # Add Diagonals
#' 
#' A Sudoku with diagonals constrains the symbols along the two main diagonals.
#' The main diagonal $D^+$ is characterized as all cells
#' $(r,c)\in R \times C$ where $r = c$.
#' The antidiagonal $D^-$ is characterized as $(r,c) \in R \times C$ such that $r+c = 10$.
#' 
#' 
#' ## Mathematical notation
#' 
#' The $18$ additional constraints for the diagonal and anti diagonal, using the same variable notation as before, then read
#' 
#' $$
#' \begin{align}
#'   & & \sum\limits_{r=c} x_{rcl} & =  1 & \forall l \in L\tag{D+}\\
#'   & & \sum\limits_{r+c=10} x_{rcl} & =  1 & \forall l \in L\tag{D-}\\
#' \end{align}
#' $$
#' 
#' 
#' ## Game Creation
#' 
#' In order to use these diagonal constraints,
#' we first create a new game with fewer initial assignments.
#' The default has about 20 initial assignments.
#' Together with diagonal constraints, this may result in an infeasible MIP.
#' 
## ----diagonals_create_game----------------------------------------------------

# create a game with slightly less initial assignments than before.
sudoku_game <- sudokuAlt::makeGame(gaps = 68)

# load it using our function.
problist <- load_sudoku_problem(sudoku_game = sudoku_game)
probdiag <- problist$prob
index.df <- problist$index.df

print(probdiag)
setoutput(probdiag)

#' 
#' ## Main Diagonal
#' 
#' We add the constraints for the main diagonal via `addrows`.
#' The main diagonal consists of all cells whose
#' row and column index are equal.
#' The sorting via `arrange` ensures the correct grouping of
#' all variables that belong to 1 symbol
#' required by `addrows`.
#' 
#' Remember that the arguments `start` and `colind` to addrows must be zero-based.
#' In addition, `start` has length 10 (`nrows + 1`), where the last entry contains the total
#' number of elements.
#' 
## ----adding_main_diagonal-----------------------------------------------------
add_main_diagonal <- function(prob, index.df) {
  diagonal.df <- index.df %>%
    dplyr::filter(R == C) %>%
    dplyr::arrange(L)

  prob <- addrows(
    prob = prob,
    rowtype = rep("E", 9),
    start = c((9 * (0:8)), 81),
    rhs = rep(1, 9),
    colind = (diagonal.df$VarIndex) - 1,
    rowcoef = rep(1,81)
  )

  prob
}

#' 
#' ## Anti Diagonal
#' 
#' In contrast to the diagonal, only the filter condition in the below selection
#' is modified for the anti diagonal.
#' 
## ----adding_antidiagonal------------------------------------------------------

add_anti_diagonal <- function(prob, index.df) {
  antidiagonal.df <- index.df %>%
    filter(R + C == 10) %>%
    dplyr::arrange(L)

  # note that only the data frame that provides column indices changes between the two calls to addrows().
  prob <- addrows(
    prob = prob,
    rowtype = rep("E", 9),
    start = c((9L * (0:8)), 81),
    rhs = rep(1, 9),
    colind = (antidiagonal.df$VarIndex) - 1,
    rowcoef = rep(1,81)
  )

  prob
}

#' 
#' 
#' ## Solve with Diagonal Constraints
#' 
#' We invoke the solution process and display the solution as before. Note that this time, the solution process is no longer trivial,
#' as the Branch-and-Bound search actually has to explore a small search tree for finding a solution.
#' 
#' As for the simple Sudoku without diagonal constraints,we use the convenience
#' function to `to_sudoku_solution` to convert the binary representation
#' obtained from the Optimizer into a 9-by-9 matrix representation
#' for plotting.
#' 
## ----solve_with_diagonals-----------------------------------------------------
probdiag <- add_main_diagonal(probdiag, index.df)
probdiag <- add_anti_diagonal(probdiag, index.df)
mipoptimize(probdiag)
summary(probdiag)
solution <- getmipsol(probdiag)$x

sudoku_solution <- to_sudoku_solution(solution, index.df)
par(mfrow = c(1,2))
plot(sudoku_game)
plot(sudoku_solution)

#' 
#' 
#' # Solving a Sudoku with Only 4 Digits
#' 
#' Finally, we turn our attention towards the following masterpiece
#' of Sudoku puzzles. In his Youtube channel
#' [Cracking the Cryptic](https://www.youtube.com/watch?v=hAyZ9K2EBF0),
#' Simon Anthony solves a Sudoku with only 4 given digits. We recommend
#' watching this video.
#' 
#' The rules that apply to this Sudoku are as follows.
#' 
#' * Classical rules (each digit once per row, column, and square)
#' * Diagonals, see Section above
#' * The central square must also form a *magic square*, meaning that
#'   the digits in each row and column of the central square must sum up
#'   to the same number.
#' * Cells that are a knight's move apart (in chess) must not contain the same digit.
#' 
#' We would like to solve this puzzle using Xpress. We have already established
#' the building blocks for the classical rules (via `load_sudoku_problem`)
#' and for the diagonals.
#' 
#' How do we model a magic square? The contribution of a single digit in cell
#' $r,c$ can be expressed as $\sum_{l=1}^{9} l \cdot x_{rcl}$ because
#' we know that exactly one of the 9 binary variables is set to 1.
#' 
#' The digit sum in the top row of the central square (row 4) can then be expressed
#' as $\sum\limits_{c=4}^{6}\sum\limits_{l=1}^{9} l \cdot x_{4cl}$.
#' Since the total sum of all 3 rows (columns) in the central square must be equal to 45,
#' the digits in each row (column) must sum up to 15.
#' 
#' We arrive at the following additional magic square constraints:
#' 
#' $$
#' \begin{align}
#'   & & \sum\limits_{c=4}^{6}\sum\limits_{l=1}^{9} l \cdot x_{rcl} & =  15 & \forall r \in \{4,5,6\}\\
#'   & & \sum\limits_{r=4}^{6}\sum\limits_{l=1}^{9} l \cdot x_{rcl} & =  15 & \forall c \in \{4,5,6\}\\
#' \end{align}
#' $$
#' We declare a function that receives a prob and an index data frame
#' and adds the 6 magic square constraints.
#' 
## ----Adding a Magic Square----------------------------------------------------
add_magic_square <- function(prob, index.df) {
  # consider only the central square S_22
  central.square.df <- index.df %>% filter(Si == 2) %>% filter(Sj == 2)

  # sort by row
  central.square.df.by.row <- central.square.df %>% arrange(R)
  # add the constraints for each row
  prob <- addrows(prob, rowtype = c("E", "E", "E"), rhs = c(15,15,15), start = c(0,27,54,81),
          colind = central.square.df.by.row$VarIndex - 1,
          rowcoef = central.square.df.by.row$L)

  # sort by column
  central.square.df.by.col <- central.square.df %>% arrange(C)
  # add the constraints for each column
  prob <- addrows(prob, rowtype = c("E", "E", "E"), rhs = c(15,15,15), start = c(0,27,54,81),
          colind = central.square.df.by.col$VarIndex - 1,
          rowcoef = central.square.df.by.col$L)

  # return prob with the added constraints
  prob
}

#' 
#' 
#' Consider the cell $r,c$ where $r \in R$ and $c \in C$. A knight in
#' chess may move by either 2 horizontal and 1 vertical
#' or 1 horizontal and 2 vertical steps. Denote by $K(r,c)\subset R\times C$ the neighbors
#' of a cell $r,c$ with respect to the knight's move.
#' 
#' We have $K(r,c)$ consisting of up to eight neighboring cells of the form
#' 
#' $$
#' K(r,c) = (\{(r \pm 2, c \pm 1)\} \cup \{(r \pm 1, c \pm 2)\}) \cap R\times C
#' $$
#' 
#' With this definition, we formulate the knight's move constraints compactly as follows:
#' 
#' $$
#' \begin{align}
#'   & & x_{rcl} + x_{r'c'l} & \leq  1 & \forall r,c,l \in R\times C\times L, (r',c') \in K(r,c)\\
#' \end{align}
#' $$
## ----Adding Knight\'s Move Constraints----------------------------------------
get_knights_move_neighbors <- function(r,c) {
  # create all possible moves, possibly outside the feasible range

  # note that this traversal will encounter each pair of conflicting variables x,y
  # twice. The first time when all neighboring cells of x are traversed,
  # and a second time when those around y are determined.
  # The resulting duplicate rows are detected by Xpress in presolving
  neighbors <- matrix(c(
           r - 2, c - 1,
           r + 2, c - 1,
           r - 2, c + 1,
           r + 2, c + 1,
           r - 1, c - 2,
           r + 1, c - 2,
           r - 1, c + 2,
           r + 1, c + 2
           ), byrow = T, ncol=2)

  # filter elements outside the feasible range and return
  neighbors[(neighbors[,1]>=1) & (neighbors[,1]<=9)  & (neighbors[,2]<=9)   & (neighbors[,2]>=1),]
}

# new we can create the knight's move constraints
add_knights_move_constraints <- function(prob, index.df) {
  # one additional constraint for each r,c,l

  # we establish the R,C,L order in index.df.sorted.
  # An element r,c,l has the index 81 * (r - 1) + 9 * (c - 1) + l
  index.df.sorted <- index.df %>% arrange(R,C,L)

  for (row in 1:9) {
    for (col in 1:9) {
      neighbors <- get_knights_move_neighbors(row, col)

      # add constraints that forbid the same symbol in the neighbors and this cell
      # one for each symbol
      for (symbol in 1:9) {
        cellvaridx <- index.df.sorted$VarIndex[81 * (row - 1) + 9 * (col - 1) + symbol]
        neighborvarindex <- index.df.sorted$VarIndex[81 * (neighbors[,1] - 1) + 9 * (neighbors[,2] - 1) + symbol]

        # add one row per neighbor
        for (n in neighborvarindex) {
          prob <- addrows(prob, rowtype = "L", rhs = 1, start = c(0,2),
                          colind = c(cellvaridx, n) - 1,
                          rowcoef = c(1,1))
        }
      }
    }
  }

  prob
}

#' 
#' We are ready to create the game from Anthony's video. First, we have to create the game by hand.
#' 
## -----------------------------------------------------------------------------
# Make a fresh game and override the given symbols by the four digits from the video.
game4digits <- makeGame()
for (i in 1:9) for (j in 1:9) game4digits[i,j] <- NA
game4digits[4,1] <- 3; game4digits[4,2] <- 8; game4digits[4,3] <- 4; game4digits[9,9] <- 2
plot(game4digits)

#' 
#' Now we load all the constraints into a fresh `XPRSprob` object.
#' 
## ----Load all constraints-----------------------------------------------------
problist <- load_sudoku_problem(prob = createprob(), sudoku_game = game4digits)
prob4digits <- problist$prob

# add diagonals
prob4digits <- add_main_diagonal(prob4digits, problist$index.df) %>%
               add_anti_diagonal(problist$index.df) %>%
# add magic square
               add_magic_square(problist$index.df) %>%
# add knight's move
              add_knights_move_constraints(problist$index.df)

setoutput(prob4digits)
summary(mipoptimize(prob4digits))

#' 
#' Here is the solution together with the original game.
#' 
## ----Plot the solution alongside the game-------------------------------------
solution <- getmipsol(prob4digits)$x

solution4digits <- to_sudoku_solution(solution, problist$index.df)
par(mfrow = c(1,2))
plot(game4digits)
plot(solution4digits)

#' 
#' 

#' 
