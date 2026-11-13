#######################################################
#  Mosel User Guide Example Problems
#  =================================
#
#  file ugiosparse.py
#  ``````````````````
#  Exchanging sparse data between Python and a model.
#
#  (c) 2026 Fair Isaac Corporation
#      author: B. Vieira, 2026
########################################################

import moselpy as mp

mp.compile_model("burglar9p.mos", "burglar9p.bim")
model = mp.load_model("burglar9p.bim")
output = model.run(
    exec_params={"DATASOURCE": "moselpy:"},
    input_data={
        "VALUE": {"camera": 15, "necklace": 100, "vase": 90,
                  "picture": 60, "tv": 40, "video": 15,
                  "chest": 10, "brick": 1},
        "WEIGHT": {"camera": 2, "necklace": 20, "vase": 20,
                   "picture": 30, "tv": 40, "video": 30,
                   "chest": 60, "brick": 10},
    })

print(f"Objective value: {model.objective_value}")
for item, val in output["soltake"].items():
    print(f" take({item}): {val}")
