/*
 * This file is part of DraWall.
 * DraWall is free software: you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 * DraWall is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU
 * General Public License along with DraWall. If not, see <http://www.gnu.org/licenses/>.
 * © 2012–2014 Nathanaël Jourdane
 * © 2014 Victor Adam
 */

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "drawall.h"

#define LINE_MAX_LENGTH 32
#define NORM(x, y) (sqrt((x) * (x) + (y) * (y)))

/// Length of the belts, in motor steps.
long beltLength[2];

// mm
unsigned int span = 3000;

// mm / microseconds
static double speed = 0.00002;

// mm / DOV
static double drawingScale = .01;

// ms
static unsigned int initialDelay = 2000;

static mmPoint drawingPos = {100, 100};
static mmPoint initPos = {0, 0};
static mmPoint endPos = {0, 0};

// TODO: Interrupt routine with ISR(INT0_vect){...}

void setup(void);
void loop(void);

static mmPoint dovToMm(dovPoint p) {
	return (mmPoint) { p.x * drawingScale, p.y * drawingScale };
}

static mmPoint stepsToMm(long offsets[]) {
	double lbl = (beltLength[LEFT] + offsets[LEFT]) * stepLength;
	double rbl = (beltLength[RIGHT] + offsets[RIGHT]) * stepLength;
	double x = (lbl * lbl - rbl * rbl + span * span) / (double) (2 * span);
	double y = sqrt(lbl * lbl - x * x);
	return (mmPoint) { x - drawingPos.x, y - drawingPos.y };
}

static void writingPen(int shouldWrite) {
	// TODO multiply by drawingPenDep
	sleepMicros(PLT_PRE_SERVO_DELAY * 1000L);
	setAngle(shouldWrite ? PLT_MIN_SERVO_ANGLE : PLT_MAX_SERVO_ANGLE);
	sleepMicros(PLT_POST_SERVO_DELAY * 1000L);
}

static void loadParameters() {
	/*
	char buffer[LINE_MAX_LENGTH + 1];
	int c;
	int i = 0;

	// Until EOF
	while ((c = getchar()) >= 0) {
		if (c != '\n') {
			if (i >= LINE_MAX_LENGTH)
				end();
			buffer[i++] = (char) c;
			continue;
		}

		buffer[i] = '\0';
		for (i = 0; buffer[i] && buffer[i] != '='; i++);
		if (!buffer[i])
			end();
		buffer[i] = '\0';
		char *key = buffer;
		unsigned int value = (unsigned) atol(&buffer[i + 1]);

		if (!strcmp(key, "drawingPosX"))
			drawingPos.x = value;
		else if (!strcmp(key, "drawingPosY"))
			drawingPos.y = value;
		else if (!strcmp(key, "span"))
			span = value;
		else if (!strcmp(key, "initDelay"))
			initialDelay = value;
		else if (!strcmp(key, "speed"))
			speed = (double) value / 1000000;
		else if (!strcmp(key, "initPosX"))
			initPos.x = value;
		else if (!strcmp(key, "initPosY"))
			initPos.y = value;
		else if (!strcmp(key, "endPosX"))
			endPos.x = value;
		else if (!strcmp(key, "endPosY"))
			endPos.y = value;
		else
			end();
	}
	*/
}

static void lineTo(mmPoint dest) {
	mmPoint moves[9];
	double distances[9];
	double deviations[9];
	static long offsets[9][2] = {{0, 0},
		{1, 0}, {-1, 0}, {0, 1}, {0, -1},
		{1, -1}, {-1, 1}, {-1, -1}, {1, 1}};
	mmPoint pos = stepsToMm(offsets[0]);
	
	// The line’s equation is ax + by + c = 0
	double a = dest.y - pos.y, b = pos.x - dest.x;
	double c = dest.x * pos.y - dest.y * pos.x;
	double norm = NORM(a, b);
	a /= norm; b /= norm; c /= norm;

	printf("\nMOVE TO: %f, %f\n", dest.x, dest.y);
	printf("a=%f, b=%f, c=%f\n", a, b, c);

	for (int min = -1; min;) {
		for (int i = 0; i < 9; ++i) {
			moves[i] = stepsToMm(offsets[i]);
			distances[i] = NORM(dest.x - moves[i].x, dest.y - moves[i].y);
			deviations[i] = fabs(a * moves[i].x + b * moves[i].y + c);
		}
		min = 0;
		for (int i = 1; i < 9; ++i)
			if (deviations[i] < 4 * stepLength && distances[i] < distances[min])
				min = i;
		beltLength[LEFT] += offsets[min][LEFT];
		beltLength[RIGHT] += offsets[min][RIGHT];
		sleepMicros((unsigned) ((distances[0] - distances[min]) / speed));
		if (offsets[min][LEFT])
			step(LEFT, offsets[min][LEFT] < 0);
		if (offsets[min][RIGHT])
			step(RIGHT, offsets[min][RIGHT] < 0);
	}
}

static void draw() {
	// process line until we can read the file
	dovPoint point;
	fread(&point, sizeof(point), 1, stdin);
	if (point.x != 0x2339 || point.y != 0xFFAF)
		end();
	fread(&point, sizeof(point), 1, stdin);
	fread(&point, sizeof(point), 1, stdin);
	fread(&point, sizeof(point), 1, stdin);
	while (fread(&point, sizeof(point), 1, stdin)) {
		lineTo(dovToMm(point));
	}
}

static long motorSteps(mmPoint pos, direction dir) {
	double x = pos.x + drawingPos.x;
	double y = pos.y + drawingPos.y;
	if (dir == RIGHT)
		x = span - x;
	return (long) (NORM(x, y) / stepLength);
}

// TODO: Wait until start button is pressed.
// TODO: Wait until start signal is raised on the serial port.
void setup(void) {
	if (!begin())
		end();
	loadParameters();
	setAngle(PLT_MAX_SERVO_ANGLE);
	beltLength[LEFT]  = motorSteps(initPos, LEFT);
	beltLength[RIGHT] = motorSteps(initPos, RIGHT);
	sleepMicros(initialDelay * 1000L);
	draw();
	writingPen(0);
	lineTo(endPos);
	end();
}

void loop() {}

int main(void) {
	setup();
}
