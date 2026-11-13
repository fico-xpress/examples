#####################################
# This file is part of the          #
# Xpress-R interface examples       #
#                                   #
#   (c) 2022-2026 Fair Isaac Corporation #
#####################################
#' ---
#' title: "Reading From/Writing to an MPS/LP File"
#' author: Gregor Hendel
#' date: Dec. 2020
#' ---
#' 

#' 
#' This example shows how to read and write from and to MPS and LP files. Both
#' formats are standard formats and can be input into most optimization solvers.
#' While the MPS file preserves the order of the columns of a matrix, the LP format
#' is more human-readable and suitable for debug purposes.
#' 
#' If you haven't done so already, please familiarize yourself with the Facility
#' Location Example, which we use throughout our quick examples.
#' 
## ----Printing out the FLP LP File---------------------------------------------
cat(readLines("flp.lp"), sep = "\n")

#' 
## ----Read the LP File into the Optimizer--------------------------------------
suppressMessages(library(xpress))
p <- createprob()
readprob(p, "flp.lp")

#' 
#' We can use Base R's `print` function to inspect the problem we just read. The
#' number of rows, columns, and nonzeros is as expected.
## ----Print the XPRSprob-------------------------------------------------------
print(p)

#' 
#' Now, let's write this problem as an MPS file.
## ----Write the Problem as MPS-------------------------------------------------
writeprob(p, "flp.mps")

#' 
#' 
#' By specifying the flag 'l', we can also write LP files to disk. Actually, this
#' is the way we created 'flp.lp' in the first place. Let's instead write the
#' presolved model to disk in LP format. For this, we set the LP Iteration limit to
#' 0, which effectively stops the solution process immediately after presolving.
#' 
#' We interrupt the Optimizer during the Root LP to have access to the presolved
#' problem
#' 
## ----Write The Presolved Problem as LP----------------------------------------
setintcontrol(p, xpress:::LPITERLIMIT, 0L);
mipoptimize(p)

# note the use of the flag 'l'
writeprob(p, "flp_presolved.lp", "l")

#' 
#' When we inspect the resulting LP file, we see that bounds have been derived for
#' all the connection variables:
#' 
## ----Inspect the Presolved Problem--------------------------------------------
cat(readLines("flp_presolved.lp"), sep = "\n")

#' 
#' This already concludes the example how to read and write from MPS and LP files.
#' Here is the content of the MPS file.
#' 
## ----Display the content of the MPS file--------------------------------------
cat(readLines("flp.mps"), sep = "\n")

#' 
#' 
#' 
