"""
   Xpress Python Example Problems
   ======================

   file sosquad.py
    ```````````````
   Approximation of a nonlinear function in two variables
   by two SOS-2.
   - Example discussed in mipformref whitepaper -

   (c) 2024 Fair Isaac Corporation
       author: B. Vieira, Sep. 2024
"""

import xpress as xp
from xpress.enums import SolStatus

NX = 10
NY = 10
RX = range(NX)
RY = range(NY)

X = [i+1 for i in RX]
Y = [j+1 for j in RY]
FXY = [[(i-4)*(j-4) for i in RX] for j in RY]

p = xp.problem()

# Create the decision variables
x = p.addVariable(name="x")
y = p.addVariable(name="y")
f = p.addVariable(lb=-xp.infinity, ub=xp.infinity, name="f")
wx = [p.addVariable(name="wx_{}".format(i)) for i in RX]
wy = [p.addVariable(name="wy_{}".format(i)) for i in RY]
wxy = [[p.addVariable(name="wxy_{}_{}".format(i,j)) for i in RX] for j in RY]

# Define the SOS-2 for coordinate x
p.addSOS(wx, X, name="sos_x", type=2)
# Define the SOS-2 for coordinate y
p.addSOS(wy, Y, name="sos_y", type=2)

# Weights must sum up to 1 along the two coordinates
p.addConstraint(xp.Sum(wx[i] for i in RX) == 1.0)
p.addConstraint(xp.Sum(wy[j] for j in RY) == 1.0)

# wx, wy and wxy must be consistent
p.addConstraint(wx[i] == xp.Sum(wxy[i][j] for j in  RY) for i in RX)
p.addConstraint(wy[j] == xp.Sum(wxy[i][j] for i in  RX) for j in RY)

# The coordinates and the corresponding function value we want to approximate
p.addConstraint(x == xp.Sum(X[i]*wx[i] for i in RX))
p.addConstraint(y == xp.Sum(Y[j]*wy[j] for j in RY))
p.addConstraint(f == xp.Sum(FXY[i][j]*wxy[i][j] for i in RX for j in RY))

# Set a lower bound on x and y to make the problem more interesting
x.lb = 2.0
y.lb = 2.0

# Set objective
p.setObjective(f)

# Write out the model in case we want to look at it
p.writeProb("sosquad.lp")

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
print(y.name,": ",p.getSolution(y))