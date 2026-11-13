# Compare matrix coefficients of two problems.
#
# Given two problems with the same number of variables, read their
# coefficient matrices into Scipy to compare each row for
# discrepancies in the coefficients.
#
# (C) 1983-2026 Fair Isaac Corporation

import xpress as xp
import scipy.sparse

p1 = xp.problem()
p2 = xp.problem()

# Read problems from file. Works also with problems created with the
# modeling features of the Python interface.

p1.readProb('Data/prob1.lp')
p2.readProb('Data/prob2.lp')

# Obtain matrix representation of the coefficient matrix for both
# problems.

beg1, ind1, coef1 = p1.getRows(0, p1.attributes.rows - 1)
beg2, ind2, coef2 = p2.getRows(0, p2.attributes.rows - 1)

# Create a Compressed Sparse Row (CSR) format matrix using the data
# from getrows() plus the numerical indices.

A1 = scipy.sparse.csr_matrix((coef1, ind1, beg1))
A2 = scipy.sparse.csr_matrix((coef2, ind2, beg2))

# Convert the CSR matrix to a NumPy array of arrays, so that each row
# is a (non-compressed) array to be compared in the loop below.

M1 = A1.toarray()
M2 = A2.toarray()

for i in range(min(p1.attributes.rows, p2.attributes.rows)):
    print(M1[i] != M2[i])
