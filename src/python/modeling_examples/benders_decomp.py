# An application of the Xpress callbacks to a Benders decomposition algorithm.
# Uses preintsol callback for cut injection.
#
# Solves the problem:
# min c1*x + c2*y
# st  A1*x + A2*y <=b (m constraints)
# x binary n1-dimensional vector
# y >=0 continuous n2-dimensional vector
#
# NOTE: This example is for illustration/tutorial purposes only. There is no
# benefit in solving the generic problem defined below using Benders
# decomposition.
# Courtesy of Georgios Patsakis (UC Berkeley, Amazon) and Richard L.-Y. Chen (Amazon).
#
# (C) 2019-2026 Fair Isaac Corporation

# Import necessary packages.
import xpress as xp
import sys

# Initialize problem parameters.
c1 = [1,6,5,7]                    # n1 x 1
c2 = [9,3,0,2,3]                  # n2 x 1
b  = [-3,-4,1,4,5]                # m  x 1
A1 = [[0, -2, 3, 2],
      [-5, 0, -3, 1],
      [1, 0, 4, -2],
      [0, -3, 4, -1],
      [-5, -4, 3, 0]]
A2 = [[3, 4, 2, 0, -5],
      [0, 2, 3, -2, 1],
      [2, 0, 1, -3, -5],
      [-5, 3, -2, -3, 0],
      [-2, 3, -1, 2, -4]]
m  = len(b)
n1 = len(c1)
n2 = len(c2)

# Absolute accuracy for optimal objective.
ObjAbsAccuracy=0.00001

# SOLVE ORIGINAL PROBLEM WITHOUT DECOMPOSING
print("**** Solving original problem without decomposing ****")
p = xp.problem()

# Define Variables.
x = [p.addVariable(vartype=xp.binary) for _ in range(n1)]
y = [p.addVariable() for _ in range(n2)] # positive real variable

# Define Constraints.
constr = [xp.Sum(A1[ii][jj]*x[jj] for jj in range(n1)) + \
          xp.Sum(A2[ii][jj]*y[jj] for jj in range(n2))   \
                        <= b[ii] for ii in range(m)]
p.addConstraint(constr)

# Define Objective.
p.setObjective(xp.Sum(c1[jj]*x[jj] for jj in range(n1)) + \
               xp.Sum(c2[jj]*y[jj] for jj in range(n2)) , \
               sense=xp.ObjSense.MINIMIZE)

p.controls.outputlog = 0  # Suppress Xpress output

# Solve and retrieve solution.
p.optimize()

if p.attributes.solvestatus != xp.SolveStatus.COMPLETED or \
   p.attributes.solstatus != xp.SolStatus.OPTIMAL:
    raise RuntimeError('Problem could not be solved to MIP optimality')

xopt = p.getSolution(x)
yopt = p.getSolution(y)
xopt_rounded = [0.0 if round(val, 2) == 0 else round(val, 2) for val in xopt]
yopt_rounded = [0.0 if round(val, 2) == 0 else round(val, 2) for val in yopt]
print(f"Solution without decomposing: obj: {p.attributes.objval:.2f}")
print(f"  x: {xopt_rounded}")
print(f"  y: {yopt_rounded}")
print()

# SOLVE DECOMPOSED PROBLEM
# We define the following functions:
# - subproblem: function that solves the subproblem for a given first stage
#   variable xhat. If the subproblem has an optimal solution, it returns the
#   the optimal dual multiplier associated with the non anticipativity
#   constraint, the optimal objective, and the label "Optimal". If the
#   subproblem is infeasible, it returns the dual ray of unboundedness and
#   the label "Infeasible".
#
# - preintsol_callback: A pre-integer solution callback is triggered every time
#   an integer solution is found, BEFORE it is accepted by the solver. This
#   callback validates the solution against the subproblem and can inject cuts
#   directly when soltype=0 (integer solution from node relaxation).

