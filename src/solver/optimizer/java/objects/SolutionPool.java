/**
   Xpress Optimizer Examples
   =========================

   file SolutionPool.java
   ````````````````````
   Capacitated Facility Location Problem (CFLP) demonstrating the
   solution pool and solution enumeration.

   25 customers, 10 potential facility sites with integer coordinates.
   Single-sourcing: each customer is assigned to exactly one open
   facility. Service cost equals the Euclidean distance from the
   customer to its assigned facility. Fixed opening cost is correlated
   with facility capacity.

   Demonstrates four uses of the built-in solution pool and enumeration
   controls on the CFLP model:
   - collecting up to ten solutions found during a normal MIP solve
     (POOLLIMIT);
   - warm-starting a re-solve from the pool after fixing a facility
     open (POOLKEEPSOLS);
   - enumerating the five best solutions overall (POOLENUMSTRATEGY,
     POOLRELGAP);
   - enumerating the five best facility assignments only, by
     restricting enumeration to the facility variables
   (XpressProblem.loadBranchDirs()).
   After each solve, the objective value, number of open facilities,
   and maximum service distance are printed for each collected solution.

   Model
   -----
   Sets
   I = {0 ... N_CUST-1}   customers
   J = {0 ... N_FAC-1}    potential facility sites

   Parameters
   d_i      demand of customer i
   f_j      fixed cost of opening facility j
   Q_j      capacity of facility j
   c_ij     Euclidean distance from customer i to facility j

   Decision variables
   y_j  in {0,1}   1 iff facility j is opened
   x_ij in {0,1}   1 iff customer i is served by facility j

   Objective
   min  sum_j f_j y_j  +  sum_i sum_j c_ij x_ij

   Constraints
   (1)  sum_j x_ij = 1               for all i  (demand satisfaction)
   (2)  sum_i d_i x_ij <= Q_j y_j    for all j  (capacity + activation)

   (c) 2026 Fair Isaac Corporation
   ********************************************************************** */

import com.dashoptimization.ColumnType;
import com.dashoptimization.DefaultMessageListener;
import com.dashoptimization.XPRSconstants;
import com.dashoptimization.objects.Variable;
import com.dashoptimization.objects.XpressProblem;
import static com.dashoptimization.objects.Utils.sum;
import static com.dashoptimization.XPRSenumerations.PoolEnumStrategy;
import static com.dashoptimization.XPRSenumerations.SolveStatus;
import static com.dashoptimization.XPRSenumerations.SolStatus;

public final class SolutionPool {
  // ---- Instance data ----------------------------------------------------
  public static final int N_CUST = 25;
  public static final int N_FAC = 10;

  public static final int[][] custXY = {
    {20,  9}, {22, 15}, { 5, 13}, { 6, 26}, {13, 14}, // cluster A -- south-west
    {15, 19}, {20, 19}, {11,  7}, { 3, 21},
    {70, 27}, {69, 33}, {82, 26}, {74, 30}, {77, 20}, // cluster B -- south-east
    {82, 11}, {72, 25}, {70, 24},
    {56, 82}, {53, 74}, {50, 84}, {39, 81}, {55, 80}, // cluster C -- north
    {43, 84},
    {46, 47}, {30, 55}                                // outliers  -- central
  };

  public static final int[] demands = {
    26, 25, 37, 22, 35, 22, 43, 21, 41,   // cluster A
    29, 61, 32, 41, 40, 39, 19, 72,       // cluster B
    23, 40, 59, 27, 45, 25,               // cluster C
    21, 40                                // outliers
  };

  public static final int[][] facXY = {
    { 7, 15}, {12, 19},  // j=0,1 -- near cluster A
    {76, 19}, {80, 21},  // j=2,3 -- near cluster B
    {56, 88}, {52, 79},  // j=4,5 -- near cluster C
    {45, 50}, {63, 48},  // j=6,7 -- central bridge
    { 3, 93}, { 3, 88}   // j=8,9 -- peripheral
  };

  public static final int[] capacities = {307, 98, 86, 298, 211, 190, 70, 205, 140, 165};
  public static final int[] fixedCosts = {787, 233, 257, 756, 567, 517, 227, 535, 367, 445};


  /** Struct for collecting model data. */
  static final class ModelData {
    final Variable[][] x;
    final Variable[] y;
    final double[][] dist;
    public ModelData(Variable[][] x, Variable[] y, double[][] dist) {
      this.x = x;
      this.y = y;
      this.dist = dist;
    }
  }


