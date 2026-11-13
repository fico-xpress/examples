#######################################################
#  Mosel User Guide Example Problems
#  =================================
#
#  file ugcb.py
#  ````````````
#  Redirecting the Mosel output in Python.
#
#  (c) 2026 Fair Isaac Corporation
#      author: B. Vieira, 2026
########################################################

import moselpy as mp
from io import StringIO

mp.compile_model("burglar2.mos", "burglar2.bim")
model = mp.load_model("burglar2.bim")

# Capture output to a Python buffer
buf = StringIO()
model.symbols["out"] = buf
model.set_default_stream(mp.StreamType.OUTPUT, "moselpy:out")
model.run()
del model.symbols["out"]

captured = buf.getvalue()
for line in captured.splitlines():
    print(f"Mosel: {line}")
