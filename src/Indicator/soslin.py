"""
   Xpress Python Example Problems
   ======================

   file soslin.py
    ```````````````
   Approximation of a nonlinear function by a SOS-2.
   - Example discussed in mipformref whitepaper -

   (c) 2024 Fair Isaac Corporation
       author: B. Vieira, Sep. 2024
"""

import xpress as xp
from xpress.enums import SolStatus

I = range(4)
R = [1, 2.5, 4.5, 6.5]
FR = [1.5, 6, 3.5, 2.5]

p = xp.problem()

# Create the decision variables
x = p.addVariable(lb=R[0],ub=R[3],name="x") # x-values interval defined by the breakpoints
y = p.addVariable(name="y")
w = [p.addVariable(name="w{0}".format(i)) for i in I]

# Define the SOS-2 with weights R
p.addSOS(w, R, name="Defx", type=2)

# Weights must sum up to 1
p.addConstraint(xp.Sum(w[i] for i in I) == 1.0)

# The variable and the corresponding function value we want to approximate
p.addConstraint(x == xp.Sum(R[i]*w[i] for i in I))
p.addConstraint(y == xp.Sum(FR[i]*w[i] for i in I))

# Set a lower bound on x to make the problem more interesting
x.lb = 2

# Set objective
p.setObjective(y)

# Write out the model in case we want to look at it
p.writeProb("soslin.lp")

# Solve the problem
p.optimize()

match p.attributes.solstatus:
    case SolStatus.FEASIBLE | SolStatus.OPTIMAL:
        print("MIP solution: ", p.attributes.objval)
    case SolStatus.INFEASIBLE:
        print("Problem is infeasible")
    case SolStatus.UNBOUNDED:
        print("LP unbounded")
    case SolStatus.NOTFOUND:
        print("Solution not found")

print(x.name,": ",p.getSolution(x))