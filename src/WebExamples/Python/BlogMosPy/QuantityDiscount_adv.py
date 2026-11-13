'''********************************************************
  * Python Example Problems                               *
  *                                                       *
  * file QuantityDiscount_adv.py                          *
  * Improved version of model 'QuantityDiscount_std.py'   *
  * -- Multiple cases within a loop --                    *
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
V = pd.read_csv(DATA_FILE_PREFIX + '_H_QuantityDiscount_V.csv', index_col = ['P']).squeeze()
print("#E:READ")

print("#S:PROC")
a = np.array([0, 1, 5, 50])
v = np.array([1.50, 1.45, 1.30, 1.25, 1.20])

W = pd.Series(data = v[np.searchsorted(a, V)], index = V.index, name = 'W')

S = W.dot(V)
print("#E:PROC")

print("#S:TEST")
print('{:.2f}'.format(S))
print("#E:TEST")
