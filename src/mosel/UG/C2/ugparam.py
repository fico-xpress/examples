#######################################################
#  Mosel User Guide Example Problems
#  =================================
#
#  file ugparam.py
#  ```````````````
#  Passing parameters and accessing sets from Python.
#
#  (c) 2026 Fair Isaac Corporation
#      author: B. Vieira, 2026
########################################################

import moselpy as mp

LIM = 500

mp.compile_model("prime.mos", "prime.bim")
model = mp.load_model("prime.bim")

model.run(exec_params={"LIMIT": LIM})

print(f"'prime' returned: {model.exit_code}")

s_prime = model.find_identifier("SPrime")   # Returns a Python set

if s_prime:
    print(f"Prime numbers from 2 to {LIM}")
    print(" " + ", ".join(str(p) for p in sorted(s_prime)))
