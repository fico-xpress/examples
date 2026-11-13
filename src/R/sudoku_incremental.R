#####################################
# This file is part of the          #
# Xpress-R interface examples       #
#                                   #
#   (c) 2022-2026 Fair Isaac Corporation #
#####################################
#' ---
#' title: "Incremental Formulation of Sudoku Puzzles"
#' author: Gregor Hendel, Y. Gu
#' date: Jun. 2021
#' ---
#' 
## ----setup, include=FALSE-----------------------------------------------------

knitr::opts_chunk$set(echo = TRUE)
knitr::opts_chunk$set(results = "hold")
knitr::opts_chunk$set(warning = FALSE,message = FALSE)


#' 
#' This example shows an incremental formulation of the same sudoku games as in the example file
#' 'sudoku.R', Here, we replace the single call to `xprs_loadproblemdata` to create all rows/columns at once by an incremental approach using `xprs_addrow` and `xprs_newcol`.
#' 
#' 
#' # Load packages and a function for pretty names
#' 
## -----------------------------------------------------------------------------
# load the xpress library
suppressMessages(library(xpress))

# load the Sudoku package
library(sudokuAlt)

# for data transformations
library(dplyr)


# a function to specify good names
pretty_name <- function(prefix, y) {
  "%s_%s" %>% sprintf(prefix,
                      paste(lapply(names(y), function(name)
                        paste(name, y[name], sep = "_")), collapse = "_"))
}


#' 
#' 
#' # Plain Sudoku Game
#' 
#' Firstly, we work with the plain sudoku game, where 4 basic types of restrictions are
#' imposed. The mathematical formulation of the plain sudoku game is:
#' 
#'  $$
#'  \begin{align}
#'    &\min & 0\\
#'    & & \sum\limits_{l = 1}^9 x_{rcl} & =  1 & \forall r\in R, c \in C\\
#'    & & \sum\limits_{r = 1}^9 x_{rcl} & =  1 & \forall c \in C, l \in L\\
#'    & & \sum\limits_{c = 1}^9 x_{rcl} & =  1 & \forall r \in R, l \in L\\
#'    & & \sum\limits_{(r,c) \in S_{ij}} x_{rcl} & =  1 & \forall i,j\in \{1,2,3\}, l \in L\\
#'    & &                       x_{rcl} & = 1 & \forall (r,c,l) \in V_0\\
#'    & &                       x_{rcl} & \in \{0,1\} & \forall r \in R, c \in C, l \in L\\
#'  \end{align}
#'  $$
#' 
#' ## Game Creation
#' 
## -----------------------------------------------------------------------------
# create a sudoku game
set.seed(123)
game <- makeGame()
plot(game)


