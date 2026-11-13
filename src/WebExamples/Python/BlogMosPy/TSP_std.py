'''********************************************************
  * Python Example Problems                               *
  *                                                       *
  * file TSP_std.py                                       *
  * -- Advanced optimization tasks (solution loading,     *
  * callbacks, cuts) --                                   *
  *                                                       *
  * (c) 2019-2026 Fair Isaac Corporation*
  ******************************************************'''
print("#S:IMPORT")
import xpress as xp
import math
import csv
import sys

if len(sys.argv) > 1:
    DATA_FILE_PREFIX = sys.argv[1]
else:
    DATA_FILE_PREFIX = "00"
print("#E:IMPORT")

def argmax(xs):
    return max((x,i) for i, x in enumerate(xs))[1]

def cb_preintsol(prob, CITIES, isheur=True, cutoff=0):
    '''Callback for checking if solution is acceptable
    '''

    n = len(CITIES)
    xsol = prob.getCallbackSolution()
    xsol = [ [ xsol[i*n+j] for j in range(n) ] for i in range(n) ] # matrix-shaped
    nextc = { i : CITIES[argmax(xsol[i])] for i in CITIES }

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


def cb_optnode(prob, CITIES):
    '''Callback used after LP solution is known at BB node. Add subtour elimination cuts
    '''

    n = len(CITIES)
    xsol = prob.getCallbackSolution()

    xsolf = xsol  # flattened
    xsol = [ [ xsolf[i*n+j] for j in range(n) ] for i in range(n) ] # matrix-shaped

    # Obtain an order by checking the maximum of the variable matrix
    # for each row
    nextc = { i : CITIES[argmax(xsol[i])] for i in CITIES }
    unchecked = [0] * n
    ngroup = 0

    # Initialize the vectors to be passed to addcuts

    cut_mstart = [0]
    cut_ind = []
    cut_coe = []
    cut_rhs = []

    nnz = 0
    ncuts = 0

    while min(unchecked) == 0 and ngroup <= n:
        '''Seek a tour
        '''

        ngroup += 1

        firstcity = unchecked.index(min(unchecked))

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
        S     = [ uci for uci, uc in enumerate(unchecked) if uc == ngroup ]
        compS = [ uci for uci, uc in enumerate(unchecked) if uc != ngroup ]

        indices = [i*n+j for i in S for j in compS]

        # Check if solution violates the cut, and if so add the cut to
        # the list.
        if sum(xsolf[i] for i in indices) < 1 - 1e-3:

            mcolsp, dvalp = [], []

            # Presolve cut in order to add it to the presolved problem
            # (the problem currently being solved by the
            # branch-and-bound).
            drhsp, status = prob.presolverow('G', indices, [1] * len(indices), 1,
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


def print_sol(p, CITIES):
    '''Print the solution: order of nodes and cost
    '''
    n = len(CITIES)
    xsol = p.getSolution()
    xsol = [ [ xsol[i*n+j] for j in range(n) ] for i in range(n) ]
    nextc = { i : CITIES[argmax(xsol[i])] for i in CITIES }

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
    CITIES = []  # set of cities

    X = []
    Y = []

    with open(DATA_FILE_PREFIX + '_H_TSP_cities.csv') as csvfile:
        reader = csv.DictReader(csvfile)
        for r in reader:
            CITIES.append(int(r['CITY']) - 1)
            X.append(float(r['X']))
            Y.append(float(r['Y']))

    n = len(CITIES)

    initial_tours = []
    with open(DATA_FILE_PREFIX + '_H_TSP_tours.csv') as csvfile:
        reader = csv.reader(csvfile)
        for r in reader:
            p = [ int(x) - 1 for x in r ]
            sd = { s : d for s, d in zip(p[:-1], p[1:]) }

            fly = { (i, j) : 1 if sd[i] == j else 0 for i in CITIES for j in CITIES }

            initial_tours.append(fly)
    print("#E:READ")

    print("#S:PROC")
    # Compute distance matrix
    def distance(i, j):
        return math.ceil(math.sqrt((X[i] - X[j])**2 + (Y[i] - Y[j])**2))

    dist = { (i, j) : distance(i, j) for i in CITIES for j in CITIES }

    p = xp.problem()

    # Create variables as a square matrix of binary variables
    fly = p.addVariables(n, n, vartype=xp.binary, name='x')

    # save the indicies of decision variables (used later when extracting solution)
    fly_i = [ [p.getIndex(fly[i, j]) for j in CITIES] for i in CITIES ]

    # Degree constraints
    for i in CITIES:
        p.addConstraint(xp.Sum(fly[i,j] for j in CITIES if i != j) == 1)

    for i in CITIES:
        p.addConstraint(xp.Sum(fly[j,i] for j in CITIES if i != j) == 1)

    # Objective function
    p.setObjective (xp.Sum(dist[i, j] * fly[i, j] for i in CITIES for j in CITIES))

    # Add callbacks
    p.addPreIntsolCallback(cb_preintsol, CITIES)
    p.addOptnodeCallback(cb_optnode, CITIES)

    p.controls.symmetry = 0
    p.controls.outputlog = 1

    # Create 10 trivial solutions: simple tour 0->1->2...->n->0
    # randomly permuted
    for (k, InitTour) in enumerate(initial_tours):
        mipsolval = [ InitTour[i, j] for i in CITIES for j in CITIES ]
        mipsolcol = [ fly[i, j] for i in CITIES for j in CITIES ]
        p.addmipsol(solval=mipsolval, colind=mipsolcol, name="InitTour_{}".format(k))

    p.controls.maxtime=-10  # set a time limit
    p.optimize()
    print("#E:PROC")

    print("#S:TEST")
    print_sol(p,CITIES)  # print solution and cost
    print("#E:TEST")

if __name__ == '__main__':
    solve_opttour()
