import com.dashoptimization.DefaultMessageListener;
import com.dashoptimization.XPRS;
import com.dashoptimization.XPRSprob;
import com.dashoptimization.XPRSprobException;

import java.util.Arrays;

import static com.dashoptimization.XPRSprob.IISData;
import static com.dashoptimization.XPRSprob.IISStatusInfo;

/** Example class to illustrate finding and iterating IISs.
 */
public class IISExample {
    /** Print information about IIS.
     * @param problem The problem instance storing the ISSs.
     * @param num     Number of the IIS to print.
     */
    public void printIIS(XPRSprob problem, int num) {
        System.out.println("IIS " + num + ":");

        // getIISData(int) returns information about a particular IIS.
        // IISData has more fields than we print here. Please refer to the
        // reference documentation for more details.
        IISData data = problem.getIISData(num);
        System.out.println("  infeasible rows: " + Arrays.toString(data.rowind));
        System.out.println("  infeasible cols: " + Arrays.toString(data.colind));

        // IISStatus() returns information about all IISs found so far.
        // IISStatusInfo has more fields than we print here, please refer to
        // the reference documentation for more details.
        IISStatusInfo info = problem.IISStatus();
        System.out.println("  infeasibilities: " + Arrays.toString(info.suminfeas));
    }

    /** Iterate over IIS in a model and print each.
     * @param iMode IIS iteration mode:
     *              0 to compute all IISs in one shot and then iterate,
     *              1 to compute IISs one by one.
     */
    public void run (int iMode) {
        try (XPRSprob problem = new XPRSprob(null)) {
            problem.setIntControl (XPRS.LPLOG, 1);
            problem.addMessageListener(DefaultMessageListener::console);

            problem.readProb ("../data/iisexample","");

            problem.setIntControl(XPRS.PRESOLVE, -1);
            problem.optimize();

            switch(iMode) {
            case(0) :
                /* Get all iis at once and then iterate through the results */
                problem.IISAll();
                for(int i = 1; i < problem.getIntAttrib(XPRS.NUMIIS); i++) {
                    printIIS(problem, i);
                    problem.writeIIS(i, "iis_result" + i, 0);
                }
                break;
            case(1) :
                /* Get the iis one at a time */
                if (problem.firstIIS(1) == 0) {
                    int i = 0;
                    do {
                        i++;
                        printIIS(problem, i);
                        problem.writeIIS(i, "iis_result" + i, 0);
                    } while (problem.nextIIS() == 0);
                }
                break;
            }
        }
    }

    public static void main(String [] args)
    {
        IISExample c = new IISExample ();
        c.run (0);
        c.run (1);
    }
}

