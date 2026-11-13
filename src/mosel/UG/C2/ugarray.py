#######################################################
#  Mosel User Guide Example Problems
#  =================================
#
#  file ugarray.py
#  ```````````````
#  Enumerating sparse arrays from Python.
#
#  (c) 2026 Fair Isaac Corporation
#      author: B. Vieira, 2026
########################################################

import moselpy as mp

mp.compile_model("transportp.mos", "transportp.bim")
model = mp.load_model("transportp.bim")
model.run()

solflow = model.find_identifier("solflow")  # dict with tuple keys

for (plant, region), val in solflow.items():
    print(f"flow({plant},{region}): {val}")
