# Example of the barstart functionality.
#
# (C) 2020-2026 Fair Isaac Corporation

import xpress as xp

# Read and solve the first problem.

ifdisk = True

p = xp.problem()

p.readProb('Data/d2q06c.mps.gz')

p.controls.baralg = 2
p.controls.barstart = 1024
p.controls.crossover = 0

p.lpOptimize('b')

print ('First problem solved with status:', p.attributes.solstatus.name)

if ifdisk:
    p.writeSlxSol('mysol.slx', 'd')
else:
    x = p.getSolution()
    s = p.getSlacks()
    d = p.getDuals()
    r = p.getRedCosts()

# Read a slightly modified problem and solve it using the solution of
# p as a warm start.

p2 = xp.problem()

p2.readProb('Data/d2q06c_mod.mps.gz')

p2.controls.baralg = 2

# This instructs barrier to use the available solution as warm-start.
p2.controls.barstart = -1

p2.controls.crossover = 0

if ifdisk:
    p2.readSlxSol('mysol.slx')
else:
    p2.loadLPSol(x, s, d, r)

# Assign an emphasis to the warm-start (0.85 is the default). With a
# higher value, the number of barrier iteration is even smaller.
p2.controls.barstartweight = 0.9

p2.lpOptimize('b')

print ('Warm-started problem solved with status:', p2.attributes.solstatus.name)
