/*******************************************************
   Mosel Example Problems
   ======================

   file mandelbrot.java
   ````````````````````
   Mandelbrot function: f(z) = z^2 + c with z,c complex numbers.

   Main program producing Java GUI and coordinating
   all submodels.
   On each node in the list NODESLIST we start K model instances.
   Each submodel instance is sent a rectangular area for which
   to calculate the pixel color values. The results are passed
   back via a file (located at the same place as this program,
   no write access to remote instances is required).
   Once the result has been displayed by Java, the submodel is
   restarted for a new data set.
   This Java program can be run on any platform (no Xpress
   installation required!), the Mosel (sub)models require Xpress.

   - Testing Java GUI with distributed computing -
   
   (c) 2011 Fair Isaac Corporation
       author: S. Heipcke, Feb. 2011
  *******************************************************/
  
import java.awt.*; 
import java.awt.image.*; 
import java.awt.event.*; 
import javax.swing.*;
import java.io.*;
import com.dashoptimization.*;

  
public class mandelbrot extends JFrame { 

  static final int K = 2;           // Number of submodels per Mosel instance
  static final int NUMPX = 1000;    // Graphic size in pixels
  static final int CWIDTH = 1100;   // Window height (>=NUMPX)
  static final int CHEIGHT = 1000;  // Window width (>=NUMPX)
  static final int SW = 100;        // Size of subproblems in pixels
  static final int XOFF = 100;      // Offset to left canvas border

/**** 
  ! Adapt this list to your local network:
  ! Use "localhost" for the current node, (under Unix also possible: "")
  ! Use machine names or IP addresses for remote machines
 ****/
  static final String NODELIST[] = {"localhost",""};

  static final int M = NODELIST.length;   // Number of remote instances
  static final int A = M*K;               // Total number of models

  static int minX,minY,ct,num,sqct,numsq;
  static double XMin,XMax,YMin,YMax,HX,HY,x,y;
  static int OffsetX,OffsetY;
  static int[][] SQUARE;
  static int[][] sol;
  static Integer[] confval;
  static boolean stoppressed;
   
  static XPRD xprd;
  static XPRDModel[] modPar;
  static XPRDMosel[] moselInst;

  static DrawingRegion canv;
  static JButton startButton,stopButton;
  static JComboBox choice;
  static JCheckBox[] check;
  static BufferedImage image;
  static mandelbrot app;

/***************** Configuration ******************/

  static void setMandelbrotConfig(int config) {
    if (config == 0) {
      XMin = -1.5; XMax = 0.5;
      YMin = -1; YMax = 1;
    }  
    else if (config == 1) {
      XMin = -0.90; XMax = -0.915;
      YMin = -0.245; YMax = -0.23;
    } 
    else if (config == 2) {
      XMin = -0.9; XMax = -0.98;
      YMin = -0.3; YMax = -0.22;
    }
    else if (config == 3) {
      XMin = -0.91; XMax = -0.94;
      YMin = -0.3; YMax = -0.27;
    }
    else if (config == 4) {
      XMin = -0.926; XMax = -0.934;
      YMin = -0.264; YMax = -0.256;
    } 

    HX = (XMax-XMin)/NUMPX;
    HY = (YMax-YMin)/NUMPX;
    OffsetX = -(int)Math.round(XMin/HX);
    OffsetY = -(int)Math.round(YMin/HY);  

    sqct=0;
    for(int s=1;s<=Math.ceil(NUMPX/SW);s++)
      for(int t=1;t<=Math.ceil(NUMPX/SW);t++) { 
        SQUARE[sqct][0] = Math.round((float)(XMin/HX))+(s-1)*SW; 
        SQUARE[sqct][1] =
          (Math.round((float)(XMin/HX))+s*SW-1<Math.round((float)(XMax/HX)))? Math.round((float)(XMin/HX))+s*SW-1:Math.round((float)(XMax/HX)); 
        SQUARE[sqct][2] = Math.round((float)(YMin/HY))+(t-1)*SW; 
        SQUARE[sqct][3] =
          (Math.round((float)(YMin/HY))+t*SW-1<Math.round((float)(YMax/HY)))? Math.round((float)(YMin/HY))+t*SW-1:Math.round((float)(YMax/HY));
	sqct+=1; 
    }
  }

/**** XAD and Java use the opposite encoding of RGB values ****/
  static int invertColorValue(Color col) {
    int r=col.getRed();
    int g=col.getGreen();
    int b=col.getBlue();
    int v=256*256;
    return r+(256*g)+(v*b);
  }

/***************** Reading result data ******************/

