// (c) 2023-2026 Fair Isaac Corporation

import static com.dashoptimization.objects.Utils.sum;

import java.util.Arrays;

import com.dashoptimization.ColumnType;
import com.dashoptimization.XPRSconstants;
import com.dashoptimization.XPRSenumerations;
import com.dashoptimization.objects.Variable;
import com.dashoptimization.objects.XpressProblem;

/**
 * Load balancing of train wagons. Second version, using heuristic solution as
 * start solution for MIP.
 */
public class Wagon {

    /* Box weights */
    private static final int[] WEIGHT = { 34, 6, 8, 17, 16, 5, 13, 21, 25, 31, 14, 13, 33, 9, 25, 25 };
    private static final int NBOXES = WEIGHT.length; /* Number of boxes */
    private static final int NWAGONS = 3; /* Number of wagons */
    private static final int WMAX = 100; /* Weight limit of the wagons */
    private static final int[] HeurSol = new int[NBOXES]; /* Heuristic solution: for each box */

    /* Variables */
    private static Variable[][] load;
    private static Variable maxweight;

    private static void model(XpressProblem p) {
        // Create load[box,wagon] (binary)
        load = p.addVariables(NBOXES, NWAGONS).withType(ColumnType.Binary)
                .withName((b, w) -> String.format("load_%d_%d", b + 1, w + 1)).toArray();

        // Create maxweight (continuous with lb=ceil((sum(b in BOXES) WEIGHT(b))/NBOXES)
        maxweight = p.addVariable(Math.ceil(Arrays.stream(WEIGHT).sum() / (double) NBOXES), XPRSconstants.PLUSINFINITY,
                ColumnType.Continuous, "maxweight");

        // Every box into one wagon: forall(b in BOXES) sum(w in WAGONS) load(b,w) = 1
        p.addConstraints(NBOXES, b -> sum(load[b]).eq(1.0));

        // Limit the weight loaded into every wagon: forall(w in WAGONS) sum(b in BOXES)
        // WEIGHT(b)*load(b,w) <= maxweight
        p.addConstraints(NWAGONS,
                w -> sum(NBOXES, b -> load[b][w].mul(WEIGHT[b])).leq(maxweight).setName(String.format("wmilit_%d", w)));

        // Minimize maxweight
        p.setObjective(maxweight, XPRSenumerations.ObjSense.MINIMIZE);
        p.writeProb("Wagon.lp", "l");
    }

    /**
     * LPT (Longest processing time) heuristic: One at a time, place the heaviest
     * unassigned box onto the wagon with the least load
     *
     * @return A dense vector with the heuristic solution.
     */
    private static double heuristic() {
        Integer[] ORDERW = new Integer[NBOXES]; // Box indices sorted in decreasing weight order
        int[] CurNum = new int[NWAGONS]; // For each wagon w, this is the number of boxes currently loaded
        int[] CurWeight = new int[NWAGONS]; // For each wagon w, this is the current weight, i.e. the sum of weights of
                                            // loaded boxes
        int[][] Load = new int[NWAGONS][NBOXES]; // Load[w][i] (for i=0..CurNum[w]-1) contains the box index of the i-th
                                                 // box loaded on wagon w

        // Copy the box indices into array ORDERW and sort them in decreasing
        // order of box weights (the sorted indices are returned in array ORDERW)
        for (int b = 0; b < NBOXES; b++)
            ORDERW[b] = b;
        Arrays.sort(ORDERW, (i1, i2) -> ((Integer) WEIGHT[i2]).compareTo(WEIGHT[i1]));

        // Distribute the loads to the wagons using the LPT heuristic
        for (int b = 0; b < NBOXES; b++) {
            int v = 0; /* Find wagon v with the smallest load */
            for (int w = 0; w < NWAGONS; w++)
                if (CurWeight[w] <= CurWeight[v])
                    v = w;
            Load[v][CurNum[v]] = ORDERW[b]; /* Add current box to wagon v */
            CurNum[v]++; /* Increase the counter of boxes on v */
            CurWeight[v] += WEIGHT[ORDERW[b]]; /* Update current weight of the wagon */
        }

        // Calculate the solution value
        double heurobj = 0; /* heuristic solution objective value (max wagon weight) */
        for (int w = 0; w < NWAGONS; w++)
            if (CurWeight[w] > heurobj)
                heurobj = CurWeight[w];

        // Solution printing
        System.out.printf("Heuristic solution:%n Max weight: %.0f\n", heurobj);

        for (int w = 0; w < NWAGONS; w++) {
            System.out.printf(" %d:", w + 1);
            for (int i = 0; i < CurNum[w]; i++)
                System.out.print(" " + (Load[w][i] + 1));
            System.out.printf(" (total weight: %d)\n", CurWeight[w]);
        }

        // Save the heuristic solution into the HeurSol array
        for (int w = 0; w < NWAGONS; w++)
            for (int i = 0; i < CurNum[w]; i++)
                HeurSol[Load[w][i]] = w;
        return heurobj;
    }

    private static void optimization(XpressProblem p) {
        // Get the solution from the heuristic solution we have found
        // Send the solution to the optimizer
        Variable[] heurmipvar = new Variable[NBOXES];
        double[] heurmipsol = new double[NBOXES];
        for (int b = 0; b < NBOXES; b++) {
            heurmipvar[b] = load[b][HeurSol[b]];
            heurmipsol[b] = 1.0;
        }
        p.addMipSol(heurmipsol, heurmipvar, "heuristic");
        p.optimize();
        if (p.attributes().getSolStatus() != XPRSenumerations.SolStatus.OPTIMAL
                && p.attributes().getSolStatus() != XPRSenumerations.SolStatus.FEASIBLE)
            throw new RuntimeException("failed to optimize with status " + p.attributes().getSolStatus());

        System.out.println(String.format(
                "Problem status:\n\tSolve status: %s\n\tLP status: %s\n\tMIP status: %s\n\tSol status: %s",
                p.attributes().getSolveStatus(), p.attributes().getLPStatus(), p.attributes().getMIPStatus(),
                p.attributes().getSolStatus()));

        // An integer solution has been found
        if (p.attributes().getMIPStatus() == XPRSenumerations.MIPStatus.OPTIMAL
                || p.attributes().getMIPStatus() == XPRSenumerations.MIPStatus.SOLUTION) {
            double[] sol = p.getSolution();
            System.out.printf("Optimal solution:\n Max weight: %.0f%n", p.attributes().getObjVal());
            for (int w = 0; w < NWAGONS; w++) {
                double tot_weight = 0.;
                System.out.printf(" %d:", w + 1);
                for (int b = 0; b < NBOXES; b++)
                    if (load[b][w].getValue(sol) * WEIGHT[b] > .5) {
                        System.out.printf(" %.0f", load[b][w].getValue(sol) * WEIGHT[b]);
                        tot_weight += load[b][w].getValue(sol) * WEIGHT[b];
                    }
                System.out.printf(" (total weight: %.0f)%n", tot_weight);
            }
        }
    }

    public static void main(String[] args) {
        if (heuristic() <= WMAX) {
            System.out.println("Heuristic solution fits capacity limits");
        } else {
            try (XpressProblem prob = new XpressProblem()) {
                /* Suppress outputs */
                model(prob);
                optimization(prob);
            }
        }
    }

}
