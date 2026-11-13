#####################################
# This file is part of the          #
# Xpress-R interface examples       #
#                                   #
#   (c) 2022-2026 Fair Isaac Corporation #
#####################################
#' ---
#' title: "Establishing A College Timetable"
#' author: Y. Gu
#' date: Jul.2021
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
#' [This is a conversion of the Mosel example 'Establishing A College Timetable'](https://www.fico.com/fico-xpress-optimization/docs/latest/examples/mosel/ApplBook/I_TimePers/i3school.mos).
#' A brief introduction to this problem is given below, and to see the full
#' mathematical modeling of this problem you may refer to section 14.3, page 218 of the
#' book 'Applications of optimization with Xpress'.
#' 
#' This example is a timetabling problem for college courses. Optimization can provide
#' useful help for this kind of problem, while since many subtle sociological and
#' psychological constraints are hard to consider in a mathematical model, the solution
#' should always be used interactively: the optimization model makes suggestions
#' and the human being plans by adjusting the solution to exact needs.
#' 
#' In our example, 9 teachers will give lessons to 2 classes for 5 days per week, and
#' each day contains 4 time slots for scheduling lessons. We set binary variables
#' 'teach(t,c,l,d)' that take value 1 if the teacher t gives a lesson to class c in
#' period l of day d. Our goal is to find a feasible timetable that satisfies all the
#' constraints, but we can set objective as minimizing the 'holes' in the timetable,
#' i.e., we minimize the sum of courses taught during slots 1 and 4 of every day.
#' 
#' To schedule a feasible timetable, many constraints should be satisfied. Firstly, all
#' lessons taught by each teacher to each class must be scheduled. Secondly, a class
#' could have at most one course at any time. Similarly, a teacher must not teach more
#' than one course at a time slot. To prevent students from getting bored, another
#' constraint is specified, where at most one lesson per subject could be taught on the
#' same day .
#' 
#' There are also some specific constraints for this example. For example, sport lessons
#' have to take place on Thursday afternoon from 14:00 to 16:00. Other specific conditions
#' will be discussed when we add rows to the problem in section 'Add Rows'.
#' 
#' The solution to this problem is not unique, and as mentioned before, the one that
#' matches the real situation most should be chosen and adjustments could always be made
#' if necessary. For mathematical formulations, please refer to the book
#' 'Applications of optimization with Xpress'
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
#' Create a new empty problem and give the problem a suitable name.
#' 
## ----Create The Problem-------------------------------------------------------
# create a new problem
prob <- createprob()

# set the problem name
setprobname(prob, "CollegeTimetable")


#' 
#' 
#' Add the values we need for this example.
#' 
## ----Data---------------------------------------------------------------------
# 1. classes
CLASS <- 1:2

# 2. periods per day
Periods <- 1:4
NP <- length(Periods)

# 3. days per week
Days <- c("Monday", "Tuesday", "Wednesday", "Thursday", "Friday")
ND <- length(Days)

# 4. teachers

Teachers <- c(
  "Mr Cheese",
  "Mrs Insulin",
  "Mr Map",
  "Mr Effofecks",
  "Mrs Derivate",
  "Mrs Electron",
  "Mr Wise",
  "Mr Muscle",
  "Mrs Biceps"
)

# 5. data frame for storing variable indices
index.df <- expand.grid(Teachers, CLASS, Periods, Days)
names(index.df) <- c("teacher", "class", "period", "day")

# 6. lessons per teacher and class
Courses <- rep(c(c(1, 3, 2, 0, 4, 3, 1, 1, 0), # lessons for class 1
                 c(1, 3, 2, 4, 0, 3, 1, 0, 1)  # lessons for class 2
                 ),
                NP * ND)

index.df$courses <- Courses # add Courses as a vector 'courses' to 'index.df' for future use


#' 
#' 
#' Create binary variables 'teach' and set objective as described in introduction.
#' 
## ----Add Columns--------------------------------------------------------------
# create binary variables 'teach' for each teacher, each class and each period. Also
# create a vector 'teach' in 'index.df' to store the indices of 'teach'
index.df$teach <- index.df %>%
  apply(1, function(x)
    xprs_newcol(
      prob,
      lb = 0,
      ub = 1,
      coltype = "B",
      objcoef = NULL,
      name = paste0(x[c("teacher", "class", "period", "day")], collapse = "_")
    ))

# set objective: minimize the number of "holes" in the class timetables, i.e., minimize the courses
# being  placed in periods 1 and 4.
colidx <- (index.df %>% filter(period == 1 | period == 4))$teach
chgobj(prob, colind = colidx, objcoef = rep(1, length(colidx)))


