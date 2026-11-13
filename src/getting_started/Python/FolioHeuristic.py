# Using a heuristic solution to perform portfolio optimization.
#
# (C) 2025-2026 Fair Isaac Corporation

import xpress as xp

# Problem Data
MAXNUM = 4
NSHARES = 10
RET = [5, 17, 26, 12, 8, 9, 7, 6, 31, 21]
RISK = [1, 2, 3, 8, 9]
NA = [0, 1, 2, 3]

def printSolution(prob, name):
    # Solution printing.
    print(f"Total return {name}:", prob.attributes.objval)
    sol = prob.getSolution()
    for i in range(NSHARES):
        print(f"{frac[i].name} : {sol[i] * 100:.2f} %")

def solveHeuristic(prob):
    # Disable automatic cuts.
    prob.controls.cutstrategy = 0
    # Switch presolve off.
    prob.controls.presolve = 0
    prob.controls.nodepresolve = 0
    # Get feasibility tolerance.
    tol = prob.controls.feastol

    prob.lpOptimize()

    # Save the current basis.
    rowstat, colstat = prob.getBasis()

    # Fix all variables 'buy' for which `frac' is at 0 relatively close to 1
    fsol = prob.getSolution(frac)      # get the solution values of `frac'
    for i in range(NSHARES):
        if fsol[i] < tol:
            buy[i].lb = 0
            buy[i].ub = 0
        elif fsol[i] > 0.2 - tol:
            buy[i].lb = 1
            buy[i].ub = 1

    prob.mipOptimize()

    print(f"Problem status: \n\t Solve status: {p.attributes.solvestatus.name} \n\t Sol status: \
        {p.attributes.solstatus.name}")

    printSolution(prob, "Heuristic solution")

    # Reset variables to their original bounds.
    for i in range(NSHARES):
        if fsol[i] < tol or fsol[i] > 0.2 - tol:
            buy[i].lb = 0
            buy[i].ub = 1

    # Load basis.
    prob.loadBasis(rowstat, colstat)

    # Set cutoff to the best known solution.
    prob.controls.mipabscutoff = prob.attributes.objval - tol

p = xp.problem(name="Folio")

# VARIABLES.
frac = p.addVariables(NSHARES, ub=0.3, name="frac")
buy = p.addVariables(NSHARES, vartype=xp.binary, name="buy")

# CONSTRAINTS.
# Limit the percentage of high-risk values.
p.addConstraint(xp.Sum(frac[i] for i in RISK) <= 1/3)

# Minimum amount of North-American values.
p.addConstraint(xp.Sum(frac[i] for i in NA) >= 0.5)

# Spend all the capital.
p.addConstraint(xp.Sum(frac) == 1)

# Limit the total number of assets.
p.addConstraint(xp.Sum(buy) <= MAXNUM)

# Linking the variables.
p.addConstraint(frac[i] <= buy[i] for i in range(NSHARES))

# Objective: maximize total return.
p.setObjective(xp.Sum(frac[i] * RET[i] for i in range(NSHARES)), sense=xp.ObjSense.MAXIMIZE)

# Solve with heuristic.
solveHeuristic(p)

# Solve original problem.
p.optimize()

# Print problem status.
print(f"Problem status: \n\t Solve status: {p.attributes.solvestatus.name} \n\t Sol status: \
    {p.attributes.solstatus.name}")

printSolution(p, "Exact Solve")
