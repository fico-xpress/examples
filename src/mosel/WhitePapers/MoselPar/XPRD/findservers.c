/*******************************************************
   Mosel Example Problems 
   ======================

   file findservers.c
   ``````````````````
   Find Mosel servers that are able to accept remote model runs

   This file only produces output on the local node, 
   it does not start any remote runs.
       
   (c) 2013 Fair Isaac Corporation
       author: S. Heipcke, Jan. 2013
*******************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif
#include "xprd.h"

#define M 20                   /* Max. number of servers to be sought */

int main(int argv,char *args[]) 
{
  XPRDcontext xprd;
  XPRDmosel mosInst;
  struct in_addr Hosts[M];
  char buf[200],*sysinfo;
  int hsize, i;

  xprd=XPRDinit();             /* Create an XPRD context */
 
  printf("Searching...\n"); fflush(stdout);
  hsize=XPRDfindxsrvs(xprd, 1,M, (unsigned int *)Hosts);
  printf("%d server(s) found.\n", hsize); fflush(stdout);
   
  for(i=0;i<hsize;i++) 
  {                            /* Open connection to a remote node */
    strcpy(buf,inet_ntoa(Hosts[i]));
    mosInst=XPRDconnect(xprd, buf, NULL, NULL, NULL, 0);
    if (mosInst!=NULL)
    {
      printf("Server %s: ", inet_ntoa(Hosts[i])); fflush(stdout);
      sysinfo=XPRDsysinfo(mosInst, XPRD_SYS_ALL, buf, sizeof(buf));
      printf("%s\n",sysinfo!=NULL?sysinfo:"?"); fflush(stdout);
      XPRDdisconnect(mosInst);     /* Disconnect remote instance */
    }
    else
    {
      printf("Connection to %s failed\n", inet_ntoa(Hosts[i]));
      fflush(stdout);
    }
  }
  
  XPRDfinish(xprd);            /* Terminate XPRD */
  return 0;
} 

