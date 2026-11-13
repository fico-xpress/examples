# Basis handling and sensitivity methods.
#
# We solve a simple 2x2 LP problem to optimality,
# which serves as a showcase for basis handling methods and
# sensitivity analysis.
#
# (C) 1983-2026 Fair Isaac Corporation

import xpress as xp

# Create and name the problem, and add variables.
p = xp.problem("firstexample")
x1 = p.addVariable(name="x_1")
x2 = p.addVariable(name="x_2", vartype = xp.continuous, lb = 0, ub = xp.infinity)

# Create two constraints.
cons1 = 5* x1 + x2 >= 7
cons2 = x1 + 4 * x2 >= 9

# Add objective function and constraints.
p.setObjective(x1 + x2)
p.addConstraint(cons1, cons2)

# Save the problem to a file.
p.writeProb("firstexample", "lp")

# Load a basis by specifying row and column basis status
# below, the optimal basis status will be printed.
p.loadBasis([2,2], [1,1])

# Solve the problem and print the solution.
p.optimize()
print("The solution is x_1 = {}, x_2 = {}".format(p.getSolution(x1), p.getSolution(x2)))

# Write the solution to a file.
p.writeSlxSol("firstexample.slx")

# Print the row and column basis status.
rowstat, colstat = p.getBasis()
print("Rows basis status:", rowstat)
print("Column basis status:", colstat)

# Write basis to a file.
p.writeBasis("basis")

print("sensitivity Analysis of the objective:")
l, u = p.objSA([x1, x2])
for i, x in enumerate([x1, x2]):
    print("The objective coefficient of {} can be varied between [{}, {}]".format(x, l[i], u[i]))
print()

print("Sensitivity Analysis of the lower and upper bounds:")
lblower, lbupper, ublower, ubupper = p.bndSA([x1, x2])
for i, x in enumerate([x1, x2]):
    print("The lower bounds of {} can be varied between [{}, {}]".format(x, lblower[i], lbupper[i]))
    print("The upper bounds of {} can be varied between [{}, {}]".format(x, lblower[i], lbupper[i]))
print()

print("Sensitivity Analysis of right-hand sides:")
rhslower, rhsupper = p.rhsSA([cons1, cons2])
for i, c in enumerate([cons1, cons2]):
    print("The right-hand side of constraint {} can be varied between [{}, {}]".format(c,
        rhslower[i], rhsupper[i]))
print()
