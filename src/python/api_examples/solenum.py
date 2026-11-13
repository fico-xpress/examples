# Example of enumerating the n-best solutions when solving a MIP.
#
# The program reads a problem from a file and collects solutions into a pool.
# Depending on the setting of a search_method parameter, it will enumerate
# additional solutions. There is an optional parameter to specify the maximum
# number of solutions to collect.
#
# The parameter search_method can be "collect", "extended" or "n-best". For
# "collect" it just collects solutions without changing the branch and bound
# search behaviour. For a value of "extended" it adjusts the branch and bound
# cutoff so that the algorithm searches suboptimal nodes for additional
# solutions. The value of "n-best" uses branching directives to force branching
# on integer feasible nodes, fully enumerating the tree and finding the best
# solutions.
#
# The example implements its own solution pool with solutions stored in order
# of objective value, and implements duplication checks. Almost all the
# interesting work happens within the preintsol callback. It collects
# solutions, checks for duplicates and adjusts the cutoff for the remaining
# search. The adjustment of the cutoff is crucial to ensure additional
# solutions are found.
#
# (C) 2025-2026 Fair Isaac Corporation

import xpress as xp
import numpy as np
import argparse
import math


COLLECT = "collect"     # Collect solutions with no change to branch and bound
EXTENDED = "extended"   # Extend the branch and bound search by adjusting the cutoff to keep suboptimal nodes
N_BEST = "n-best"       # Use branching directives to fully enumerate the branch and bound tree


def hash_solution(solution, col_is_enumerated):
    """Calculates a simple hash across the solution values of enumerated columns
    """
    return hash(tuple(round(x) for x, ifenum in zip(solution, col_is_enumerated) if ifenum))


class Solution:
    """Solution class for solutions in a pool
    """
    def __init__(self, col_is_enumerated, x, objval):
        self.x = x
        self.col_is_enumerated = col_is_enumerated
        self.hash = hash_solution(x, col_is_enumerated)
        self.objval = objval

    def __eq__(self, other):
        """Compares solutions for equality based on the integer columns
        """
        for col in range(len(self.x)):
            if self.col_is_enumerated[col] == 1:
                # integral value of should be the same
                if math.fabs(self.x[col] - other.x[col]) > 0.5:
                    return False

        return True

    def __hash__(self):
        return self.hash


class SolutionPool:
    """SolutionPool class for storing a set of distinct best solutions
    """
    def __init__(self, ncols, obj_sense, max_solutions, objgap):
        self.sol_list = []       # Solutions ordered from best to worst
        self.sol_dict = dict()   # For identifying duplicate solutions
        self.ncols = ncols
        self.obj_sense = obj_sense
        self.max_solutions = max_solutions
        self.objgap = objgap

    def is_sol_better(self, sol1, sol2):
        return self.obj_sense * sol1.objval < self.obj_sense * sol2.objval

    def del_solution(self, sol):
        del self.sol_dict[sol]
        self.sol_list.remove(sol)

    def add_solution(self, sol):
        if sol in self.sol_dict:
            # The solution is already known, keep the duplicate with the best objective
            duplicate_sol = self.sol_dict[sol]

            if not self.is_sol_better(sol, duplicate_sol):
                return

            # Previous solution had worse objective value, delete it from the pool
            self.del_solution(duplicate_sol)

        # Add solution to the pool
        self.sol_dict[sol] = sol

        # Insert the solution in the correct position in the list
        idx = 0
        while idx < len(self.sol_list) and not self.is_sol_better(sol, self.sol_list[idx]):
            idx += 1

        self.sol_list.insert(idx, sol)

        # Remove the worst solutions if we have too many
        while len(self.sol_list) > self.max_solutions:
            self.del_solution(self.sol_list[-1])

        # If we are using an objective gap, drop any solutions which are too poor
        if self.objgap is not None:
            cutoff_obj = self.get_cutoff_obj()
            while len(self.sol_list) > 0 and self.sol_list[-1].objval * self.obj_sense > cutoff_obj * self.obj_sense:
                self.del_solution(self.sol_list[-1])

    def get_cutoff_obj(self):
        """Returns the worst objective that should be accepted"""
        if self.objgap is not None and len(self.sol_list) > 0:
            # Reject solutions that are objgap worse than the best solution found
            best_obj = self.sol_list[0].objval
            return best_obj + self.obj_sense * (abs(best_obj) * self.objgap)
        elif len(self.sol_list) >= self.max_solutions:
            # We already have the required number of solutions, so set the
            # cutoff such that we only search for improving solutions
            worst_obj = self.sol_list[-1].objval
            return worst_obj
        else:
            # No cutoff: search for any solution
            return self.obj_sense * xp.infinity


