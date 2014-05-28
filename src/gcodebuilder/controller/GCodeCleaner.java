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

package controller;

import java.io.*;

public class GCodeCleaner {
	private static double pos_x, pos_y, pos_z, width, height;

	public void clean(BufferedReader in, PrintStream out) {
		clean(in, out, "G00|G01");
	}

	public void clean(BufferedReader in, PrintStream out, String mask) {
		try {
			while (in.ready()) {
				// TODO: remove empty lines
				out.println(cleanLine(in.readLine()));
			}
		} catch (IOException e) {
			// TODO: handle this
		}
	}

	private static String cleanLine(String line) {
		String cleaned = line;

		// Remove whitespace
		cleaned = cleaned.replaceAll("[ \\t]", "");
		// Remove comments
		cleaned = cleaned.replaceAll("^;.*", "");
		cleaned = cleaned.replaceAll("\\(.*", "");
		// Insert G01 if the line begins with an argument
		cleaned = cleaned.replaceAll("([IJKXYZPQ])", " \\1");
		// Insert spaces before each argument
		// TODO: this doesn’t work in Java
		// cleaned = cleaned.replaceAll("([GM])(\\d\\s)"n "\\g<1>0\\2", "");

		boolean pos = false;
		if (cleaned.indexOf('X') >= 0) {
			pos_x = getValue(cleaned, "X");
			pos = true;
		}
		if (cleaned.indexOf('Y') >= 0) {
			pos_y = getValue(cleaned, "Y");
			pos = true;
		}
		if (cleaned.indexOf('Z') >= 0) {
			pos_y = getValue(cleaned, "Z");
			pos = true;
		}
		if (pos) {
			cleaned = cleaned.replaceAll("([GM][0-9][0-9]).*",
					"\\1" + " X" + Double.toString(pos_x) + " Y" + Double.toString(pos_y) + " Z"
							+ Double.toString(pos_z));
		}

		cleaned = cleaned.replaceAll("^G0[01]$", "");
		return cleaned;
	}

	private static double getValue(String line, String param) {
		return Double.parseDouble(line.replaceAll(".*" + param + "([-0-9.]+).*", "\\1"));
	}
}
