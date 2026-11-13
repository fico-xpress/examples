// (c) 2026-2026 Fair Isaac Corporation
import java.util.Arrays;
import java.util.Collection;

import com.dashoptimization.ColumnType;
import com.dashoptimization.DefaultMessageListener;
import com.dashoptimization.maps.HashMap3;
import com.dashoptimization.objects.Variable;
import com.dashoptimization.objects.XpressProblem;
import static com.dashoptimization.objects.Utils.sum;

/** 9x9 Sudoku puzzle to illustrate solution enumeration.
 * This is a pure feasibility problem, i.e., the objective function is all-zero.
 * We first solve with prefixed values that allow exactly one solution. Then
 * we add clear one of the fixings and enumerate all solutions the new board
 * allows.
 */
public final class Sudoku {
  /** The available row indices (first row has index zero). */
  private static final Collection<Integer> ROWS = Arrays.asList(0, 1, 2, 3, 4, 5, 6, 7, 8);
  /** The available column indices (first column has index zero). */
  private static final Collection<Integer> COLS = Arrays.asList(0, 1, 2, 3, 4, 5, 6, 7, 8);
  /** The available values (1 through 9). */
  private static final Collection<Integer> VALS = Arrays.asList(1, 2, 3, 4, 5, 6, 7, 8, 9);
  /** Values that are fixed in the beginning.
   * These fixings allow exactly one feasible solution.
   * Data set from "The Guardian", 15 July, 2005. http://www.guardian.co.uk/sudoku
   */
  private static final int[][] PRESET = {
    { 0, 0, 0, 7, 0, 5, 6, 9, 0 },
    { 0, 0, 8, 0, 4, 0, 0, 0, 7 },
    { 0, 2, 0, 0, 0, 3, 0, 0, 0 },
    { 0, 0, 2, 0, 0, 0, 9, 0, 0 },
    { 0, 0, 0, 9, 0, 0, 0, 8, 0 },
    { 0, 0, 0, 0, 1, 0, 0, 0, 5 },
    { 0, 0, 0, 0, 0, 4, 0, 0, 0 },
    { 0, 0, 6, 0, 0, 0, 3, 0, 4 },
    { 0, 0, 0, 8, 2, 1, 0, 5, 0 },
  };
  /** Solve the 9x9 Sudoku puzzle with the given fixed values.
   * @param fixed      The values that are fixed on the board.
   * @param enumerate  If <code>true</code> then we enumerate all solutions,
   *                   otherwise we stop on the first solution. Note that
   *                   enumeration can become expensive the more solutions
   *                   there are.
   */
  private static void solve(int[][] fixed, boolean enumerate) {
    try (XpressProblem prob = new XpressProblem()) {
      // Create the variables. One binary variable for each combination of
      // row, column, value.
      // We store the variables in a map rather than a 3D array since rows and
      // columns index from 0, but values index from 1.
      HashMap3<Integer,Integer,Integer,Variable> b = prob.addVariables(ROWS, COLS, VALS)
        .withName("b(%d,%d,%d)")
        .withType(ColumnType.Binary)
        .toMap();

      // Fix the variables that should be fixed on the initial board.
      for (int r : ROWS) {
        for (int c : COLS) {
          if (fixed[r][c] > 0) {
            b.get(r, c, fixed[r][c]).fix(1.0);
          }
        }
      }

      // Each cell must have a number.
      prob.addConstraints(ROWS, COLS, (r, c) -> sum(VALS, v -> b.get(r, c, v)).eq(1.0));

      // Each number must appear exactly once in a row.
      prob.addConstraints(ROWS, VALS, (r,v) -> sum(COLS, c -> b.get(r, c, v)).eq(1.0));
      // Each number must appear exactly once in a column.
      prob.addConstraints(COLS, VALS, (c,v) -> sum(ROWS, r -> b.get(r, c, v)).eq(1.0));
      // In 3x3 squares all values must be different.
      for (int k = 0; k < 3; ++k) {
        Collection<Integer> colidx = Arrays.asList(3 * k, 1 + 3 * k, 2 + 3 * k);
        for (int v : VALS) {
          prob.addConstraint(sum(Arrays.asList(0, 1, 2), colidx,
                                 (x, y) -> b.get(x, y, v)).eq(1.0));
          prob.addConstraint(sum(Arrays.asList(3, 4, 5), colidx,
                                 (x, y) -> b.get(x, y, v)).eq(1.0));
          prob.addConstraint(sum(Arrays.asList(6, 7, 8), colidx,
                                 (x, y) -> b.get(x, y, v)).eq(1.0));
        }
      }
      // Enable log output and prepare for numeration (if requested).
      prob.callbacks.addMessageCallback(DefaultMessageListener::console);
      if (enumerate) {
        prob.controls().setPoolEnumStrategy(com.dashoptimization.XPRSenumerations.PoolEnumStrategy.FULL);
        prob.controls().setPoolLimit(Integer.MAX_VALUE); // Find all solutions.
      }
      // Find all solutions.
      prob.optimize();
      // Print solutions.
      // Note that we only print the first 10 solutions to not flood the screen.
      for (int s = 0; s < Math.min(10, prob.attributes().getPoolSize()); ++s) {
        System.out.println("Solution " + s + ":");
        printSolution(b, prob.getPoolSolution(s));
      }
    }
  }

  /** Print a board to the console.
   * @param board  A 9x9 array that specifies the numbers that are set in
   *               board. A zero means "not set".
   */
  private static void printBoard(int[][] board) {
    for (int r = 0; r < board.length; ++r) {
      String fill = (r == 3 || r == 6) ? "===+" : "---+";
      System.out.print("+");
      for (int c = 0; c < board[r].length; ++c) {
        System.out.print(fill);
        if (c == 2 || c == 5)
          System.out.print('+');
      }
      System.out.println();
      System.out.print("|");
      for (int c = 0; c < board[r].length; ++c) {
        System.out.printf(" %c |", (board[r][c] == 0) ? ' ' : (char)(board[r][c] + '0'));
        if (c == 2 || c == 5)
          System.out.print('|');
      }
      System.out.println();
    }
    System.out.print("+");
    for (int c = 0; c < board[0].length; ++c) {
      System.out.print("---+");
      if (c == 2 || c == 5)
        System.out.print('+');
    }
    System.out.println();
  }

  /** Print a soolution.
   * Constructs a board from a solution and then prints it.
   * @param b    The binary variables.
   * @param sol  The solution values for <code>b</code>.
   */
  private static void printSolution(HashMap3<Integer,Integer,Integer,Variable> b, double[] sol) {
    int[][] result = new int[9][9];
    for (int r : ROWS) {
      for (int c : COLS) {
        for (int v : VALS) {
          if (b.get(r, c, v).getValue(sol) > 0.5) {
            result[r][c] = v;
          }
        }
      }
    }
    printBoard(result);
  }

  public static void main(String[] args) {
    // First solve with the preset board. This should allow exactly one solution.
    solve(PRESET, false);
    // Now remove some fixed values and enumerate all solutions.
    PRESET[0][3] = 0;
    solve(PRESET, true);
  }
}
