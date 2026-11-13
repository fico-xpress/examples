# Capacitated Facility Location Problem (CFLP) demonstrating the
# solution pool and solution enumeration.
#
# 25 customers, 10 potential facility sites with integer coordinates.
# Single-sourcing: each customer is assigned to exactly one open
# facility. Service cost equals the Euclidean distance from the
# customer to its assigned facility. Fixed opening cost is correlated
# with facility capacity.
#
# Demonstrates four uses of the built-in solution pool and enumeration
# controls on the CFLP model:
#   - collecting up to ten solutions found during a normal MIP solve
#     (poollimit);
#   - warm-starting a re-solve from the pool after fixing a facility
#     open (poolkeepsols);
#   - enumerating the five best solutions overall (poolenumstrategy,
#     poolrelgap);
#   - enumerating the five best facility assignments only, by
#     restricting enumeration to the facility variables
#     (loadBranchDirs).
# After each solve, the objective value, number of open facilities,
# and maximum service distance are printed for each collected
# solution.
#
# Model
# -----
# Sets
#     I = {0 ... N_CUST-1}   customers
#     J = {0 ... N_FAC-1}    potential facility sites
#
# Parameters
#     d_i      demand of customer i
#     f_j      fixed cost of opening facility j
#     Q_j      capacity of facility j
#     c_ij     Euclidean distance from customer i to facility j
#
# Decision variables
#     y_j  in {0,1}   1 iff facility j is opened
#     x_ij in {0,1}   1 iff customer i is served by facility j
#
# Objective
#     min  sum_j f_j y_j  +  sum_i sum_j c_ij x_ij
#
# Constraints
#     (1)  sum_j x_ij = 1               for all i  (demand satisfaction)
#     (2)  sum_i d_i x_ij <= Q_j y_j    for all j  (capacity + activation)
#
# (C) 2026 Fair Isaac Corporation

import math
import xpress as xp


N_CUST = 25
N_FAC = 10

cust_xy = [
    (20, 9), (22, 15), (5, 13), (6, 26), (13, 14),  # cluster A -- south-west
    (15, 19), (20, 19), (11, 7), (3, 21),
    (70, 27), (69, 33), (82, 26), (74, 30), (77, 20),  # cluster B -- SE
    (82, 11), (72, 25), (70, 24),
    (56, 82), (53, 74), (50, 84), (39, 81), (55, 80),  # cluster C -- north
    (43, 84),
    (46, 47), (30, 55),  # outliers -- central
]

demands = [
    26, 25, 37, 22, 35, 22, 43, 21, 41,   # cluster A
    29, 61, 32, 41, 40, 39, 19, 72,       # cluster B
    23, 40, 59, 27, 45, 25,               # cluster C
    21, 40,                               # outliers
]

fac_xy = [
    (7, 15), (12, 19),   # j=0,1 -- near cluster A
    (76, 19), (80, 21),  # j=2,3 -- near cluster B
    (56, 88), (52, 79),  # j=4,5 -- near cluster C
    (45, 50), (63, 48),  # j=6,7 -- central bridge
    (3, 93), (3, 88),    # j=8,9 -- peripheral
]

capacities = [307, 98, 86, 298, 211, 190, 70, 205, 140, 165]
fixed_costs = [787, 233, 257, 756, 567, 517, 227, 535, 367, 445]

dist = [[math.hypot(cust_xy[i][0] - fac_xy[j][0], cust_xy[i][1] - fac_xy[j][1])
         for j in range(N_FAC)]
        for i in range(N_CUST)]


def build_model():
    """Builds the CFLP model and returns the problem along with the
    facility and assignment variables.
    """
    prob = xp.problem(name="CFLP")

    y = [prob.addVariable(vartype=xp.binary, name=f"y_{j}")
         for j in range(N_FAC)]
    x = [[prob.addVariable(vartype=xp.binary, name=f"x_{i}_{j}")
          for j in range(N_FAC)]
         for i in range(N_CUST)]

    prob.setObjective(
        xp.Sum(fixed_costs[j] * y[j] for j in range(N_FAC))
        + xp.Sum(dist[i][j] * x[i][j]
                 for i in range(N_CUST) for j in range(N_FAC)),
        sense=xp.ObjSense.MINIMIZE,
    )

    # (1) Demand satisfaction: sum_j x_ij = 1 for all i
    prob.addConstraint(
        xp.Sum(x[i][j] for j in range(N_FAC)) == 1
        for i in range(N_CUST)
    )

    # (2) Capacity and activation: sum_i d_i x_ij <= Q_j y_j for all j
    prob.addConstraint(
        xp.Sum(demands[i] * x[i][j] for i in range(N_CUST))
        <= capacities[j] * y[j]
        for j in range(N_FAC)
    )

    return prob, y, x


def print_solutions(prob, y, x):
    """Prints the collected solutions. For each solution, lists:
      - Objective function value
      - Number of facilities opened
      - Maximum distance between a facility and a customer it serves
    """
    pool_size = prob.attributes.poolsize
    print(f"\nCollected {pool_size} solution(s):")
    print(f"  {'Sol#':>4}  {'Objective':>12}  {'Facilities':>11}"
          f"  {'Max dist':>9}")
    for p in range(pool_size):
        pool_obj = prob.getPoolSolObjVal(p)
        sol = prob.getPoolSolution(p)

        n_open = sum(round(sol[y[j].index]) for j in range(N_FAC))
        max_dist = 0.0
        for i in range(N_CUST):
            for j in range(N_FAC):
                if round(sol[x[i][j].index]):
                    max_dist = max(max_dist, dist[i][j])
                    break

        print(f"  {p:4d}  {pool_obj:12.2f}  {n_open:11d}  {max_dist:9.2f}")
    print()


