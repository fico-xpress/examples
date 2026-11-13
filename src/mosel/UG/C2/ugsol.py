#######################################################
#  Mosel User Guide Example Problems
#  =================================
#
#  file ugsol.py
#  `````````````
#  Retrieving solution values from Python.
#
#  (c) 2026 Fair Isaac Corporation
#      author: B. Vieira, 2026
########################################################

import moselpy as mp
import sys

mp.compile_model("burglar3p.mos", "burglar3p.bim")
model = mp.load_model("burglar3p.bim")
model.run()

if not model.prob_status.is_solution_optimal:
    sys.exit(1)                          # Stop if no solution found

print(f"Objective value: {model.objective_value}")

soltake = model.find_identifier("soltake")  # dict: {index: solution_value, ...}
value = model.find_identifier("VALUE")      # dict: {index: data_value, ...}

for item in sorted(soltake):
    print(f"take({item}): {soltake[item]}\t (item value: {value[item]})")
