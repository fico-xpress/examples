'''********************************************************
  * Python Example Problems                               *
  *                                                       *
  * file ProductsMachines_std.py                          *
  * -- Multiple conditions on loop over block of          *
  * statements --                                         *
  *                                                       *
  * (c) 2019-2026 Fair Isaac Corporation*
  ******************************************************'''
#S:IMPORT
import xpress as xp
import csv
import sys

if len(sys.argv) > 1:
    DATA_FILE_PREFIX = sys.argv[1]
else:
    DATA_FILE_PREFIX = "00"
print("#E:IMPORT")

print("#S:READ")
with open(DATA_FILE_PREFIX + '_H_ProductsMachines_K.csv') as f:
    reader = csv.DictReader(f)
    K = { (int(row['PROD']),int(row['MACH'])) : int(row['K']) for row in reader }

with open(DATA_FILE_PREFIX + '_H_ProductsMachines_T.csv') as f:
    reader = csv.DictReader(f)
    T = { int(row['MACH']) : int(row['T']) for row in reader }

with open(DATA_FILE_PREFIX + '_H_ProductsMachines_U.csv') as f:
    reader = csv.DictReader(f)
    U = { int(row['PROD']) : int(row['U']) for row in reader }
print("#E:READ")

print("#S:PROC")
IND = [ (p, m) for (p, m) in K.keys() if K[p, m] == 1 and 3 * T[m] < 13 ]

prob = xp.problem()

z = [ prob.addVariable(ub = U[p]) for (p, m) in IND ]
y = [ prob.addVariable(vartype = xp.binary) for (p, m) in IND ]
print("#E:PROC")

print("#S:TEST")
for (p, m) in IND:
    print('{} {}'.format(p, m))
print("#E:TEST")
