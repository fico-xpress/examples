# Example for the use of the Python language (Network problem).
#
# Transport from depots to customers.
#
# (C) 2018-2026 Fair Isaac Corporation

import xpress as xp
from Data.trans_data import AVAIL, DEMAND, COST

p = xp.problem()

Suppliers = AVAIL.keys()
Customers = DEMAND.keys()
Pairs = COST.keys()

x = {(i, j): p.addVariable() for i, j in Pairs}

p.setObjective(xp.Sum(COST[i, j] * x[i, j] for i, j in Pairs))

p.addConstraint((xp.Sum([x[s, c] for c in Customers if (s, c) in Pairs])
                <= AVAIL[s] for s in Suppliers), name="supply")
p.addConstraint((xp.Sum([x[s, c] for s in Suppliers if (s, c) in Pairs])
                >= DEMAND[c] for c in Customers), name="demand")

p.optimize()

# Print shadow prices labelled by constraint name.
names = p.getNameList(xp.Namespaces.ROW)
duals = p.getDuals()
print("Shadow prices:")
for name, dual in zip(names, duals):
    print(f"  {name}: {dual:.4f}")
