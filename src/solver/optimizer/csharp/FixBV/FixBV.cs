/***********************************************************************
   Xpress Optimizer Examples
   =========================

   file FixBV.cs
   `````````````
   Apply a binary fixing heuristic to an unpresolved MIP problem.

   We take a production plan model and solve its LP relaxation.
   Next we fix those binary variables that are very
   near zero to 0.0, and those that are almost one to 1.0, by changing
   their respective upper and lower bounds. Finally, we solve the
   modified problem as a MIP.
   This heuristic will speed up solution - though may fail to optimise
   the problem.
   The results are displayed on screen and the problem statistics
   stored in a log file.

   (c) 2021-2026 Fair Isaac Corporation
***********************************************************************/

using System;
using System.IO;
using Optimizer;


namespace XPRSExamples
{
	class FixBV
	{
		public static void Main(string[] args)
		{
			FixBV example = new FixBV();
			example.Run();
		}

		private const double TOL=0.0005;            // Tolerance on binary variables

		private void Run()
		{
			try
			{
				string sProblem="coco";      // Problem name
				string sLogFile="fixbv.log"; // Log file name

				int nCol;                    // Number of columns

				// Global problem information
				int nGlEnt;                  // Number of global entities: binary, integer, semi-continuous and partial integer variables
				int nSet;                    // Number of S1 and S2 sets
				int[] pGlInd;                 // Column indices of the global entities
				char[] pGlType;               // Global entity types

				// Bound changes
				int[] pBndInd;                // Column indices of the bounds to be changed
				char[] pBndType;              // New bound type
				double[] pBndVal;             // New bound values
				int nBnd;                    // Bound counter
				int i;                       // Loop counter
				int j;                       // Holder for the bound indices

				// Solution information
				double[] x;                  // LP solution values
				int nGlStatus;               // Global status
				int nNodes;                  // Number of nodes solved so far in the global search
				double dObjVal;              // Objective value of the best integer solution

				// Initialise Optimizer
				XPRS.Init("");

				prob = new XPRSprob();

				// Delete and define log file
				// Delete the file if it exists.
				if (File.Exists(sLogFile))
				{
					File.Delete(sLogFile);
				}
				prob.SetLogFile(sLogFile);

				// Tell Optimizer to call optimizermsg whenever a message is output
				prob.AddMessageCallback(this.OptimizerMsg);

				// Turn off presolve and permit no cuts - to slow down solution and allow
				// the effect of the heuristic to be be seen
				prob.Presolve = 0;
				prob.CutStrategy = 0;

				// Read the problem file
				prob.MPSFormat = -1;
				prob.ReadProb(sProblem,"");

				// Solve the LP relaxation

				// Get the number of columns
				nCol = prob.Cols;

				// Solve the LP
				prob.MIPStopStage = MIPStopStage.InitialRelaxation;
				prob.Optimize();
				prob.SetDefaultControl(XPRScontrol.MIPStopStage);
		
				// Get LP solution values
				x = prob.GetLpSolX();

				// Fix the binary variables that are at their bounds

				// Allocate memory for global entity arrays
				pGlInd = new int[nCol];
				pGlType = new char[nCol];

				// Get global entity information

				prob.GetMIPEntities(out nGlEnt, out nSet, pGlType, pGlInd, null, null, (int[])null, null, null);

				// Allocate memory for bound arrays
				pBndInd = new int[nGlEnt];
				pBndVal = new double[nGlEnt];
				pBndType = new char[nGlEnt];

				// Initialise bound counter
				nBnd=0;

				// Go through the global entities
				for(i=0; i<nGlEnt; i++)
				{
					// Test whether each is a binary variable
					if (pGlType[i] == 'B')
					{
						// Hold the index of the BV
						j=pGlInd[i];

						// If the value of the BV is within TOL of zero, store its index,
						//   set its upper bound to 0, and increment the bound counter
						if (x[j]<=TOL)
						{
							pBndInd[nBnd]=j;
							pBndType[nBnd]='U';
							pBndVal[nBnd]=0.0;
							nBnd++;

							// If the value of the BV is within TOL of one,store its index,
							//   set its lower bound to 1, and increment the bound counter
						}
						else if ((1-x[j])<=TOL)
						{
							pBndInd[nBnd]=j;
							pBndType[nBnd]='L';
							pBndVal[nBnd]=1.0;
							nBnd++;
						}
					}
				}
				// Instruct Optimizer to change the bounds of the appropriate BVs,
				//   and tell the user how many have been fixed
				prob.ChgBounds(nBnd,pBndInd,pBndType,pBndVal);

				Console.WriteLine("Solving problem {0} with a binary fixing heuristic:\n\n",sProblem);
				Console.WriteLine("   After the LP optimisation {0} binary variables were fixed\n\n",nBnd);

				// Solve the modified problem as a MIP

				// Search for an integer solution
				prob.Optimize();

				// Get the number of nodes solved in the global search
				nNodes = prob.Nodes;

				// Get the objective value of the best integer solution
				dObjVal = prob.MIPObjVal;

				// Check the global status and display the results of the global search
				nGlStatus = (int)prob.MIPStatus;

				switch (nGlStatus)
				{
					case 0:
						Console.WriteLine("   Problem has not been loaded");
						break;
					case 1:
						Console.WriteLine("   Search has not begun - LP has not been optimised");
						break;
					case 2:
						Console.WriteLine("   Search has not begun - LP has been optimised");
						break;
					case 3:
						Console.WriteLine("   Search interrupted - No integer solution was found");
						break;
					case 4:
						Console.WriteLine("   Search interrupted - Integer solution found: %g",dObjVal);
						break;
					case 5:
						Console.WriteLine("   No integer solution was found");
						break;
					case 6:
						Console.WriteLine("   Integer solution found: {0}",dObjVal);
						break;
				}
				Console.WriteLine("\n\n   The MIP optimisation took {0} nodes\n\n",nNodes);

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
