/**
 * Example Java class, defining file-related utility functions
 *
 * @author  J.Farmer, (c) Fair Isaac Corporation, 2016
 **/

import java.io.*;

public class FileUtils {

	/**
	 * Given a file path, read the file content into a string
	 **/
	public static String readFileToString(String path) throws IOException {
		StringWriter result = new StringWriter();
		try (Reader in = new BufferedReader(new FileReader(path))) {
			int c;
			while ((c=in.read())!=-1) {
				result.write(c);
			}
		}
		return result.toString();
	}

}