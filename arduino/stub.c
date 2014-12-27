#include <stdio.h>
#include <math.h>

#include "drawall.h"

static void printPos() {
	double lbl = beltLength[LEFT] * stepLength;
	double rbl = beltLength[RIGHT] * stepLength;
	double x = (lbl * lbl - rbl * rbl + span * span) / (double) (2 * span);
	double y = sqrt(lbl * lbl - x * x);
	printf("%f, %f\n", x, y);
}

int begin(void) {
	puts("init\n");
	return 1;
}

void end(void) {
	puts("end\n");
}

void setAngle(int angle) {
	printf("Angle: %d\n", angle);
}

void sleepMicros(uint32_t micros) {
	(void) micros;
	/* printf("Delay: %d\n", micros); */
}

static int blah = 0;

void step(direction dir, int pull) {
	(void) dir;
	(void) pull;
	if (++blah % 9999 == 0)
		printPos();
}