  /** Builds the CFLP model on prob. */
  static ModelData buildModel(XpressProblem prob) {
    // ---- Compute service costs ----
    double[][] dist = new double[N_CUST][];
    for (int i = 0; i < N_CUST; i++) {
      dist[i] = new double[N_FAC];
      for (int j = 0; j < N_FAC; j++) {
        double dx  = (double)(custXY[i][0] - facXY[j][0]);
        double dy  = (double)(custXY[i][1] - facXY[j][1]);
        dist[i][j] = Math.sqrt(dx * dx + dy * dy);
      }
    }

    // ---- Build the variables ----
    // Add the N_FAC facility binary variables.
    Variable[] y = prob.addVariables(N_FAC)
      .withType(ColumnType.Binary)
      .withName(i -> String.format("y(%d)", i))
      .toArray();
    // Add the N_CUST*N_FAC service binary variables.
    Variable[][] x = prob.addVariables(N_CUST, N_FAC)
      .withType(ColumnType.Binary)
      .withName((i, j) -> String.format("x(%d,%d)", i, j))
      .toArray();

    // ---- Build the constraints ----
    // Demand satisfaction constraints.
    //    sum_j x_ij = 1 for all i
    prob.addConstraints(N_CUST, i -> sum(N_FAC, j -> x[i][j]).eq(1.0));
    // Capacity and activation constraints.
    //    sum_i d_i x_ij <= Q_j y_j for all j
    prob.addConstraints(N_FAC, j -> sum(N_CUST, i -> x[i][j].mul(demands[i])).leq(y[j].mul(capacities[j])));
    // ---- Build the objective function ----
    //    sum_j f_j y_j  +  sum_i sum_j c_ij x_ij
    prob.setObjective(sum(N_FAC, j -> y[j].mul(fixedCosts[j])).plus(sum(N_CUST, N_FAC, (i, j) -> x[i][j].mul(dist[i][j]))));

    return new ModelData(x, y, dist);
  }


  /** Print the list of collected solutions. We list the following information:
   *   - Objective function value
   *   - Number of facilities opened
   *   - Maximum distance between a facility and a customer it serves.
   */
  static void printSolutions(XpressProblem prob, ModelData modelData) {
    int poolSize = prob.attributes().getPoolSize();

    System.out.printf("%nCollected %d solution(s):%n", poolSize);
    System.out.printf("  %4s  %12s  %11s  %9s%n", "Sol#", "Objective", "Facilities", "Max dist");
    for (int p = 0; p < poolSize; p++) {
      double poolObj = prob.getPoolSolObjVal(p);
      double[] sol = prob.getPoolSolution(p);

      int nOpen = 0;
      double maxDist = 0.0;
      for (int j = 0; j < N_FAC; j++)
        nOpen += (modelData.y[j].getValue(sol) > 0.1) ? 1 : 0;
      for (int i = 0; i < N_CUST; i++)
        for (int j = 0; j < N_FAC; j++)
          if (modelData.x[i][j].getValue(sol) > 0.1) {
            maxDist = Math.max(maxDist, modelData.dist[i][j]);
            break;
          }
      System.out.printf("  %4d  %12.2f  %11d  %9.2f%n", p, poolObj, nOpen, maxDist);
    }
    System.out.printf("%n");
  }

  /* Solve our model and collect up to 10 solutions into the pool.
   */
  static void solveCollectSolutions(XpressProblem prob, ModelData modelData) {
    // Collect up to ten solutions during the MIP search. Solutions are
    // stored from best to worst; index 0 always holds the best solution.
    prob.controls().setPoolLimit(10);

    System.out.printf("%nSolving problem to collect solutions%n%n");
    prob.optimize();
    SolveStatus solveStatus = prob.attributes().getSolveStatus();
    SolStatus solStatus = prob.attributes().getSolStatus();
    if (solveStatus == SolveStatus.COMPLETED && solStatus == SolStatus.OPTIMAL) {
      System.out.printf("Solved problem to optimality.%n");
    } else {
      System.out.printf("Failed to solve problem with solvestatus %s and solstatus %s%n", solveStatus, solStatus);
      return;
    }

    // ---- Print the solution pool ----
    printSolutions(prob, modelData);
  }

  /** Assumes that we have already solved the problem and that the pool contains
   * a set of solutions.
   *
   * This function forces a single facility to be open and uses the pooled
   * solutions to warm-start the next MIP solve.
   */
  static void solveWarmStartFromPool(XpressProblem prob, ModelData modelData) {
    // Make sure the problem is in its original state before we change it.
    // If the problem is already in its original state, this call will simply
    // return without doing anything.
    prob.postSolve();

    // We need to set the control POOLKEEPSOLS to transfer any existing
    // solutions to the warm-start pool before we fix a variable, since any
    // modifications will invalidate all collected selections.
    prob.controls().setPoolKeepSols(-1);

    // Force a facility to be open. If we open an additional facility, all
    // existing solution should remain feasible - the presolver should take
    // care of satisfying the fixed binary. The warm-start solutions will not
    // be optimal since no customers are served by the new facility.
    final int yIdx = 2; // Pick a facility that should not be open in our current optimal solution.
    modelData.y[yIdx].fix(1.0);

    // ---- Re-solve ----
    System.out.printf("%nReoptimizing with fixed facility %d.%n%n", yIdx);
    prob.optimize();
    SolveStatus solveStatus = prob.attributes().getSolveStatus();
    SolStatus solStatus = prob.attributes().getSolStatus();
    if (solveStatus == SolveStatus.COMPLETED && solStatus == SolStatus.OPTIMAL) {
      System.out.printf("Resolved problem to optimality.%n");
    } else {
      System.out.printf("Failed to resolve problem with solvestatus %s and solstatus %s%n", solveStatus, solStatus);
      return;
    }

    // ---- Print the new solution pool ----
    printSolutions(prob, modelData);

    // Reset controls. We should do this before modifying the problem
    // to avoid automatically transferring the solutions when restoring
    // the bound below.
    prob.controls().resetPoolKeepSols();

    // Undo the fixed facility by returning the lower bound to 0.0.
    modelData.y[yIdx].setLB(0.0);
  }

