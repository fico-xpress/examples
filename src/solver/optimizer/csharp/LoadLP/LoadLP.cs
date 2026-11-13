/***********************************************************************
   Xpress Optimizer Examples
   =========================

   file LoadLP.cs
   ``````````````
   Load an LP problem directly into Optimizer and modify it by adding an
   extra constraint.

   The problem
     Maximise
            2x + y
     subject to
       c1:  x + 4y <= 24
            c2:       y <=  5
            c3: 3x +  y <= 20
            c4:  x +  y <=  9
    and
            0 <= x, y <= +infinity
    and the extra constraint
            c5: 6x + y <= 20
   are first stored in the user's data structures. The LP is then loaded
   into Optimizer, using loadprob, and solved using the primal simplex
   algorithm. Next, the extra constraint is added to the problem matrix,
   using addrows, and the revised problem solved using the dual algorithm.
   In each case, the problem matrix is output to a file, the objective
   function value displayed on screen, and the problem statistics are
   are stored in a log file.

   (c) 2021-2026 Fair Isaac Corporation
***********************************************************************/

using System;
using Optimizer;

namespace XPRSExamples
{
	class LoadLP
	{
		public static void Main(string[] args)
		{
			LoadLP example = new LoadLP();
			example.Run();
		}

		private void Run()
		{

			// Store the problem
			// Row data
			int nRow = 4;
			char[] sRowType = {'L', 'L', 'L', 'L'};
			double[] dRHS = { 24.0, 5.0, 20.0, 9.0 };
			string[] sRowName = { "c1", "c2", "c3", "c4"  };

			// Column data
			int nCol = 2;
			double[] dObj = { 2.0, 1.0 };
			double[] dLowerBd = { 0,0};
			double[] dUpperBd = { XPRS.PLUSINFINITY, XPRS.PLUSINFINITY };

			string[] sColName = { "x", "y" };

			// Matrix data
			int[] nColStart = { 0,3,7 };
			int[] nRowInd = {0,2,3,0,1,2,3 };
			double[] dMatElem = { 1,3,1,4,1,1,1 };

			// Store extra constraint
			int nNewRow = 1;
			int nNewElem = 2;
			char[] sNewRowType = {'L'};
			string[] sNewRowName = {"c5"};

			double[] dNewRHS = {20};
			double[] dNewRowElem = { 6,1 };
			int[] nNewRowStart = { 0,2 };
			int[] nNewColInd = { 0,1 };

			double dObjValue;


			string sLogFile = "loadlp.log";
			string sProblem1 = "lp";
			string sProblem2 = "revised";

			try
			{
				XPRS.Init("");

				prob = new XPRSprob();
				prob.SetLogFile(sLogFile);

				// Tell Optimizer to call OptimizerMsg whenever a message is output
				prob.AddMessageCallback(this.OptimizerMsg);

				prob.LoadLP(sProblem1, nCol, nRow, sRowType, dRHS, null, dObj, nColStart, null, nRowInd, dMatElem, dLowerBd, dUpperBd);
                prob.ChgObjSense(ObjSense.Maximize);

				// Add row names
				prob.AddNames(1, sRowName, 0, nRow -1 );

				// Add column names
				prob.AddNames(2, sColName, 0, nCol -1 );

				// Output the matrix
				prob.WriteProb(sProblem1, "");

				// Solve the LP problem
				prob.Optimize();

				// Get and display the value of the objective function
				dObjValue = prob.LPObjVal;
				Console.WriteLine("The optimal objective value is {0}", dObjValue);

				// ** Add the extra constraint and solve again **

				// Add new row
				prob.AddRows(nNewRow, nNewElem, sNewRowType, dNewRHS, null, nNewRowStart, nNewColInd, dNewRowElem);

				// Add new row name
				prob.AddNames(1, sNewRowName, nRow, nRow);

				// Output the revised matrix
				prob.WriteProb(sProblem2, "");
				Console.WriteLine("Matrix file {0}.mat has been created", sProblem2);

				// Solve with dual - since the revised problem inherits dual feasibility
				// from the original
				prob.Optimize("d");

				// Get and display the value of the objective function
				dObjValue = prob.LPObjVal;
				Console.WriteLine("The revised optimal objective value is {0}",dObjValue);

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
			switch(msglvl)
			{
				case 3:
				case 4:
					Console.WriteLine ("{0}" + message, data);
					break;
			}
		}

		private XPRSprob prob;
	}
}
