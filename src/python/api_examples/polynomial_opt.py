# A problem with a polynomial constraint constructed with the xpress.Dot product.
#
# Generate a random coefficient tensor T of dimension k + 1 and sizes
# n+1 for each dimension except for the first, which is h, then use it
# to create h polynomial constraints. The lhs of each constraint has a
# polynomial of degree k, and not homogeneous as we amend the vector
# of variable with the constant 1. This is accomplished via a single
# dot product.
#
# (C) 1983-2026 Fair Isaac Corporation

import xpress as xp
import numpy as np

n = 4   # dimension of variable space
h = 3   # number of polynomial constraints
k = 3   # degree of each polynomial

p = xp.problem()

# Vector of n elements: (1, x1, ..., x_{n-1})
x = xp.array([1] + [p.addVariable(lb=-10, ub=10) for _ in range(n-1)])

sizes = [n]*k  # creates list [n,n,...,n] of k elements

# Operator * before a list translates the list into its
# (unparenthesized) tuple, i.e., the result is a reshaped list of
# arguments that looks like (h, n, n, ..., n).
np.random.seed(1)
T = np.random.random(h * n ** k).reshape(h, *sizes)

T2list = [x]*k

compact = xp.Dot(T, *T2list) <= 0

p.addConstraint(compact)

# Maximize the sum of the variables.
p.setObjective(xp.Sum(x), sense=xp.ObjSense.MAXIMIZE)

# Solve this problem with a local nonlinear solver.
p.controls.nlpsolver = xp.constants.NLPSOLVER_LOCAL
p.optimize()