# Subproblem Solver Function.
def subproblem(xhat):
    # Input:
    # - xhat: n1x1 array is the first stage variable,
    # passed to the subproblem.
    #
    # Output:
    # return (lamb: n1x1 array, beta: scalar, flag:string)
    # - If the subproblem is Infeasible, the flag is 'Infeasible', and
    #   lamb (n1x1) and beta (1x1) are the components of the dual ray of
    #   unboundedness necessary to write a feasibility cut of the form:
    #       sum(x[i]*lamb[i] for all i) +beta >= 0
    # - If the subproblem is Optimal, the flag is 'Optimal', beta (1x1) is the
    #   optimal objective, and lamb (n1x1) is the optimal dual multiplier
    #   associated with the non anticipativity constraint, so that the
    #   optimality cut will have the form:
    #       theta >= sum(x[i]*lamb[i] for all i) + beta

    # IMPORTANT NOTE: The subproblem needs to ensure that upper bounds or non
    # zero lower bounds should ONLY BE DEFINED THROUGH CONSTRAINTS, not as part
    # of the variable definition. This is to ensure the feasibility cuts are
    # correct.
    # Side Note: If there are violated feasibility cuts, then the subproblem will be infeasible.
    # In such a case, it is a good practice to ensure that the subproblem is always
    # feasible through modeling, for instance by adding slacks to all the constraints.
    # This often improves performance because it tends to separate stronger feasibility cuts.
    # However, this is an advanced technique that is not implemented in this illustrative example.

    # Define and Solve Subproblem.

    # Initialize Problem.
    r=xp.problem()

    # Define Variables.
    y=[r.addVariable() for _ in range(n2)] # Positive real variable - second stage.
    z=[r.addVariable(lb=-xp.infinity) for _ in range(n1)] # Dummy copy variable,
                                           # must have the exact shape as xhat.
    epsilon=r.addVariable(lb=-xp.infinity) # Dummy variable to extract part of the
                                           # infeasibility ray (if infeasible).

    # Define Constraints.
    # Dummy constraint for optimality/feasibility cuts.
    # Note: make sure the index of the variable and of the position in the
    # constraint array is the same (or has a known mapping).
    dummy1= [z[i]==xhat[i] for i in range(n1)]
    # Dummy constraint for feasibility cut.
    dummy2= epsilon==1
    # Second stage constraints, where RHS b has been multiplied by epsilon.
    constr=[xp.Sum(A1[ii][jj]*z[jj] for jj in range(n1)) + \
            xp.Sum(A2[ii][jj]*y[jj] for jj in range(n2))   \
            - epsilon*b[ii]<=0 for ii in range(m)]
    r.addConstraint(constr,dummy1,dummy2)

    # Define Objective.
    r.setObjective(xp.Sum(c2[jj]*y[jj] for jj in range(n2)),
            sense=xp.ObjSense.MINIMIZE)

    # Disable presolve. The only reason to do this is because in the case of
    # an infeasible problem, the presolve might identify infeasibility of the
    # problem without running simplex. Because of that, no dual infeasibility
    # certificate will be found, hence a dual ray of unboundedness will not be
    # returned, which means that a feasibility cut can not be formed.
    r.controls.presolve = 0

    # Silence output.
    r.controls.outputlog = 0

    # Solve optimization.
    r.optimize()

    # Find the indices of constraint dummy1, which will be used to access the
    # dual multipliers corresponding to the entries of xhat.
    xind1=[dummy1[ii].index for ii in range(n1)]

    if r.attributes.solvestatus != xp.SolveStatus.COMPLETED:
        print("ERROR: Subproblem was not solved. Terminating.")
        sys.exit()

    # Take cases depending on subproblem status.
    if r.attributes.solstatus == xp.SolStatus.OPTIMAL:
        # Optimal subproblem.
        # Retrieve optimal dual multiplier and objective and return.
        dualmult=r.getDuals()
        lamb=[dualmult[ii] for ii in xind1]
        beta=r.attributes.objval
        return(lamb,beta,'Optimal')

    elif r.attributes.solstatus == xp.SolStatus.INFEAS:
        # Infeasible subproblem.
        dray = r.getDualRay()
        if dray is None:
            print ("Could not retrieve a dual ray, return no good cut instead:")
            # This is just a cheat if the subproblem is found infeasible and
            # the optimizer fails to find a dual ray. Since all the first stage
            # variables are binary, we add a No-Good-Cut to exclude the point
            # xhat instead of an infeasibility cut.
            lamb=[2*xhat[ii]-1 for ii in range(n1)]
            beta=-sum(xhat)+1
        else:
            # Extract the dual ray.
            print ("Dual Ray:", dray)
            # Extract the part of the dual ray corresponding to the first stage
            # variables xhat.
            lamb=[dray[ii] for ii in xind1]
            # Extract the constant from the dual ray entry of constraint dummy 2.
            beta=dray[dummy2.index]
        return(lamb,beta,'Infeasible')
    else:
        print("ERROR: Subproblem not optimal or infeasible. Terminating.")
        sys.exit()

