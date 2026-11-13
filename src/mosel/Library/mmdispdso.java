/********************************************************/
/*  Mosel Library Examples                              */
/*  ======================                              */
/*                                                      */
/*  file mmdispdso.java                                 */
/*  ```````````````````                                 */
/*  Example for the use of the Mosel libraries          */
/*  (display the contents of a module)                  */
/*                                                      */
/*  (c) 2008 Fair Isaac Corporation                     */
/*      author: Y. Colombani, 2004                      */
/********************************************************/

import com.dashoptimization.*;

public class mmdispdso
{
public static void main(String[] args) throws Exception
{
 XPRM mosel;
 XPRMModule module;

 if(args.length!=1)
 {
  System.out.println("Usage: mmdispdso modulename");
 }
 else
 {
  mosel=new XPRM();                        // Initialize Mosel
  module=mosel.loadModule(args[0]);
  System.out.print("Module `" + module.getName() +
                   "' version "+ module.getVersion());
  if(module.getCertificate()!=null)
   System.out.println(" ("    + module.getCertificate() +")");
  else
   System.out.println();
  System.out.println("File: "+ module.getPath() +"/"+ module.getName() +".dso");
  System.out.println("Priority: "+ module.getPriority());

  System.out.println("Constants:");        // List of constants
  for(XPRMConstants par=module.constants(); par.hasNext();)
  {
   XPRMConstant p=par.next();
   System.out.println(" "+p.getName()+"="+p);
  }
  System.out.println();

  System.out.println("Types:");             // List of types
  for(XPRMNativeTypes par=module.types(); par.hasNext();)
  {
   XPRMNativeType p=par.next();
   System.out.print(" "+p.getName()+" (");
   if(p.hasCreate()) System.out.print("create");
   if(p.hasDelete()) System.out.print(",delete");
   if(p.isProblem()) System.out.print(",problem");
   if(p.hasRefCount()) System.out.print(",refcnt");
   if(p.hasToString()) System.out.print(",tostring");
   if(p.hasPRTBL()) System.out.print("+");
   if(p.hasFromString()) System.out.print(",fromstring");
   if(p.hasBinary()) System.out.print(",bin");
   if(p.hasShare()) System.out.print(",share");
   if(p.isOrdered()) System.out.print(",ord");
   if(p.hasConst()) System.out.print(",const");
   if(p.hasResetOnly()) System.out.print(",reset");
   if(p.hasCopy()) System.out.print(",copy");
   if(p.hasAppend()) System.out.print("+");
   System.out.println(")");
  }
  System.out.println();

  System.out.println("Control Parameters:"); // List of control parameters
  for(XPRMParameters par=module.parameters(); par.hasNext();)
  {
   XPRMParameter p=par.next();
   System.out.print(" "+p.getName()+": "+p.getTypeName()+" (");
   if(p.getDescription()!=null)
    System.out.print(p.getDescription()+",");
   System.out.println(rwstatus(p)+")");
  }
  System.out.println();

                                             // Subroutines: show the prototype
  System.out.println("Procedure and functions:");
  for(XPRMProcedures par=module.procedures(); par.hasNext();)
  {
   dispprocfct(par.next());
  }
  System.out.println();

  System.out.println("I/O drivers:");        // IO drivers
  for(XPRMIODrivers ids=module.IODrivers(); ids.hasNext();)
  {
   XPRMIODriver p=ids.next();
   System.out.println(" "+p.getName()+":"+(p.getInfo()!=null?p.getInfo():""));
  }
 }
}

/************************************************/
/* Return the r/w status of a control parameter */
/************************************************/
static String rwstatus(XPRMParameter p)
{
 if(p.isReadable())
  if(p.isWriteable())
   return "r/w";
  else
   return "r";
 else
  if(p.isWriteable())
   return "w";
  else
   return "?";
}

/***************************************/
/* Diplay a prototype from a signature */
/***************************************/
static void dispprocfct(XPRMProcedure proc)
{
 char[] parms;
 int i;

 if(proc.getTypeCode()!=proc.TYP_NOT)
  System.out.print(" function "+proc.getName());
 else
  System.out.print(" procedure "+proc.getName());

 if(proc.getNbParameters()>0)
 {
  System.out.print("(");
  parms=proc.getParameterTypes().toCharArray();
  i=0;
  while(i<parms.length)
  {
   if(i>0) System.out.print(",");
   i=disptyp(i,parms)+1;
  }
  System.out.print(")");
 }

 if(proc.getTypeCode()!=proc.TYP_NOT)
  System.out.println(":"+proc.getTypeName());
 else
  System.out.println();
}

/****************************************/
/* Display a type name from a signature */
/****************************************/
static int disptyp(int i, char[] parms)
{
 int j;

 switch(parms[i])
 {
  case 'i': System.out.print("integer");break;
  case 'r': System.out.print("real");break;
  case 'S':
  case 's': System.out.print("string");break;
  case 'b': System.out.print("boolean");break;
  case 'v': System.out.print("mpvar");break;
  case 'c': System.out.print("linctr");break;
  case 'I': System.out.print("range");break;
  case 'a': System.out.print("array");break;
  case 'e': System.out.print("set");break;
  case 'l': System.out.print("list");break;
  case '|':
        i++;
        do
        {
         System.out.print(parms[i++]);
        } while(parms[i]!='|');
        break;
  case '!':
        i++;
        do
        {
         System.out.print(parms[i++]);
        } while(parms[i]!='!');
        break;
  case 'A':
        System.out.print("array (");
        j=++i;
        while(parms[i]!='.')
        {
         if(j!=i) System.out.print(",");
         i=disptyp(i,parms)+1;
        }
        System.out.print(") of ");
        i=disptyp(++i,parms);
        break;
  case 'E':
        System.out.print("set of ");
        i=disptyp(++i,parms);
        break;
  case 'L':
        System.out.print("list of ");
        i=disptyp(++i,parms);
        break;
  case '*':
        System.out.print("...");
        break;
  default: System.out.print("?");
 }
 return i;
}
}