#' 
#' ## Incrementally Formulate The Sudoku Problem
#' 
#' We write a function `incrementally_formulate_sudoku` to incrementally formulate the
#' sudoku problem. Like `load_sudoku_problem`, `incrementally_formulate_sudoku`
#' returns a 'prob' and 'index.df' for further processing.
#' 
## -----------------------------------------------------------------------------
incrementally_formulate_sudoku <- function(prob = NULL, game) {
  # create a new prob object if none was specified
  if (is.null(prob)) {
    prob <- createprob()
  }

  # create some sets to match the notation above
  R <- 1:9
  C <- 1:9
  L <- 1:9

  # create a convenient data frame that contains all combinations of R,C,and L to simplify indexing
  index.df <- expand.grid(R, C, L)
  names(index.df) <- c("R", "C", "L")

  # infer the subcell for each row/column combination.
  index.df$Si <- (index.df$R - 1) %/% 3 + 1
  index.df$Sj <- (index.df$C - 1) %/% 3 + 1

  # add columns
  index.df$VarIdx <- index.df %>%
    apply(1, function(x)
      xprs_newcol(
        prob,
        lb = 0,
        ub = 1,
        coltype = "B",
        name = paste0("X_", paste(x[c("R", "C", "L")], collapse = "_"))
      ))

  # We use the designGame function to query a data frame representation of the Sudoku.
  game.df <- sudokuAlt::designGame(game)

  # We convert the game.df and merge the symbols
  game.df <- game.df[game.df$Symbol != "LNA", ]
  game.df$R <- as.integer(gsub("R", "", game.df$Row))
  game.df$C <- as.integer(gsub("C", "", game.df$Col))
  game.df$Si <- as.integer(gsub("S(.).", "\\1", game.df$Square))
  game.df$Sj <- as.integer(gsub("S.(.)", "\\1", game.df$Square))
  index.df <- merge(index.df, game.df, all.x = T)
  index.df <- index.df[order(index.df$VarIdx), ]

  # fix the initial assignment by setting variable lower bounds
  initial.assignment <-
    which(sprintf("L%d", index.df$L) == index.df$Symbol)
  initial.length <- length(initial.assignment)

  # change the lower bounds of the initial assignment columns
  chgbounds(
    prob,
    colind = index.df$VarIdx[initial.assignment],
    bndtype = rep("L", initial.length),
    bndval = rep(1, initial.length)
  )


  # add rows according to the 4 sets of constraints

  # There are two possible approaches to add rows and  function `group_map` is used
  # in both of them. Using `group_map` will be more effective than using 'for loop' and
  # filtering the data frame for indices we want in each iteration.
  # Approach 1 firstly creates a list to store the column indices and then add rows,
  # and approach 2 adds rows inside of the function 'group_map'.
  # For the first two sets of constraints, we use approach 1 and for the last two sets
  # of constraints, we use approach 2.

  # 1. only one symbol in each cell
  idxlist.cell <-
    index.df %>% group_by(R, C) %>% group_map( ~ .x$VarIdx)

  for (i in 1:length(idxlist.cell)) {
    xprs_addrow(
      prob,
      colind = idxlist.cell[[i]],
      rowcoef = rep(1, 9),
      rowtype = "E",
      rhs = 1,
      name = paste0("cell_constraint", i, collapse = "_")
    )
  }

  # 2. each symbol only once per column
  idxlist.column <-
    index.df %>% group_by(C, L) %>% group_map( ~ .x$VarIdx)

  for (i in 1:length(idxlist.column)) {
    xprs_addrow(
      prob,
      colind = idxlist.column[[i]],
      rowcoef = rep(1, 9),
      rowtype = "E",
      rhs = 1,
      name = paste0("column_constraint", i, collapse = "_")
    )
  }


  # 3. each symbol only once per row
  index.df %>% group_by(R, L) %>%
    group_map(
      ~ xprs_newrow(
        prob,
        colind = .x$VarIdx,
        rowcoef = rep(1, 9),
        rowtype = 'E',
        rhs = 1,
        name = pretty_name("Row", .y)
      )
    )

  # 4. each symbol only once in each square:
  index.df %>% group_by(Si, Sj, L) %>%
    group_map(
      ~ xprs_newrow(
        prob,
        colind = .x$VarIdx,
        rowcoef = rep(1, 9),
        rowtype = 'E',
        rhs = 1,
        name = pretty_name("Square", .y)
      )
    )



  # specify the name of the problem
  setprobname(prob, "Sudoku")

  # returns prob and index.df
  return(list(prob = prob, index.df = index.df))
}


#' 
#' ## Solve The Game With Xpress
#' 
## -----------------------------------------------------------------------------
problist <- incrementally_formulate_sudoku(game = game)
prob <- problist$prob

print(prob)
setoutput(prob)
mipoptimize(prob)
summary(prob)


#' 
#' ## Plot The Solution
#' 
## -----------------------------------------------------------------------------
solution <- getmipsol(prob)$x

