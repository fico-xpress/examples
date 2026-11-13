/***********************************************************************
   Xpress Optimizer Examples
   =========================

   file MostViolated.java
   ```````````````````
   A Branching rule branching on the most violated Integer/Binary

   Demonstrate the Xpress-MP change branch callbacks.

   (c) 2020-2026 Fair Isaac Corporation
***********************************************************************/

import com.dashoptimization.AbstractChangeBranchObjectListener;
import com.dashoptimization.DefaultMessageListener;
import com.dashoptimization.XPRS;
import com.dashoptimization.XPRSbranchobject;
import com.dashoptimization.XPRSprob;
import static com.dashoptimization.XPRSenumerations.CutStrategy;
import static com.dashoptimization.XPRSenumerations.MIPStopStage;

import java.io.IOException;

/** Example for a branch callback that always branches on the most infeasible
 * variable.
 *
 * <p>
 * Here "most infeasible" means the variable with the biggest fractionality,
 * i.e., the integer variable that is farthest away from an integral value.
 * </p>
 *
 * <p>
 * In order to find the variable with the biggest fractionality, we need the
 * variable types (so that we don't end up branching on a fractional variable)
 * and the values of the variables in the current relaxation. This data is
 * typically queried into arrays <code>cType</code> and <code>x</code> at each
 * node at which we have to find a branching variable.
 * Allocation of these arrays may take time, so it is desirable to cache those
 * arrays so that we don't have to (re)allocate them at each and every node.
 * However, when caching these arrays a number of things have to be considered,
 * most of which are implemented in this example:
 * <dl>
 *   <dt>Multi-Threading</dt>
 *   <dd>
 *     In a multi-threaded solve, callbacks may execute in parallel. This means
 *     that each thread requires its own <code>x</code> array since otherwise
 *     multiple threads might read into the same array from different nodes.
 *     The <code>cType</code> array can be shared between threads since this is
 *     a read-only array.
 *     By default Xpress optimizer serializes the execution of callbacks
 *     using a mutex. That is, by default callbacks will not be executed
 *     simultaneously. In that case the threads can share the same
 *     <code>x</code> array. You can tell the XPress optimizer to not
 *     serialize callbacks by setting the <code>MUTEXCALLBACKS</code> control
 *     to 0. In that case, the threads cannot share the same <code>x</code>
 *     array and each thread has to allocate its own array.
 *   </dd>
 *   <dt>Presolved or original space</dt>
 *   <dd>
 *     We can choose whether we want to make branching decisions in the
 *     original problem space or in the presolved problem space. Branching
 *     in the original space has the advantage that variables have their
 *     original meaning and that all variables are still present. Given that
 *     internally the optimizer works with the presolved problem, a disadvantage
 *     is that relaxation values and branching decisions must be translated
 *     back and forth between the presolved and original space.
 *     Branching in the presolved space allows working directly with the same
 *     data as the optimizer does internally. This is usually faster but fewer
 *     assumptions can be made about the presolved model (variables may have
 *     a different meaning, may have disappeared, etc.).
 *     When setting up the <code>cType</code> an <code>x</code> arrays one
 *     has to be careful that they are setup for the right model.
 *   </dd>
 *   <dt>Restarts</dt>
 *   <dd>
 *     For performance reasons the optimizer may decide to discard the current
 *     search tree and restart optimization from scratch, exploting what it
 *     has learned about the problem so far. In this case, the presolved model
 *     may change (for example, it is common that more variables can be fixed
 *     and thus the size of the presolved model reduces). This means that in
 *     case of branching on the presolved model the arrays <code>cType</code>
 *     and <code>x</code> have to be resetup at a restart.
 *   </dd>
 *   <dt>Constraints by branching</dt>
 *   <dd>
 *     It is possible to not put all constraints into the initial model
 *     description but to enforce certain constraints by means of branching
 *     decisions. An example for this is the handling of symmetries. In that
 *     case the optimizer does not see all constraints in the presolve phase.
 *     However, by default the optimizer assumes that the submitted problem
 *     is complete. If it is not then you may have to disable certain
 *     reductions (in particular dual reductions) since otherwise you may
 *     end up with incorrect results. This special case is not illustrated
 *     in this example since the "branch on most fractional" strategy does not
 *     interfere with any presolve reductions.
 *   </dd>
 * </dl>
 * In this example we illustrate how to implement the callback in the different
 * contexts mentioned above. For each combination of contexts we have a
 * class that creates and caches the <code>cType</code> and <code>x</code>
 * arrays in a way that is safe in the given context.
 * </p>
 *
 * <p>
 * Note that in this example we always go through the whole list of variables
 * and check for each variable whether it is integer or not. This is for
 * illustrative purposes and to keep things simple. In a production code you
 * may want to consider using {@link XPRSprob#getMIPEntities()} or
 * {@link XPRSprob#getDiscreteCols()} to get a list of integer variables.
 * </p>
 */
