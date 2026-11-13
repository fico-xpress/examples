'''********************************************************
  * Python Example Problems                               *
  *                                                       *
  * file SparseVariables_adv.py                           *
  * Improved version of model 'SparseVariables_std.py'    *
  * -- Enumeration of sparse multidimensional arrays --   *
  *                                                       *
  * (c) 2019-2026 Fair Isaac Corporation*
  ******************************************************'''
#S:IMPORT
import pandas as pd
import xpress as xp
import sys

if len(sys.argv) > 1:
    DATA_FILE_PREFIX = sys.argv[1]
else:
    DATA_FILE_PREFIX = "00"
print("#E:IMPORT")

print("#S:READ")
C = pd.read_csv(DATA_FILE_PREFIX + '_H_SparseVariables_C.csv', index_col = ['i', 'j', 'k', 'l']).squeeze()
print("#E:READ")

print("#S:PROC")
p = xp.problem()

f = pd.Series(
    data = [ p.addVariable() for _ in range(C.shape[0]) ],
    name = 'f', index = C.index)
print("#E:PROC")

print("#S:TEST")
for (i, j, k, l) in C.index.values:
    print('{} {} {} {}'.format(i, j, k, l))
print("#E:TEST")
