// (c) 2026-2026 Fair Isaac Corporation

using System;
using Optimizer;
using Optimizer.Maps;
using Optimizer.Objects;
using static Optimizer.Objects.Utils;


namespace XpressExamples {
  /// <summary>9x9 Sudoku puzzle to illustrate solution enumeration.</summary>
  /// <remarks>
  /// This is a pure feasibility problem, i.e., the objective function is
  /// all-zero.
  /// We first solve with prefixed values that allow exactly one solution.
  /// Then we add clear one of the fixings and enumerate all solutions the
  /// new board allows.
  /// </remarks>
  class Sudoku {
    /// <summary>The available row indices (first row has index zero).</summary>
    static readonly int[] ROWS = {0, 1, 2, 3, 4, 5, 6, 7, 8};
    /// <summary>The available column indices (first column has index zero). </summary>
    static readonly int[] COLS = {0, 1, 2, 3, 4, 5, 6, 7, 8};
    /// <summary>The available values (1 through 9).</summary>
    static readonly int[] VALS = {1, 2, 3, 4, 5, 6, 7, 8, 9};

    /// <summary>Values that are fixed in the beginning.</summary>
    /// <remarks>
    /// These fixings allow exactly one feasible solution.
    /// Data set from "The Guardian", 15 July, 2005. http://www.guardian.co.uk/sudoku
    /// </remarks>
    static int[,] PRESET = {
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

    /// <summary>Print a board to the console.</summary>
    /// <param name='board'>A 9x9 array that specifies the numbers that are
    /// set in board. A zero means "not set".</param>
    static void PrintBoard(int[,] board) {
      foreach (int r in ROWS) {
        string fill = (r == 3 || r == 6) ? "===+" : "---+";
        Console.Write("+");
        foreach (int c in COLS) {
          Console.Write(fill);
          if (c == 2 || c == 5)
            Console.Write('+');
        }
        Console.WriteLine();
        Console.Write("|");
        foreach (int c in COLS) {
          Console.WriteLine($" {((board[r,c] == 0) ? ' ' : (char)(board[r,c] + '0'))} |");
          if (c == 2 || c == 5)
            Console.Write("|");
        }
        Console.WriteLine();
      }
      Console.Write("+");
      foreach (int c in COLS) {
        Console.Write("---+");
        if (c == 2 || c == 5)
          Console.Write('+');
      }
      Console.WriteLine();
    }

    /// <summary>Print a soolution.</summary>
    /// <remarks>
    /// Constructs a board from a solution and then prints it.
    /// </remarks>
    /// <param name='b'>The binary variables.</param>
    /// <param name='sol'>The solution values for b.</param>
    static void PrintSolution(HashMap3<int, int, int, Variable> b, double[] sol) {
      int[,] result = new int[9,9];
      foreach (int r in ROWS) {
        foreach (int c in COLS) {
          foreach (int v in VALS) {
            if (b[r, c, v].GetValue(sol) > 0.5) {
              result[r,c] = v;
            }
          }
        }
      }
      PrintBoard(result);
    }

    /// <summary>Solve the 9x9 Sudoku puzzle with the given fixed values.</summary>
    /// <param name='preset'>The values that are fixed on the board.</param>
    /// <param name='enumerate'>If true then we enumerate all solutions,
    ///                   otherwise we stop on the first solution. Note that
    ///                   enumeration can become expensive the more solutions
    ///                   there are.</param>
    static void Solve(int[,] preset, bool enumerate) {
      using (XpressProblem prob = new XpressProblem()) {
        // Create the variables. One binary variable for each combination of
        // row, column, value.
        // We store the variables in a map rather than a 3D array since rows and
        // columns index from 0, but values index from 1.
        HashMap3<int, int, int, Variable> b = prob.AddVariables(ROWS, COLS, VALS)
          .WithName("b({0},{1},{2})")
          .WithType(ColumnType.Binary)
          .ToMap();

        // Fix the variables that should be fixed on the initial board.
        foreach (int r in ROWS) {
          foreach (int c in COLS) {
            if (preset[r,c] > 0) {
              b[r, c, preset[r, c]].Fix(1.0);
            }
          }
        }

        // Each cell must have a number.
        prob.AddConstraints(ROWS, COLS, (r, c) => Sum(VALS, v => b[r, c, v]) == 1.0);

        // Each number must appear exactly once in a row.
        prob.AddConstraints(ROWS, VALS, (r, v) => Sum(COLS, c => b[r, c, v]) == 1.0);
        // Each number must appear exactly once in a column.
        prob.AddConstraints(COLS, VALS, (c, v) => Sum(ROWS, r => b[r, c, v]) == 1.0);
        // In 3x3 squares all values must be different.
        for (int k = 0; k < 3; ++k) {
          int[] colidx = {3 * k, 1 + 3 * k, 2 + 3 * k};
          foreach (int v in VALS) {
            prob.AddConstraint(Sum(new int[]{0, 1, 2}, colidx,
                                   (x, y) => b[x, y, v]) == 1.0);
            prob.AddConstraint(Sum(new int[]{3, 4, 5}, colidx,
                                   (x, y) => b[x, y, v]) == 1.0);
            prob.AddConstraint(Sum(new int[]{6, 7, 8}, colidx,
                                   (x, y) => b[x, y, v]) == 1.0);
          }
        }
        // Enable log output and prepare for numeration (if requested).
        prob.callbacks.AddMessageCallback(DefaultMessageListener.Console);
        if (enumerate) {
          prob.PoolEnumStrategy = PoolEnumStrategy.Full;
          prob.PoolLimit = Int32.MaxValue; // Find all solutions.
        }
        // Find all solutions.
        prob.Optimize();
        // Print solutions.
        // Note that we only print the first 10 solutions to not flood the screen.
        for (int s = 0; s < Math.Min(10, prob.PoolSize); ++s) {
          Console.WriteLine($"Solution {s}:");
          PrintSolution(b, prob.GetPoolSolution(s));
        }
      }
    }


    public static void Main(string[] args) {
      // First solve with the preset board. This should allow exactly one solution.
      Solve(PRESET, false);
      // Now remove some fixed values and enumerate all solutions.
      PRESET[0, 3] = 0;
      Solve(PRESET, true);
    }
  }
}
