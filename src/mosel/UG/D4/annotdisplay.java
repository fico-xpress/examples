/*******************************************************
   Mosel User Guide Example Problems
   ================================= 

   file annotdisplay.java
   ``````````````````````
   Compiling and loading a model to retrieve some
   annotations from it for display.

   (c) 2015 Fair Isaac Corporation
       author: S. Heipcke, Mar. 2015
********************************************************/

import com.dashoptimization.*;

public class annotdisplay
{
 public static void main(String[] args) throws Exception
 {
  XPRM mosel;
  XPRMModel mod;
  XPRMAnnotation ann[];                     // List of annotations

  mosel = new XPRM();                       // Initialize Mosel

  mosel.compile("annottest.mos");           // Compile the model into a BIM 
  mod = mosel.loadModel("annottest.bim");   // Load the BIM file

// Retrieve and display global annotations
  ann=mod.getAnnotations("");
  System.out.println("Global annotations (total: "+ ann.length +"):");
  for(int i=0;i<ann.length;i++) System.out.println("   "+ann[i]);

// Retrieve and display all annotations associated with model objects
  System.out.println("Annotations associated with objects:");
  for(XPRMIdentifiers ids=mod.annotatedIdentifiers(); ids.hasNext();)
  {
   XPRMIdentifier id=(XPRMIdentifier)ids.next();
   ann=mod.getAnnotations(id,"");
   System.out.println(" "+id.getName()+"->");
   for(int i=0;i<ann.length;i++) System.out.println("   "+ann[i]);
  }

// Retrieve and display annotations for model object 'myint' 
  ann=mod.getAnnotations("myint","");
  System.out.println("Annotations defined for 'myint' (total: "+ ann.length +"):");
  for(int i=0;i<ann.length;i++) System.out.println("   "+ann[i]);

  mod.close();                              // Unload model
 }
}
