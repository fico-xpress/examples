// (c) 2024-2026 Fair Isaac Corporation

// Load balancing of train wagons. Second version, using heuristic solution as
// start solution for MIP.

#include <iostream>
#include <numeric>
#include <vector>
#include <xpress.hpp>

using namespace xpress;
using namespace xpress::objects;
using xpress::objects::utils::sum;

// Problem Data:
/** Box weights */
static int const WEIGHT[] = {34, 6,  8,  17, 16, 5, 13, 21,
                             25, 31, 14, 13, 33, 9, 25, 25};
/** Number of boxes. */
static int const NBOXES = sizeof(WEIGHT) / sizeof(WEIGHT[0]);
/** Number of wagons. */
static int const NWAGONS = 3;
/** Weight limit of the wagons. */
static int const WMAX = 100;

/** Create the optimization model in P.
 * @param p          The Xpress solver instance in which the problem is
 *                   created.
 * @param load       Where to store the load variables in the model.
 */
void model(XpressProblem &p, std::vector<std::vector<Variable>> &load) {
  // Create load[box,wagon] (binary)
  load = p.addVariables(NBOXES, NWAGONS)
             .withType(ColumnType::Binary)
             .withName([](auto b, auto w) {
               return xpress::format("load_%d_%d", b + 1, w + 1);
             })
             .toArray();

  // Create maxWeight, continuous with lb=ceil((sum(b in BOXES)
  // WEIGHT(b))/NBOXES)
  double lb = 0.0;
  for (int i = 0; i < NBOXES; ++i)
    lb += WEIGHT[i];
  lb /= static_cast<double>(NBOXES);
  Variable maxWeight =
      p.addVariable(lb, XPRS_PLUSINFINITY, ColumnType::Continuous, "maxWeight");

  // Every box into one wagon:
  //   forall(b in BOXES)
  //      sum(w in WAGONS) load(b,w) = 1
  p.addConstraints(NBOXES, [&](auto b) { return sum(load[b]) == 1.0; });

  // Limit the weight loaded into every wagon:
  //    forall(w in WAGONS)
  //       sum(b in BOXES) WEIGHT(b)*load(b,w) <= maxWeight
  p.addConstraints(NWAGONS, [&](auto w) {
    return sum(NBOXES, [&](auto b) { return load[b][w] * WEIGHT[b]; }) <=
           maxWeight;
  });

  // Minimize maxWeight
  p.setObjective(maxWeight, ObjSense::Minimize);
  p.writeProb("Wagon.lp", "l");
}

/** LPT (Longest processing time) heuristic: One at a time, place the heaviest
 * unassigned box onto the wagon with the least load.
 * @param heurobj  The objective value of the heuristic solution is returned
 *                 here.
 * @return The heuristic solution vector.
 */
