#include <stdint.h>

#include "plotter.h"

/// Conversion ratio: mm per motor step
#define stepLength (3.14159265358979323846 * (PLT_PINION_DIAMETER / 1000) \
		/ (PLT_STEPS << PLT_STEP_MODE))

typedef enum { LEFT, RIGHT } direction;
typedef struct { uint16_t x; uint16_t y; } dovPoint;
typedef struct { double x; double y; } mmPoint;

extern long beltLength[2];
extern unsigned int span;

int begin(void);
void end(void);
void setAngle(int angle);
void sleepMicros(uint32_t micros);
void step(direction dir, int pull);

