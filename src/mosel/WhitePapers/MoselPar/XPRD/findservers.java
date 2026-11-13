/*******************************************************
   Mosel Example Problems 
   ======================

   file findservers.java
   `````````````````````
   Find Mosel servers that are able to accept remote model runs

   This file only produces output on the local node, 
   it does not start any remote runs.
       
   (c) 2013 Fair Isaac Corporation
       author: S. Heipcke, Jan. 2013
*******************************************************/

import com.dashoptimization.*;
import java.lang.*;
import java.io.*;
import java.util.*;


public class findservers
{
 static final int M=20;
 
 public static void main(String[] args) throws Exception
 {
  XPRD xprd=new XPRD();
  XPRDMosel mosInst=null;
  Set<String> Hosts=new HashSet<String>();

  xprd.findXsrvs(1, M, Hosts);
  System.out.println(Hosts.size() + " servers found.");
  
  for(Iterator<String> h=Hosts.iterator(); h.hasNext();)
  {
   String i=h.next();
                               // Open connection to a remote node
   try {
    mosInst=xprd.connect(i);
    
    System.out.println("Server " + i + ": " + mosInst.getSystemInformation());
    mosInst.disconnect();        // Disconnect remote instance
   }
   catch(IOException e) {
    System.out.println("Connection to " + i + " failed");
   }
  }
  
 }
} 
