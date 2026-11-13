# Example of using the load*() functions for several problem types.
#
# Usage: define PROBTYPE parameter.
#
# (C) 1983-2026 Fair Isaac Corporation

import xpress as xp

PROBTYPE = 'LP'

'''Select problem type:
    'LP': LP problem with three variables and four constraints
    'QP': LP with a quadratic objective
    'QCQP': quadratically constrained quadratic problem
    'MIQCQP': QCQP where variables are integer or binary
    'MIQCQPSOS': MIQCQP with Special Ordered Sets (SOSs)
'''

def loadlp():
    '''LP problem with three variables and four constraints.'''
    p = xp.problem()

    # Fill in an LP problem with three variables and four constraints.
    p.loadLP("",                           # probname
                ['G', 'G', 'E', 'L'],      # rowtype
                [-2.4, -3, 4, 5],          # rhs
                None,                      # rng
                [3, 4, 5],                 # objcoef
                [0, 2, 4, 8],              # start
                None,                      # collen
                [0, 1, 2, 3, 0, 1, 2, 3],  # rowind
                [1, 1, 1, 1, 1, 1, 1, 1],  # rowcoef
                [-1, -1, -1],              # lb
                [3, 5, 8])                 # ub

    p.writeProb("loadedlp", "lp")
    p.optimize()

    # Get a variable and modify the objective
    # function. Note that the objective function is replaced by, not
    # amended with, the new objective.
    x1 = p.getVariable(0)

    p.setObjective(x1 ** 2 + 2 * x1 + 444)
    p.optimize()
    p.writeProb("updatedlp", "lp")

def loadqp():
    '''LP problem with a quadratic objective.'''
    p = xp.problem()

    # Fill in a QP problem with three variables and four constraints.
    p.loadQP("",                           # probname
                ['G', 'G', 'E', 'L'],      # rowtype
                [-2.4, -3, 4, 5],          # rhs
                None,                      # rng
                [3, 4, 5],                 # objcoef
                [0, 2, 4, 8],              # start
                None,                      # collen
                [0, 1, 2, 3, 0, 1, 2, 3],  # rowind
                [1, 1, 1, 1, 1, 1, 1, 1],  # rowcoef
                [-1, -1, -1],              # lb
                [3, 5, 8],                 # ub
                [0, 0, 0, 1, 1, 2],        # objqcol1
                [0, 1, 2, 1, 2, 2],        # objqcol2
                [2, 1, 1, 2, 1, 2])        # objqcoef

    p.writeProb("loadedqp", "lp")
    p.optimize()

def loadqcqp():
    '''Quadratically constrained quadratic problem.'''
    p = xp.problem()

    # Fill in a QCQP problem with three variables and four constraints.
    p.loadQCQP("",                         # probname
                ['G', 'G', 'L', 'L'],      # rowtype
                [-2.4, -3, 4, 5],          # rhs
                None,                      # rng
                [3, 4, 5],                 # objcoef
                [0, 2, 4, 8],              # start
                None,                      # collen
                [0, 1, 2, 3, 0, 1, 2, 3],  # rowind
                [1, 1, 1, 1, 1, 1, 1, 1],  # rowcoef
                [-1, -1, -1],              # lb
                [3, 5, 8],                 # ub
                [0, 0, 0, 1, 1, 2],        # objqcol1
                [0, 1, 2, 1, 2, 2],        # objqcol2
                [2, 1, 1, 2, 1, 2],        # objqcoef
                [2, 3],                    # qrowind
                [2, 3],                    # nrowqcoef
                [1, 2, 0, 0, 2],           # rowqcol1
                [1, 2, 0, 2, 2],           # rowqcol2
                [3, 4, 1, 1, 1])           # rowqcoef

    p.writeProb("loadedqcqp", "lp")
    p.optimize()

def loadmiqcqp():
    '''QCQP where variables are integer or binary.'''
    p = xp.problem()

    # Fill in an MIQCQP problem with three variables and four constraints.
    p.loadMIQCQP("",                       # probname
                ['G', 'G', 'L', 'L'],      # rowtype
                [-2.4, -3, 4, 5],          # rhs
                None,                      # rng
                [3, 4, 5],                 # objcoef
                [0, 2, 4, 8],              # start
                None,                      # collen
                [0, 1, 2, 3, 0, 1, 2, 3],  # rowind
                [1, 1, 1, 1, 1, 1, 1, 1],  # rowcoef
                [-1, -1, -1],              # lb
                [3, 5, 8],                 # ub
                [0, 0, 0, 1, 1, 2],        # objqcol1
                [0, 1, 2, 1, 2, 2],        # objqcol2
                [2, 1, 1, 2, 1, 2],        # objqcoef
                [2, 3],                    # qrowind
                [2, 3],                    # nrowqcoefs
                [1, 2, 0, 0, 2],           # rowqcol1
                [1, 2, 0, 2, 2],           # rowqcol2
                [3, 4, 1, 1, 1],           # rowqcoef
                ['I', 'S'],                # coltype
                [0, 1],                    # entind
                [0, 2])                    # limit

    p.writeProb("loadedmiqcqp", "lp")
    p.optimize()
    print(p.getSolution())

def loadmiqcqp_sos():
    '''MIQCQP with Special Ordered Sets (SOSs).'''
    p = xp.problem()

    # Fill in an MIQCQP problem with two Special Ordered Sets (SOSs).
    p.loadMIQCQP("",                       # probname
                ['G', 'G', 'L', 'L'],      # rowtype
                [-2.4, -3, 4, 5],          # rhs
                None,                      # rng
                [3, 4, 5],                 # objcoef
                [0, 2, 4, 8],              # start
                None,                      # collen
                [0, 1, 2, 3, 0, 1, 2, 3],  # rowind
                [1, 1, 1, 1, 1, 1, 1, 1],  # rowcoef
                [-1, -1, -1],              # lb
                [3, 5, 8],                 # ub
                [0, 0, 0, 1, 1, 2],        # objqcol1
                [0, 1, 2, 1, 2, 2],        # objqcol2
                [2, 1, 1, 2, 1, 2],        # objqcoef
                [2, 3],                    # qrowind
                [2, 3],                    # nrowqcoefs
                [1, 2, 0, 0, 2],           # rowqcol1
                [1, 2, 0, 2, 2],           # rowqcol2
                [3, 4, 1, 1, 1],           # rowqcoef
                ['I', 'S'],                # coltype
                [0, 1],                    # entind
                [0, 2],                    # limit
                ['1', '1'],                # settype
                [0, 2, 4],                 # setstart
                [0, 1, 0, 2],              # setind
                [1.1, 1.2, 1.3, 1.4])      # refval

    p.writeProb("loadedmiqcqp_sos", "lp")
    p.optimize()
    print(p.getSolution())

if PROBTYPE == 'LP':
    loadlp()
elif PROBTYPE == 'QP':
    loadqp()
elif PROBTYPE == 'QCQP':
    loadqcqp()
elif PROBTYPE == 'MIQCQP':
    loadmiqcqp()
elif PROBTYPE == 'MIQCQPSOS':
    loadmiqcqp_sos()
else:
    print("Invalid choice of PROBTYPE parameter. Please try again.")