# we write a function that enables Sudoku style plotting from ROI example:
to_sudoku_solution <- function(solution, index.df) {
  # filter the rows with solution value very close to 1 instead of exactly 1, because
  # it will be safer to have a tolerance when comparing values.
  rowbyrowsol <-
    index.df %>% filter(near(solution, 1) == TRUE) %>% arrange(R, C)

  matrix_solution <- matrix(rowbyrowsol$L, 9, 9, byrow = T)
  sudoku_solution <-
    structure(matrix_solution, class = c("sudoku", "matrix"))
  sudoku_solution
}

sudoku_solution <- to_sudoku_solution(solution, problist$index.df)
par(mfrow = c(1, 2))
plot(game)
plot(sudoku_solution)


#' 
#' 
#' # Sudoku With Diagonal Constraints
#' 
#' A sudoku game with diagonal constraints requires that along the main diagonal each
#' symbol appears only once, and the same holds for anti diagonal. For mathematical
#' notations, please refer to the 'sudoku.R' file.
#' 
#' ## Game Creation
#' 
## -----------------------------------------------------------------------------
# create a game with slightly less initial assignments than before.
sudoku_game <- sudokuAlt::makeGame(gaps = 68)

plot(sudoku_game)


#' 
#' ## Add Diagonal And Antidiagonal Constraints
#' 
## -----------------------------------------------------------------------------
# add main diagonal constraints
add_main_diagonal <- function(prob, index.df) {
  diagonal.df <- index.df %>%
    filter(R == C) %>%
    group_by(L) %>%
    group_map(
      ~ xprs_addrow(
        prob = prob,
        rowtype = "E",
        rhs = 1,
        colind = .x$VarIdx,
        rowcoef = rep(1, 9),
        name = pretty_name("diagonal_", .y)
      )
    )
  return(prob)
}

# add anti-diagonal constraints
add_anti_diagonal <- function(prob, index.df) {
  antidiagonal.df <- index.df %>%
    filter(R + C == 10) %>%
    group_by(L) %>%
    group_map(
      ~ xprs_addrow(
        prob = prob,
        rowtype = "E",
        rhs = 1,
        colind = .x$VarIdx,
        rowcoef = rep(1, 9),
        name = pretty_name("antidiagonal_", .y)
      )
    )
  return(prob)
}


#' 
#' ## Solve The Game
#' 
## -----------------------------------------------------------------------------
# incrementally formulate the game and add diagonals
problist <- incrementally_formulate_sudoku(game = sudoku_game)
probdiag <- problist$prob
index.df <- problist$index.df

print(probdiag)
setoutput(probdiag)

probdiag <- add_main_diagonal(probdiag, index.df)
probdiag <- add_anti_diagonal(probdiag, index.df)
mipoptimize(probdiag)
summary(probdiag)


#' 
#' ## Plot The Solution
#' 
## -----------------------------------------------------------------------------
solution <- getmipsol(probdiag)$x

sudoku_solution <- to_sudoku_solution(solution, index.df)
par(mfrow = c(1, 2))
plot(sudoku_game)
plot(sudoku_solution)


#' 
#' 
#' # Solving a Sudoku with Only 4 Digits
#' 
#' Simon Anthony solves a Sudoku with only 4 given digits in his Youtube channel
#' [Cracking the Cryptic](https://www.youtube.com/watch?v=hAyZ9K2EBF0), and following
#' rules apply to this game:
#' 
#' * Classical rules (each digit once per row, column, and square)
#' * Diagonals, see Section above
#' * The central square must also form a *magic square*, meaning that
#'   the digits in each row and column of the central square must sum up
#'   to the same number.
#' * Cells that are a knight's move apart (in chess) must not contain the same digit.
#' 
#' For the mathematical formulation of this problem, please refer to the 'sudoku.R' file.
#' 
#' ## Game Creation
#' 
## -----------------------------------------------------------------------------

# Make a fresh game and override the given symbols by the four digits from the video.
game4digits <- makeGame()
for (i in 1:9) for (j in 1:9) game4digits[i,j] <- NA
game4digits[4,1] <- 3; game4digits[4,2] <- 8; game4digits[4,3] <- 4; game4digits[9,9] <- 2
plot(game4digits)


