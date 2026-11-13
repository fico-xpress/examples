/********************************************************/
/*  Mosel Library Examples                              */
/*  ======================                              */
/*                                                      */
/*  file mmdispmod.java                                 */
/*  ```````````````````                                 */
/*  Example for the use of the Mosel libraries          */
/*  (display the contents of a model)                   */
/*                                                      */
/*  (c) 2021 Fair Isaac Corporation                     */
/*      author: Y. Colombani, 2006                      */
/********************************************************/

import com.dashoptimization.*;

public class mmdispmod
{

 public static void main(String[] args) throws Exception
 {
  XPRM mosel;
  XPRMModel mod;
  XPRMAnnotation anns[];

  if(args.length!=1)
  {
   System.out.println("Usage: mmdispmod modname.bim");
  }
  else
  {
   mosel=new XPRM();                        // Initialize Mosel
   mod=mosel.loadModel(args[0]);            // Load a BIM file

   if(mod.getSysComment().indexOf("PKG")==0)
    System.out.print("Package ");
   else
    System.out.print("Model ");
   System.out.println("`" + mod.getName() +
                      "' version "+ mod.getVersion());

   System.out.println("Parameters:");       // List of parameters
   for(XPRMParameters par=mod.parameters(); par.hasNext();)
   {
    XPRMParameter p=(XPRMParameter)par.next();
    System.out.println(" "+p.getName());
   }
   System.out.println();

   System.out.println("Requirements:");     // List of required symbols
   for(XPRMRequirements req=mod.requirements(); req.hasNext();)
   {
    dispsymb(mod,req.next(),true);
   }
   System.out.println();

   System.out.println("Symbols:");          // List of symbols
   for(XPRMIdentifiers ids=mod.identifiers(); ids.hasNext();)
   {
    dispsymb(mod,ids.next(),false);
   }
   System.out.println();

   System.out.println("Dependencies:");       // List of required pkgs/modules
   for(XPRMDependencies dep=mod.dependencies(); dep.hasNext();)
   {
    XPRMDependency d=dep.next();
    System.out.println((d.isPackage?" package ":" module ") + d.name +
                       " (" + d.version + ") ");
   }
   System.out.println();

   System.out.println("Annotations:");          // List of annotations
   anns=mod.getAnnotations("");
   if(anns.length>0)
   {
    System.out.println(" [global]->");
    for(int i=0;i<anns.length;i++)
     System.out.println("   "+anns[i]);
   }
   for(XPRMIdentifiers ids=mod.annotatedIdentifiers(); ids.hasNext();)
   {
    XPRMIdentifier id=ids.next();
    anns=mod.getAnnotations(id,"");
    System.out.println(" "+id.getName()+"->");
    for(int i=0;i<anns.length;i++)
     System.out.println("   "+anns[i]);
   }

   mod.close();
  }
 }

                /*******************************************/
                /* An extended Model claas to display info */
                /*******************************************/
  /**************************************/
  /* Display information about a symbol */
  /**************************************/
  static void dispsymb(XPRMModel mod, XPRMIdentifier symb,boolean isRequirement)
  {
   XPRMProcedure proc;

   switch(symb.getStructCode())
   {
    case XPRMTyped.STR_CONST:
      System.out.println(" " + symb.getName() + "= " + symb);
      break;
    case XPRMTyped.STR_PROC:
      proc=(XPRMProcedure)symb;
      if(isRequirement)
       dispprocfct(mod,proc);               // Display the prototype
      else
       do                               // Look for all overloading proc/func
       {
        dispprocfct(mod,proc);              // Display the prototype
       }while((proc=proc.next())!=null);
      break;
    case XPRMTyped.STR_UTYP:
      {
       XPRMTyped expn;
       XPRMIdentifier alias;

       if(symb.getName().startsWith("&")) break;
       System.out.print(" " + symb.getName() + "= ");
       expn=((XPRMUserType)symb).expand();
       if(((XPRMUserType)symb).isUnion())
       {
        dispunion(mod,((XPRMUserType)symb).types());
        System.out.println();
       }
       else
       if(((XPRMUserType)symb).isProcedure())
       {
        dispprocedure(mod,(XPRMUserType)symb);
        System.out.println();
       }
       else
       if(expn==symb)
       {
        alias=(XPRMIdentifier)(mod.expandType(symb.getTypeCode()));
        if(alias.getName().equals(symb.getName()))
        {
         if(((XPRMUserType)symb).isProblem())
          dispproblem(mod,(XPRMUserType)symb);
         else
         if(((XPRMUserType)symb).isRecord())
          disprecord(mod,(XPRMUserType)symb);
        }
        else
         System.out.println(alias.getName());
       }
       else
       {
        dispfulltype(mod,expn);
        System.out.println();
       }
       break;
      }
    case XPRMTyped.STR_NTYP:
      break;                       // ignore native types
    default:
      System.out.print(" " + symb.getName() + ": ");
      dispfulltype(mod,symb);
      System.out.println();
   }
  }