std::vector<int> heuristic(double &heurobj) {
  std::vector<int> orderW(
      NBOXES); // Box indices sorted in decreasing weight order
  std::vector<int> curNum(NWAGONS); // For each wagon w, this is the number of
                                    // boxes currently loaded
  std::vector<int> curWeight(NWAGONS); // For each wagon w, this is the current
                                       // weight, i.e. the sum of weights of
  // loaded boxes
  std::vector<std::vector<int>> load(
      NWAGONS, std::vector<int>(NBOXES)); // load[w][i] (for i=0..curNum[w]-1)
                                          // contains the box index of the i-th
  // box loaded on wagon w

  // Copy the box indices into array orderW and sort them in decreasing
  // order of box weights (the sorted indices are returned in array orderW)
  std::iota(orderW.begin(), orderW.end(), 0);
  std::sort(orderW.begin(), orderW.end(),
            [&](auto a, auto b) { return WEIGHT[b] - WEIGHT[a]; });

  // Distribute the loads to the wagons using the LPT heuristic
  for (int b = 0; b < NBOXES; b++) {
    int v = 0; // Find wagon v with the smallest load
    for (int w = 0; w < NWAGONS; w++)
      if (curWeight[w] <= curWeight[v])
        v = w;
    load[v][curNum[v]] = orderW[b];    // Add current box to wagon v
    curNum[v]++;                       // Increase the counter of boxes on v
    curWeight[v] += WEIGHT[orderW[b]]; // Update current weight of the wagon
  }

  // Calculate the solution value
  heurobj = 0; // heuristic solution objective value (max wagon weight)
  for (int w = 0; w < NWAGONS; w++)
    if (curWeight[w] > heurobj)
      heurobj = curWeight[w];

  // Solution printing
  std::cout << "Heuristic solution:" << std::endl
            << "Max weight: " << heurobj << std::endl;

  for (int w = 0; w < NWAGONS; w++) {
    std::cout << " " << (w + 1) << ": ";
    for (int i = 0; i < curNum[w]; i++)
      std::cout << " " << (load[w][i] + 1);
    std::cout << " (total weight: " << curWeight[w] << ")" << std::endl;
  }

  // Save the heuristic solution into the heurSol array
  std::vector<int> heurSol(NBOXES);
  for (int w = 0; w < NWAGONS; w++)
    for (int i = 0; i < curNum[w]; i++)
      heurSol[load[w][i]] = w;
  return heurSol;
}

static void optimization(XpressProblem &p,
                         std::vector<std::vector<Variable>> &load,
                         std::vector<int> const &heurSol) {
  // Get the solution from the heuristic solution we have found
  // Send the solution to the optimizer
  std::vector<Variable> heurmipvar(NBOXES);
  std::vector<double> heurmipsol(NBOXES);
  for (int b = 0; b < NBOXES; b++) {
    heurmipvar[b] = load[b][heurSol[b]];
    heurmipsol[b] = 1.0;
  }
  p.addMipSol(heurmipsol, heurmipvar, "heuristic");
  p.optimize();
  if (p.attributes.getSolStatus() != SolStatus::Optimal &&
      p.attributes.getSolStatus() != SolStatus::Feasible)
    throw std::runtime_error("failed to optimize with status " +
                             to_string(p.attributes.getSolStatus()));

  std::cout << "Problem status:" << std::endl
            << "\tSolve status: " << to_string(p.attributes.getSolveStatus())
            << std::endl
            << "\tLP status: " << to_string(p.attributes.getLpStatus())
            << std::endl
            << "\tMIP status: " << to_string(p.attributes.getMipStatus())
            << std::endl
            << "\tSol status: " << to_string(p.attributes.getSolStatus())
            << std::endl;

  // An integer solution has been found
  if (p.attributes.getMipStatus() == MIPStatus::Optimal ||
      p.attributes.getMipStatus() == MIPStatus::Solution) {
    auto sol = p.getSolution();
    std::cout << "Optimal solution:" << std::endl
              << " Max weight: " << p.attributes.getObjVal() << std::endl;
    for (int w = 0; w < NWAGONS; w++) {
      double tot_weight = 0.;
      std::cout << " " << (w + 1) << ": ";
      for (int b = 0; b < NBOXES; b++)
        if (load[b][w].getValue(sol) * WEIGHT[b] > 0.5) {
          std::cout << " " << load[b][w].getValue(sol) * WEIGHT[b];
          tot_weight += load[b][w].getValue(sol) * WEIGHT[b];
        }
      std::cout << " (total weight: " << tot_weight << ")" << std::endl;
    }
  }
}

int main() {
  double heurobj;
  std::vector<int> heursol = heuristic(heurobj);
  if (heurobj <= WMAX) {
    std::cout << "Heuristic solution fits capacity limits" << std::endl;
  } else {
    XpressProblem prob;
    std::vector<std::vector<Variable>> load;
    model(prob, load);
    optimization(prob, load, heursol);
  }
  return 0;
}
