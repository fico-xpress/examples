"""
   Xpress Python Example Problems
   ======================

   file pwlinsos1.py
    ```````````````
   Modeling a piecewise linear function:
   SOS-1 formulation
   - Example discussed in mipformref whitepaper -

   (c) 2024 Fair Isaac Corporation
       author: B. Vieira, Sep. 2024
"""

import xpress as xp

NB = 3                  # Number of values
BREAKS = range(NB)      # Set of break points
B = [0, 50, 120, 200]   # Break points (X coordinates)
VALUE = [1, 1.5, 2]     # Coefficient value at break points

p = xp.problem()

x = p.addVariable()                                     # The actual decision variable x
fx = p.addVariable()                                    # The actual decision variable y
xparts = [p.addVariable() for i in BREAKS]              # Parts of 'x'
b = [p.addVariable(vartype=xp.binary) for i in BREAKS]  # Indicators of price bands

# Definition of x
p.addConstraint(x == xp.Sum(xparts[i] for i in BREAKS))

# Resulting function value f(x)
p.addConstraint(fx == xp.Sum(VALUE[i]*xparts[i] for i in BREAKS))

# Lower and upper bounds on quantities
p.addConstraint(xparts[i] >= B[i]*b[i] for i in BREAKS)
p.addConstraint(xparts[i] <= B[i+1]*b[i] for i in BREAKS)

# The value of x lies in exactly one interval
p.addConstraint(xp.Sum(b[i] for i in BREAKS) == 1)
# Alternatively:
#p.addSOS(b, [B[i] for i in BREAKS])

# **** Example of objective function: minimize 'fx' ****
# (possible interpretation: cost function associated with x)

# Add a bound on x to get a somewhat more meaningful model
x.lb = 150

p.setObjective(fx)

# Solve the problem
p.optimize()

print("Solution:", p.attributes.objval)
print("fx:", p.getSolution(fx), ", x:", p.getSolution(x))
for i in BREAKS:
    print(p.getSolution(xparts[i]))