public class MostViolated {

    /** Branch callback that branches on the most fractional variable.
     *
     * The actual branching happens in function
     *     {@link #changeBranchObjectEvent(XPRSprob, XPRSbranchobject)}
     * This function uses the two abstract functions {@link #getCType(XPRSprob)}
     * and {@link #getX(XPRSprob)} to get the arrays of variable types and
     * current relaxation values.
     * We provide different subclasses of <code>BranchBase</code> that have
     * different implementations of these functions, depending on the context
     * in which the class can be used.
     */
    private abstract static class BranchBase extends AbstractChangeBranchObjectListener {
        /** Tolerance used for checking fractionality. */
        private final double tolerance;
        /** Flag to indicate whether branches are created in terms of
         * the original model.
         * If they are not created in terms of the original model, they are
         * created in terms of the presolved model.
         */
        private final boolean isOriginal;

        /** Create a new branching callback for <code>prob</code>.
         * @param prob The problem on which this callback will be invoked.
         * @param isOriginal  Flag that indicates whether the data returned
         *                    by {@link #getX(XPRSprob)} and
         *                    {@link #getCType(XPRSprob)} is in terms of the
         *                    original problem.
         */
        protected BranchBase(XPRSprob prob, boolean isOriginal) {
            tolerance = prob.controls().getMatrixTol();
            this.isOriginal = isOriginal;
        }

        /** Get the relaxation at the current node.
         * @param prob The problem on which the callback is invoked.
         * @return the relaxation vector for the current node.
         */
        protected abstract double[] getX(XPRSprob prob);

        /** Get the variable types at the current node.
         * @param prob The problem on which the callback is invoked.
         * @return the variable type vector at the current node.
         */
        protected abstract byte[] getCType(XPRSprob prob);

        /** Get the number of MIP threads available for use when solving a problem
         * @param prob The problem to query.
         * @return the number of MIP threads that may be used when solving the problem.
        */
        protected int getNumThreads(XPRSprob prob)
        {
            int threads = prob.controls().getMIPThreads();
            if (threads == -1)
                // MIP threads set to -1 means use the Threads control
                threads = prob.controls().getThreads();
            if (threads == -1)
                // If still at the default then optimizer will use as many
                // threads as there are cores
                threads = prob.attributes().getCoresDetected();
            return threads;
        }

