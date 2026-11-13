"""
   Xpress Python Example Problems
   ======================

   file indicators.py
    ```````````````
   Indicator constraint definition example.
   - Example discussed in mipformref whitepaper -

   (c) 2024 Fair Isaac Corporation
       author: B. Vieira, Sep. 2024
"""

import xpress as xp
from xpress.enums import SolStatus

N = 2

p = xp.problem()

# Create the decision variables
x = [p.addVariable(name="x_{}".format(i)) for i in range(N)]
b = [p.addVariable(name="b_{}".format(i),vartype=xp.binary) for i in range(N)]

# b[0] = 1 -> x[0]+x[1] >= 12
p.addIndicator(b[0] == 1, x[0] + x[1] >= 12)
# b[1] = 0 -> x[1] <= 5
p.addIndicator(b[1] == 0, x[1] <= 5)

# Set objective
p.setObjective(b[0] + b[1], sense=xp.ObjSense.MAXIMIZE)

# Write out the model in case we want to look at it.
p.writeProb("indicators.lp")

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

for i in range(N):
    print(x[i].name,": ",p.getSolution(x[i]))