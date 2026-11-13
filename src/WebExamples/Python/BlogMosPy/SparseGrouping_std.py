'''********************************************************
  * Python Example Problems                               *
  *                                                       *
  * file SparseGrouping_std.py                            *
  * -- Multiple cases within a loop over a sparse         *
  * array --                                              *
  *                                                       *
  * (c) 2019-2026 Fair Isaac Corporation*
  ******************************************************'''
#S:IMPORT
import csv
import sys

if len(sys.argv) > 1:
    DATA_FILE_PREFIX = sys.argv[1]
else:
    DATA_FILE_PREFIX = "00"
print("#E:IMPORT")

print("#S:READ")
with open(DATA_FILE_PREFIX + '_H_SparseGrouping_C.csv') as f:
    reader = csv.DictReader(f)
    C = { (int(row['i']), int(row['j']), int(row['k']), int(row['l'])) : int(row['C']) for row in reader }
print("#E:READ")

print("#S:PROC")
def f(x):
    if x < 100:     return 1
    elif x < 1000:  return 2
    else:           return 3

G = { k : f(x) for (k, x) in C.items() }
print("#E:PROC")

print("#S:TEST")
for (i, j, k, l) in C.keys():
    print('{} {} {} {} {}'.format(i, j, k, l, G[i, j, k, l]))
print("#E:TEST")
