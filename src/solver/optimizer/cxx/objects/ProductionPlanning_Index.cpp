// (c) 2024-2026 Fair Isaac Corporation

/** Production planning problem. */

#include <iostream>
#include <xpress.hpp>

using namespace xpress;
using namespace xpress::objects;
using xpress::objects::utils::sum;

int const PMAX = 2; // number of products
int const FMAX = 2; // number of factories
int const RMAX = 2; // number of raw material
int const TMAX = 4; // number of time periods

double const CPSTOCK = 2.0; // unit cost to store a product
double const CRSTOCK = 2.0; // unit cost to store a raw material
double const MAXRSTOCK =
    300; // maximum number of raw material that can be stocked in a factory

std::vector<std::vector<double>> REV{
    // REV[p][t] equals unit price for selling product p in period t
    std::vector<double>{400, 380, 405, 350},
    std::vector<double>{410, 397, 412, 397}};

std::vector<std::vector<double>> CMAK{
    // CMAK[p][f] unit cost for producing product p at factory f
    std::vector<double>{150, 153}, std::vector<double>{75, 68}};

std::vector<std::vector<double>> CBUY{
    // CBUY[r][t] unit cost to buy raw material r in period t
    std::vector<double>{100, 98, 97, 100},
    std::vector<double>{200, 195, 198, 200}};

std::vector<double> COPEN{
    // COPEN[f] fixed cost for factory f being open for one period
    50000, 63000};

std::vector<std::vector<double>> REQ{
    // REQ[p][r] raw material requirement (in units) of r to make one unit of p
    std::vector<double>{1.0, 0.5}, std::vector<double>{1.3, 0.4}};

std::vector<std::vector<double>> MAXSELL{
    // MAXSELL[p][t] maximum number of units that can be sold of product p in
    // period t
    std::vector<double>{650, 600, 500, 400},
    std::vector<double>{600, 500, 300, 250}};

std::vector<double> MAXMAKE{// MAXMAKE[f] maximum number of units (over all
                            // products) a factory can produce per period
                            400, 500};

std::vector<std::vector<double>> PSTOCK0{
    // PSTOCK0[p][f] initial stock of product p at factory f
    std::vector<double>{50, 100}, std::vector<double>{50, 50}};

std::vector<std::vector<double>> RSTOCK0{
    // RSTOCK0[r][f] initial stock of raw material r at factor f
    std::vector<double>{100, 150}, std::vector<double>{50, 100}};

/**
 * Convenience function for printing solution values stored in a 3-dimensional
 * Variable array
 *
 * @param sol      solution object, obtained via prob.getSolution()
 * @param array    3-dimensional array of Xpress Variables
 * @param max1     First index varies between 0 and max1
 * @param max2     Second index varies between 0 and max2
 * @param max3     Third index varies between 0 and max3
 * @param dimNames An array with a name for every dimension
 * @param name     The name of the array
 */
void writeSol3D(std::vector<double> const &sol,
                std::vector<std::vector<std::vector<Variable>>> const &array,
                int max1, int max2, int max3,
                std::vector<std::string> const &dimNames,
                std::string const &name) {
  for (int i1 = 0; i1 < max1; ++i1)
    for (int i2 = 0; i2 < max2; ++i2)
      for (int i3 = 0; i3 < max3; ++i3)
        std::cout << dimNames[0] << " " << i1 << "\t" << dimNames[1] << " "
                  << i2 << "\t" << dimNames[2] << " " << i3 << " : " << name
                  << " = " << array[i1][i2][i3].getValue(sol) << std::endl;
}

