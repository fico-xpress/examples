# Test problem on a dot product between matrices of scalars and/or of
# variables. Note that the problem cannot be solved by the Optimizer
# as it is nonconvex.
#
# (C) 1983-2026 Fair Isaac Corporation

import xpress as xp
import numpy as np

a = 0.1 + np.arange(21).reshape(3, 7)

p = xp.problem()

# Create NumPy vectors of variables.
y = p.addVariables(3, 7, name='')
x = p.addVariables(7, 5, name='')

p.addConstraint(xp.Dot(y, x) <= 0)
p.addConstraint(xp.Dot(a, x) == 1)

p.setObjective(x[0, 0])

p.optimize()

# Turns out the problem is infeasible, so let's use nonlinear IIS using the global solver to find out why.

# Find the first IIS and stop once it is found.
p.firstIIS(0)

# Get data for the IIS.
miisrow, miiscol, constrainttype, colbndtype, \
      duals, rdcs, isolationrows, isolationcols = p.getIISData(1)

# Print the IIS.
print("iis data:", miisrow, miiscol, constrainttype, colbndtype,
      duals, rdcs, isolationrows, isolationcols)
