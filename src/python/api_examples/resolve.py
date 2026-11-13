# Reads a problem, solves it, then adds a constraint and re-solves it.
#
# (C) 1983-2026 Fair Isaac Corporation

import xpress

p = xpress.problem()

p.readProb("Data/example.lp")
p.optimize()
print("solution of the original problem: ", p.getVariable(), "-->",
      p.getSolution())

x = p.getVariable()
p.addConstraint(xpress.Sum(x) <= 1.1)
p.optimize()
print("New solution: ", p.getSolution())
