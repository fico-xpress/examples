/***********************************************************************
   Xpress Optimizer Examples
   =========================

   file Knapsack.cs
   ````````````````
   Apply a primal heuristic to a knapsack problem.
   The program demonstrates the use of the global log callback.

   We take the knapsack problem stored in burglar.mps and instigate a
   global search. At each node, so long as the current solution is
   both LP optimal and integer infeasible, we truncate the solution
   values to create a feasible integer solution. We then update the
   cutoff, if the new objective value has improved it, and continue
   the search.

   (c) 2021-2026 Fair Isaac Corporation
***********************************************************************/

using System;
using Optimizer;

namespace XPRSExamples
{
	class Knapsack
	{
		public static void Main(string[] args)
		{
			Knapsack example = new Knapsack();
			example.Run();
		}

		private void Run()
		{
			string sLogFile = "knapsack.log";
			string sProblem = "burglar";
			
			try
			{
				// initialise optimizer
				XPRS.Init("");

				prob = new XPRSprob();

				// define log file
				prob.SetLogFile(sLogFile);

				// Tell Optimizer to call OptimizerMsg whenever a message is output
				prob.AddMessageCallback(this.OptimizerMsg);

				//Turn off presolve and disallow cuts - to slow solution and allow the
				// effect of the heuristic to be seen
				prob.Presolve = 0;
				prob.CutStrategy = 0;

				// Read the problem file
        prob.MPSFormat = -1;
				prob.ReadProb(sProblem,"");

				// Prepare to apply the heuristic

				// Get the number of columns
				gnCol = prob.Cols;

				// Allocate memory for the coefficient arrays
				gpObjCoef = new double[gnCol];

				// Get the objective function coefficients
				prob.GetObj(gpObjCoef, 0,gnCol-1);

				// Get integer feasibility tolerance
				gdIntTol = prob.MIPTol;

				// Tell Optimizer to print global information to the log file at each node
				prob.MIPLog = 3;

				// Tell Optimizer to call truncsol at each node and apply the heuristic
				prob.AddMiplogCallback(this.TruncSol);

				// Perform the global search - in the course of which the heuristic will
				// be applied
				Console.WriteLine("Applying a primal heuristic to problem {0}",sProblem);
				prob.Optimize();
				
			}
			catch (XPRSException e)
			{
				Console.WriteLine(e.ToString());
        throw e;
			}
			finally
			{
				prob.Destroy();
				XPRS.Free();
			}
		}

		public void OptimizerMsg (XPRSprob prob, object data, string message, int len, int msglvl)
		{
			Console.WriteLine ("{0}" + message, data);
		}

		public int TruncSol(XPRSprob prob, object data)
		{

			int nNodeNum;               // Number of nodes solved
			double dObjVal;            // Objective value
			double dCutoff;            // Cutoff value
			LPStatus nLPStatus;        // LP solution status
			int nIntInf;               // Number of integer infeasibilities
			int i;                     // Loop counter
			double dHeurObj;           // Objective value after the solution values have been truncated
			string [] sLPStatus =
			{
					"Optimal","Infeasible","Objective worse than cutoff",
					"Unfinished","Unbounded","Cutoff in dual"
			};


			// Get the current node number
			nNodeNum = prob.CurrentNode;
			
			// Get objective value at the current node
			dObjVal = prob.LPObjVal;

			// Get the current cutoff value
			dCutoff = prob.MIPAbsCutoff;

			// Get LP solution status and the number of integer infeasibilities
			nLPStatus = prob.LPStatus;
			nIntInf = prob.MIPInfeas;

			// Apply heuristic if nodal solution is LP optimal and integer infeasible
			if (nLPStatus == LPStatus.Optimal && nIntInf>0)
			{

				// Get LP solution
				double[] x = prob.GetCallbackSolution();
			
				// Truncate each solution value - making allowance for the integer
				//	tolerance - and calculate the new "heuristic" objective value
				for (dHeurObj=0, i=0; i<gnCol; i++)
				{
					dHeurObj += gpObjCoef[i] * (int) (x[i] + gdIntTol);
				}
				Console.WriteLine("   Node {0}: Objective Value: ORIGINAL {1};  HEURISTIC {2}\n\n",
				                  nNodeNum,dObjVal,dHeurObj);

				// If the "heuristic" objective value is better, update the cutoff -
				//	we assume that all the objective coefficents are integers
				if( dHeurObj > dCutoff)
				{
					prob.MIPAbsCutoff = dHeurObj + 0.9999;
					Console.WriteLine("              ** Cutoff updated to {0} **\n\n",dHeurObj+1.0);
				}
			}

				/* If the nodal solution is not LP optimal do not apply the heuristic */
			else if (nLPStatus != LPStatus.Optimal)
			{
				Console.WriteLine("   Node {0}: LP solution not optimal, not applying heuristic\n",nNodeNum);
				Console.WriteLine("           ({0})\n\n",sLPStatus[(int)nLPStatus-1]);
			}

				/* If the nodal solution is integer feasible print the objective value */
			else if (nIntInf == 0)
			{
				Console.WriteLine("   Node {0}: Integer solution found: Objective Value {1}\n\n",
				                  nNodeNum,dObjVal);
			}

			return 0;
		}

		private XPRSprob prob;

		// converted from the C globals
		private double[] gpObjCoef;     // Objective function coefficients
		private double gdIntTol;        // Integer feasibility tolerance
		private int gnCol;              // Number of columns
	}
}
