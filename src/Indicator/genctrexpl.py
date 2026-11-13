"""
   Xpress Python Example Problems
   ======================

   file genctrexpl.py
    ```````````````
   MIP problem formulation using general constraints.
   - Example discussed in mipformref whitepaper -

   (c) 2024 Fair Isaac Corporation
       author: B. Vieira, Sep. 2024
"""

import xpress as xp
from xpress.enums import SolStatus

R = range(3)

p = xp.problem()

x = [p.addVariable(name="x_{}".format(i)) for i in R]
y = p.addVariable()
z = p.addVariable()

p.addConstraint(x[i] <= 20 for i in R)
p.addConstraint(xp.abs(x[0] - 2*x[1]) <= 10)
p.addConstraint(xp.min(x) >= 5)
p.addConstraint(y == xp.min(x[2], 20, x[0]-z))

p.setObjective(xp.Sum(x[i] for i in R), sense=xp.ObjSense.MAXIMIZE)

p.optimize()

if p.attributes.solstatus in [SolStatus.FEASIBLE, SolStatus.OPTIMAL]:
    print("Solution:", p.attributes.objval)
    for i in R:
        print(x[i].name,"=",p.getSolution(x[i]))
    print("y=", p.getSolution(y), ", z=", p.getSolution(z))
    print("abs=", p.getSolution(xp.abs(x[0]-2*x[1])), ", Min of x(i)=", min(p.getSolution(x)))
else:
    print("No solution")