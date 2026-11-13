# Example that shows how to treat an infeasible problem.
#
# (C) 1983-2026 Fair Isaac Corporation

import xpress as xp

minf = xp.problem("ex-infeas")

x0 = minf.addVariable()
x1 = minf.addVariable()
x2 = minf.addVariable(vartype=xp.binary)

c1 = x0 + 2 * x1 >= 1
c2 = 2 * x0 + x1 >= 1
c3 = x0 + x1 <= .5

minf.addConstraint(c1, name="lb_combo_1")
minf.addConstraint(c2, name="lb_combo_2")
minf.addConstraint(c3, name="ub_sum")
minf.optimize()
minf.IISAll()
print("there are ", minf.attributes.numiis, " iis's")


# Get data for the first IIS.
miisrow, miiscol, constrainttype, colbndtype, \
      duals, rdcs, isolationrows, isolationcols = minf.getIISData(1)

# Use constraint names to make the IIS output human-readable.
row_names = minf.getNameList(xp.Namespaces.ROW)
print("iis constraints:", [row_names[r] for r in miisrow])
print("iis data:", miisrow, miiscol, constrainttype, colbndtype,
      duals, rdcs, isolationrows, isolationcols)

# Another way to check IIS isolations.
minf.IISIsolations(1)

num_iis, rowsizes, colsizes, suminfeas, numinfeas = minf.IISStatus()

print("number iis's:", num_iis)
print("vectors:", rowsizes, colsizes, suminfeas, numinfeas)
