# Indicator constraint definition with nonlinear constraints.
# - Example discussed in mipformref whitepaper -
#
#     min   b1 + b2
#     s.t.  b1 = 1  ->  x1*x2 = 12
#           b2 = 0  ->  sin(x2) >= 0.5
#           b1, b2 in {0,1}
#
# (C) 2026 Fair Isaac Corporation

import xpress as xp
from xpress.enums import SolStatus

p = xp.problem(name="NL indicator constraints")

x = p.addVariables(2, name="x")
# Binary variables driving the indicator constraints
b = p.addVariables(2, vartype=xp.binary, name="b")

# Indicator constraint: condition (b[i] == 0 or 1) paired with the
# constraint that must hold when the condition is true.
ind1 = (b[0] == 1, x[0] * x[1] == 12)  # b[0]=1 -> x[0]*x[1]=12
ind2 = (b[1] == 0, xp.sin(x[1]) >= 0.5)  # b[1]=0 -> sin(x[1])>=0.5

# Both constraints are nonlinear (quadratic / trigonometric)
p.addIndicator(ind1, ind2)

p.setObjective(b[0] + b[1], sense=xp.ObjSense.MINIMIZE)

p.optimize()

print("Problem status:", p.attributes.solstatus.name)

if p.attributes.solstatus in [SolStatus.FEASIBLE, SolStatus.OPTIMAL]:
    for i in range(2):
        print(f"x[{i}] = {p.getSolution(x[i])}")

    # getIndicators returns the indicator variable index and the sense flag
    # for each row.
    ind_vars, ind_flags = p.getIndicators(0, 1)
    for i in range(2):
        print(f"indicator {i}: b[{i}] = {p.getSolution(b[i])}, flag = {ind_flags[i]}")
