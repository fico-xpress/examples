# Modeling a small LP problem to perform portfolio optimization.
#
# (C) 2025-2026 Fair Isaac Corporation

import xpress as xp

# Problem Data
NSHARES = 10
RET = [5, 17, 26, 12, 8, 9, 7, 6, 31, 21]
RISK = [1, 2, 3, 8, 9]
NA = [0, 1, 2, 3]

p = xp.problem(name="Folio")

# VARIABLES.
frac = p.addVariables(NSHARES, ub=0.3, name="frac")

# CONSTRAINTS.
# Limit the percentage of high-risk values.
p.addConstraint(xp.Sum(frac[i] for i in RISK) <= 1/3)

# Minimum amount of North-American values.
p.addConstraint(xp.Sum(frac[i] for i in NA) >= 0.5)

# Spend all the capital.
p.addConstraint(xp.Sum(frac) == 1)

# Objective: maximize total return.
p.setObjective(xp.Sum(frac[i] * RET[i] for i in range(NSHARES)), sense=xp.ObjSense.MAXIMIZE)

# Solve.
p.optimize()

# Print problem status.
print(f"Problem status: \n\t Solve status: {p.attributes.solvestatus.name} \n\t Sol status: \
    {p.attributes.solstatus.name}")

# Solution printing.
print("Total return:", p.attributes.objval)
sol = p.getSolution(frac)
for i in range(NSHARES):
    print(f"{frac[i].name} : {sol[i]*100:.2f} %")
