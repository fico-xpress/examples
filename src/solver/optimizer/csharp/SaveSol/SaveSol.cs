/***********************************************************************
   Xpress Optimizer Examples
   =========================

   file SaveSol.cs
   ```````````````
   Show how to save a postsolved solution to memory and access the
   results of a global search. The program demonstrates the use of the
   integer solution callback.

   We take the knapsack problem in burglar.mps and instigate a global
   search. Whenever an integer solution is found it is postsolved,
   stored in memory, and printed to an output file. The best and final
   solution values, and other global search information, are displayed
   on screen. A log file is also created.

   (c) 2021-2026 Fair Isaac Corporation
***********************************************************************/

using System;
using System.IO;
using Optimizer;

namespace XPRSExamples
{
	class SaveSol
	{
		public static void Main(string[] args)
		{
			SaveSol example = new SaveSol();
			example.Run();
		}

		private void Run()
		{
			try
			{
				string sProblem="burglar";     /* Problem name */
				string sOutFile="savesol.out"; /* Output file name */

				int nSol;                     /* Number of integer solutions found */
				double dBestObj;              /* Best objective value found */
				int nNodes;                   /* Number of nodes solved in the global search */
				int nActNodes;                /* Number of active nodes ignored by the search */
				int nLastNode;                /* Node at which the last integer solution was found */
				double dBestBnd;              /* Best bound found in the global search */
				int nGlStatus;                /* Global search status - complete, incomplete, etc */
				int i;                        /* Loop counter*/

				// open the output file
				// Delete the file if it exists.
				if (File.Exists(sOutFile))
				{
					File.Delete(sOutFile);
				}
				pOutput = new StreamWriter(sOutFile);

				// Initialise Optimizer
				XPRS.Init("");

				prob = new XPRSprob();

				// Tell Optimizer to call optimizermsg whenever a message is output
				prob.AddMessageCallback(this.OptimizerMsg);

				// Allow no cuts - so the problem does not solve too quickly
				prob.CutStrategy = 0;
				
				// Read the problem file
				prob.MPSFormat = -1;
				prob.ReadProb(sProblem,"");

				// Tell Optimizer to postsolve every integer solution found, save it to memory and
				// print the solution values to the output file

				// Call function printsol whenever an integer solution is found
				prob.AddIntsolCallback(this.PrintSol);

				// Get the number of columns
				gnCol = prob.Cols;

				// Search for integer solutions
				Console.WriteLine("Solving problem {0}\n\n",sProblem);
				prob.Optimize();

				// Retrieve the results of the global search

				// Get the number of integer solutions found
				nSol = prob.MIPSols;

				// Get the objective value of the best integer solution found
				dBestObj = prob.MIPObjVal;

				// Get the number of outstanding nodes
				nActNodes = prob.ActiveNodes;

				// Get the node at which the last feasible integer solution was found
				nLastNode = prob.MIPSolNode;

				// Get the number of nodes solved
				nNodes = prob.Nodes;

				// Get the value of the best bound
				dBestBnd = prob.BestBound;

				// Get the global status
				nGlStatus = (int)prob.MIPStatus;

				// Display the results of the global search

				switch (nGlStatus)
				{
					case 3:
						Console.WriteLine("Global search incomplete\n\n");
						Console.WriteLine("   No integer solution found\n");
						Console.WriteLine("   {0} nodes searched\n",nNodes);
						Console.WriteLine("   {0} nodes remaining in search\n",nActNodes);
						Console.WriteLine("   Best bound {0}\n\n",dBestBnd);
						break;
					case 4:
						Console.WriteLine("Global search incomplete\n\n");
						Console.WriteLine("   {0} integer solution{1} found\n",nSol,(nSol==1) ? "" : "s");
						Console.WriteLine("   {0} nodes searched\n",nNodes);
						Console.WriteLine("   {0} nodes remaining in search\n",nActNodes);
						Console.WriteLine("   Best bound {0}\n\n",dBestBnd);
						Console.WriteLine("   Best integer solution at node {0}\n\n",nLastNode);
						Console.WriteLine("      Objective value {0}\n\n",dBestObj);
						Console.WriteLine("      Solution values\n");
						for (i=0; i<gnCol; i++)
							Console.WriteLine("          x[{0}]={1}\n",i,gpIntSol[i]);
						Console.WriteLine("\n");
						break;
					case 5:
						Console.WriteLine("Global search complete\n\n");
						Console.WriteLine("   No integer solution found\n");
						Console.WriteLine("   {0} nodes searched\n",nNodes);
						Console.WriteLine("   Best bound {0}\n",dBestBnd);
						break;
					case 6:
						Console.WriteLine("Global search complete\n\n");
						Console.WriteLine("   {0} nodes searched\n",nNodes);
						Console.WriteLine("   {0} integer solution%s found\n\n",nSol,(nSol==1) ? "" : "s");
						Console.WriteLine("   Best integer solution at node {0}\n\n",nLastNode);
						Console.WriteLine("      Objective value {0}\n\n",dBestObj);
						Console.WriteLine("      Solution values\n");
						for (i=0; i<gnCol; i++)
							Console.WriteLine("          x[{0}]={1}\n",i,gpIntSol[i]);
						Console.WriteLine("\n");
						break;
					default:
						Console.WriteLine("Global search did not take place\n\n");
						break;
				}
			}

			catch (XPRSException e)
			{
				Console.WriteLine(e.ToString());
				throw e;
			}
			finally
			{
				prob.Destroy();
				pOutput.Close();
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

		public void PrintSol(XPRSprob prob, object data)
		{
			int nNodeNum;            // Current node number
			double dObjVal;          // Objective value of the nodal integer solution
			int i;                   // Loop counter

			// Get the current node number
			nNodeNum = prob.CurrentNode;
	
			// Get the objective value of the current integer solution
			dObjVal = prob.LPObjVal;

			// Retrieve the postsolved solution values from memory
			gpIntSol = prob.GetCallbackSolution();

			// Print the solution to the output file
			pOutput.Write("Node " + nNodeNum + "\n");
			pOutput.Write("   Integer solution has objective value " + dObjVal + "\n");
			pOutput.Write("   Postsolved solution values are:\n");
			for (i=0; i<gnCol; i++)
				pOutput.Write("      x[" + i + "]=" + gpIntSol[i] + "\n");
			pOutput.Write("\n");
		}

		private XPRSprob prob;

		private int gnCol;                        // Numberfile of columns in the problem matrix
		private double[] gpIntSol;                // Integer solution values
		private StreamWriter pOutput;

	}
}
