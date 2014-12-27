#include "drawall.h"
#include <avr/io.h>

void sleepMicros(uint32_t us) {
	while (TCNT1 < us);
	TCNT1 = 0;
}

#define BITSET(reg, bit, on) ((on) ? ((reg) |= (1 << (bit))) : ((reg) &= ~(1 << (bit))))
#define PINSET(pin, on) (BITSET(pin ## _PORT, pin ## _BIT, (on)))

static void pinInitialization() {
	// Set all pins to output mode
	DDRB = DDRC = DDRD = 0xFF;

#if EN_LIMIT_SENSORS
	pinMode(PIN_LEFT_CAPTOR, 0);
	pinMode(PIN_RIGHT_CAPTOR, 0);
	// Enable sensors internal pull-ups
	digitalWrite(PIN_LEFT_CAPTOR, 1);
	digitalWrite(PIN_RIGHT_CAPTOR, 1);
	// TODO: set sensors interrupt here
#endif

#if EN_REMOTE_SUPPORT
	pinMode(PIN_REMOTE, 0);
#endif

	// Timer
	BITSET(TCCR1B, CS10, 1);
	TCNT1 = 0;
}

int begin() {
	pinInitialization();
#if EN_STEP_MODES
	PORTD |= PLT_STEP_MODE << 2;
#endif
	PINSET(ENABLE_MOTORS, 0);
	return 1;
}

void setAngle(int angle) {
	(void) angle;
	/* servo.write(angle); */
}

/* File configFile = SD.open("config", FILE_READ); */
/* if (!configFile) { */
	/* error(ERR_FILE_NOT_READABLE); */
/* } */

void step(direction dir, int pull) {
	if ((dir == RIGHT) ^ PLT_REVERSE_MOTORS) {
		PINSET(RIGHT_MOTOR_DIR, pull ^ PLT_RIGHT_DIRECTION);
		PINSET(RIGHT_MOTOR_STEP, 0);
		PINSET(RIGHT_MOTOR_STEP, 1);
	} else {
		PINSET(LEFT_MOTOR_DIR, pull ^ PLT_LEFT_DIRECTION);
		PINSET(LEFT_MOTOR_STEP, 0);
		PINSET(LEFT_MOTOR_STEP, 1);
	}
}

void __attribute__((noreturn)) end() {
	// TODO ring buzzer
	setAngle(PLT_MAX_SERVO_ANGLE);
	PINSET(ENABLE_MOTORS, 1);
	for (;;);
}
