# General constraints example using the xpress.abs() operator and
# the problem.addgencons() API method.
#
# Find the point that minimizes the l-1 norm within a given polytope,
# i.e. the sum of the absolute values of the coordinates of a point in
# a polytope.
#
# (C) 1983-2026 Fair Isaac Corporation

import xpress as xp

formulate_using_abs = True  # If True - will use xpress.abs, else - will use problem.addgencons.

p = xp.problem()

# Read data from a problem of MIPLIB 2017.
p.readProb('Data/pk1.mps.gz')

# Retrieve all variables of the original problem.
x = p.getVariable()

if formulate_using_abs:
    # Here we use the abs operator of the Python interface to define the
    # objective function.

    # Change objective function to the l-1 norm of the variable vector, to
    # be minimized.
    p.setObjective(xp.Sum(xp.abs(v) for v in x))
else:
    # Here we do so by using the API function problem.addgencons().

    # First create a set of variables which will be used in the
    # call to problem.addgencons(), and in the objective function later.
    abs_x = [p.addVariable() for v in x]

    N = len(x)

    p.addGenCons([xp.GenConsType.ABS] * N, abs_x, [i for i in range(N)], x, [], [])

    # Change objective function to the l-1 norm of the variable vector, to
    # be minimized.
    p.setObjective(xp.Sum(abs_x))

# Set time limit to 5 seconds.
p.controls.timelimit = 5
p.optimize()

print("Solution:", p.getSolution())
