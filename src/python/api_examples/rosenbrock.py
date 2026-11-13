# Define the well-known Rosenbrock function and minimize it.
#
# (C) 1983-2026 Fair Isaac Corporation

import xpress as xp

p = xp.problem()

x = p.addVariable(lb=-xp.infinity)
y = p.addVariable(lb=-xp.infinity)

# Parameters of the Rosenbrock function.
a = 1
b = 100

p.setObjective((a - x)**2 + b * (y - x**2)**2)

# Solve this problem with a local nonlinear solver.
p.controls.nlpsolver = xp.constants.NLPSOLVER_LOCAL

p.optimize()

print('solution: ', p.getSolution(), '; value: ', p.attributes.objval)
