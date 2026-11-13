'''********************************************************
  * Python Example Problems                               *
  *                                                       *
  * file ComplexConstraints_adv.py                        *
  * Improved version of model 'ComplexConstraints_std.py' *
  * -- Defining constraints with multiple conditions --   *
  *                                                       *
  * (c) 2019-2026 Fair Isaac Corporation*
  ******************************************************'''
#S:IMPORT
import pandas as pd
import numpy as np
import xpress as xp
import sys

if len(sys.argv) > 1:
    DATA_FILE_PREFIX = sys.argv[1]
else:
    DATA_FILE_PREFIX = "00"
print("#E:IMPORT")

print("#S:READ")
D = pd.read_csv(DATA_FILE_PREFIX + '_H_ComplexConstraints_D.csv', index_col = ['RN1', 'RN2']).squeeze()
print("#E:READ")

print("#S:PROC")
F = 0.8
G = 0.7

N = D.index.max()[0] + 1
RN = np.arange(N)
[i, k, j, m] = np.meshgrid(RN, RN, RN, RN)
i = i.ravel()
k = k.ravel()
j = j.ravel()
m = m.ravel()

D = np.reshape(D.values, (N, N)).T
Dmax = D.max(axis = 1)

sel = (2 * D[i, k] <= G) & \
        (D[i, k] + F * Dmax[k] <= G) & \
        (2 * D[j, m] <= G) & (D[j, m] + F * Dmax[m] <= G) & \
        (i < j) & (k != j) & (i != m) & (D[i, k] + F * D[k, m] + D[m, j] > G)

i = i[sel]
k = k[sel]
j = j[sel]
m = m[sel]

p = xp.problem()

q = p.addVariables(N,N)

constr = q[i, k] + q[j, m] <= 1

p.addConstraint(constr)
print("#E:PROC")

print("#S:TEST")
for (ii, kk, jj, mm) in zip(i, k, j, m):
    print('{} {} {} {}'.format(ii, kk, jj, mm))
print("#E:TEST")
