"""
   Xpress Python Example Problems
   ======================

   file intromip.py
   `````````````````
   Various code snippest from introductory section of
   the mipformref whitepaper

   (c) 2024 Fair Isaac Corporation
       author: B. Vieira, Sep. 2024, rev. Aug. 2026
"""

import xpress as xp
from xpress.enums import SolStatus

MAXVAL = 50
NP = 5
NL = 6

p = xp.problem()

d = p.addVariable(vartype=xp.binary, name="d")                      # A single binary variable

ifmake = p.addVariables(NP, NL, vartype=xp.binary, name="ifmake")   # A NumPy array of variables

x = p.addVariable(lb=0, ub=MAXVAL, vartype=xp.integer, name="x")    # An integer variable with an upper bound

x.vartype = xp.semicontinuous    # Change type to semi-continuous
x.threshold = 10

x.vartype = xp.continuous        # Change type to continuous

p.setObjective(d)

p.optimize()

match p.attributes.solstatus:
    case SolStatus.FEASIBLE | SolStatus.OPTIMAL:
        print("MIP solution: ", p.attributes.objval)
    case SolStatus.INFEASIBLE:
        print("Problem is infeasible")
    case SolStatus.UNBOUNDED:
        print("LP unbounded")
    case SolStatus.NOTFOUND:
        print("Solution not found")

print(x.name,": ",p.getSolution(x))
