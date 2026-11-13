// (c) 2024-2026 Fair Isaac Corporation
#include <stdexcept> // For throwing exceptions
#include <xpress.hpp>

using namespace xpress;
using namespace xpress::objects;
using xpress::objects::utils::div;
using xpress::objects::utils::mul;
using xpress::objects::utils::pow;
using xpress::objects::utils::scalarProduct;
using xpress::objects::utils::sum;

/**
 * A craftsman makes small wooden boxes for sale. He has four different types of
 * box, and can make each type in any size (keeping all the dimensions in
 * proportion), but all boxes of the same type must have the same size. The
 * profit he makes on a box depends on the size. He has only a limited amount of
 * the necessary wood available and a limited amount of time in the week to do
 * the work. How many boxes should he make, and what size should they be, in
 * order to maximize his profit?
 */

// A box.
class Box {
public:
  const std::string name;   // The name of this box.
  const double lengthCoeff; // Relative length of this box.
  const double widthCoeff;  // Relative width of this box.
  const double heightCoeff; // Relative height of this box.
  const double profitCoeff; // Coefficient for the profit of this box, relative
                            // to its size.
  const double timeCoeff;   // Coefficient for the production time of this box,
                            // relative to its ply.

  // Constructor
  Box(std::string name, double lengthCoeff, double widthCoeff,
      double heightCoeff, double profitCoeff, double timeCoeff)
      : name(name), lengthCoeff(lengthCoeff), widthCoeff(widthCoeff),
        heightCoeff(heightCoeff), profitCoeff(profitCoeff),
        timeCoeff(timeCoeff) {}

  // Override toString() method
  std::string toString() const { return name; }

  // resources required: nrBattens = size * 4 * (lengthCoeff + widhtCoeff +
  // heightCoeff)
  double getBattensCoeff() const {
    return 4 * (lengthCoeff + widthCoeff + heightCoeff);
  }

  // resources required: ply = size^2 * 2 * (lengthCoeff * widthCoeff + widthC *
  // heightC + heightC * lengthC)
  double getPlyCoeff() const {
    return 2 * (lengthCoeff * widthCoeff + widthCoeff * heightCoeff +
                heightCoeff * lengthCoeff);
  }
};

// The boxes used in this example.
const std::vector<Box> boxTypeArray = {
    Box("Cube", 1, 1, 1, 20, 1), Box("Oblong", 1, 2, 1, 27.3, 1),
    Box("Flat", 4, 4, 1, 90, 1), Box("Economy", 1, 2, 1, 10, 0.2)};

// The resource constraints used in this example.
const double maxSize = 2.0;
const int maxNumProducedPerBoxType = 6;
const double maxNrBattens = 200.0;
const double maxPly = 210.0;
const double maxTime = 35.0;

