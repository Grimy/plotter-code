#include "plotter.h"

/// Conversion ratio: mm per motor step
// PLT_STEPS * 2 because it is the rising edge which drive the motor steps.
#define PI 3.14159265358979323846
#define stepLength ((PI * PLT_PINION_DIAMETER / 1000) / (PLT_STEPS * 2 * (1 << PLT_STEP_MODE)))

typedef enum { LEFT, RIGHT } direction;
typedef struct { unsigned int x; unsigned int y; } dovPoint;
typedef struct { double x; double y; } mmPoint;

extern long beltLength[2];
extern unsigned int span;

int init(void);
void end(void);
void setAngle(int angle);
void delay(unsigned int micros);
void step(direction dir, int pull);

