'''********************************************************
  * Python Example Problems                               *
  *                                                       *
  * file MatrixVariables_adv.py                           *
  * Improved version of model 'MatrixVariables_std.py'    *
  * -- Defining constraints over a 2-dimensional array    *
  * of variables --                                       *
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
S = pd.read_csv(DATA_FILE_PREFIX + '_H_MatrixVariables_S.csv', index_col = ['T']).squeeze().values
V = pd.read_csv(DATA_FILE_PREFIX + '_H_MatrixVariables_V.csv', index_col = ['P']).squeeze().values
print("#E:READ")

print("#S:PROC")
nT = len(S)
nP = len(V)

p = xp.problem()

Q = p.addVariables(nT, nP)

p.addConstraint(xp.Dot(Q, V) <= S)
print("#E:PROC")