def solve_collect_solutions(prob, y, x):
    """Solve our model and collect up to ten solutions into the pool."""

    # Collect up to ten solutions during the MIP search. Solutions are
    # stored from best to worst; index 0 always holds the best solution.
    prob.controls.poollimit = 10

    print("\nSolving problem to collect solutions\n")
    prob.optimize()
    if prob.attributes.solstatus != xp.SolStatus.OPTIMAL:
        print("Failed to solve problem with solstatus "
              f"{prob.attributes.solstatus.name}")
        return

    print("Solved problem to optimality.")
    print_solutions(prob, y, x)


def solve_warm_start_from_pool(prob, y, x):
    """Assumes that we have already solved the problem and that the pool
    contains a set of solutions.

    This function forces a single facility to be open and uses the pooled
    solutions to warm-start the next MIP solve.
    """

    # Make sure the problem is in its original state before we change it.
    # If the problem is already in its original state, this call will
    # simply return without doing anything.
    prob.postSolve()

    # We need to set the control poolkeepsols to transfer any existing
    # solutions to the warm-start pool before we fix a variable, since any
    # modifications will invalidate all collected selections.
    prob.controls.poolkeepsols = -1

    # Force a facility to be open. If we open an additional facility, all
    # existing solutions should remain feasible - the presolver should take
    # care of satisfying the fixed binary. The warm-start solutions will
    # not be optimal since no customers are served by the new facility.
    # Pick a facility that should not be open in our current optimal
    # solution.
    fac = 2
    prob.chgBounds([y[fac]], ['B'], [1.0])

    print(f"\nReoptimizing with fixed facility {fac}.\n")
    prob.optimize()
    if prob.attributes.solstatus != xp.SolStatus.OPTIMAL:
        print("Failed to resolve problem with solstatus "
              f"{prob.attributes.solstatus.name}")
        return

    print("Resolved problem to optimality.")
    print_solutions(prob, y, x)

    # Reset controls. We should do this before modifying the problem to
    # avoid automatically transferring the solutions when restoring the
    # bound below.
    prob.setDefaultControl('poolkeepsols')

    # Undo the fixed facility by returning the lower bound to 0.0.
    prob.chgBounds([y[fac]], ['L'], [0.0])


def enumerate_all_binaries(prob, y, x):
    """Use the built-in solution enumeration to find the 5 best solutions
    to our problem. The solver, by default, considers two solutions as
    being distinct if they differ in any of our binary restricted
    variables. To help speed up the solve, we limit the search to
    solutions whose objective value is within 50% of the best solution.
    """

    # Collect up to 5 best solutions.
    prob.controls.poollimit = 5

    # Enable automatic solution enumeration.
    prob.controls.poolenumstrategy = xp.PoolEnumStrategy.FULL

    # We limit collection to solutions that are within 50% of the best
    # solution.
    prob.controls.poolrelgap = 0.50

    # Solve the problem, which will automatically trigger an extended
    # branch-and-bound search to find all 5 best solutions.
    print("\nSolving problem to find 5 best solutions.\n")
    prob.optimize()
    if prob.attributes.solstatus != xp.SolStatus.OPTIMAL:
        print("Failed to solve problem with solstatus "
              f"{prob.attributes.solstatus.name}")
        return

    print("Solved problem to optimality.")
    print_solutions(prob, y, x)

    # Reset controls.
    prob.setDefaultControl('poollimit')
    prob.setDefaultControl('poolenumstrategy')
    prob.setDefaultControl('poolrelgap')


def enumerate_facilities(prob, y, x):
    """We want to find the 5 best combinations of opening facilities but
    the demand sourcing should be optimal in each solution. To achieve
    this, we need to tell the solver to enumerate only through solutions
    that differ in facility variables. We can do this through solution
    enumeration by specifying branching directives only on the facility
    variables. The solver will automatically treat two solutions with the
    same facility assignment as duplicates and only keep the best one,
    which should result in a solution with the optimal demand assignment
    for each facility assignment.
    """

    # Make sure the problem is in its original state before we change it.
    # If the problem is already in its original state, this call will
    # simply return without doing anything.
    prob.postSolve()

    # Collect up to 5 best solutions.
    prob.controls.poollimit = 5

    # Enable automatic solution enumeration.
    prob.controls.poolenumstrategy = xp.PoolEnumStrategy.FULL

    # Apply branching directives to the facility binaries.
    # Binaries without a directive will not be enumerated.
    prob.loadBranchDirs(y, [1] * N_FAC)

    # Solve the problem, which will automatically trigger an extended
    # branch-and-bound search to find the 5 best facility assignments.
    print("\nSolving problem to find 5 best facility assignments.\n")
    prob.optimize()
    if prob.attributes.solstatus != xp.SolStatus.OPTIMAL:
        print("Failed to solve problem with solstatus "
              f"{prob.attributes.solstatus.name}")
        return

    print("Resolved problem to optimality.")
    print_solutions(prob, y, x)

    # Reset controls.
    prob.setDefaultControl('poollimit')
    prob.setDefaultControl('poolenumstrategy')

    # Clear branching directives.
    prob.loadBranchDirs(y, [0] * N_FAC)


def main():
    prob, y, x = build_model()

    # Solve the model and collect solutions.
    solve_collect_solutions(prob, y, x)

    # Try forcing a facility to be opened and reoptimize. We carry the
    # existing solutions through as warm-start solutions for the next
    # solve.
    solve_warm_start_from_pool(prob, y, x)

    # Run an enumeration search to find the 5 best solutions.
    enumerate_all_binaries(prob, y, x)

    # Run an enumeration search to find the 5 best facility assignments.
    enumerate_facilities(prob, y, x)


main()
