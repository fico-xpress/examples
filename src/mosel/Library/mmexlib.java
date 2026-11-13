/********************************************************/
/*  Mosel Library Examples                              */
/*  ======================                              */
/*                                                      */
/*  file mmexlib.java                                   */
/*  `````````````````                                   */
/*  Example for the use of the Mosel libraries          */
/*  (working with models and accessing the dynamic      */
/*  libraries loaded by Mosel)                          */
/*                                                      */
/*  (c) 2008 Fair Isaac Corporation                     */
/*      author: S. Heipcke, 2004                        */
/********************************************************/

import com.dashoptimization.*;

public class mmexlib
{
public static void main(String[] args) throws Exception
{
 XPRM mosel;
 XPRMModel[] mod=new XPRMModel[3];

 mosel=new XPRM();                              // Initialize Mosel
 mod[0]=mosel.loadModel("Models/burglari.bim"); // Load the BIM files
 mod[1]=mosel.loadModel("Models/chess2.bim");
 mod[2]=mosel.loadModel("Models/trans.bim");

                                                // Enumerate all loaded models
                                                // and display information
 System.out.println("Models loaded:");
 for(int i=0;i<3;i++)
  System.out.println("   "+ mod[i].getNumber() +
                     ": " + mod[i].getName() +
                     " (" + mod[i].getSysComment() +
                     ", `"+ mod[i].getUserComment() +
                     "' size:"+ mod[i].getSize() +
                     ")" );

                                                // Enumerate all loaded modules
                                                // and display information
 System.out.println("Additional libraries loaded:");
 for(XPRMModules ms=mosel.modules(); ms.hasNext();)
 {
  XPRMModule m=ms.next();
  System.out.println("  "        + m.getName() +
                     " (version "+ m.getVersion() +
                     ") used by "+ m.getNumberOfReferences() +
                     " model(s)");
 }

 for(int i=0;i<3;i++) mod[i].close();
}
}