# Pre-Integer Solution Callback Function
def preintsol_callback(p, data, soltype, cutoff):
    # Input:
    # NOTE: the input is populated by the solver when an integer solution is
    # found, BEFORE it is accepted.
    # - p: an xp.problem() (a thread-local copy of the problem from which the callback was triggered)
    # - data: structure that is passed to the callback from the problem,
    #   specified in the addPreIntsolCallback function. No data is needed here,
    #   so the addPreIntsolCallback() call uses None.
    # - soltype: 0 if the integer solution was found by a node relaxation,
    #   1 if found by a heuristic, 2 if provided by user
    # - cutoff: the cutoff value that the Optimizer will use if the solution
    #   is accepted.
    #
    # Output:
    # return (reject, newcutoff)
    # - reject: 1 to reject the solution, 0 to accept it
    # - newcutoff: new cutoff value (or None to keep the current one)

    # Retrieve xhat and thetahat from the candidate integer solution.
    xhat = p.getCallbackSolution([x[ii] for ii in range(n1)])
    thetahat = p.getCallbackSolution(theta)
    xhat_rounded = [0.0 if round(val, 2) == 0 else round(val, 2) for val in xhat]

    print(f"Integer solution found! x={xhat_rounded}, theta={thetahat:.2f}")

    # Solve the subproblem to check feasibility.
    (dual_mult, opt, status) = subproblem(xhat)

    print(f"  Solving subproblem... subproblem obj = {opt:.2f}")

    if status == 'Infeasible':
        # The subproblem is infeasible. Add a feasibility cut.
        # For heuristic solutions (soltype != 0), we cannot add cuts,
        # and we can only reject the solution if it is not feasible.
        # Depending on the specific application, the heuristics could be
        # very unlikely to find valid feasible solutions (they don't
        # respect the Benders cuts we're generating), and the Benders
        # subproblem could be expensive to solve.
        # In such cases, it could be more convenient to reject all
        # heuristic solutions without solving the subproblem, or to
        # simply disable heuristics completely.
        if soltype != 0:
            print("  -> Subproblem infeasible - Heuristic solution, rejecting")
            return (1, None)
        print("  -> Subproblem infeasible - Adding feasibility cut")
        xind = [x[ii].index for ii in range(n1)]
        coefficients = dual_mult
        rhs = -opt
        p.addCuts([1], ['L'], [rhs], [0, len(xhat)], xind, coefficients)
        # IMPORTANT: Return 0 (accept) to keep the cuts. Returning reject=1 would drop the cuts.
        # When soltype=0, addCuts() causes the node LP to be resolved with the new cuts.
        return (0, None)

    elif status == 'Optimal':
        # Check if theta is sufficient for the optimal subproblem value.
        if thetahat >= opt - ObjAbsAccuracy:
            # The solution is feasible to the full problem.
            print(f"  -> Subproblem obj <= theta -> Valid solution!")
            return (0, None)
        else:
            # The solution is infeasible; theta is too small. Add an optimality cut.
            # For heuristic solutions (soltype != 0), we cannot add cuts.
            if soltype != 0:
                print("  -> Subproblem obj > theta - Heuristic solution, rejecting")
                return (1, None)
            print(f"  -> Subproblem obj > theta -> Adding optimality cut")
            xind = [x[ii].index for ii in range(n1)]
            thetaind = [theta.index]
            coefficients = [1] + [-dual_mult[ii] for ii in range(len(dual_mult))]
            rhs = opt - sum(dual_mult[ii]*xhat[ii] for ii in range(len(dual_mult)))
            p.addCuts([1], ['G'], [rhs], [0, len(xhat)+1],
                     thetaind+xind, coefficients)
            # IMPORTANT: Return 0 (accept) to keep the cuts. Returning reject=1 would drop the cuts.
            # When soltype=0, addCuts() causes the node LP to be resolved with the new cuts.
            return (0, None)
    else:
        print("ERROR: Unexpected subproblem status. Rejecting solution.")
        return (1, None)

