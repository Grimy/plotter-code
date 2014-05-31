/*
 * This file is part of DraWall.
 * DraWall is free software: you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 * DraWall is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU
 * General Public License along with DraWall. If not, see <http://www.gnu.org/licenses/>.
 * Copyright (c) 2012-2014 Nathanaël Jourdane.
 */

package model;

import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Properties;

public class ConfigParser {

	public static String dependenciesFileName = "drawall_dependencies.properties";

	/**
	 * Save properties to the project’s root folder.
	 */
	public void writeProperties() {
		Properties prop = new Properties();
		prop.setProperty("database", "localhost");
		prop.setProperty("dbuser", "mkyong");
		prop.setProperty("dbpassword", "password");

		try (FileOutputStream output = new FileOutputStream(dependenciesFileName)) {
			prop.store(output, null);
		} catch (IOException e) {
			// TODO: handle this
		}
	}

	public void readProperties() {
		Properties prop = new Properties();

		try (FileInputStream input = new FileInputStream(dependenciesFileName)) {
			prop.load(input);
		} catch (IOException e) {
			// TODO: handle this
		}

		// get the property value and print it out
		System.out.println(prop.getProperty("database"));
		System.out.println(prop.getProperty("dbuser"));
		System.out.println(prop.getProperty("dbpassword"));
	}
}