class CallbackData:
    """Callback data
    """
    def __init__(self):
        self.pool = None
        self.col_is_enumerated = None
        self.obj_sense = xp.ObjSense.MINIMIZE
        self.search_method = N_BEST


def preintsol_callback(prob, cb_data, soltype, cutoff):

    # Collect the new solution.

    # We have to use prob.getCallbackSolution() to retrieve the solution since
    # it has not been installed as the incumbent yet
    x = prob.getCallbackSolution()

    # Get solution objective value
    objval = prob.attributes.lpobjval

    # Add the new solution to our pool.
    sol = Solution(cb_data.col_is_enumerated, x, objval)
    cb_data.pool.add_solution(sol)

    if cb_data.search_method == COLLECT:
        # We just collect the solutions and don't adjust the search.
        newcutoff = cutoff
    else:
        # Adjust the cutoff so we continue finding solutions.
        newcutoff = cb_data.pool.get_cutoff_obj() + (-1e-6 * cb_data.obj_sense)

    return (0, newcutoff)


def select_enumeration_columns(prob, ncols, col_is_enumerated):
    coltype = prob.getColType()

    # Identify the integer restricted columns.
    for col in range(ncols):
        if (coltype[col] == 'B') or (coltype[col] == 'I'):
          col_is_enumerated[col] = 1
        else:
          col_is_enumerated[col] = 0


def enforce_enumeration_columns(prob, ncols, col_is_enumerated):
  # We use the Xpress branch directives to force Xpress to continue branching
  # on any column that we want enumerated.
  colSelect = []
  for col in range(ncols):
    if col_is_enumerated[col] == 1:
      colSelect.append(col)

  prob.loadBranchDirs(colSelect)


def main():
    parser = argparse.ArgumentParser(
        description='Enumerates and collects multiple solutions to a MIP'
    )
    parser.add_argument('filename', help='MPS or LP file to solve')
    parser.add_argument('search_method', help='How to collect solutions',
                        choices=[COLLECT, EXTENDED, N_BEST])
    parser.add_argument('--maxsols', help='Maximum number of solutions to collect',
                        type=int, default=10)
    parser.add_argument('--objgap', help='If specified, solutions more than objgap worse than the best solution will be discarded',
                        type=float, required=False)
    args = parser.parse_args()

    # Create the problem
    prob = xp.problem(f'solenum {args.filename}')

    # Read the file
    prob.readProb(args.filename)

    # Set up the callback data for the preintsol callback
    ncols = prob.attributes.cols
    cb_data = CallbackData()
    cb_data.obj_sense = prob.attributes.objsense
    cb_data.pool = SolutionPool(ncols, cb_data.obj_sense, max(1, args.maxsols), args.objgap)
    cb_data.search_method = args.search_method

    # Set up the enumeration columns
    cb_data.col_is_enumerated = np.zeros(ncols, dtype=np.int8)
    select_enumeration_columns(prob, ncols, cb_data.col_is_enumerated)
    if args.search_method == N_BEST:
        enforce_enumeration_columns(prob, ncols, cb_data.col_is_enumerated)

    # Add the callback
    prob.addPreIntsolCallback(preintsol_callback, cb_data, 0)

    # Set important controls
    prob.controls.serializepreintsol = 1
    prob.controls.miprelstop = 0
    if args.search_method == N_BEST:
        prob.controls.mipdualreductions = 0

    # Optimize
    prob.optimize()

    # Print out results
    print()
    if len(cb_data.pool.sol_list) == 0:
        print('No solutions collected!')
    else:
        print(f'Collected {len(cb_data.pool.sol_list)} solutions with objective values:')
        for sol in cb_data.pool.sol_list:
            print(f'   {sol.objval}')


main()
