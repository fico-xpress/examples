#######################################################
#  Mosel User Guide Example Problems
#  =================================
#
#  file ugiodense.py
#  `````````````````
#  Exchanging dense data between Python and a model.
#
#  (c) 2026 Fair Isaac Corporation
#      author: B. Vieira, 2026
########################################################

import moselpy as mp

vdata = [15, 100, 90, 60, 40, 15, 10, 1]     # VALUE
wdata = [ 2,  20, 20, 30, 40, 30, 60, 10]    # WEIGHT

mp.compile_model("burglar8p.mos", "burglar8p.bim")
model = mp.load_model("burglar8p.bim")
output = model.run(
    exec_params={"DATASOURCE": "moselpy:"},
    input_data={
        "VALUE":  {i+1: v for i, v in enumerate(vdata)},
        "WEIGHT": {i+1: w for i, w in enumerate(wdata)},
    })

print(f"Objective value: {model.objective_value}")
for i in range(8):
    print(f" take({i+1}): {output['soltake'][i+1]}")
