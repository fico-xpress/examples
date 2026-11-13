#####################################
# This file is part of the          #
# Xpress-R interface examples       #
#                                   #
#   (c) 2022-2026 Fair Isaac Corporation #
#####################################
#' ---
#' title: "Portfolio Selection"
#' author: Y. Gu
#' date: Jun.2021
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
#' This is a conversion of the Mosel example 'Portfolio Selection', <https://www.fico.com/fico-xpress-optimization/docs/latest/examples/mosel/ApplBook/H_EconFin/h3portf.mos>.
#' A brief introduction to this problem is given below, and to see the full
#' mathematical modeling of this problem you may refer to section 13.3, page 199 of the
#' book 'Applications of optimization with Xpress'.
#' 
#' This example concerns the optimal composition of investment portfolios according to
#' maximum total return. So, we define decision variable 'buy' for each share to denote
#' the amount of money invested in it. Given the criterion of optimum composition, the
#' objective we want to maximize is the return on investment of all shares, or we minimize
#' the negative of total return.
#' 
#' In this example, there are 6 shares from different countries and different categories.
#' According to the requirements of the customer, the countries can be divided into
#' two groups: 'EU' and 'non-EU', and the categories are divided into 'technology' and
#' 'non-technology'. In the beginning, the customer specifies that she wishes to invest
#' at least 5,000 and at most 40,000 into any share. She further wishes to invest half
#' of her capital in European shares and at most 30% in technology shares. Besides these
#' two constraints, another constraint is that the total invested sum must correspond to
#' the initial capital.
#' 
#' Later, it turns out that the customer actually wants to invest either 0 or at least
#' 5,000 and at most 40,000 into any share. So, we should change the type of decision
#' variables from continuous to semi-continuous using the function `chgcoltype` to satisfy
#' this requirement.
#' 
#' As mentioned in the book, it is possible to restart the optimization by varying certain
#' parameters and in this case, Mathematical Programming becomes an efficient simulation
#' tool in financial applications. In our example, we try to change the upper bound of
#' the investment into technology shares from 30% of initial capital to 40% of initial
#' capital and see how the optimum solution will change.
#' 
#' For mathematical formulations of this example, please refer to the book 'Applications
#' of optimization with Xpress'.
#' 
#' 
#' For this example, we need packages 'xpress' and 'dplyr'. Besides, we use the
#' function `pretty_name` to give the variables and constraints concise names.
#' 
## ----Load The Packages And The Function To Give Names-------------------------
library(xpress)
library(dplyr)

pretty_name <- function(prefix, y) {
  "%s_%s" %>% sprintf(prefix,
                      paste(lapply(names(y), function(name)
                        paste(name, y[name], sep = "_")), collapse = "_"))
}


#' 
#' 
#' Add the values we need for this example.
#' 
## ----Data---------------------------------------------------------------------
# maximum investment into tech. shares
MAXTECH = 0.3

# minimum investment into European shares
MINEU = 0.5

# minimum amount for a single share
VMIN = 5000

# maximum amount for a single share
VMAX = 40000

# capital
CAPITAL = 100000

# information about shares
shares.df <-
  data.frame(
    "SHARE" = 1:6,
    # shares chosen for the investment
    "RET" = c(5.3, 6.2, 5.1, 4.9, 6.5, 3.4),
    # expected return on investment of each share
    "EU" = c(FALSE, TRUE, TRUE, FALSE, TRUE, TRUE),
    # 'TRUE' if the share is of European origin
    "TECH" = c(TRUE, TRUE, TRUE, FALSE, FALSE, FALSE) # 'TRUE' if it is a technology share
  )


#' 
#' 
#' Since we need to solve several variants of the initial problem, we can create a function
#' to construct the initial problem. When creating the variants, we will call this function
#' and we just need to make adjustments on the problem this function returns.
#' 
## ----The Initial Problem------------------------------------------------------
initprob <- function(shares){
  # create a new problem
  prob <-  createprob()

  # change this problem to a maximization problem
  chgobjsense(prob, objsense = xpress:::OBJ_MAXIMIZE)

  # set the problem name
  setprobname(prob, "Portfolio")

  # Add column
  # firstly, add continuous variables 'buy'
  shares$buy <-
    shares %>% apply(1, function(x)
      xprs_newcol(
        prob,
        lb = VMIN,
        ub = VMAX,
        coltype = "C",
        name = pretty_name("buy", x["SHARE"]),
        objcoef = (x["RET"] /
                     100)
      ))


  # Add row
  # 1. Requirements concerning portfolio composition
  # 1.1 the sum invested in technology must not exceed MAXTECH*CAPITAL, i.e., 0.3*100000=30000
  techlimit <-
    xprs_newrow(
      prob,
      colind = (shares %>% filter(TECH == TRUE) %>% select(buy))$buy,
      rowcoef = rep(1, sum(shares$TECH == TRUE)),
      rowtype = "L",
      rhs = MAXTECH * CAPITAL,
      name = "technology_invest"
    )

  # 1.2 the sum invested in EU shares must be at least MINEU*CAPITAL, i.e., 0.5*100000=50000
  xprs_newrow(
    prob,
    colind = (shares %>% filter(EU == TRUE) %>% select(buy))$buy,
    rowcoef = rep(1, sum(shares$EU == TRUE)),
    rowtype = "G",
    rhs = MINEU * CAPITAL,
    name = "EU_invest"
  )


  # 2. the total invested sum must correspond to the initial capital 'CAPITAL'
  xprs_newrow(
    prob,
    colind = shares$buy,
    rowcoef = rep(1, length(shares$buy)),
    rowtype = "E",
    rhs = CAPITAL,
    name = "capital"
  )


  return(list(
    prob = prob,
    techidx = techlimit,
    shares.df = shares
  ))

}