# Main Problem.

print("**** Benders decomposition algorithm ****")

# Define main problem.
p=xp.problem()

# Define first stage variables.
x=[p.addVariable(vartype=xp.binary) for _ in range(n1)]
theta=p.addVariable() # (positive) second stage cost - change the default lower
                      # bound if second stage cost may not be positive.

# Define objective.
p.setObjective(xp.Sum(c1[jj]*x[jj] for jj in range(n1)) +  theta,
               sense=xp.ObjSense.MINIMIZE)

# Add Pre-Integer Solution callback.
# This callback is called when an integer solution is found, BEFORE it is
# accepted by the solver. When soltype=0 (node relaxation solution), we can
# add cuts directly from this callback, which will cause the node LP to be
# resolved.
p.addPreIntsolCallback(preintsol_callback, None, 0)

# Deactivate presolve and symmetry detection. This is to ensure that the solver will not eliminate
# variables, so the indices we retrieve for the variables and the cuts we add
# correspond to/contain existing optimization variables. Presolve can be
# activated, but additional steps are required for that (presolving the cuts
# the same way as the rows of the matrix).
p.setControl({"presolve":0,"nodepresolve":0,"symmetry":0})

p.controls.outputlog = 0  # Suppress Xpress output

# Solve the main problem.
p.optimize()

print()
if p.attributes.solvestatus == xp.SolveStatus.COMPLETED and \
   p.attributes.solstatus == xp.SolStatus.OPTIMAL:
    # Print results.
    print("**** Final solution (Benders) ****")
    xsol = p.getSolution(x)

    # Solve subproblem one final time to get y values
    final_sub = xp.problem()
    final_sub.controls.outputlog = 0
    y_final = [final_sub.addVariable() for _ in range(n2)]
    constr_final = [xp.Sum(A1[ii][jj]*xsol[jj] for jj in range(n1)) + \
                    xp.Sum(A2[ii][jj]*y_final[jj] for jj in range(n2)) <= b[ii]
                    for ii in range(m)]
    final_sub.addConstraint(constr_final)
    final_sub.setObjective(xp.Sum(c2[jj]*y_final[jj] for jj in range(n2)), sense=xp.ObjSense.MINIMIZE)
    final_sub.optimize()
    ysol = final_sub.getSolution(y_final)

    xsol_rounded = [0.0 if round(val, 2) == 0 else round(val, 2) for val in xsol]
    ysol_rounded = [0.0 if round(val, 2) == 0 else round(val, 2) for val in ysol]

    print(f"Solution with Benders decomposition: obj: {p.attributes.objval:.2f}")
    print(f"  x: {xsol_rounded}")
    print(f"  y: {ysol_rounded}")
else:
    print("Could not solve the problem")
