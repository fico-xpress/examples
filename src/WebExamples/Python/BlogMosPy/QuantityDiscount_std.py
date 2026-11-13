'''********************************************************
  * Python Example Problems                               *
  *                                                       *
  * file QuantityDiscount_std.py                          *
  * -- Multiple cases within a loop --                    *
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
with open(DATA_FILE_PREFIX + '_H_QuantityDiscount_V.csv') as f:
    reader = csv.DictReader(f)
    V = { int(row['P']) : int(row['V']) for row in reader }
print("#E:READ")

print("#S:PROC")
def f(v):
    if v == 0:  return 1.50
    if v == 1:  return 1.45
    if v <= 5:  return 1.30
    if v <= 50: return 1.25
    else:       return 1.20

W = { p : v * f(v) for p, v in V.items() }

S = sum(W.values())
print("#E:PROC")

print("#S:TEST")
print('{:.2f}'.format(S))
print("#E:TEST")
