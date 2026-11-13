# facility_location.py
#
# Example of a facility location problem solved with Xpress
# The problem is to decide where to build parks to serve schools
# to minimize the average distance from schools to their assigned parks.
#
# This source code is provided to illustrate the use
# of Xpress inside Docker containers and is not intended for production use.
# 
# (C) Copyright 2025-2026 Fair Isaac Corporation

import os
import sys

import xpress as xp
import numpy as np
import pandas as pd
from scipy.spatial.distance import cdist

num_schools = 9  # Number of schools to be served.
num_sites = 11   # Number of candidate sites to build parks

SCHOOLS = range(num_schools)  # Set of schools
SITES = range(num_sites)      # Set of candidate sites

coord_schools = 10 * np.random.random((num_schools, 2))  # x-y coordinates between 0 and 10 (in km)
coord_sites   = 10 * np.random.random((num_sites, 2))

# Use scipy for efficient distance computation
dist_matrix = cdist(coord_schools, coord_sites)
dist = {(i, j): dist_matrix[i, j] for i in SCHOOLS for j in SITES}

def optimizer(num_parks: int):
    prob = xp.problem()

    serves = prob.addVariables(SCHOOLS, SITES, vartype=xp.binary)
    build = prob.addVariables(SITES, vartype=xp.binary)

    # Objective function
    prob.setObjective(xp.Sum(dist[i,j] * serves[i,j] for i in SCHOOLS for j in SITES))

    # Every school must be served by one park
    prob.addConstraint(xp.Sum(serves[i,j] for j in SITES) == 1 for i in SCHOOLS)

    # Exactly n parks are built:
    prob.addConstraint(xp.Sum(build[j] for j in SITES) <= num_parks)

    # Only parks that are built can serve schools
    prob.addConstraint(xp.Sum(serves[i,j] for i in SCHOOLS) <= num_schools * build[j] for j in SITES)

    prob.controls.outputlog = 0

    solvestatus, solstatus = prob.optimize()

    if solvestatus == xp.SolveStatus.COMPLETED:

        # Get the solution and return metrics
        sol = prob.getSolution(serves)

        # Create a Pandas DataFrame to compute metrics
        assignment_data = []
        for i in SCHOOLS:
            for j in SITES:
                if sol[i, j] > 0.5:
                    assignment_data.append({"School": f"School_{i}", "Assigned_Park": f"Site_{j}", "Distance_km": round(dist[i, j], 2)})
        
        assignment_df = pd.DataFrame(assignment_data)
        avg_dist = assignment_df["Distance_km"].mean()
        max_dist = assignment_df["Distance_km"].max()

        return avg_dist, max_dist
    else:
        return None, None

if __name__ == "__main__":

    rndseed = 10
    np.random.seed(rndseed)

    # Check if the user provided the required argument
    if len(sys.argv) < 2:
        print("Usage: python facility_location.py <number_of_parks>")
        sys.exit(1)

    try:
        num_parks = int(sys.argv[1])
    except ValueError:
        print("Error: number_of_parks must be an integer.")
        sys.exit(1)

    # Run optimization for the pre-defined number of parks
    try:
    	with xp.init():	# Acquire the Xpress license, license will be released in case of an error
    		avg_dist, max_dist = optimizer(num_parks)
    		print(f"Results for building {num_parks} parks: ")
    		print(f"      Average distance: {round(avg_dist,2)}")
    		print(f"      Maximum distance: {round(max_dist,2)}")
    		# Xpress license is implicitly released here
    except xp.ModelError as e:
        print(f'Failed to initialize Xpress: {e}')