  /*****************************************/
  /* Decode and display a type information */
  /*****************************************/
  static void dispfulltype(XPRMModel mod, XPRMTyped type)
  {
   XPRMTyped expn;

   switch(type.getStructCode())
   {
    case XPRMTyped.STR_CONST:
    case XPRMTyped.STR_REF:
       if(type.getTypeCode()<=XPRMTyped.TYP_LINCTR)
        System.out.print(type.getTypeName());
       else
        dispfulltype(mod,mod.expandType(type.getTypeCode()));
       break;
    case XPRMTyped.STR_ARRAY:
       if(type instanceof XPRMArray)
        System.out.print("array (" +((XPRMArray)type).getSignature()+ ") of ");
       else
        System.out.print("array of ");
       dispfulltype(mod,mod.expandType(type.getTypeCode()));
       break;
    case XPRMTyped.STR_SET:
       System.out.print("set of ");
       dispfulltype(mod,mod.expandType(type.getTypeCode()));
       break;
    case XPRMTyped.STR_LIST:
       System.out.print("list of ");
       dispfulltype(mod,mod.expandType(type.getTypeCode()));
       break;
    case XPRMTyped.STR_UNION:
       dispunion(mod,((XPRMUnion)type).types());
       break;
    case XPRMTyped.STR_UTYP:
       expn=((XPRMUserType)type).expand();
       if(expn!=type)
        dispfulltype(mod,expn);
       else
       if(((XPRMUserType)type).isUnion())
        dispunion(mod,((XPRMUserType)type).types());
       else
       if(((XPRMUserType)type).isProcedure())
        dispprocedure(mod,(XPRMUserType)type);
       else
        System.out.print(type.getTypeName());
       break;
    default:
       System.out.print(type.getTypeName());
   }
  }

  /****************************************************/
  /* Diplay all the published fields of a record type */
  /****************************************************/
  static void disprecord(XPRMModel mod, XPRMUserType rec)
  {
   XPRMRecordFields fields;

   fields=rec.fields();
   if(fields.hasNext())
   {
    System.out.println("record publishing:");
    for(; fields.hasNext();)
    {
     System.out.print("     ");
     dispsymb(mod,fields.next(),false);
    }
   }
   else
    System.out.println("record (no public field)");
  }

  /*******************************************/
  /* Diplay compatible types of a union type */
  /*******************************************/
  static void dispunion(XPRMModel mod, XPRMUnionTypes types)
  {
   System.out.print(((XPRMType)types.next()).getTypeName());
   for(; types.hasNext();)
   {
    XPRMType t=(XPRMType)types.next();

    if(t.getTypeCode()==0)
     System.out.print(" or any");
    else
     System.out.print(" or "+t.getTypeName());
   }
  }

  /*******************************************************/
  /* Diplay all the problem components of a problem type */
  /*******************************************************/
  static void dispproblem(XPRMModel mod, XPRMUserType prob)
  {
   XPRMProblemComps comps;

   comps=prob.components();
   if(comps.hasNext())
   {
    for(; comps.hasNext();)
     System.out.print(((XPRMNativeType)comps.next()).getTypeName()+" ");
    System.out.println();
   }
   else
    System.out.println("problem");
  }

  /*******************************************/
  /* Display properties of a subroutine type */
  /*******************************************/
  static void dispprocedure(XPRMModel mod, XPRMUserType ptyp)
  {
   char[] parms;
   int i;

   if(ptyp.getReturnTypeCode()!=ptyp.TYP_NOT)
    System.out.print("function");
   else
    System.out.print("procedure");

   if(ptyp.getNbParameters()>0)
   {
    System.out.print("(");
    parms=ptyp.getParameterTypes().toCharArray();
    i=0;
    while(i<parms.length)
    {
     if(i>0) System.out.print(",");
     i=disptyp(mod,i,parms)+1;
    }
    System.out.print(")");
   }

   if(ptyp.getReturnTypeCode()!=ptyp.TYP_NOT)
   {
    System.out.print(":");
    dispfulltype(mod,mod.expandType(ptyp.getReturnTypeCode()));
   }
  }

  /***************************************/
  /* Diplay a prototype from a signature */
  /***************************************/
  static void dispprocfct(XPRMModel mod, XPRMProcedure proc)
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
     i=disptyp(mod,i,parms)+1;
    }
    System.out.print(")");
   }

   if(proc.getTypeCode()!=proc.TYP_NOT)
   {
    System.out.print(":");
    dispfulltype(mod,mod.expandType(proc.getTypeCode()));
   }
   System.out.println();
  }

  /****************************************/
  /* Display a type name from a signature */
  /****************************************/
  static int disptyp(XPRMModel mod, int i, char[] parms)
  {
   int j;

   switch(parms[i])
   {
    case 'i': System.out.print("integer");break;
    case 'r': System.out.print("real");break;
    case 's': System.out.print("string");break;
    case 'b': System.out.print("boolean");break;
    case 'v': System.out.print("mpvar");break;
    case 'c': System.out.print("linctr");break;
    case 'I': System.out.print("range");break;
    case 'a': System.out.print("array");break;
    case 'e': System.out.print("set");break;
    case 'l': System.out.print("list");break;
    case 'u': System.out.print("any");break;
    case '%':
          {
           String nstr=new String(parms,i+1,3);

           i+=3;
           dispfulltype(mod,mod.expandType(Integer.parseInt(nstr,16)));
           break;
          }
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
           i=disptyp(mod,i,parms)+1;
          }
          System.out.print(") of ");
          i=disptyp(mod,++i,parms);
          break;
    case 'E':
          System.out.print("set of ");
          i=disptyp(mod,++i,parms);
          break;
    case 'L':
          System.out.print("list of ");
          i=disptyp(mod,++i,parms);
          break;
    case '*':
          System.out.print("...");
          break;
    default: System.out.print("?");
   }
   return i;
  }
}