  /** Use the built-in solution enumeration to find the 5 best solutions to
   * our problems. The solver, by default, considers two solutions as being
   * distinct if they differ in any of our binary restricted variables.
   * To help speed up the solve, we limit the search to solutions whose
   * objective value is within 50% of the best solution.
   */
  static void enumerateAllBinaries(XpressProblem prob, ModelData modelData) {
    // Collect up to 5 best solutions.
    prob.controls().setPoolLimit(5);

    // Enable automatic solution enumeration.
    prob.controls().setPoolEnumStrategy(PoolEnumStrategy.FULL);

    // We limit collection to solutions that are within 50% of the best
    // solution.
    prob.controls().setPoolRelGap(0.50);

    // Solve the problem, which will automatically trigger an extended
    // branch-and-bound search to find all 5 best solutions.
    System.out.printf("%nSolving problem to find 5 best solutions.%n%n");
    prob.optimize();
    SolveStatus solveStatus = prob.attributes().getSolveStatus();
    SolStatus solStatus = prob.attributes().getSolStatus();
    if (solveStatus == SolveStatus.COMPLETED && solStatus == SolStatus.OPTIMAL) {
      System.out.printf("Solved problem to optimality.%n");
    } else {
      System.out.printf("Failed to solve problem with solvestatus %s and solstatus %s%n", solveStatus, solStatus);
      return;
    }

    // Print the collected solutions.
    printSolutions(prob, modelData);
  }

  /** We want to find the 5 best combinations of opening facilities but the
   * demand sourcing should be optimal in each solution. To achieve this,
   * we need to tell the solver to enumerate only through solutions that
   * differ in facility variables. We can do this through solution enumeration
   * by specifying branching directives only on the facility variables.
   * The solver will automatically treat two solutions with the same
   * facility assignment as duplicates and only keep the best one, which
   * should result in a solution with the optimal demand assignment for each
   * facility assignment.
   */
  static void enumerateFacilities(XpressProblem prob, ModelData modelData) {
    // Make sure the problem is in its original state before we change it.
    // If the problem is already in its original state, this call will simply
    // return without doing anything.
    prob.postSolve();

    // Collect up to 5 best solutions.
    prob.controls().setPoolLimit(5);

    // Enable automatic solution enumeration.
    prob.controls().setPoolEnumStrategy(PoolEnumStrategy.FULL);

    // Apply branching directives to the facility binaries.
    // Binaries without a directive will not be enumerated.
    int[] directivesOn = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
    prob.loadBranchDirs(modelData.y, directivesOn);

    // Solve the problem, which will automatically trigger an extended
    // branch-and-bound search to find the 5 best facility assignments.
    System.out.printf("%nSolving problem to find 5 best facility assignments.%n%n");
    prob.optimize();
    SolveStatus solveStatus = prob.attributes().getSolveStatus();
    SolStatus solStatus = prob.attributes().getSolStatus();
    if (solveStatus == SolveStatus.COMPLETED && solStatus == SolStatus.OPTIMAL) {
      System.out.printf("Resolved problem to optimality.%n");
    } else {
      System.out.printf("Failed to solve problem with solvestatus %s and solstatus %s%n", solveStatus, solStatus);
      return;
    }

    // Print the collected solutions.
    printSolutions(prob, modelData);

    // Reset controls.
    prob.controls().resetPoolLimit();
    prob.controls().resetPoolEnumStrategy();

    // Clear branching directives.
    int[] directivesOff = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    prob.loadBranchDirs(modelData.y, directivesOff);

  }

  public static void main(String[] args) {
    // Create a new problem and immediately register a message handler.
    // Once we have a message handler installed, errors will produce verbose
    // error messages on the console and we can limit ourselves to minimal
    // error handling in the code here.
    try (XpressProblem prob = new XpressProblem()) {
      prob.addMessageListener(DefaultMessageListener::console);

      // ---- Build the model ----
      ModelData modelData = buildModel(prob);

      // Solve the model and collect solutions.
      solveCollectSolutions(prob, modelData);

      // Try forcing a facility to be opened and reoptimize. We carry the
      // existing solutions through as warm-start solutions for the next
      // solve.
      solveWarmStartFromPool(prob, modelData);

      // Run an enumeration search to find the 5 best solutions.
      enumerateAllBinaries(prob, modelData);

      // Run an enumeration search to find the 5 best facility assignments.
      enumerateFacilities(prob, modelData);

    }
  }
}
