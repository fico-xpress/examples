# General constraints example using the xpress.max() operator and
# the problem.addgencons() API method.
#
# Finds the point that minimizes the maximum variable within a given
# polytope.
#
# (C) 1983-2026 Fair Isaac Corporation

import xpress as xp

formulate_using_max = True  # If True - will use xpress.max, else - will use problem.addgencons.

p = xp.problem()

# Read data from a problem of MIPLIB 2017.
p.readProb('Data/pk1.mps.gz')

# Retrieve all variables of the original problem.
x = p.getVariable()

if formulate_using_max:
    # Here we use the max operator of the Python interface to create a new
    # optimization problem.

    # Change objective function to the maximum of all variables, to
    # be minimized.
    p.setObjective(xp.max(x))
else:
    # Here we use the API function problem.addgencons().

    # First create a variable that will be used in the
    # call to problem.addgencons() and in the objective function.
    max_x = p.addVariable()

    p.addGenCons([xp.GenConsType.MAX], [max_x], [0], x, [], [])

    # Change objective function to the maximum of all variables, to
    # be minimized.
    p.setObjective(max_x)

# Set time limit to 5 seconds.
p.controls.timelimit = 5
p.optimize()

print("Solution: x = ", p.getSolution())
