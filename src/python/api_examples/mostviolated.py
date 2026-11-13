# A Branching rule branching on the most violated Integer/Binary.
#
# This example demonstrates the Xpress change branch callbacks.
#
# (C) 2024-2026 Fair Isaac Corporation

import xpress as xp
import math


# Class for data object to pass to variable selection callback.
class SolData:
    def __init__(self, colind, coltype, tol):
        self.colind = colind
        self.coltype = coltype
        self.tol = tol


# This function will be called every time the Optimizer has selected a candidate entity for branching.
def varselectioncb(prob, soldata, obranch):

    dsol = prob.getCallbackPresolveSolution()   # Get solution in the presolved space.

    branchcol = None    # To store column index of the variable to branch on, if any.

    # Find the most fractional column.
    maxdist = 0
    for i in soldata.colind:
        updist = math.ceil(dsol[i]) - dsol[i]
        downdist = dsol[i] - math.floor(dsol[i])

        dist = min(updist, downdist)

        if dist > soldata.tol and dist > maxdist:
            branchcol = i
            maxdist = dist

    # If we found a column to branch on then create a branching object
    # (in the presolved space) that reflects branching on that column and
    # return it to the caller.
    if branchcol is not None:
        UB = math.floor(dsol[branchcol])
        LB = math.ceil(dsol[branchcol])

        new_branch = xp.branchobj(prob, isoriginal=False)   # Create new branch object to be returned.
        new_branch.addBranches(2)                           # Number of branches to add.
        new_branch.addBounds(0, ["U"], [branchcol], [UB])   # Add upper bound to first branch.
        new_branch.addBounds(1, ["L"], [branchcol], [LB])   # Add lower bound to second branch.

        return new_branch
    else:
        return None


# Create a new problem.
p = xp.problem()

modelfile = "Data/burglar.mps"
p.readProb(modelfile)

# Turn off dual presolve reductions for this example,
# otherwise the problem is reduced to an empty problem.
p.controls.mipdualreductions = 0

print(f"Start solving problem {modelfile}.")

# Stop after solving the root relaxation
p.controls.mipstopstage = xp.MIPStopStage.INITIALRELAXATION
p.mipOptimize("")
p.setDefaultControl('mipstopstage')

p.controls.miplog = 3

# Change some controls to make sure we actually need to branch to solve the problem.
p.controls.heuremphasis = 0
p.controls.cutstrategy = 0

# Get solution data (presolved problem) to pass to callback.
coltype, colind, limit, settype, start, setcols, refval = p.getMIPEntities()
mtol = p.controls.miptol

sol = SolData(colind, coltype, mtol)

p.addChangeBranchObjectCallback(varselectioncb, sol)

p.mipOptimize()

print("Solving complete.")
