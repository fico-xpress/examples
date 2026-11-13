'''********************************************************
  * Python Example Problems                               *
  *                                                       *
  * file MatrixVariables_std.py                           *
  * -- Defining constraints over a 2-dimensional array    *
  * of variables --                                       *
  *                                                       *
  * (c) 2019-2026 Fair Isaac Corporation*
  ******************************************************'''
#S:IMPORT
import csv
import xpress as xp
import sys

if len(sys.argv) > 1:
    DATA_FILE_PREFIX = sys.argv[1]
else:
    DATA_FILE_PREFIX = "00"
print("#E:IMPORT")

print("#S:READ")
with open(DATA_FILE_PREFIX + '_H_MatrixVariables_S.csv') as f:
    reader = csv.DictReader(f)
    S = { int(row['T']) : int(row['S']) for row in reader }
with open(DATA_FILE_PREFIX + '_H_MatrixVariables_V.csv') as f:
    reader = csv.DictReader(f)
    V = { int(row['P']) : int(row['V']) for row in reader }
print("#E:READ")

print("#S:PROC")
T = list(S.keys())
P = list(V.keys())

p = xp.problem()

Q = p.addVariables(T, P)

p.addConstraint(xp.Sum(Q[t, p] * V[p] for p in P) <= S[t] for t in T)
print("#E:PROC")
