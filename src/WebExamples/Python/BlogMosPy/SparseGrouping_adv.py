'''********************************************************
  * Python Example Problems                               *
  *                                                       *
  * file SparseGrouping_adv.py                            *
  * Improved version of model 'SparseGrouping_std.py'     *
  * -- Multiple cases within a loop over a sparse         *
  * array --                                              *
  *                                                       *
  * (c) 2019-2026 Fair Isaac Corporation*
  ******************************************************'''
#S:IMPORT
import pandas as pd
import numpy as np
import sys

if len(sys.argv) > 1:
    DATA_FILE_PREFIX = sys.argv[1]
else:
    DATA_FILE_PREFIX = "00"
print("#E:IMPORT")

print("#S:READ")
C = pd.read_csv(DATA_FILE_PREFIX + '_H_SparseGrouping_C.csv', index_col = ['i', 'j', 'k', 'l']).squeeze()
print("#E:READ")

print("#S:PROC")
a = np.array([100, 1000]) - 1
v = np.array([1, 2, 3])

G = pd.Series(data = v[np.searchsorted(a, C)], index = C.index, name = 'G')
print("#E:PROC")

print("#S:TEST")
for ((i, j, k, l), v) in G.items():
    print('{} {} {} {} {}'.format(i, j, k, l, v))
print("#E:TEST")


# Pietro suggestion
