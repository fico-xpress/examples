# Example to show how to retrieve the coefficient matrix from a
# problem.
#
# (C) 1983-2026 Fair Isaac Corporation

import xpress as xp
import scipy.sparse

p = xp.problem()

p.readProb('Data/prob1.lp')

# Obtain matrix representation of the coefficient matrix for problem.

beg, ind, coef = p.getRows(0, p.attributes.rows - 1)

# Create a Compressed Sparse Row (CSR) format matrix using the data
# from getRows.

A = scipy.sparse.csr_matrix((coef, ind, beg))

# Convert the CSR matrix to a NumPy array of arrays, so that each row
# is a (non-compressed) array.

M = A.toarray()

print(A)
print(M)

c = p.getObj(0, p.attributes.cols - 1)
b = p.getRHS(0, p.attributes.rows - 1)

print(b)
print(c)
