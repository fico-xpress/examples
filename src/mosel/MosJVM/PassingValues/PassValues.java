/**
 * Example Java class, containing a method that returns a string based on arguments passed to it
 *
 * @author  J.Farmer, (c) Fair Isaac Corporation, 2016
 **/
public class PassValues {
	public static String repeatValue(String value, int nRepetitions) {
		StringBuilder result = new StringBuilder();
		for (int i=0;i<nRepetitions;i++) {
			result.append(value);
		}
		return result.toString();
	}
}