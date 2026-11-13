#####################################
# This file is part of the          #
# Xpress-R interface examples       #
#                                   #
#   (c) 2022-2026 Fair Isaac Corporation #
#####################################
#' ---
#' title: "Adding Indicator Constraints"
#' ---
#' 

#' 
#' An indicator constraint is made of a condition and a linear constraint. The
#' condition is of the type `bin = value`, where `bin` is a binary variable and
#' `value` is either 0 or 1. The linear constraint is any linear row. During branch and bound
#' search, a row configured as an indicator constraint is enforced only when
#' condition holds, that is only if the indicator variable `bin` has the specified
#' value.
#' 
#' We work with the facility location running
#' example. Please familiarize yourself with this example first.
#' 
#' We load the facility location example and optimize it first.
## ----Solving the Unmodified Facility Location Problem-------------------------
library(magrittr)
suppressMessages(library(xpress))
# quickly optimize the Facility location problem from a pipe
p <- createprob() %>% readprob("flp.lp") %>% mipoptimize()

print("MIP solution for the 5 facilities")
getsolution(p)$x[1:5]

#' 
#' # Adding a New Row
#' 
#' We would like to express that if facility $F_5$ is open, then both $F_2$ and
#' $F_4$ must be open, as well. Mathematically, we want to impose the implication
#' 
#' $$
#' \begin{align}
#' x_5 = 1 \Rightarrow x_2 + x_4 = 2
#' \end{align}
#' $$
#' 
#' Since $x_5$ is currently open in the basic example problem, this additional
#' restriction will be very expensive. Can we do without opening $x_5$? First, we
#' now add the new row to the existing constraint matrix that expresses the
#' cardinality restriction on variables $x_3$ and $x_5$
#' 
#' We use `addrows` to add the new row to the existing instance.
#' The `addrows` function can be used to add only a single
#' or even multiple new rows.
#' For each row, we have to indicate its type and right hand side.
#' Since we only add a single row, the rowtype and rhs can be passed
#' as scalars.
#' 
#' In addition, we pass the nonzero row coefficients in sparse row major format.
#' When we pass column indices of these entries, we have to remember
#' to use 0-based indexing, because these values are directly passed
#' to Xpress.
#' The 0-based column indices that correspond to variables $x_2$ and $x_4$
#' are 1 and 3. Here we use the syntax `c(2,4) - 1` which subtracts
#' 1 from each 1-based index to obtain a 0-based index.
#' 
## ----Adding a New Row to the Problem------------------------------------------
print(addrows(p,
        rowtype = "E",
        rhs = 2,
        start=c(0,2), # 0-based indices
        colind = c(2,4) - 1,# 0-based indices
        rowcoef = c(1,1)
        )
      )

#' 
#' We verify that the new linear constraint changes the
#' solution to the problem (the linear constraint is now active)
#' 
## ----Verify row addition------------------------------------------------------
summary(mipoptimize(p))
print("MIP solution for the 5 facilities after adding the additional row")
print(getsolution(p)$x[1:5])


#' # Declaration as Indicator Row
#' 
#' Now we are ready to add the indicator implication between variable $x_5$ and the
#' newly created row.
#' Recall 0-based indexing:
#' This newly created row has index `xpress:::ROWS - 1`.
#' Similarly variable x_5 has column index 5 in R, but 4 in C
#' 
#' The `print` statement assures us that we now have 1 indicator
#' constraint in our formulation.
#' 
## ----Marking the New Row As Indicator-----------------------------------------
print(setindicators(p,
  # 0-based. use the last added row
  rowind=getintattrib(p, xpress:::ROWS) - 1L,
  colind = 5L - 1L,
  complement = 1L # row is active when x_5 = 1
  ))

#' 
#' After resolving the indicator formulation, we immediately notice that $F_5$ is
#' not opened, and therefore, the restriction on opening both $F_2$ and $F_4$ is
#' lifted.
#' 
## ----Resolve with the Indicator Constraint------------------------------------
summary(mipoptimize(p))
print("MIP solution for the 5 facilities after adding the indicator constraint")
print(getsolution(p)$x[1:5])

#' 
