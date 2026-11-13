'''********************************************************
  * Python Example Problems                               *
  *                                                       *
  * file SparseVariables_std.py                           *
  * -- Enumeration of sparse multidimensional arrays --   *
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
with open(DATA_FILE_PREFIX + '_H_SparseVariables_C.csv') as f:
    reader = csv.DictReader(f)
    C = { (int(row['i']), int(row['j']), int(row['k']), int(row['l'])) : int(row['C']) for row in reader }
print("#E:READ")

print("#S:PROC")
p = xp.problem()

f = { (i, j, k, l) : p.addVariable() for (i, j, k, l) in C.keys() }
print("#E:PROC")

print("#S:TEST")
for (i, j, k, l) in C.keys():
    print('{} {} {} {}'.format(i, j, k, l))
print("#E:TEST")