int main() {
  std::cout << "Formulating the production planning problem" << std::endl;

  XpressProblem prob;
  // make[p][f][t]: Amount of product p to make at factory f in period t
  auto make = prob.addVariables(PMAX, FMAX, TMAX)
                  .withName("make_p%d_f%d_t%d")
                  .toArray();

  // sell[p][f][t]: Amount of product p to sell from factory f in period t
  auto sell = prob.addVariables(PMAX, FMAX, TMAX)
                  .withName("sell_p%d_f%d_t%d")
                  .toArray();

  // pstock[p][f][t]: Stock level of product p at factor f at start of period t
  auto pstock = prob.addVariables(PMAX, FMAX, TMAX)
                    .withName("pstock_p%d_f%d_t%d")
                    .toArray();

  // buy[r][f][t]: Amount of raw material r bought for factory f in period t
  auto buy =
      prob.addVariables(RMAX, FMAX, TMAX).withName("buy_p%d_f%d_t%d").toArray();
  // rstock[r][f][t]: Stock level of raw material r at factory f at start of
  // period t
  auto rstock = prob.addVariables(RMAX, FMAX, TMAX)
                    .withName("rstock_p%d_f%d_t%d")
                    .toArray();

  // openm[f][t]: If factory f is open in period t
  auto openm = prob.addVariables(FMAX, TMAX)
                   .withType(ColumnType::Binary)
                   .withName("openm_f%d_t%d")
                   .toArray();

  // ## Objective:
  // Maximize total profit
  // revenue from selling products
  // + REV[p][t] * sell[p][f][t]
  Expression revenue = sum(PMAX, FMAX, TMAX, [&](auto p, auto f, auto t) {
    return REV[p][t] * sell[p][f][t];
  });

  // cost for making products (must be subtracted from profit)
  // - CMAK[p, f] * make[p][f][t]
  Expression prodCost = sum(PMAX, FMAX, TMAX, [&](auto p, auto f, auto t) {
                          return -CMAK[p][f] * make[p][f][t];
                        });

  // cost for storing products (must be subtracted from profit)
  // - CPSTOCK * pstock[p][f][t]
  Expression prodStorageCost = sum(PMAX, FMAX, TMAX, [&](auto p, auto f, auto t) {
                                 return -CPSTOCK * pstock[p][f][t];
                               });

  // cost for opening a factory in a time period
  // - openm[f][t] * COPEN[f]
  Expression factoryCost = sum(FMAX, TMAX, [&](auto f, auto t) {
                             return -COPEN[f] * openm[f][t];
                           });

  // cost for buying raw material in time period t
  // - buy[r][f][t] * CBUY[r, t]
  Expression rawMaterialBuyCost = sum(PMAX, FMAX, TMAX, [&](auto r, auto f, auto t) {
                                    return -CBUY[r][t] * buy[r][f][t];
                                  });

  // cost for storing raw material (must be subtracted from profit)
  // - rstock[r][f][t] * CRSTOCK
  // an alternative way of setting an objective Expression is to pass
  // the stream directly to the sum function
  Expression rawMaterialStorageCost = sum(FMAX, RMAX, TMAX, [&](auto f, auto r, auto t) {
                                        return -CRSTOCK * rstock[r][f][t];
                                      });

  // sum up the 6 individual contributions to the overall profit
  Expression profit = revenue + prodCost + prodStorageCost + factoryCost +
                      rawMaterialStorageCost + rawMaterialBuyCost;

  // set maximization of profit as objective function
  prob.setObjective(profit, ObjSense::Maximize);

  // constraints
  // Product stock balance
  prob.addConstraints(PMAX, FMAX, TMAX, [&](auto p, auto f, auto t) {
    // for each time period except the last time period, surplus is available as
    // stock for the next time period
    if (t < TMAX - 1) {
      return (pstock[p][f][t] + make[p][f][t] ==
              sell[p][f][t] + pstock[p][f][t + 1])
          .setName(xpress::format("prod_stock_balance_p%d_f%d_t%d", p, f, t));
    } else {
      return (pstock[p][f][t] + make[p][f][t] >= sell[p][f][t])
          .setName(xpress::format("prod_stock_balance_p%d_f%d_t%d", p, f, t));
    }
  });

  // Raw material stock balance
  prob.addConstraints(PMAX, FMAX, TMAX, [&](auto r, auto f, auto t) {
    if (t < TMAX - 1) {
      return (rstock[r][f][t] + buy[r][f][t] ==
              rstock[r][f][t + 1] +
                  sum(PMAX, [&](auto p) { return REQ[p][r] * make[p][f][t]; }))
          .setName(xpress::format("raw_material_stock_balance_r%d_f%d_t%d", r,
                                  f, t));
    } else {
      return (rstock[r][f][t] + buy[r][f][t] >=
              sum(PMAX, [&](auto p) { return REQ[p][r] * make[p][f][t]; }))
          .setName(xpress::format("raw_material_stock_balance_r%d_f%d_t%d", r,
                                  f, t));
    }
  });

  // Limit on the amount of product p to be sold
  // exemplifies how to loop through multiple ranges
  prob.addConstraints(PMAX, TMAX, [&](auto p, auto t) {
    return (sum(FMAX, [&](auto f) { return sell[p][f][t]; }) <= MAXSELL[p][t])
        .setName(xpress::format("maxsell_p%d_t%d", p, t));
  });

  // Capacity limit at factory f
  // exemplifies how to loop through multiple ranges
  prob.addConstraints(FMAX, TMAX, [&](auto f, auto t) {
    return (sum(PMAX, [&](auto p) { return make[p][f][t]; }) <=
            MAXMAKE[f] * openm[f][t])
        .setName(xpress::format("capacity_f%d_t%d", f, t));
  });

  // Raw material stock limit
  // exemplifies how to loop through multiple ranges
  prob.addConstraints(FMAX, TMAX, [&](auto f, auto t) {
    return (sum(RMAX, [&](auto r) { return rstock[r][f][t]; }) <= MAXRSTOCK)
        .setName(xpress::format("raw_material_stock_limit_f%d_t%d", f, t));
  });

  // Initial product storage
  prob.addConstraints(PMAX, FMAX, [&](auto p, auto f) {
    // pstock is indexed (p, f, t), PSTOCK0 is indexed (p, f)
    return (pstock[p][f][0] == PSTOCK0[p][f])
        .setName(xpress::format("initial_product_stock_p%d_f%d", p, f));
  });

  // Initial raw material storage
  // classic for loop
  prob.addConstraints(PMAX, FMAX, [&](auto r, auto f) {
    return (rstock[r][f][0] == RSTOCK0[r][f])
        .setName(xpress::format("initial_raw_material_stock_r%d_f%d", r, f));
  });

  // write the problem in LP format for manual inspection
  std::cout << "Writing the problem to 'ProductionPlanning.lp'" << std::endl;
  prob.writeProb("ProductionPlanning.lp");

  // Solve the problem
  std::cout << "Solving the problem" << std::endl;
  prob.optimize();

  std::cout << "Problem finished with SolStatus "
            << prob.attributes.getSolStatus() << std::endl;

  if (prob.attributes.getSolStatus() != SolStatus::Optimal) {
    throw std::runtime_error("Problem not solved to optimality");
  }

  std::cout << "Solution has objective value (profit) of "
            << prob.attributes.getObjVal() << std::endl;

  std::cout << std::endl;
  std::cout << "*** Solution ***" << std::endl;
  auto sol = prob.getSolution();

  // Is factory f open at time period t?
  for (int f = 0; f < FMAX; f++) {
    for (int t = 0; t < TMAX; t++) {
      std::cout << "Factory " << f << "\tTime Period " << t
                << " : Open = " << openm[f][t].getValue(sol) << std::endl;
    }
  }
  std::cout << std::endl;

  // Production plan for producing
  writeSol3D(sol, make, PMAX, FMAX, TMAX,
             std::vector<std::string>{"Product", "Factory", "Time Period"},
             "Make");
  std::cout << std::endl;

  // Production plan for selling products
  writeSol3D(sol, sell, PMAX, FMAX, TMAX,
             std::vector<std::string>{"Product", "Factory", "Time Period"},
             "Sell");
  std::cout << std::endl;

  // Production plan for keeping products in stock
  writeSol3D(sol, pstock, PMAX, FMAX, TMAX,
             std::vector<std::string>{"Product", "Factory", "Time Period"},
             "Pstock");
  std::cout << std::endl;

  // Production plan for keeping raw material in stock
  writeSol3D(sol, rstock, RMAX, FMAX, TMAX,
             std::vector<std::string>{"Material", "Factory", "Time Period"},
             "Rstock");
  std::cout << std::endl;

  // Buying plan for raw material
  writeSol3D(sol, buy, RMAX, FMAX, TMAX,
             std::vector<std::string>{"Material", "Factory", "Time Period"},
             "Buy");
  std::cout << std::endl;


  return 0;
}
