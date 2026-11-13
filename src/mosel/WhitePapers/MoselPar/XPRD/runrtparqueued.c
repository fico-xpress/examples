/*******************************************************
   Mosel Example Problems 
   ======================

   file runrtparqueued.c
   `````````````````````
   Running several instances of a model from another
   Mosel model.
   - Queuing submodels for parallel execution in a
     distributed architecture (one or several models per node) -

   Before running this model, you need to set up the list
   NodeList with machine names/addresses of your local network.
   All nodes that are used need to have the same version of
   Xpress installed and suitably licensed, and the server 
   "xprmsrv" must have been started on these machines.

   The maximum number of models per node in array MaxMod needs
   to be adapted to the number of executions licensed on 
   the corresponding nodes.
   
   All files are local to the root node, no write access is
   required at remote nodes.
       
   (c) 2012 Fair Isaac Corporation
       author: S. Heipcke & Y. Colombani, Apr. 2012
*******************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xprd.h"

#define J 10         /* Number of jobs to run */
#define NUMPAR 2     /* Number of parallel model executions */
                     /* (preferrably <= no. of processors) */

#define MAXQSIZE 20  /* Maximum number of elements in a queue (>=J) */

                     /* Use the name or IP address of a machine in
                        your local network, or "" for current node */
const char *NodeList[]={"localhost","localhost"};
#define NodeListSize (sizeof(NodeList)/sizeof(NodeList[0]))
#define nbNodes (NodeListSize<J?NodeListSize:J)

int *jobid,*modid;
const char **modNode;

typedef struct
	{
	 int head;
	 int tail;
	 int size;
	 int buf[MAXQSIZE];
	} s_queue;

void start_next_job(s_queue *jobList,XPRDmodel model);
void queue_init(s_queue *q);
int queue_add(s_queue *q,int e);
int queue_get(s_queue *q);
int queue_size(s_queue *q);

int main(int argv,char *args[]) 
{
 XPRDcontext xprd;
 XPRDmosel mosInst[nbNodes];
 int MaxMod[nbNodes];
 XPRDmodel modPar[nbNodes*NUMPAR];
 int i,j,m,n,nct;
 char bufd[200];
 s_queue JobList,JobsRun;
 XPRDmodel sender;
 double eventValue;
 int eventClass;
 int lastId=0;
 int JobSize;

 xprd=XPRDinit();                  /* Create an XPRD context */

 /**** Setting up remote Mosel instances ****/
 for(n=0;n<nbNodes;n++)
 {
  mosInst[n]=XPRDconnect(xprd,NodeList[n], NULL, NULL, NULL, 0);
  if(mosInst[n]==NULL)
  {
   printf("Failed to connect to %s - aborting.\n",NodeList[n]);
   exit(1);
  }
  MaxMod[n]= NUMPAR;
        /* Adapt this setting to number of processors and licences per node */
 }

                                   /* Compile the model file on first node */
 XPRDcompmod(mosInst[0], "", "rmt:rtparams.mos", "rmt:rtparams.bim", "");

 /**** Loading model instances ****/
 nct=0;
 for(n=0;(n<nbNodes) && (nct<J);n++)
  for(m=0;(m<MaxMod[n]) && (nct<J);m++)
  {                                /* Load the bim file */
   modPar[nct]=XPRDloadmod(mosInst[n], "rmt:rtparams.bim"); 
   if(XPRDgetnumber(modPar[nct])>lastId) lastId=XPRDgetnumber(modPar[nct]);
   nct++;
  }

 jobid=malloc(sizeof(int)*(lastId+1));
 modid=malloc(sizeof(int)*(lastId+1));
 modNode=malloc(sizeof(char *)*(lastId+1));

 for(j=0;j<nct;j++)
 {
  i=XPRDgetnumber(modPar[j]);
  modid[i]=j;                     /* Store the model ID */
  modNode[i]=strdup(XPRDsysinfo(XPRDgetmosel(modPar[j]),XPRD_SYS_NODE, bufd, sizeof(bufd)));
 }

 queue_init(&JobList);
 queue_init(&JobsRun);
 for(i=0;i<J;i++)                 /* Define the list of jobs (instances) */
  queue_add(&JobList,i);
 JobSize=queue_size(&JobList);    /* Store the number of jobs */

 /**** Start initial lot of model runs ****/
 for(j=0;j<nct;j++)
  start_next_job(&JobList,modPar[j]);

 /**** Run all remaining jobs ****/
 while(queue_size(&JobsRun)<JobSize)
 {
  XPRDwaitevent(xprd,-1);         /* Wait for model termination */
  XPRDgetevent(xprd,&sender,&eventClass,&eventValue);
  if(eventClass==XPRD_EVENT_END)  /* We are only interested in "end" events */
  {                               /* Keep track of job termination */
   queue_add(&JobsRun,jobid[XPRDgetnumber(sender)]);
   printf("End of job %d (model %d)\n", jobid[XPRDgetnumber(sender)] ,
                             modid[XPRDgetnumber(sender)]);
   if(queue_size(&JobList)>0)     /* Start a new run if queue not empty */
    start_next_job(&JobList,sender);
  }
 }

 for(n=0;n<nbNodes;n++)
   XPRDdisconnect(mosInst[n]);   /* Disconnect remote instances */
 XPRDfinish(xprd);               /* Terminate XPRD */
 
 remove("rtparams.bim");         /* Cleaning up */

 return 0;
}

/*****************************/
/* Start next job in a queue */
/*****************************/
void start_next_job(s_queue *jobList,XPRDmodel model)
{
 char params[200];
 int i;

 i=queue_get(jobList);      /* Retrieve and remove first entry from queue */
 printf("Start job %d (model %d) on %s\n", i , modid[XPRDgetnumber(model)],
                         modNode[XPRDgetnumber(model)]);
 sprintf(params, "PARAM1=%d,PARAM2=%g,PARAM3='a string',PARAM4=%s", i,
           0.1*i, (i%2==0)?"true":"false");
 XPRDrunmod(model, params);
 jobid[XPRDgetnumber(model)]=i;
}

/**********************/
/* Initialize a queue */
/**********************/
void queue_init(s_queue *q)
{ memset(q,0,sizeof(s_queue)); }

/*****************************/
/* Add an element to a queue */
/*****************************/
int queue_add(s_queue *q,int e)
{
 if(q->size>=MAXQSIZE) return -1;
 else
 {
  q->buf[q->head++]=e;
  if(q->head>=MAXQSIZE) q->head=0;
  return ++q->size;
 }
}

/**********************************/
/* Remove an element from a queue */
/**********************************/
int queue_get(s_queue *q)
{
 int rts;

 if(q->size<1) return -1;
 else
 {
  rts=q->buf[q->tail++];
  if(q->tail>=MAXQSIZE) q->tail=0;
  q->size--;
  return rts;
 }
}

/***************************/
/* Get the size of a queue */
/***************************/
int queue_size(s_queue *q)
{ return q->size; }
