# Changing an optimization problem using the Xpress Python interface.
#
# (C) 1983-2026 Fair Isaac Corporation

import xpress as xp

p = xp.problem()

x = p.addVariable()
y = p.addVariable()

cons1 = x + y >= 2
upperlim = 2*x + y <= 3

p.setObjective((x-4)**2 + (y-1)**2)
p.addConstraint(cons1, upperlim)

p.writeProb('original', 'lp')

p.chgCoef(cons1, x, 3)  # Coefficient of x in cons1    becomes 3.
p.chgCoef(1, 0, 4)      # Coefficient of y in upperlim becomes 4.

p.writeProb('changed', 'lp')
