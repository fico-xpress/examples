'''********************************************************
  * Python Example Problems                               *
  *                                                       *
  * file ComplexConstraints_std.py                        *
  * -- Defining constraints with multiple conditions --   *
  *                                                       *
  * (c) 2019-2026 Fair Isaac Corporation*
  ******************************************************'''
#S:IMPORT
import xpress as xp
import itertools
import csv
import sys

if len(sys.argv) > 1:
    DATA_FILE_PREFIX = sys.argv[1]
else:
    DATA_FILE_PREFIX = "00"
print("#E:IMPORT")

print("#S:READ")
with open(DATA_FILE_PREFIX + '_H_ComplexConstraints_D.csv') as f:
    reader = csv.DictReader(f)
    D = { (int(row['RN1']),int(row['RN2'])) : float(row['D']) for row in reader }
print("#E:READ")

print("#S:PROC")
F = 0.8
G = 0.7

N = max(D.keys())[0] + 1
RN = list(range(N))

Dmax = { mm : max(D[mm, p] for p in RN) for mm in RN }

p = xp.problem()

#q = {(i,j): xp.var() for i,j in itertools.product(RN, repeat = 2)}
q = p.addVariables(RN,RN)

constr = [ q[i,k]+q[j,m]<=1 for i,j,k,m in itertools.product(RN, repeat = 4) \
                                  if 2 * D[i, k] <= G and D[i, k] + F * Dmax[k] <= G and \
                                     2 * D[j, m] <= G and D[j, m] + F * Dmax[m] <= G and \
                                     i < j and k != j and i != m and \
                                     D[i, k] + F * D[k, m] + D[m, j] > G  ]

p.addConstraint(constr)
print("#E:PROC")

print("#S:TEST")
inds = [ (i, k, j, m) for i,j,k,m in itertools.product(RN, repeat = 4) \
                                if 2 * D[i, k] <= G and D[i, k] + F * Dmax[k] <= G and \
                                    2 * D[j, m] <= G and D[j, m] + F * Dmax[m] <= G and \
                                    i < j and k != j and i != m and \
                                    D[i, k] + F * D[k, m] + D[m, j] > G  ]
for (ii, kk, jj, mm) in inds:
    print('{} {} {} {}'.format(ii, kk, jj, mm))
print("#E:TEST")