        /** Branch on the most fractional integer.
         * This is the function that gets invoked by the optimizer whenever
         * a branching decision has to be made.
         * @param prob The problem on which the callback is invoked.
         * @param oBranch The branch that the optimizer plans to perform.
         * @return the branch we want the optimizer to perform (may be
         *         <code>oBranch</code> or <code>null</code> in which case the
         *         optimizer will perform <code>oBranch</code>).
         */
        public XPRSbranchobject changeBranchObjectEvent(XPRSprob prob, XPRSbranchobject oBranch)
        {
            // Get the current relaxation
            byte[] cType = getCType(prob);
            double[] x = getX(prob);

            // This is just a safeguard. The two arrays should always have the
            // same length.
            if (x.length != cType.length)
                throw new IllegalArgumentException("Mismatch: " + cType.length + " vs. " + x.length);

            if (oBranch == null)
                return null;

            // Default branch is the original branch.
            XPRSbranchobject newBranch = oBranch;

            // Find the most fractional variable.
            double maxDist = 0.0;
            int branchCol = -1;
            double branchUp = 0;
            double branchDown = 0;
            for (int j = 0; j < x.length; ++j) {
                if (cType[j] == 'I' || cType[j] == 'B') {
                    double upDist = Math.ceil(x[j]) - x[j];
                    double downDist = x[j] - Math.floor(x[j]);
                    double dist = Math.min(upDist, downDist);
                    if (downDist > tolerance && upDist > tolerance) {
                        if (dist > maxDist) {
                            branchCol = j;
                            branchUp = Math.ceil(x[j]);
                            branchDown = Math.floor(x[j]);
                            maxDist = dist;
                        }
                    }
                }
            }

            // If we found a branching candidate then create the new branch
            // object for branching on that candidate.
            if (branchCol != -1) {
                // Create a branch object for the presolved space.
                // Wrap registering of the branch into try-catch so that
                // we properly delete the object in case of any error.
                // If there is no error then the optimizer will take care
                // of cleaning up the branch object.
                XPRSbranchobject bo = prob.createBranchObject(isOriginal);

                try {
                    // Create the two branches.
                    bo.addBranches(2);
                    bo.addBounds(0, 1, new byte[]{'L'}, new int[]{branchCol},
                                 new double[]{branchUp});
                    bo.addBounds(1, 1, new byte[]{'U'}, new int[]{branchCol},
                                 new double[]{branchDown});
                    // Prefer the up branch.
                    bo.setPreferredBranch(0);

                    // Set a small priority so that the branch is actually used.
                    bo.setPriority(100);

                    // Submit the new branch to the solver and clear the
                    // bo reference since the object is now owned (and cleaned
                    // up) by the optimizer.
                    newBranch = bo;
                    bo = null;
                }
                finally {
                    if (bo != null)
                        bo.close();
                }
            }

            return newBranch;
        }
    }

    /** Branch callback implementation that branches in the original space.
     * Instances of this class cache the <code>cType</code> array since the
     * types of variables never change in the original model. They also cache
     * the <code>x</code> array since the size of the original model never
     * changes. The <code>x</code> array is shared between threads, so the
     * the class cannot be used for parallel execution of callbacks.
     */
    private static final class BranchOriginal extends BranchBase {
        private final byte[] cType;
        private final double[] x;
        /** Create a new instance.
         * @param prob The problem that this callback will be used for.
         * @param origCType Column types in the <em>original</em> model.
         */
        public BranchOriginal(XPRSprob prob, byte[] origCType) {
            super(prob, true);
            if (prob.controls().getMutexCallBacks() == 0)
                throw new IllegalArgumentException("this implementation cannot be used for parallel execution");
            cType = origCType;
            x = new double[cType.length];
        }

        /** Get the current relaxation in terms of the <em>original</em> model.
         */
        @Override
        protected double[] getX(XPRSprob prob) {
            prob.getCallbackSolution(null, x, 0, x.length - 1);
            return x;
        }
        /** Get column types in terms of the <em>original</em> model. */
        @Override
        protected byte[] getCType(XPRSprob prob) {
            return cType;
        }
    }

    /** Branch callback implementation that branches in the original space.
     * Instances of this class cache the <code>cType</code> array since the
     * types of variables never change in the original model. They also cache
     * the <code>x</code> array since the size of the original model never
     * changes. Each thread has its own <code>x</code> array, so the class
     * can be used when callbacks are allowed to run in parallel.
     */
    private static final class BranchOriginalMT extends BranchBase {
        private final byte[] cType;
        private final double[][] threadX;
        /** Create a new instance.
         * @param prob The problem that this callback will be used for.
         * @param origCType Column types in the <em>original</em> model.
         */
        public BranchOriginalMT(XPRSprob prob, byte[] origCType) {
            super(prob, true);
            cType = origCType;
            int threads = getNumThreads(prob);
            threadX = new double[threads][];
        }

        /** Get an x vector buffer for the invoking thread. */
        private synchronized double[] getThreadX(XPRSprob prob) {
            // Get the thread id from the optimizer
            int myId = prob.attributes().getMIPThreadID();
            // Create (and cache) an x vector buffer
            if (threadX[myId] == null)
                threadX[myId] = new double[cType.length];
            // Return the buffer for this thread
            return threadX[myId];
        }

        /** Get the current relaxation in terms of the <em>original</em> model.
         */
        @Override
        protected double[] getX(XPRSprob prob) {
            double[] x = getThreadX(prob);
            prob.getCallbackSolution(null, x, 0, x.length - 1);
            return x;
        }
        /** Get column types in terms of the <em>original</em> model. */
        @Override
        protected byte[] getCType(XPRSprob prob) {
            return cType;
        }
    }

