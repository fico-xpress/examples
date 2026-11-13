/*******************************************************
   Mosel Library Examples
   ====================== 

   file mmexlst.java
   `````````````````
   Accessing modeling objects 
   (enumerating the elements of a list).

   (c) 2008 Fair Isaac Corporation
       author: S. Heipcke, 2007
********************************************************/

import com.dashoptimization.*;

public class mmexlst
{
 public static void main(String[] args) throws Exception
 {
  XPRM mosel;
  XPRMModel mod;
  XPRMList lst;

  mosel = new XPRM();                  // Initialize Mosel
                           // Disable output from Mosel (make model silent)
  mosel.setDefaultStream(XPRM.F_OUTPUT, "null:");

  mosel.compile("Models/euler.mos");   // Compile, load & run the model
  mod = mosel.loadModel("Models/euler.bim");
  mod.run();

                           // Get the model object named 'TOUR'
  lst=(XPRMList)mod.findIdentifier("TOUR");      

  System.out.print("Tour: ");          // Print out all list elements
  for(XPRMListElements el=lst.elements(); el.hasNext();)
  {
   int value=el.nextAsInteger();
   if (el.hasNext())  
    System.out.print(value + " -> ");
   else
    System.out.println(value);   
  }

  mod.close();                         // Unload the model 
 }
}
