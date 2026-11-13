// (c) 2026-2026 Fair Isaac Corporation
//
// 9x9 Sudoku puzzle to illustrate solution enumeration.
// This is a pure feasibility problem, i.e., the objective function is all-zero.
// We first solve with prefixed values that allow exactly one solution. Then
// we add clear one of the fixings and enumerate all solutions the new board
// allows.
#include <limits>
#include <algorithm>
#include <xpress.hpp>
#include <xpress_objects.hpp>

using namespace xpress;
using namespace xpress::objects;
using xpress::objects::utils::sum;

/** The available row indices (first row has index zero). */
std::vector<int> const ROWS{0, 1, 2, 3, 4, 5, 6, 7, 8};
/** The available column indices (first column has index zero). */
std::vector<int> const COLS{0, 1, 2, 3, 4, 5, 6, 7, 8};
/** The available values (1 through 9). */
std::vector<int> const VALS{1, 2, 3, 4, 5, 6, 7, 8, 9};

/** Structure to describe a Sudoku board.
 * Elements in the cells are either numbers 1-9 or zero to indicate that there
 * is no number in this position.
 */
struct Board {
  int cells[9][9];
};
/** Values that are fixed in the beginning.
 * These fixings allow exactly one feasible solution.
 * Data set from "The Guardian", 15 July, 2005. http://www.guardian.co.uk/sudoku
 */
Board PRESET = { {
  { 0, 0, 0, 7, 0, 5, 6, 9, 0 },
  { 0, 0, 8, 0, 4, 0, 0, 0, 7 },
  { 0, 2, 0, 0, 0, 3, 0, 0, 0 },
  { 0, 0, 2, 0, 0, 0, 9, 0, 0 },
  { 0, 0, 0, 9, 0, 0, 0, 8, 0 },
  { 0, 0, 0, 0, 1, 0, 0, 0, 5 },
  { 0, 0, 0, 0, 0, 4, 0, 0, 0 },
  { 0, 0, 6, 0, 0, 0, 3, 0, 4 },
  { 0, 0, 0, 8, 2, 1, 0, 5, 0 },
  } };

/** Print a board to the console.
 * @param board  A 9x9 array that specifies the numbers that are set in
 *               board. A zero means "not set".
 */
void printBoard(Board const &board) {
  for (auto r : ROWS) {
    char const *fill = (r == 3 || r == 6) ? "===+" : "---+";
    std::cout << "+";
    for (auto c : COLS) {
      std::cout << fill;
      if (c == 2 || c == 5)
        std::cout << '+';
    }
    std::cout << std::endl;
    std::cout << "|";
    for (auto c : COLS) {
      std::cout << " " << ((board.cells[r][c] == 0) ? ' ' : (char)(board.cells[r][c] + '0')) << " |";
      if (c == 2 || c == 5)
        std::cout << "|";
    }
    std::cout << std::endl;
  }
  std::cout << "+";
  for (auto c : COLS) {
    std::cout << "---+";
    if (c == 2 || c == 5)
      std::cout << '+';
  }
  std::cout << std::endl;
}

/** Print a soolution.
 * Constructs a board from a solution and then prints it.
 * @param b    The binary variables.
 * @param sol  The solution values for <code>b</code>.
 */
void printSolution(maps::HashMap3<int, int, int, xpress::objects::Variable> const &b, std::vector<double> const &sol) {
  Board result;
  for (int r : ROWS) {
    for (int c : COLS) {
      result.cells[r][c] = 0;
    }
  }
  for (int r : ROWS) {
    for (int c : COLS) {
      for (int v : VALS) {
        if (b(r, c, v).getValue(sol) > 0.5) {
          result.cells[r][c] = v;
        }
      }
    }
  }
  printBoard(result);
}



/** Solve the 9x9 Sudoku puzzle with the given fixed values.
 * @param fixed      The values that are fixed on the board.
 * @param enumerate  If <code>true</code> then we enumerate all solutions,
 *                   otherwise we stop on the first solution. Note that
 *                   enumeration can become expensive the more solutions
 *                   there are.
 */
void solve(Board const &fixed, bool enumerate) {
  XpressProblem prob;
  // Create the variables. One binary variable for each combination of
  // row, column, value.
  // We store the variables in a map rather than a 3D array since rows and
  // columns index from 0, but values index from 1.
  auto b = prob.addVariables(ROWS, COLS, VALS)
    .withName("b(%d,%d,%d)")
    .withType(ColumnType::Binary)
    .toMap();

  // Fix the variables that should be fixed on the initial board.
  for (auto r : ROWS) {
    for (auto c : COLS) {
      if (fixed.cells[r][c] > 0) {
        b(r, c, fixed.cells[r][c]).fix(1.0);
      }
    }
  }

  // Each cell must have a number.
  prob.addConstraints(ROWS, COLS, [&](auto r, auto c){ return sum(VALS, [&](auto v){ return b(r, c, v); }) == 1.0; });

  // Each number must appear exactly once in a row.
  prob.addConstraints(ROWS, VALS, [&](auto r, auto v){ return sum(COLS, [&](auto c){ return b(r, c, v); }) == 1.0; });
    // Each number must appear exactly once in a column.
    prob.addConstraints(COLS, VALS, [&](auto c, auto v){ return sum(ROWS, [&](auto r){ return b(r, c, v); }) == 1.0; } );
  // In 3x3 squares all values must be different.
  for (int k = 0; k < 3; ++k) {
    std::vector<int> colidx{3 * k, 1 + 3 * k, 2 + 3 * k};
    for (auto v : VALS) {
      prob.addConstraint(sum(std::vector<int>{0, 1, 2}, colidx,
                             [&](auto x, auto y){ return b(x, y, v); }) == 1.0);
      prob.addConstraint(sum(std::vector<int>{3, 4, 5}, colidx,
                             [&](auto x, auto y){ return b(x, y, v); }) == 1.0);
      prob.addConstraint(sum(std::vector<int>{6, 7, 8}, colidx,
                             [&](auto x, auto y){ return b(x, y, v); }) == 1.0);
    }
  }
  // Enable log output and prepare for enumeration (if requested).
  prob.callbacks.addMessageCallback(XpressProblem::console);
  if (enumerate) {
    prob.controls.setPoolEnumStrategy(PoolEnumStrategy::Full);
    prob.controls.setPoolLimit(std::numeric_limits<int>::max()); // Find all solutions.
  }
  // Find all solutions.
  prob.optimize();
  // Print solutions.
  // Note that we only print the first 10 solutions to not flood the screen.
  for (int s = 0; s < std::min(10, prob.attributes.getPoolSize()); ++s) {
    std::cout << "Solution " << s << ":" << std::endl;
    printSolution(b, prob.getPoolSolution(s));
  }
}


int main(void) {
  // First solve with the preset board. This should allow exactly one solution.
  solve(PRESET, false);
  // Now remove some fixed values and enumerate all solutions.
  PRESET.cells[0][3] = 0;
  solve(PRESET, true);
  return 0;
}