#' 
#' 
#' We store some outputs of the function `initprob` that will be used later.
#' 
## ----Some Output Of The Initial Problem---------------------------------------
# create a list to store the returned objects from `initprob`
contlst <- initprob(shares.df)

# the data frame 'shares.df' containing column indices
shares.df <- contlst$shares.df

# the row index of the row concerning technology limit
techidx <- contlst$techidx



#' 
#' 
#' Now we solve the initial problem and display the solutions.
#' 
## ----Solve The Initial Problem------------------------------------------------
prob.init <- contlst$prob

setoutput(prob.init)
summary(xprs_optimize(prob.init))

shares.df$contsol <- getsolution(prob.init)$x

print(
  paste0(
    "If we define continuous variables, the optimum expected return is :",
    getdblattrib(prob.init, xpress:::LPOBJVAL)
  )
)

print("The amount to invest in each share is:")
for (i in shares.df$SHARE) {
  cat(shares.df$contsol[i],
      labels = as.character(shares.df$SHARE[i]),
      fill = TRUE)
}


#' 
#' 

#' 
#' Then we change the upper bound of technology investment and keep everything
#' else the same, and solve this variant and display the solutions.
#' 
## ----Change MAXTECH In Initial Problem----------------------------------------
MAXTECH2 = 0.4
chgrhs(prob.init, techidx, MAXTECH2 * CAPITAL)

setoutput(prob.init)
summary(xprs_optimize(prob.init))

shares.df$contsol2 <- getsolution(prob.init)$x

print(
  paste0(
    "If we define continuous variables and change the upper bound of technology investment, the optimum expected return is :",
    getdblattrib(prob.init, xpress:::LPOBJVAL)
  )
)

print("The amount to invest in each share is:")
for (i in shares.df$SHARE) {
  cat(shares.df$contsol2[i],
      labels = as.character(shares.df$SHARE[i]),
      fill = TRUE)
}


#' 
#' 

#' 
#' 
#' According to the customer's requirement, we change the variable type to semi-continuous
#' and specify the lower bounds of these variables as 'VMIN', and then we solve this variant
#' and display the solutions.
#' 
#' In the first problem with continuous variables, the lower bound for variables is 5000.
#' But now we set the variables as semi-continuous ones, thus the variables can be either
#' 0 or greater than 5000, which allows 0 investment on some shares with low expected ROI.
#' Therefore, this problem is actually a relaxation of the first one in view of the larger
#' feasible domain.
#' 
## ----Change Variable Type To Semi-continuous----------------------------------
prob.semicont <- initprob(shares.df)$prob

# change the column types
chgcoltype(prob.semicont, shares.df$buy, rep('S', length(shares.df$buy)))

# set the lower bound for semi-continuous variables
chgglblimit(prob.semicont, shares.df$buy, rep(VMIN, length(shares.df$buy)))

setoutput(prob.semicont)
summary(xprs_optimize(prob.semicont))

shares.df$semicontsol <- getsolution(prob.semicont)$x

print(
  paste0(
    "If we define semi-continuous variables, the optimum expected return is :",
    getdblattrib(prob.semicont, xpress:::MIPOBJVAL)
  )
)

print("The amount to invest in each share is:")
for (i in shares.df$SHARE) {
  cat(shares.df$semicontsol[i],
      labels = as.character(shares.df$SHARE[i]),
      fill = TRUE)
}


#' 
#' 

#' 
#' 
#' In this part, we change the upper bound of technology investment and keep everything
#' else the same. Then we solve this variant and display the solutions.
#' 
## ----Change MAXTECH In Modified Problem---------------------------------------
chgrhs(prob.semicont, techidx, MAXTECH2 * CAPITAL)

setoutput(prob.semicont)
summary(xprs_optimize(prob.semicont))

shares.df$semicontsol2 <- getsolution(prob.semicont)$x

print(paste0("If we define semi-continuous variables and change the upper bound of technology investment, the optimum expected return is :",getdblattrib(prob.semicont, xpress:::MIPOBJVAL)))

print("The amount to invest in each share is:")
for (i in shares.df$SHARE) {
  cat(shares.df$semicontsol2[i],
      labels = as.character(shares.df$SHARE[i]),
      fill = TRUE)
}



#' 
#' 
