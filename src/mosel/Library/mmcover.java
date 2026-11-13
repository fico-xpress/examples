/********************************************************/
/*  Mosel Library Examples                              */
/*  ======================                              */
/*                                                      */
/*  file mmcover.java                                   */
/*  `````````````````                                   */
/*  Example for the use of the Mosel libraries          */
/*  (using debugger interface)                          */
/*                                                      */
/*  (c) 2008 Fair Isaac Corporation                     */
/*      author: Y. Colombani, 2004                      */
/********************************************************/

import com.dashoptimization.*;

public class mmcover
{
 static int [] count;

 public static void main(String[] args) throws Exception
 {
  XPRM mosel;
  Dbg dbg= new Dbg();

  if(args.length!=1)
  {
   System.out.println("Usage: mmcover modname");
  }
  else
  {
   mosel=new XPRM();            // Initialize Mosel

                                // Compile the model with debug info
   mosel.compile("G",args[0]+".mos");
   try (XPRMModel mod=mosel.loadModel(args[0]+".bim"))
   {

                               // Create a table to store the number of times
                               // each statement is executed
    count=new int[mod.lastLineIndex+1];

                               // Disable model output
    mod.setDefaultStream(mod.F_OUTPUT,"null:");
                               // Put a breakpoint before the first statement
    mod.setBreakpoint(mod.getLocation(0));
    mod.debugger=dbg;

    System.out.print("Running model...");
    mod.run();
    System.out.println(" done.");

    printReport(mod);
   }
  }
 }

 /**********************/
 /* Debugger interface */
 /**********************/
 public static class Dbg implements XPRMDebugger
 {
  public XPRMLocation debug(int vmstat,XPRMLocation loc)
  {
   count[loc.lineIndex]++;     // Increase counter for current statement
   return DBG_STEP;            // Continue until next one
  }
 }

 /***********************************/
 /* Display results after execution */
 /***********************************/
 static void printReport(XPRMModel mod)
 {
  int l,neverDone;
  int nbstat;
  XPRMProcedure proc;
  XPRMLocation loc;

                           // Count statements that have never been executed
  nbstat=count.length;
  neverDone=0;
  for(l=0;l<nbstat;l++)
   if(count[l]==0) neverDone++;

  System.out.println((100.0-(double)neverDone*100/nbstat) +
                     "% (" + (nbstat-neverDone) + ") of the " + nbstat +
                     " statements of model `" + mod.getName() +
                     "' have been executed.");

                           // Look for all subroutines...
  for(XPRMIdentifiers ids=mod.identifiers(); ids.hasNext();)
  {
   XPRMIdentifier symb=(XPRMIdentifier)ids.next();

   if(symb instanceof XPRMProcedure)
   {
    proc=(XPRMProcedure)symb;
    do
    {                      // Display location
     loc=proc.findLocation();

     System.out.print(
          ((proc.getTypeCode()!=proc.TYP_NOT) ? "function `" : "procedure `") +
          proc.getName()+"' at ("+loc+")");

                          // Display number of times it has been called
     if(count[loc.lineIndex]>1)
      System.out.println(" has been called "+count[loc.lineIndex]+" times");
     else if(count[loc.lineIndex]==1)
      System.out.println(" has been called once");
     else
      System.out.println(" has never been called");

                          // Do the same for all overloaded routine
    }while((proc=proc.next())!=null);
   }
  }
 }
}
