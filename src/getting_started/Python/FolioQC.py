# Modeling a small QC problem to perform portfolio optimization.
# - Maximize return with limit on variance.
#
# (C) 2025-2026 Fair Isaac Corporation

import xpress as xp
import csv

# Read the CSV file and store each row as a 2D list
file_path = 'Data/foliocppqp.csv'
VAR = []
with open(file_path, 'r') as file:
    reader = csv.reader(file)
    for row in reader:
        VAR.append([float(value) for value in row])

# Problem Data
MAXVAR = 0.55
MAXNUM = 4
NSHARES = 10
RET = [5, 17, 26, 12, 8, 9, 7, 6, 31, 21]
NA = [0, 1, 2, 3]

p = xp.problem(name="Folio")

# VARIABLES.
frac = p.addVariables(NSHARES, ub=0.3, name="frac")

# CONSTRAINTS.
# Minimum amount of North-American values.
p.addConstraint(xp.Sum(frac[i] for i in NA) >= 0.5)

# Spend all the capital.
p.addConstraint(xp.Sum(frac) == 1)

# Limit variance.
variance = [frac[s]*frac[t]*VAR[s][t] for s in range(NSHARES) for t in range(NSHARES)]
p.addConstraint(xp.Sum(variance) <= MAXVAR)

# Objective: maximize total return.
p.setObjective(xp.Sum(frac[i] * RET[i] for i in range(NSHARES)), sense=xp.ObjSense.MAXIMIZE)

# Solve.
p.optimize()

# Print problem status.
print(f"Problem status: \n\t Solve status: {p.attributes.solvestatus.name} \n\t Sol status: \
    {p.attributes.solstatus.name}")

# Solution printing.
print(f"With a max variance of {MAXVAR} total return is {p.attributes.objval}")
sol = p.getSolution(frac)
for i in range(NSHARES):
    print(f"{frac[i].name} : {sol[i]*100:.2f} %")
