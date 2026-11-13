/*********************************************************************
 * Example Java class, implementing the area calculation of the 
 * Polygon example as black box Java functions, both with and without  
 * returning self-calculated derivatives
 *
 * @author  Z.Csizmadia, (c) 2018 Fair Isaac Corporation
 *********************************************************************/
public class Polygon {    

    public static double CalculateArea(double[] rho, double[] theta) {
    double area = 0;
    int N = rho.length;
    assert rho.length == theta.length;
    // Calculate the area
    for (int i=1;i<N;i++) {      
      area = area + rho[i]*rho[i-1]*Math.sin(theta[i]-theta[i-1]) * 0.5; 
    }          
    return area;
  }

  public static double[] AreaWithDeltas(double[] rho, double[] theta, int[] I, 
                                        int[] J, double[] Deltas) {
    double area = 0;    
    double[] D;
    double temp;
    double Left, Right;
    int N = rho.length;
    int ND = Deltas.length;    
    assert rho.length == theta.length;
    assert I.length   == J.length;
    assert J.length   ==  Deltas.length;
    // Calculate result
    area = CalculateArea(rho, theta);
    D = new double[1+ND];
    D[0] = area;
    // Get delta requests and calculate finite differences to approximate the partials
    for (int i=0;i<ND;i++) {            
      if (J[i] == 1) {        
        temp = rho[I[i]];
      rho[I[i]] = temp - Deltas[i];      
      } else {
        temp = theta[I[i]];
      theta[I[i]] = temp - Deltas[i];      
      }      
      Left = CalculateArea(rho, theta);
      if (J[i] == 1) {        
      rho[I[i]] = temp + Deltas[i];      
      } else {        
      theta[I[i]] = temp + Deltas[i];      
      }  
      Right = CalculateArea(rho, theta);
      if (J[i] == 1) {        
      rho[I[i]] = temp;      
      } else {        
      theta[I[i]] = temp;      
      }    
      D[i+1] = (Right - Left ) / ( 2 * Deltas[i] );            
    }               
    return D;
  }
  
}





