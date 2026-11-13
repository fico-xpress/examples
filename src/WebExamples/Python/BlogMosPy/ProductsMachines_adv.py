'''********************************************************
  * Python Example Problems                               *
  *                                                       *
  * file ProductsMachines_adv.py                          *
  * Improved version of model 'ProductsMachines_std.py'   *
  * -- Multiple conditions on loop over block of          *
  * statements --                                         *
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
K = pd.read_csv(DATA_FILE_PREFIX + '_H_ProductsMachines_K.csv', index_col = ['PROD', 'MACH']).squeeze()
T = pd.read_csv(DATA_FILE_PREFIX + '_H_ProductsMachines_T.csv', index_col = 'MACH').squeeze()
U = pd.read_csv(DATA_FILE_PREFIX + '_H_ProductsMachines_U.csv', index_col = 'PROD').squeeze()
print("#E:READ")

print("#S:PROC")
SEL = 3 * T[K.index.get_level_values('MACH')] < 13
SEL.index = K.index

IND = K[(K == 1) & SEL]

p = xp.problem()

z = pd.Series(
    data = [ p.addVariable(ub = u) for u in U.loc[IND.index.get_level_values('PROD')].values ],
    name = 'z', index = IND.index, dtype = object)

y = pd.Series(
    data = [ p.addVariable(vartype = xp.binary) for _ in range(IND.shape[0]) ],
    name = 'y', index = IND.index, dtype = object)
print("#E:PROC")

print("#S:TEST")
for (p, m) in IND.index.values:
    print('{} {}'.format(p, m))
print("#E:TEST")