 static void readSol(String filename) throws IOException
 {
   boolean ndxOK=false;
   BinDrvReader bdrv;
   FileInputStream f;
   int i,j,ctrl;

   f=new FileInputStream(filename);
   bdrv=new BinDrvReader(f);              // Use Mosel bin reader

   if((bdrv.getControl()==BinDrvReader.CTRL_LABEL)&&
     bdrv.getString().equals("sol")&&
     (bdrv.getControl()==BinDrvReader.CTRL_OPENLST))
   {
     while(bdrv.nextToken()>=0) {
     ctrl=bdrv.getControl();
     if(ctrl==BinDrvReader.CTRL_CLOSELST) break;
     else
       if(ctrl==BinDrvReader.CTRL_OPENNDX)
       {
         if(ndxOK) {
           i=bdrv.getInt()-minX;
           j=bdrv.getInt()-minY;
         }
         else {
           minX=bdrv.getInt();
           minY=bdrv.getInt();
           i=j=0;
           ndxOK=true;
         }
         if(bdrv.getControl()==BinDrvReader.CTRL_CLOSENDX)
         {
           sol[i][j]=bdrv.getInt();
         }
         else
           throw new IOException("Wrong file format. ')' expected.");
       }
       else
         throw new IOException("Wrong file format. '(' expected.");
     }
   }
   else
     throw new IOException("Wrong file format");
   f.close();
 }
  
/***************** Application setup ******************/
  
  public mandelbrot() {
    super("Mandelbrot");
    setLocation(50,25);
    
  // Initialize data structures  
    numsq=(int)Math.round(Math.ceil((float)(NUMPX/SW))*
                                     Math.ceil((float)(NUMPX/SW)));
    SQUARE = new int[numsq][4];
    sol=new int[SW][SW];
    setMandelbrotConfig(0);                 // Default config values

  // Create graphic objects
    startButton = new JButton("Start");
    startButton.setAlignmentX(Component.CENTER_ALIGNMENT);
    startButton.setMaximumSize(new Dimension(100,30));
    startButton.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent ae) { 
        int c = (Integer)(choice.getSelectedItem());
        startButton.setEnabled(false);
        setMandelbrotConfig(c);
        new RenderingThread(c).start();     
      } 
    });
 
    confval = new Integer[5];
    for(int i=0;i<=4;i++) confval[i]=i;    
    choice = new JComboBox<Integer>(confval);
    choice.setMaximumSize(new Dimension(40,30));
    JLabel label = new JLabel("Config:");

    JPanel buttonsPanel = new JPanel();
    buttonsPanel.setLayout(new BoxLayout(buttonsPanel,BoxLayout.Y_AXIS));
    buttonsPanel.add(startButton);
    Box confBox = new Box(BoxLayout.X_AXIS);
    confBox.setMaximumSize(new Dimension(100,60));
    confBox.add(label);
    confBox.add(confBox.createHorizontalStrut(10));
    confBox.add(choice);
    confBox.add(confBox.createHorizontalStrut(10));
    buttonsPanel.add(confBox);

    stopButton = new JButton("Stop");
    stopButton.setAlignmentX(Component.CENTER_ALIGNMENT);
    stopButton.setMaximumSize(new Dimension(100,30));
    stopButton.setEnabled(false);
    stoppressed=false;
    stopButton.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent ae) { 
        stoppressed=true; 
      } 
    });
    buttonsPanel.add(stopButton);
    
    Box checkBox = new Box(BoxLayout.Y_AXIS);
    checkBox.setMaximumSize(new Dimension(100,A*30));
    checkBox.setAlignmentX(Component.CENTER_ALIGNMENT);
    checkBox.add(checkBox.createHorizontalStrut(10));
    check = new JCheckBox[A];
    for(int i=0;i<A;i++) {
      check[i] = new JCheckBox((i+1)+ ":" + 
                (NODELIST[i%M]==""?"(local)":NODELIST[i%M]), false);
      check[i].setAlignmentX(Component.LEFT_ALIGNMENT);
      checkBox.add(check[i]);
    }
    buttonsPanel.add(checkBox);
    this.getContentPane().add(BorderLayout.WEST, buttonsPanel);

    canv = new DrawingRegion();
    JScrollPane canvasScroller = new JScrollPane(canv);
    canvasScroller.setPreferredSize(new Dimension(NUMPX+20,NUMPX+20));
    this.getContentPane().add(canvasScroller);

    pack();
    setVisible( true );

    image = (BufferedImage)(canv.createImage(NUMPX, NUMPX));
   
  // Window closing event: stop application
    addWindowListener(new WindowAdapter() {
      public void windowClosing(WindowEvent e) {
        setVisible(false);
        dispose();
                                        // Disconnect remote connections
        for(int i=0;i<M;i++) moselInst[i].disconnect();
        System.exit(0);
      }
    });
  }
  

