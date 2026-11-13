#######################################################
#  Mosel User Guide Example Problems
#  =================================
#
#  file ugcomp.py
#  ``````````````
#  Compiling and executing a model from Python.
#
#  (c) 2026 Fair Isaac Corporation
#      author: B. Vieira, 2026
########################################################

import moselpy as mp

print("Compiling 'burglar2'")
mp.compile_model("burglar2.mos", "burglar2.bim")

print("Loading 'burglar2'")
model = mp.load_model("burglar2.bim")

print("Executing 'burglar2'")
model.run()

print(f"'burglar2' returned: {model.exit_code}")