#' 
#' ## Add Square Constraints, Get Knight's Move Neighbours And Add Knight's Move Constraints
#' 
## -----------------------------------------------------------------------------
# add magic square constraints
add_magic_square <- function(prob, index.df) {
  # consider only the central square S_22
  central.square.df <-
    index.df %>% filter(Si == 2) %>% filter(Sj == 2)

  # group by row and add the constraints for each row
  central.square.df.by.row <- central.square.df %>%
    group_by(R) %>%
    group_map(
      ~ xprs_addrow(
        prob,
        rowtype = "E",
        rhs = 15,
        colind = .x$VarIdx,
        rowcoef = .x$L,
        name = pretty_name("central.row_", .y)
      )
    )


  # group by column and add the constraints for each column
  central.square.df.by.col <- central.square.df %>%
    group_by(C) %>%
    group_map(
      ~ xprs_addrow(
        prob,
        rowtype = "E",
        rhs = 15,
        colind = .x$VarIdx,
        rowcoef = .x$L,
        name = pretty_name("central.col_", .y)
      )
    )



  # return prob with the added constraints
  return(prob)
}

# get knight's move neighbours
get_knights_move_neighbors <- function(r, c) {
  # create all possible moves, possibly outside the feasible range

  # note that this traversal will encounter each pair of conflicting variables x,y
  # twice. The first time when all neighboring cells of x are traversed,
  # and a second time when those around y are determined.
  # The resulting duplicate rows are detected by Xpress in presolving
  neighbors <- matrix(
    c(
      r - 2, c - 1,
      r + 2, c - 1,
      r - 2, c + 1,
      r + 2, c + 1,
      r - 1, c - 2,
      r + 1, c - 2,
      r - 1, c + 2,
      r + 1, c + 2
    ),
    byrow = T,
    ncol = 2
  )

  # filter elements outside the feasible range and return
  neighbors[(neighbors[, 1] >= 1) &
              (neighbors[, 1] <= 9)  &
              (neighbors[, 2] <= 9)   & (neighbors[, 2] >= 1), ]
}

# add the knight's move constraints
add_knights_move_constraints <- function(prob, index.df) {
  # one additional constraint for each r,c,l

  # we establish the R,C,L order in index.df.sorted.
  # An element r,c,l has the index 81 * (r - 1) + 9 * (c - 1) + l
  index.df.sorted <- index.df %>% arrange(R, C, L)

  for (row in 1:9) {
    for (col in 1:9) {
      neighbors <- get_knights_move_neighbors(row, col)

      # add constraints that forbid the same symbol in the neighbors and this cell
      # one for each symbol
      for (symbol in 1:9) {
        cellvaridx <-
          index.df.sorted$VarIdx[81 * (row - 1) + 9 * (col - 1) + symbol]
        neighborvarindex <-
          index.df.sorted$VarIdx[81 * (neighbors[, 1] - 1) + 9 * (neighbors[, 2] - 1) + symbol]

        # add one row per neighbor
        for (n in neighborvarindex) {
          prob <- xprs_addrow(
            prob,
            rowtype = "L",
            rhs = 1,
            colind = c(cellvaridx, n),
            rowcoef = c(1, 1),
            name = sprintf("knights_move_cell_%d_%d", cellvaridx, n)
          )
        }
      }
    }
  }

  return(prob)
}


#' 
#' ## Load All Constraints And Solve The Game
#' 
## -----------------------------------------------------------------------------
# incrementally formulate the problem
problist <- incrementally_formulate_sudoku(game = game4digits)
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
#' 
#' ## Plot The Solution
#' 
## -----------------------------------------------------------------------------
solution <- getmipsol(prob4digits)$x

solution4digits <- to_sudoku_solution(solution, problist$index.df)
par(mfrow = c(1, 2))
plot(game4digits)
plot(solution4digits)

#' 
#' 
#' 
