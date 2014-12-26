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
#define max(a, b) ((a) > (b) ? (a) : (b))
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

static mmPoint stepsToMm(long offsets[static 2]) {
	double lbl = (beltLength[LEFT] + offsets[LEFT]) * stepLength;
	double rbl = (beltLength[RIGHT] + offsets[RIGHT]) * stepLength;
	double x = (lbl * lbl - rbl * rbl + span * span) / (double) (2 * span);
	double y = sqrt(max(0, lbl * lbl - x * x));
	return (mmPoint) { x - drawingPos.x, y - drawingPos.y };
}

static double distance(mmPoint a, mmPoint b) {
	return sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}

/* static double distance(mmPoint dest, long offsets[static 2]) { */
	/* double lbl = (beltLength[LEFT] + offsets[LEFT]) * stepLength; */
	/* double rbl = (beltLength[RIGHT] + offsets[RIGHT]) * stepLength; */
	/* double x = (lbl * lbl - rbl * rbl + span * span) / (double) (2 * span); */
	/* double y = sqrt(max(0, lbl * lbl - x * x)); */
	/* x -= drawingPos.x + dest.x; */
	/* y -= drawingPos.y + dest.y; */
	/* return sqrt(x * x + y * y); */
/* } */

static void writingPen(int shouldWrite) {
	// TODO multiply by drawingPenDep
	delay(PLT_PRE_SERVO_DELAY * 1000);
	setAngle(shouldWrite ? PLT_MIN_SERVO_ANGLE : PLT_MAX_SERVO_ANGLE);
	delay(PLT_POST_SERVO_DELAY * 1000);
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
	double distances[4];
	static long offsets[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
	mmPoint pos = stepsToMm(offsets[0]);
	int prevMin = -1;
	
	// The line’s equation is ax + by + c = 0
	double a = dest.y - pos.y, b = dest.x - pos.x;
	double c = dest.x * pos.y - dest.y * pos.x;
	double norm = sqrt(a * a + b * b);
	a /= norm; b /= norm; c /= norm;

	printf("\nMOVE TO: %f, %f\n", dest.x, dest.y);

	for (;;) {
		for (int i = 0; i < 4; ++i) {
			mmPoint new = stepsToMm(offsets[i]);
			distances[i] = distance(dest, new) + fabs(a * new.x + b * new.y + c);
		}
		int min = 0;
		for (int i = 1; i < 4; ++i)
			if (distances[i] < distances[min])
				min = i;
		if (min == (prevMin ^ 1))
			break;
		prevMin = min;
		beltLength[LEFT] += offsets[min][LEFT];
		beltLength[RIGHT] += offsets[min][RIGHT];
		delay((unsigned) ((distances[0] - distances[min]) / speed));
		direction dir = offsets[min][LEFT] ? LEFT : RIGHT;
		step(dir, offsets[min][dir] < 0);
	}
}

static int readPoint(dovPoint *p) {
	int x = getchar();
	if (x < 0)
		return 0;
	x = x << 8 | getchar();
	int y = getchar();
	if (y < 0)
		return 0;
	y = y << 8 | getchar();
	p->x = (unsigned) x;
	p->y = (unsigned) y;
	return 1;
}

static void draw() {
	// process line until we can read the file
	dovPoint point;
	readPoint(&point);
	if (point.x != 0x2339 || point.y != 0xFFAF)
		end();
	readPoint(&point);
	readPoint(&point);
	readPoint(&point);
	while (readPoint(&point)) {
		lineTo(dovToMm(point));
	}
}

static long motorSteps(mmPoint pos, direction dir) {
	double x = pos.x + drawingPos.x;
	double y = pos.y + drawingPos.y;
	if (dir == RIGHT)
		x = span - x;
	return (long) (sqrt(x * x + y * y) / stepLength);
}

// TODO: Wait until start button is pressed.
// TODO: Wait until start signal is raised on the serial port.
void setup(void) {
	if (!init())
		end();
	loadParameters();
	setAngle(PLT_MAX_SERVO_ANGLE);
	beltLength[LEFT]  = motorSteps(initPos, LEFT);
	beltLength[RIGHT] = motorSteps(initPos, RIGHT);
	delay(initialDelay * 1000);
	draw();
	writingPen(0);
	lineTo(endPos);
	end();
}

void loop() {}

int main(void) {
	setup();
}