/*******************************************/
/****************** Main *******************/

  public static void main(String[] args) { 
    xprd = new XPRD();
    modPar = new XPRDModel[A];
    moselInst = new XPRDMosel[M];

    try {
      for(int i=0;i<M;i++)               // Establish remote connections
        moselInst[i]=xprd.connect(NODELIST[i]);
    }
    catch(IOException e) {
      System.out.println("Connection failed: " + e);
      System.exit(1);
    }

    try {                               // Compile the model
      moselInst[0].compile("", "rmt:mandelbrotsub.mos", "rmt:mb.bim");
    }
    catch(Exception e) {
      System.out.println("Compilation failed: " + e);
      System.exit(2);
    }
      
    try {
      for(int j=0;j<A;j++)               // Load models
      modPar[j]=moselInst[j%M].loadModel("rmt:mb.bim");
    }
    catch(IOException e) {
      System.out.println("Loading failed: " + e);
      System.exit(3);
    }  
    new File("mb.bim").delete();         // Cleaning up

  // Define and start GUI
    app = new mandelbrot();
  } 
  
/*******************************************/
/***************** Canvas ******************/

  class DrawingRegion extends JComponent { 
    public DrawingRegion() {  
      super();
      setPreferredSize(new Dimension(NUMPX,NUMPX));    
    } 

    protected void paintComponent(Graphics g){
      super.paintComponent(g);
     g.drawImage(image, 0, 0, this);
    } 
  }  

/***************** Drawing ******************/
  class RenderingThread extends Thread {
    int c;
    
    RenderingThread(int c)
    { this.c=c;}
    
    void resetCanvas() {
        Graphics buffer = image.getGraphics();
        buffer.setColor( Color.white );
        buffer.fillRect( 0, 0, NUMPX, NUMPX );
    }
    
    public void run(){      
      resetCanvas();
      stopButton.setEnabled(true);

   // Start first lot of remote model executions
      int modstartct=0; 
      int nbrunning=0;
      for(int n=0;n<((M*K<sqct)?M*K:sqct);n++) {
        modPar[n].setExecParam("MINX", SQUARE[modstartct][0]);
        modPar[n].setExecParam("MAXX", SQUARE[modstartct][1]); 
        modPar[n].setExecParam("MINY", SQUARE[modstartct][2]);
        modPar[n].setExecParam("MAXY", SQUARE[modstartct][3]); 
        modPar[n].setExecParam("NUM", n);
        modPar[n].setExecParam("HX", HX);
        modPar[n].setExecParam("HY", HY);
        modPar[n].setExecParam("NUMPX", NUMPX); 
        modPar[n].setExecParam("CONFIG", c);
        modPar[n].setExecParam("IODRV", "bin:");
	check[n].setSelected(true);
        try {
          modPar[n].run();
        }
        catch(IOException e) {
          System.out.println("Model run failed: " + e);
          System.exit(1);
        }
        modstartct++;
	nbrunning++;
      }
 
   // Wait for termination and start remaining
   // Add up counts returned by child models
      int modendct=0; 
      XPRDEvent ev;
      XPRDModel evmod;
      while ((modendct< sqct)&&(nbrunning>0)) {
        xprd.waitForEvent();
        ev=xprd.getNextEvent();
        if (ev.eventClass==ev.EVENT_END) {
          modendct++; 
          evmod=ev.sender;
	  num = evmod.getResult();
	  check[evmod.getNumber()-1].setSelected(false);

     // Read result data
        try {
          readSol("solmod"+num+".txt"); 
        } catch(IOException e) {
          System.out.println("Could not read file "+num);
          System.exit(1);
        }
        new File("solmod"+num+".txt").delete();

     // Draw result square on buffer
        for(int s=0;s<SW;s++)
	  for(int t=0;t<SW;t++) {
            image.setRGB(SQUARE[num][0]+s+OffsetX,SQUARE[num][2]+t+OffsetY,
                    invertColorValue(new Color(sol[s][t])));
          }	  
        canv.repaint();

     // Start new instance
        if ((modstartct< sqct) && (!stoppressed)) {
          evmod.execParams="MINX="+SQUARE[modstartct][0] + ",MAXX="+
                    SQUARE[modstartct][1] + ",MINY="+ SQUARE[modstartct][2] +
                    ",MAXY="+SQUARE[modstartct][3] + ",NUM="+modstartct +
                    ",HX="+HX + ",HY="+HY + ",NUMPX="+NUMPX + 
                    ",CONFIG="+c+",IODRV='bin:'";
	  check[evmod.getNumber()-1].setSelected(true);
          try {
	    evmod.run();
          }
          catch(IOException e) {
            System.out.println("Model run failed: " + e);
            System.exit(1);
          }
          modstartct++;
        }
	else
	  nbrunning--;
        }  
      }  // while

      stopButton.setEnabled(false);
      startButton.setEnabled(true);
      stoppressed=false;
    }
  }   
 
}  
