// (c) 2023-2026 Fair Isaac Corporation

import static com.dashoptimization.objects.Utils.sum;

import com.dashoptimization.ColumnType;
import com.dashoptimization.DefaultMessageListener;
import com.dashoptimization.XPRSenumerations;
import com.dashoptimization.objects.Variable;
import com.dashoptimization.objects.XpressProblem;

/** Contract allocation example. */
public class ContractAllocation {

    static final int NDISTRICT = 6; /* Number of districts */
    static final int NCONTRACT = 10; /* Number of contracts */

    private static final double[] output = new double[] { /* Max. output per district */
            50, 40, 10, 20, 70, 50 };

    private static final double[] cost = new double[] { /* Cost per district */
            50, 20, 25, 30, 45, 40 };

    private static final double[] volume = new double[] { /* Volume of contracts */
            20, 10, 30, 15, 20, 30, 10, 50, 10, 20 };

    public static void main(String[] args) {

        try (XpressProblem prob = new XpressProblem()) {
            // Output all messages.
            prob.callbacks.addMessageCallback(DefaultMessageListener::console);

            /**** VARIABLES ****/
            /* Variables indicating whether a project is chosen */
            Variable[][] x = prob.addVariables(NDISTRICT, NCONTRACT).withType(ColumnType.Binary)
                    .withName((d, c) -> "x_d" + (d + 1) + "_c" + (c + 1)).toArray();
            /* Quantities allocated to contractors */
            Variable[][] y = prob.addVariables(NDISTRICT, NCONTRACT).withType(ColumnType.SemiContinuous)
                    .withUB((d, c) -> output[d]).withName((d, c) -> "q_d" + (d + 1) + "_c" + (c + 1)).withLimit(5) // Set
                                                                                                                   // limit
                                                                                                                   // for
                                                                                                                   // the
                                                                                                                   // semi-continous
                                                                                                                   // variable
                    .toArray();

            /**** CONSTRAINTS ****/
            // Cover the required volume
            // for all c in [0,NCONTRACT[
            // sum(d in [0,NDISTRICT[) y[d][c] >= volume[c]
            prob.addConstraints(NCONTRACT, c -> sum(NDISTRICT, d -> y[d][c]).geq(volume[c]).setName("Size_" + c));

            // "Min": at least 2 districts / contract
            // for all c in [0,NCONTRACT[
            // sum(d in [0,NDISTRICT[) x[d][c] >= 2
            prob.addConstraints(NCONTRACT, c -> sum(NDISTRICT, d -> x[d][c]).geq(2.0).setName("Min_" + c));

            // Do not exceed max. output
            // for all d in [0,NDISTRICT[
            // sum(c in [0,NCONTRACT[) y[d][c] <= output[d]
            prob.addConstraints(NDISTRICT, d -> sum(y[d]).leq(output[d]).setName("Output_" + d));

            // If a contract is allocated to a district, then at least 1 unit is allocated
            // to it
            // for all d in [0,NDISTRICT[
            // for all c in [0,NCONTRACT[
            // x[d][c] <= y[d][c]
            prob.addConstraints(NDISTRICT, NCONTRACT, (d, c) -> x[d][c].leq(y[d][c]).setName("XY_" + d + c));

            /**** OBJECTIVE ****/
            prob.setObjective(sum(NCONTRACT, c -> sum(NDISTRICT, d -> y[d][c].mul(cost[d]))),
                    XPRSenumerations.ObjSense.MINIMIZE);

            // Output the matrix in LP format
            prob.writeProb("Contract.lp", "l");
            // Solve
            prob.optimize();
            if (prob.attributes().getSolStatus() != XPRSenumerations.SolStatus.OPTIMAL
                    && prob.attributes().getSolStatus() != XPRSenumerations.SolStatus.FEASIBLE)
                throw new RuntimeException("optimization failed with status " + prob.attributes().getSolStatus());
            // Print out the solution
            double[] sol = prob.getSolution();
            System.out.println("Objective: " + prob.attributes().getMIPObjVal());
            for (Variable[] var : y) {
                for (Variable v : var) {
                    if (v.getValue(sol) > 0.5)
                        System.out.print(v.getName() + ":" + v.getValue(sol) + ", ");
                }
                System.out.println();
            }
        }
    }
}
