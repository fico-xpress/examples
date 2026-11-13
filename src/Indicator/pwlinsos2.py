"""
   Xpress Python Example Problems
   ======================

   file pwlinsos2.py
    ```````````````
   Modeling a piecewise linear function:
   SOS-2 formulation
   - Example discussed in mipformref whitepaper -

   (c) 2024 Fair Isaac Corporation
       author: B. Vieira, Sep. 2024
"""

import xpress as xp

NB = 3                    # Number of values
BREAKS = range(NB)        # Set of break points
BREAKS2 = range(2*NB)     # Set of duplicated break points
B = [0, 50, 120, 200]     # Break points (X coordinates)
VALUE = [1, 1.5, 2]       # Coefficient value at break points
delta = 0.10              # Coarsening factor

BC = [0 for _ in BREAKS2] # Coarsened break points
VC = [0 for _ in BREAKS2] # Value at coarsened break points

for i in BREAKS:
  if i == 0:
    BC[2*i] = B[0]
  else:
    BC[2*i]= B[i]+delta
  BC[2*i+1] =B[i+1]-delta
  VC[2*i] = VALUE[i]*BC[2*i]
  VC[2*i+1] =VALUE[i]*BC[2*i+1]

p = xp.problem()

x = p.addVariable()                     # The actual decision variable x
fx = p.addVariable()                    # The actual decision variable y
b = [p.addVariable() for i in BREAKS2]  # Indicators of price bands

# Definition of x
p.addConstraint(x == xp.Sum(BC[i]*b[i] for i in BREAKS2))

# Resulting function value f(x)
p.addConstraint(fx == xp.Sum(VC[i]*b[i] for i in BREAKS2))

# Define the SOS2 to interpolate linearly between the breakpoints
p.addSOS(b, [BC[i]+delta for i in BREAKS2], type=2)

# Convexity row
p.addConstraint(xp.Sum(b[i] for i in BREAKS2) == 1)

# **** Example of objective function: minimize 'fx' ****
# (possible interpretation: cost function associated with x)

# Add a bound on x to get a somewhat more meaningful model
x.lb = 150

p.setObjective(fx)

p.optimize()

print("Solution:", p.attributes.objval)
print("fx", p.getSolution(fx), ", x", p.getSolution(x))