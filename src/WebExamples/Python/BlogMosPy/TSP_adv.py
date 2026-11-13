'''********************************************************
  * Python Example Problems                               *
  *                                                       *
  * file TSP_adv.py                                       *
  * Improved version of model 'TSP_std.py'                *
  * -- Advanced optimization tasks (solution loading,     *
  * callbacks, cuts) --                                   *
  *                                                       *
  * (c) 2019-2026 Fair Isaac Corporation*
  ******************************************************'''
#S:IMPORT
import xpress as xp
import numpy as np
import pandas as pd
import sys

if len(sys.argv) > 1:
    DATA_FILE_PREFIX = sys.argv[1]
else:
    DATA_FILE_PREFIX = "00"
print("#E:IMPORT")

def cb_preintsol(prob, data, isheur=True, cutoff=0):
    '''Callback for checking if solution is acceptable
    '''

    n = data
    xsol = np.array(prob.getCallbackSolution()).reshape(n,n)
    nextc = np.argmax(xsol, axis=1)

    i = 0
    ncities = 1

    # Scan cities in order until we get back to 0 or the solution is
    # wrong and we're diverging
    while nextc[i] != 0 and ncities < n:
        ncities += 1
        i = nextc[i]

    # If the cities visited before getting back to 0 is less than n-1,
    # we just closed a subtour, hence the solution is infeasible
    return (ncities < n-1, None)


def cb_optnode(prob, data):
    '''Callback used after LP solution is known at BB node. Add subtour elimination cuts
    '''

    n = data
    xsol = prob.getCallbackSolution()
    xsolf = np.array(xsol)  # flattened
    xsol  = xsolf.reshape(n,n)  # matrix-shaped

    # Obtain an order by checking the maximum of the variable matrix
    # for each row
    nextc = np.argmax(xsol, axis=1)
    unchecked = np.zeros(n)
    ngroup = 0

    # Initialize the vectors to be passed to addcuts

    cut_mstart = [0]
    cut_ind = []
    cut_coe = []
    cut_rhs = []

    nnz = 0
    ncuts = 0

    while np.min(unchecked) == 0 and ngroup <= n:
        '''Seek a tour
        '''

        ngroup += 1

        firstcity = np.argmin(unchecked)

        assert (unchecked[firstcity] == 0)

        i = firstcity
        ncities = 0

        # Scan cities in order
        while True:

            unchecked[i] = ngroup  # mark city i with its new group, to be used in addcut
            ncities += 1
            i = nextc[i]

            if i == firstcity or ncities > n + 1:
                break

        if ncities == n and i == firstcity:
            return 0  # Nothing to add, solution is feasible

        # unchecked[unchecked == ngroup] marks nodes to be made part of subtour
        # elimination inequality

        # Find indices of current subtour. S is the set of nodes
        # traversed by the subtour, compS is its complement.
        S     = np.where(unchecked == ngroup)[0].tolist()
        compS = np.where(unchecked != ngroup)[0].tolist()

        indices = [i*n+j for i in S for j in compS]

        # Check if solution violates the cut, and if so add the cut to
        # the list.
        if sum(xsolf[i] for i in indices) < 1 - 1e-3:

            mcolsp, dvalp = [], []

            # Presolve cut in order to add it to the presolved problem
            # (the problem currently being solved by the
            # branch-and-bound).
            drhsp, status = prob.presolverow('G', indices, np.ones(len(indices)), 1,
                                             prob.attributes.cols,
                                             mcolsp, dvalp)

            nnz += len(mcolsp)
            ncuts += 1

            cut_ind.extend(mcolsp)
            cut_coe.extend(dvalp)
            cut_rhs.append(drhsp)
            cut_mstart.append(nnz)

    if ncuts > 0:

        assert (len(cut_mstart) == ncuts + 1)
        assert (len(cut_ind) == nnz)

        if status >= 0:
            prob.addcuts([0] * ncuts, ['G'] * ncuts, cut_rhs, cut_mstart, cut_ind, cut_coe)

    return 0


def print_sol(p, n):
    '''Print the solution: order of nodes and cost
    '''

    xsol = np.array(p.getSolution()).reshape(n,n)
    nextc = np.argmax(xsol, axis=1)

    i = 0

    # Scan cities in order
    s = []
    while True:
        s.append(i + 1)
        i = nextc[i]
        if i == 0:
            break
    s.append(i + 1)
    print('Total distance: {}'.format(int(p.attributes.objval)))
    print(" - ".join(map(str, s)))


def solve_opttour():
    '''Read a TSP problem
    '''

    print("#S:READ")
    df = pd.read_csv(DATA_FILE_PREFIX + '_H_TSP_cities.csv')

    CITIES = df['CITY'].values - 1 # set of cities
    X = df['X'].values
    Y = df['Y'].values

    n = len(CITIES)

    initial_tours_a = np.loadtxt(DATA_FILE_PREFIX + '_H_TSP_tours.csv', delimiter = ',', dtype='int') - 1
    initial_tours_a = initial_tours_a[:,:-1]
    print("#E:READ")

    print("#S:PROC")
    initial_tours = []
    for p in initial_tours_a:
        P = np.eye(n)[p]  # random permutation

        I = np.eye(n)
        S = np.vstack((I[1:,:],I[0,:]))  # Creates trivial tour
        initial_tours.append(np.dot(P.T, S, P).flatten())  # Permutes the tour

    # Compute distance matrix
    dist = np.ceil(np.sqrt ((X.reshape(n,1) - X.reshape(1,n))**2 +
                            (Y.reshape(n,1) - Y.reshape(1,n))**2))

    p = xp.problem()

    # Create variables as a square matrix of binary variables
    fly = p.addVariables(n, n, vartype=xp.binary, name='x')

    # Degree constraints
    p.addConstraint(xp.Sum(fly[i,:]) - fly[i,i] == 1  for i in CITIES)
    p.addConstraint(xp.Sum(fly[:,i]) - fly[i,i] == 1  for i in CITIES)

    # Objective function
    p.setObjective (xp.Sum((dist * fly).flatten()))

    # Add callbacks
    p.addPreIntsolCallback(cb_preintsol, n)
    p.addOptnodeCallback(cb_optnode, n)

    # Reset some presolve options -- unnecessary as we presolve the cuts
    # p.controls.presolve = 0
    # p.controls.presolveops &= ~(1 | 8 | 64 | 1024)
    # p.controls.nodedualreductions = 0
    p.controls.symmetry = 0
    p.controls.outputlog = 0

    # Create 10 trivial solutions: simple tour 0->1->2...->n->0
    # randomly permuted
    for (k, InitTour) in enumerate(initial_tours):
        p.addmipsol(solval=InitTour, name="InitTour_{}".format(k))

    p.controls.maxtime=-10  # set a time limit
    p.optimize()
    print("#E:PROC")

    print("#S:TEST")
    print_sol(p,n)  # print solution and cost
    print("#E:TEST")


if __name__ == '__main__':
    solve_opttour()
