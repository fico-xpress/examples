# General constraints example using the Xpress And/Or operators and
# the problem.addgencons() API method.
#
# Solves a simple SAT problem by finding the solution with the fewest
# 'True' variables that satisfy all clauses.
#
# (C) 1983-2026 Fair Isaac Corporation

import xpress as xp

formulate_using_andor = True  # If True - will use xpress operators, else - will use problem.addgencons.

p = xp.problem()

N = 10
k = 5

x = [p.addVariable(vartype=xp.binary) for _ in range(N)]

if formulate_using_andor:
    # Here we use the And/Or operators of the Python interface to create a new
    # optimization problem.

    # At most one of each pair can be True.
    con0 = [(x[i] & x[i+1]) == 0 for i in range(0, N-1, 2)]

    # At least a quarter of all OR clauses on continuous groups of k
    # clauses must be True.
    con1 = xp.Sum(xp.Or(*(x[i:i+k])) for i in range(N-k)) >= N/4

    p.addConstraint(con0, con1)

else:
    # Here we use the API function problem.addgencons().

    # Creates a continuous list despite the 2 step in range().
    y_and = [p.addVariable(vartype=xp.binary) for i in range(0, N - 1, 2)]

    y_or = [p.addVariable(vartype=xp.binary) for i in range(N - k)]

    p.addGenCons([xp.GenConsType.AND] * (N // 2) + [xp.GenConsType.OR] * (N - k),
                 y_and + y_or,  # two list of resultants
                 [2 * i for i in range(N // 2)] +  # colstart is the list [0, 2, 4...] for the AND constraint
                 [2 * (N // 2) + k * i for i in range(N - k)],  # ... and then the list [0, k, 2*k...] displaced by 2N
                 x[:2 * (N // 2)] +  # consider all original variables in this order
                 [x[i + j] for j in range(k) for i in range(N - k)],  # and then variables [0..k-1, 1..k, 2..k+1, ...]
                 [], [])

# Set time limit to 5 seconds.
p.controls.timelimit = 5
p.optimize()

print("Solution: x = ", p.getSolution())
