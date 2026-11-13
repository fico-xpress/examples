/**
 * Example Java class, containing a method that returns a value based on an array passed to it
 *
 * @author  J.Farmer, (c) Fair Isaac Corporation, 2016
 **/
public class PassArray {
	public static double sumValues(double[] values) {
		double sum = 0;
		for (int i=0;i<values.length;i++) {
			sum += values[i];
		}
		return sum;
	}
}