int main() {
  try {
    // Create a problem instance with verbose messages printed to Console
    XpressProblem prob;
    prob.callbacks.addMessageCallback(XpressProblem::console);

    // The number of each of the type of boxes that should be produced
    std::vector<Variable> numProduced =
        prob.addVariables(boxTypeArray.size())
            .withType(ColumnType::Integer)
            .withName("numProduced_%d")
            .withUB(maxNumProducedPerBoxType)
            .toArray();

    // The relative size (a multiplier of length/width/height) of each of the
    // box types to produce
    std::vector<Variable> size =
        prob.addVariables(boxTypeArray.size())
            .withType(ColumnType::Continuous) // This is the default value so
                                              // not required to specify
            .withName("size_%d")
            .withUB(maxSize)
            .toArray();

    /* Resource Availabililty Constraints */

    // Construct Expression for ply required for each type of box
    std::vector<Expression> plyPerBoxType(boxTypeArray.size());
    for (std::size_t i = 0; i < boxTypeArray.size(); i++) {
      // plyPerBoxType = numProduced * size^2 * plyCoeff
      plyPerBoxType[i] = mul(numProduced[i], pow(size[i], 2.0)) *
                         boxTypeArray[i].getPlyCoeff();
    }
    // Inspect the constructed Expression
    std::cout << "Total ply: " << sum(plyPerBoxType).toString() << std::endl;
    prob.addConstraint(sum(plyPerBoxType) <= maxPly);

    /* Next, we construct a same type of resource constraint as above, but then
     * for the total battens instead of ply. Except, instead of using sum and
     * mul, we use scalarProduct.
     */
    // First collect the BattensCoefficients into one vector. We do this using
    // the std::transform function along with a lambda function from Box to
    // double (similar to `map` function in Python if you are familiar)
    std::vector<double> battensCoeffs(boxTypeArray.size());
    std::transform(boxTypeArray.begin(), boxTypeArray.end(),
                   battensCoeffs.begin(),
                   [](Box box) { return box.getBattensCoeff(); });
    // battensPerBox = numProduced * size * battensCoeff
    prob.addConstraint(scalarProduct(numProduced, size, battensCoeffs) <=
                       maxNrBattens);

    // Again a similar resource constraint as above, now for total time.
    // We use a different way to use sum (now with a lambda function)
    Expression totalTime =
        sum(boxTypeArray.size(), [&](auto i) {
          // timeNeededForBox = 1 + timeCoeff * 1.5^(ply/10)
          Expression plyNeeded =
              pow(size[i], 2.0) * boxTypeArray[i].getPlyCoeff();
          Expression timeNeeded =
              1 + mul(boxTypeArray[i].timeCoeff, pow(1.5, div(plyNeeded, 10)));
          return numProduced[i] * timeNeeded;
        });
    // Inspect the constructed Expression
    std::cout << "Total time: " << totalTime.toString() << std::endl;
    prob.addConstraint(totalTime <= maxTime);

    /* Construct the objective */
    Expression totalProfit =
        sum(boxTypeArray.size(), [&](auto i) {
          // profit = numProduced * size^1.5 * profitCoeff
          return numProduced[i] * pow(size[i], 1.5) *
                 boxTypeArray[i].profitCoeff;
        });
    // Inspect the constructed Expression
    std::cout << "Total profit: " << totalProfit.toString() << std::endl;

    // To make the objective linear, just maximize the objtransfercol and add
    // non-linear constraint
    Variable objtransfercol =
        prob.addVariable(ColumnType::Continuous, "objTransferCol");
    prob.setObjective(objtransfercol, xpress::ObjSense::Maximize);
    // Add the objective transfer row: objtransfercol = totalProfit
    prob.addConstraint(objtransfercol == totalProfit);

    // Dump the problem to disk so that we can inspect it.
    prob.writeProb("Boxes02.lp");

    // By default we will solve to global optimality, uncomment for an MISLP
    // solve to local optimality prob.setNLPSolver(XPRS_NLPSOLVER_LOCAL);

    // Solve
    prob.optimize();

    // Check the solution status
    if (prob.attributes.getSolStatus() != SolStatus::Optimal &&
        prob.attributes.getSolStatus() != SolStatus::Feasible) {
      std::ostringstream oss;
      oss << prob.attributes
                 .getSolStatus(); // Convert xpress::SolStatus to String
      throw std::runtime_error("Optimization failed with status " + oss.str());
    }

    // Get the solution and print it
    std::vector<double> sol = prob.getSolution();
    std::cout << std::endl << "*** Solution ***" << std::endl;
    std::cout << "Objective: " << prob.attributes.getObjVal() << std::endl;
    // Print out the variables
    for (std::size_t i = 0; i < boxTypeArray.size(); ++i) {
      if (numProduced[i].getValue(sol) > 0.0) {
        std::cout << "Producing " << numProduced[i].getValue(sol) << " "
                  << boxTypeArray[i].toString() << " boxes of size "
                  << size[i].getValue(sol) << "." << std::endl;
      }
    }
    return 0;
  } catch (std::exception &e) {
    std::cout << "Exception: " << e.what() << std::endl;
    return -1;
  }
}