    /** Branch callback implementation that branches in the presolved space.
     * Instances of this class assume that there will be no restarts and thus
     * cache the <code>cType</code> array. They also cache
     * the <code>x</code> array since without restarts the size of the presolved
     * model does not change. The <code>x</code> vector is shared between
     * threads, so this class cannot be used if callbacks are invoked in
     * parallel.
     */
    private static final class BranchPresolved extends BranchBase {
        private final byte[] cType;
        private final double[] x;
        /** Create a new callback.
         * @param prob Problem for which the callback will be used. The active
         *             problem referenced by this must be the presolved problem.
         */
        public BranchPresolved(XPRSprob prob) {
            super(prob, false);
            if (prob.controls().getMipRestart() != 0)
                throw new IllegalArgumentException("this class is not compatible with restarts");
            if (prob.controls().getMutexCallBacks() == 0)
                throw new IllegalArgumentException("this implementation cannot be used for parallel execution");
            int cols = prob.attributes().getCols();
            cType = new byte[cols];
            prob.getColType(cType, 0, cols - 1);
            x = new double[cols];
        }

        /** Get the current relaxation in terms of the <em>presolved</em> model.
         */
        @Override
        protected double[] getX(XPRSprob prob) {
            int cols = prob.attributes().getCols();
            prob.getCallbackPresolveSolution(null, x, 0, cols - 1);
            return x;
        }
        /** Get the current relaxation in terms of the <em>presolved</em> model.
         */
        @Override
        protected byte[] getCType(XPRSprob prob) {
            return cType;
        }
    }

    /** Branch callback implementation that branches in the presolved space.
     * Instances of this class assume that there will be no restarts and thus
     * cache the <code>cType</code> array. They also cache
     * the <code>x</code> array since without restarts the size of the presolved
     * model does not change. The class uses a different <code>x</code> array
     * for each thread and is thus safe to use for parallel callback execution.
     */
    private static final class BranchPresolvedMT extends BranchBase {
        private final byte[] cType;
        private final double[][] threadX;
        /** Create a new callback.
         * @param prob Problem for which the callback will be used. The active
         *             problem referenced by this must be the presolved problem.
         */
        public BranchPresolvedMT(XPRSprob prob) {
            super(prob, false);
            if (prob.controls().getMipRestart() != 0)
                throw new IllegalArgumentException("this class is not compatible with restarts");
            int cols = prob.attributes().getCols();
            cType = new byte[cols];
            prob.getColType(cType, 0, cols - 1);
            int threads = getNumThreads(prob);
            threadX = new double[threads][];
        }

        /** Get an x vector buffer for the invoking thread. */
        private synchronized double[] getThreadX(XPRSprob prob) {
            // Get the thread id from the optimizer
            int myId = prob.attributes().getMIPThreadID();
            // Create (and cache) an x vector buffer
            if (threadX[myId] == null)
                threadX[myId] = new double[cType.length];
            // Return the buffer for this thread
            return threadX[myId];
        }

        /** Get the current relaxation in terms of the <em>presolved</em> model.
         */
        @Override
        protected double[] getX(XPRSprob prob) {
            double[] x = getThreadX(prob);
            int cols = prob.attributes().getCols();
            prob.getCallbackPresolveSolution(null, x, 0, cols - 1);
            return x;
        }
        /** Get the current relaxation in terms of the <em>presolved</em> model.
         */
        @Override
        protected byte[] getCType(XPRSprob prob) {
            return cType;
        }
    }

    /** Callback implementation that does not cache any arrays.
     * The class recreates the <code>cType</code> and <code>x</code> arrays
     * at every node and is thus safe to use in any context.
     */
    private static final class BranchPresolvedUncached extends BranchBase {
        /** Create a new callback.
         * @param prob Problem for which the callback will be used.
         */
        public BranchPresolvedUncached(XPRSprob prob) {
            super(prob, false);
        }

        /** Get the current relaxation in terms of the <em>presolved</em> model.
         */
        @Override
        protected double[] getX(XPRSprob prob) {
            int cols = prob.attributes().getCols();
            double[] x = new double[cols];
            prob.getCallbackPresolveSolution(null, x, 0, cols - 1);
            return x;
        }
        /** Get the current relaxation in terms of the <em>presolved</em> model.
         */
        @Override
        protected byte[] getCType(XPRSprob prob) {
            int cols = prob.attributes().getCols();
            byte[] cType = new byte[cols];
            prob.getColType(cType, 0, cols - 1);
            return cType;
        }
    }

