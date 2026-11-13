# Use xpress.array for creating a 3-dimensional array of variables,
# then use it to create a model.
#
# (C) 1983-2026 Fair Isaac Corporation

import xpress as xp

S1 = range(2)
S2 = range(3)
S3 = range(4)

m = xp.problem()

# Create an array of variables
h = xp.array([[[m.addVariable(vartype=xp.binary) for i in S1]
               for j in S2] for k in S3])

m.setObjective(h[0][0][0] * h[0][0][0] +
               h[1][0][0] * h[0][0][0] +
               h[1][0][0] * h[1][0][0] +
               xp.Sum(h[i][j][k]
                      for i in S3 for j in S2 for k in S1))

cons00 = - h[0][0][0] * h[0][0][0] + \
         xp.Sum(i * j * k * h[i][j][k]
                for i in S3 for j in S2 for k in S1) >= 11

m.addConstraint(cons00)

# By default, the problem is solved to global optimality.
# Setting the nlpsolver control to one ensures the problem is
# solved the local nonlinear solver.
m.controls.nlpsolver = 1

m.optimize()

# Get the matrix representation of the quadratic part of the single
# constraint.

mstart1, mclind1, dqe1 = m.getQRowQMatrix(cons00, h[0][0][0], h[3][2][1])
print("row 0:", mstart1, mclind1, dqe1)
