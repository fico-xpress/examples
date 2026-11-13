/***********************************************************************
   Xpress Optimizer Examples
   =========================

   file IISExample.cs
   ``````````````````
   (c) 2021-2026 Fair Isaac Corporation
***********************************************************************/

using System;
using System.IO;
using Optimizer;


namespace XPRSExamples
{
  class IISExample
  {
    public static void Main(string[] args)
    {
      XPRS.Init(); // Initialize license
      try
      {
        IISExample example = new IISExample();
        example.Run(0); // Run example in mode 0
        example.Run(1); // Run example in mode 1
      }
      finally
      {
        XPRS.Free();  // Release license
      }
    }

    public void Run_GetIISData(XPRSprob problem, int i)
    {
      int ncol, nrow;
      problem.GetIISData(i, out nrow, out ncol, null, null, null, null, null, null, null, null);
      int[] miisrow = new int[nrow];
      int[] miiscol = new int[ncol];
      char[] constrainttype = new char[nrow];
      char[] colbndtype = new char[ncol];
      double[] duals = new double[nrow];
      double[] rdcs = new double[ncol];
      char[] isolationrows = new char[nrow];
      char[] isolationcols = new char[ncol];
      problem.GetIISData(i, out nrow, out ncol, miisrow, miiscol, constrainttype, colbndtype, duals, rdcs, isolationrows, isolationcols);
    }


    public void Run_IISStatus(XPRSprob problem, int i)
    {
      int count;
      problem.IISStatus(out count, null, null, null, null);
      int[] rowsizes = new int[count + 1];
      int[] colsizes = new int[count + 1];
      double[] suminfeas = new double[count + 1];
      int[] numinfeas = new int[count + 1];
      problem.IISStatus(out count, rowsizes, colsizes, suminfeas, numinfeas);
    }

    public void Run(int iMode)
    {
      XPRSprob problem = new XPRSprob();

      // Configure logging
      problem.LPLog = 1;
      problem.AddMsgHandlerCallback(Console.Out);

      // Input problem
      problem.ReadProb("iisexample");

      // Solve problem
      problem.Presolve = -1;
      problem.ChgObjSense( ObjSense.Minimize );
      problem.Optimize();

      switch (iMode)
      {
        case 0:
          /* Get all iis at once then iterate through results. */
          problem.IISAll();
          for (int i = 1; i < problem.NumIIS; i++)
          {
            Run_GetIISData(problem, i);
            Run_IISStatus(problem, i);
            problem.WriteIIS(i, "iis_result" + i, 0);
          }
          break;

        case 1:
          int j = 0;
          /* Get the IIS one at a time */
          if (problem.FirstIIS(1)==0) {
            do
            {
              j++;
              Run_GetIISData(problem, j);
              Run_IISStatus(problem, j);
              problem.WriteIIS(j, "iis_result" + j, 0);
            } while (problem.NextIIS() == 0);
          }
          break;
      }

    }


  }

}
