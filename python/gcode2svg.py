#!/usr/bin/python3.2
# -*- coding: utf-8 -*-

import re
import gcodecleaner

#Move_mode = enum(none, , )

def getSize(input_file) :
	width = 0
	height = 0
	for line in open(input_file).readlines() :
		if 'X' in line :
			x = gcodecleaner.getValue('X', line);
			if x > width :
				width = x
			
		if 'Y' in line :
			y = gcodecleaner.getValue('Y', line);	
			if y > height :
				height = y

	return (width, height)

def convert(input_file, output_file, show_move) :
	width, height = getSize(input_file)
	output = open(output_file, "w")
	output.write('''\
<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<!-- Generated by drawall.py, by Nathanaël Jourdane : http://drawall.cc -->

<svg xmlns="http://www.w3.org/2000/svg" width="''' + str(width) + '''mm" height="''' + str(height) + '''mm">
	<g style="fill:none" transform="scale(3.543307) matrix(1,0,0,-1,0,''' + str(height) + ''')">''')

	letter = 'L'
	switching = False
	line_count = 1;
	oldline = '0 0\n'
	
	for i, line in enumerate(open(input_file).readlines()) :
		line = re.sub('G0[01] (.*)', r'\1', line) # del function names
		
		if 'Z' in line : # get moving state
			z = gcodecleaner.getValue('Z', line)
			if z <= 0 and letter == 'M' :
				letter = 'L'
				switching = True
			elif z > 0 and letter == 'L' :
				letter = 'M'
				switching = True
			continue
		
		line = re.sub('[XY]', '', line) # del X and Y
		
		if switching or i == 0:
			color = '#000000'

			if letter == 'M' and show_move:
				color = '#0000FF'

			if line_count > 1:
				output.write('\t\t"/>')

			output.write('\n\n\t\t<path style="stroke:' + color + ';stroke-width:' + str(width/1000) + '" d="\n')
			output.write('\t\t\tM ' + oldline)

		if show_move:
			output.write('\t\t\tL ' + line)
		else:
			output.write('\t\t\t' + letter + ' ' + line)

		switching = False
		line_count += 1
		oldline = line

	output.write('"/>\n\t</g>\n</svg>')