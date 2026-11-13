# Given an infeasible LP, find an (infeasible) solution that minimize
# the total distance from the constraints. Then solve the obtained MaxFS problem.
#
# (C) 1983-2026 Fair Isaac Corporation

import xpress as xp

p = xp.problem()

x = p.addVariable()
y = p.addVariable()

# Build a very simple problem with pairs of incompatible constraints.

lhs1 = 2*x + 3*y
lhs2 = 3*x + 2*y
lhs3 = 4*x + 5*y

p.addConstraint(lhs1 >= 6, lhs1 <= 5)
p.addConstraint(lhs2 >= 5, lhs2 <= 4)
p.addConstraint(lhs3 >= 8, lhs3 <= 7)

p.optimize()

assert(p.attributes.solstatus == xp.SolStatus.INFEASIBLE)

# We verified the problem is infeasible. Add one binary for each
# constraint to selectively relax them.

m = p.attributes.rows

# Get the signs of all constraints: 'E', 'L', or 'G'. Note that this example
# only works with inequality constraints.
sign = p.getRowType(0, m - 1)

# Big-M: large-enough constant to relax all constraints (quite conservative
# here).
M = 1e3

matval = [M]*m
for i in range(m):
    if sign[i] == 'L':
        matval[i] = -M

# Add m new binary columns.

p.addCols([1]*m,  # obj. coefficients (as many 1s as there are constraints)
          range(m + 1),  # cumulative number of terms in each column:
                         # 0,1,2,...,m as there is one term per column
          range(m), matval,  # pairs (row_index, coefficient) for each column
          [0]*m, [1]*m  # lower, upper bound (binary variables, so {0,1})
)
p.addNames(xp.Namespaces.COLUMN, ['b_{}'.format(i) for i in range(m)], 2, p.attributes.cols - 1)
p.chgColType(range(2, p.attributes.cols), ['B']*m)  # change type to binary
p.optimize()

# Print constraints constituting a Maximum Feasible Subsystem.

b = p.getSolution(range(p.attributes.cols - m, p.attributes.cols))

maxfs = [i for i in range(m) if b[i] > 0.5]

print('MaxFS has ', len(maxfs), 'constraints:', maxfs)
