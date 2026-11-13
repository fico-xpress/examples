# Modeling a small QP problem to perform portfolio optimization.
# 1. QP: minimize variance.
# 2. MIQP: limited number of assets.
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
TARGET = 9
MAXNUM = 4
NSHARES = 10
RET = [5, 17, 26, 12, 8, 9, 7, 6, 31, 21]
NA = [0, 1, 2, 3]

# *******FIRST PROBLEM: UNLIMITED NUMBER OF ASSETS********
p = xp.problem(name="Folio")

# VARIABLES.
frac = p.addVariables(NSHARES, ub=0.3, name="frac")

# CONSTRAINTS.
# Minimum amount of North-American values.
p.addConstraint(xp.Sum(frac[i] for i in NA) >= 0.5)

# Spend all the capital.
p.addConstraint(xp.Sum(frac) == 1)

# Target yield.
p.addConstraint(xp.Sum(frac[i] * RET[i] for i in range(NSHARES)) >= TARGET)

# Objective: minimize mean variance.
variance = [frac[s]*frac[t]*VAR[s][t] for s in range(NSHARES) for t in range(NSHARES)]
p.setObjective(xp.Sum(variance))

# Solve.
p.optimize()

# Print problem status.
print(f"Problem status: \n\t Solve status: {p.attributes.solvestatus.name} \n\t Sol status: \
    {p.attributes.solstatus.name}")

# Solution printing.
print(f"With a target of {TARGET} minimum variance is {p.attributes.objval}")
sol = p.getSolution(frac)
for i in range(NSHARES):
    print(f"{frac[i].name} : {sol[i]*100:.2f} %")

# *******SECOND PROBLEM: LIMIT NUMBER OF ASSETS********
buy = p.addVariables(NSHARES, vartype=xp.binary, name="buy")

# CONSTRAINTS.
# Minimum amount of North-American values.
p.addConstraint(xp.Sum(frac[i] for i in NA) >= 0.5)

# Limit the total number of assets.
p.addConstraint(xp.Sum(buy) <= MAXNUM)

# Linking the variables.
p.addConstraint(frac[i] <= buy[i] for i in range(NSHARES))

# Solve.
p.optimize()

# Print problem status.
print(f"Problem status: \n\t Solve status: {p.attributes.solvestatus.name} \n\t Sol status: \
    {p.attributes.solstatus.name}")

# Solution printing.
print(f"With a target of {TARGET} minimum variance is {p.attributes.objval}")
sol = p.getSolution(frac)
for i in range(NSHARES):
    print(f"{frac[i].name} : {sol[i]*100:.2f} %")