    /** Run this example.
     * Command line arguments are:
     * <dl>
     *  <dt>-mt</dt>      <dd>To invoke callbacks in parallel.</dd>
     *  <dt>-original</dt><dd>Make branching decisions in original space.</dd>
     *  <dt>-restart</dt> <dd>Allow restarts.</dd>
     *  <dt>model</dt>    <dd>Path to the model file to solve.</dd>
     * </dl>
     */
    public static void main(String[] args) {
        // Process the command line.
        // By default we don't use parallel callbacks, don't branch on the
        // original model and don't use restarts. Any of this can be changed
        // by the corresponding command line argument.
        boolean mt = false;
        boolean orig = false;
        boolean restart = false;
        String model = "../data/burglar"; // default model
        for (String arg : args) {
            if (arg.equals("-mt"))
                mt = true;
            else if (arg.equals("-original"))
                orig = true;
            else if (arg.equals("-restart"))
                restart = true;
            else
                model = arg;
        }

        try (XPRSprob prob = new XPRSprob(null)) {
            prob.setLogFile("mvb.log");

            prob.readProb(model);

            // Install default output
            prob.addMessageListener(DefaultMessageListener::console);

            // Get the original column types (in case we need them later
            // for branching in the original space)
            int cols = prob.attributes().getCols();
            byte[] origCType = new byte[cols];
            prob.getColType(origCType, 0, cols - 1);

            // If branching decisions are performed in the original space
            // then we must be sure they can be translated to the presolved
            // space, hence we must disable dual reductions in this case.
            // And we also turn off dual reductions when branching in the
            // presolved, space because they reduce this problem to an empty
            // problem.
            prob.controls().setMIPDualReductions(0);

            // Solve the LP relaxation.
            // Note that this will change the active model to the presolved
            // model. So after this call to optimize() all queries for
            // attributes etc. will refer to the presolved model.
            System.out.println("Start solving problem " + model);
            prob.controls().setMIPStopStage(MIPStopStage.INITIALRELAXATION);
            prob.optimize("");
            prob.controls().resetMIPStopStage();

            prob.controls().setMIPLog(3);

            // Change some controls to make the problem "harder" for the
            // optimizer. This is to make sure we actually need to branch
            // to solve the problem.
            prob.controls().setHeurEmphasis(0);
            prob.controls().setCutStrategy(CutStrategy.NONE);

            // Register the branch callback.
            // Depending on what the user chose, we have to register a
            // different implementation of the callback.
            BranchBase callback = null;
            if (orig) {
                // User requested branching in the original space. In that
                // case restarts do not matter since the number and types of
                // the columns in the original space will never change. So
                // we ignore the 'restart' flag and only check whether we
                // want parallel execution of callbacks or not.
                if (mt) {
                    prob.controls().setMutexCallBacks(0);
                    callback = new BranchOriginalMT(prob, origCType);
                }
                else
                    callback = new BranchOriginal(prob, origCType);
            }
            else {
                // We are asked to branch on the presolved model.
                if (restart) {
                    // Restarts are allowed. We use the "safe" implementation
                    // of the callback that re-allocates arrays at every
                    // invocation. Since it does not cache any arrays, that
                    // implementation is safe for parallel callbacks as well.
                    if (mt)
                        prob.controls().setMutexCallBacks(0);
                    callback = new BranchPresolvedUncached(prob);
                }
                else {
                    // No restarts. If there were restarts then the column count
                    // and type could change. Thus we would not be able to cache
                    // that information and would have to re-query it at each
                    // callback invocation. That would slow down things a lot.
                    prob.controls().setMipRestart(0);
                    if (mt) {
                        prob.controls().setMutexCallBacks(0);
                        callback = new BranchPresolvedMT(prob);
                    }
                    else
                        callback = new BranchPresolved(prob);

                }
            }
            prob.addChangeBranchObjectListener(callback);

            prob.optimize();

            System.out.println("Solving complete.\n");

        }
        // Note: We don't catch XPRSexception or XPRSprobException here.
        //       Instead we rely on the JVM's default handling of an exception
        //       being thrown from main() which should terminate the program
        //       ungracefully without an appropriate message.
    }
}