#' 
#' 
#' Add the constraints described in introduction and some specific constraints related to
#' the teachers in this example.
#' 
## ----Add Rows, results='hide'-------------------------------------------------
# 1. all lessons taught by the teacher t to class c must be scheduled
index.df %>% group_by(teacher, class) %>%
  group_map(
    ~ xprs_newrow(
      prob,
      colind = .x$teach,
      rowcoef = rep(1, nrow(.x)),
      rowtype = 'E',
      rhs = .x$courses[1],
      name = pretty_name("allcourses", .y)
    )
  )

# 2. a class has at most one course at any time
index.df %>% group_by(class, period, day) %>%
  group_map(
    ~ xprs_newrow(
      prob,
      colind = .x$teach,
      rowcoef = rep(1, nrow(.x)),
      rowtype = 'L',
      rhs = 1,
      name = pretty_name("onecourse", .y)
    )
  )


# 3. a teacher must not teach more than one lesson at a time
index.df %>% group_by(teacher, period, day) %>%
  group_map(
    ~ xprs_newrow(
      prob,
      colind = .x$teach,
      rowcoef = rep(1, nrow(.x)),
      rowtype = 'L',
      rhs = 1,
      name = pretty_name("oneteacher", .y)
    )
  )


# 4. at most one two-hour lesson per subject is taught on the same day
index.df %>% group_by(teacher, class, day) %>%
  group_map(
    ~ xprs_newrow(
      prob,
      colind = .x$teach,
      rowcoef = rep(1, nrow(.x)),
      rowtype = 'L',
      rhs = 1,
      name = pretty_name("samesubject", .y)
    )
  )



# 5. the specific conditions for this example

# Note that these constraints require us to fix some 'teach' variables to 0 or 1, and
# to realize this we change the bounds of these variables. An equivalent way to
# achieve this is to set additional constraints such as: teach(t,c,l,d) = 0.

# 5.1 the sport lessons taught by Mr Muscle and Mrs Biceps have to take place on Thursday afternoon
# from 14:00 to 16:00 (time slot 3 on Thursday)
colidx <- c((index.df %>% filter(period==3 & teacher=="Mr Muscle" & class==1 & day=="Thursday"))$teach,
            (index.df %>% filter(period==3 & teacher=="Mrs Biceps" & class==2 & day=="Thursday"))$teach)
chgbounds(
  prob,
  colind = colidx,
  bndtype = rep("L", length(colidx)),
  bndval = rep(1, length(colidx))
)

# 5.2 no course may be scheduled during Monday period 1
colidx <- (index.df %>% filter(period==1 & day=="Monday"))$teach
chgbounds(
  prob,
  colind = colidx,
  bndtype = rep("U", length(colidx)),
  bndval = rep(0, length(colidx))
)


# 5.3 Mr Effofecks does not teach on Monday morning (time slots 1 & 2 on Monday)
colidx <- (index.df %>% filter(teacher=="Mr Effofecks" & (period==1 | period==2 ) & day=="Monday"))$teach
chgbounds(
  prob,
  colind = colidx,
  bndtype = rep("U", length(colidx)),
  bndval = rep(0, length(colidx))
)


# 5.4 Mrs Insulin does not teach on Wednesday
colidx <- (index.df %>% filter(teacher=="Mrs Insulin" & day=="Wednesday"))$teach
chgbounds(
  prob,
  colind = colidx,
  bndtype = rep("U", length(colidx)),
  bndval = rep(0, length(colidx))
)



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
index.df$solution <- getsolution(prob)$x

# timetable for class 1
c1_timetable <- as.data.frame(matrix("-", nrow = NP, ncol = ND))
names(c1_timetable) <- Days
c1_lessons <- index.df %>% filter(class == 1 & solution == 1)

for (i in 1:nrow(c1_lessons)) {
  day <- as.character(c1_lessons$day[i])
  period <- c1_lessons$period[i]
  c1_timetable[period, day] <- as.character(c1_lessons$teacher[i])
}

print("The timetable of class 1 is:")
c1_timetable

cat("\n")

# timetable for class 2
c2_timetable <- as.data.frame(matrix("-", nrow = NP, ncol = ND))
names(c2_timetable) <- Days
c2_lessons <- index.df %>% filter(class == 2 & solution == 1)

for (i in 1:nrow(c2_lessons)) {
  day <- as.character(c2_lessons$day[i])
  period <- c2_lessons$period[i]
  c2_timetable[period, day] <- as.character(c2_lessons$teacher[i])
}
print("The timetable of class 2 is:")
c2_timetable


#' 
#' 
