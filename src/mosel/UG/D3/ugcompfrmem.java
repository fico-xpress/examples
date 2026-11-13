/*******************************************************
   Mosel User Guide Example Problems
   ================================= 

   file ugcompfrmem.java
   `````````````````````
   Compiling a model from memory.

   (c) 2008 Fair Isaac Corporation
       author: S. Heipcke, 2006
********************************************************/

import java.nio.*;
import com.dashoptimization.*;

public class ugcompfrmem
{
                // The source of the model as a string
 static final String source_of_model=
  "model Burglar\n"+
  "uses 'mmxprs'\n"+

  "declarations\n"+
  " WTMAX = 102                    ! Maximum weight allowed\n"+
  " ITEMS = 1..8                   ! Index range for items\n"+
  " VALUE: array(ITEMS) of real    ! Value of items\n"+
  " WEIGHT: array(ITEMS) of real   ! Weight of items\n"+
  " take: array(ITEMS) of mpvar    ! 1 if we take item i; 0 otherwise\n"+
  "end-declarations\n"+

  "VALUE :: [15, 100, 90, 60, 40, 15, 10,  1]\n"+
  "WEIGHT:: [ 2,  20, 20, 30, 40, 30, 60, 10]\n"+

  "! Objective: maximize total value\n"+
  "MaxVal:= sum(i in ITEMS) VALUE(i)*take(i)\n"+

  "! Weight restriction\n"+
  "sum(i in ITEMS) WEIGHT(i)*take(i) <= WTMAX\n"+
  "! All variables are 0/1\n"+
  "forall(i in ITEMS) take(i) is_binary\n"+

  "maximize(MaxVal)                ! Solve the problem\n"+

  "! Print out the solution\n"+
  "writeln(\"Solution:\\n Objective: \", getobjval)\n"+
  "forall(i in ITEMS)  writeln(' take(', i, '): ', getsol(take(i)))\n"+

  "end-model";


 public static void main(String[] args) throws Exception
 {
  XPRM mosel;
  ByteBuffer mosfile;                   // Buffer to store source file

  mosel = new XPRM();                   // Initialize Mosel

                                        // Prepare file names for compilation:
                                        // Wrap source in a byte buffer
  mosfile=ByteBuffer.wrap(source_of_model.getBytes());
  mosel.bind("modelfile", mosfile);     // Associate Java object with a name 
                                        // in Mosel

  try
  {                                     // Compile model from memory
   mosel.compile("", "java:modelfile", "burglar.bim", "");
  }
  catch(XPRMCompileException e)
  {
   System.out.println(e.getMessage());
  }
  mosel.unbind("mosfile");              // Release memory
  mosfile=null;

  try (XPRMModel mod=mosel.loadModel("burglar.bim")) // Load BIM file
  { mod.run(); }                        // Run the model
 }
}
