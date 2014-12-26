
static void pinInitialization() {
	// Motors

	pinMode(PIN_ENABLE_MOTORS, OUTPUT);

	pinMode(PIN_LEFT_MOTOR_STEP, OUTPUT);
	pinMode(PIN_RIGHT_MOTOR_STEP, OUTPUT);

	pinMode(PIN_LEFT_MOTOR_DIR, OUTPUT);
	pinMode(PIN_RIGHT_MOTOR_DIR, OUTPUT);

#if EN_STEP_MODES
	pinMode(PIN_STEP_MODE_0, OUTPUT);
	pinMode(PIN_STEP_MODE_1, OUTPUT);
	pinMode(PIN_STEP_MODE_2, OUTPUT);
#endif

	// Memory card
	pinMode(PIN_SD_CS, OUTPUT);

#if EN_LIMIT_SENSORS
	// Limit sensors

	pinMode(PIN_LEFT_CAPTOR, INPUT);
	pinMode(PIN_RIGHT_CAPTOR, INPUT);

	// Enable sensors internal pull-ups
	digitalWrite(PIN_LEFT_CAPTOR, HIGH);
	digitalWrite(PIN_RIGHT_CAPTOR, HIGH);
	// TODO: set sensors interrupt here
#endif

#if EN_REMOTE_SUPPORT
	pinMode(PIN_REMOTE, INPUT);
#endif

#if EN_SCREEN
	// Screen
	pinMode(PIN_SCREEN_SCE, OUTPUT);
	pinMode(PIN_SCREEN_RST, OUTPUT);
	pinMode(PIN_SCREEN_DC, OUTPUT);
	pinMode(PIN_SCREEN_SDIN, OUTPUT);
	pinMode(PIN_SCREEN_SCLK, OUTPUT);
#endif
}

int init() {
#if EN_SERIAL
	Serial.begin(SERIAL_BAUDS);
#endif
	if (!SD.begin(PIN_SD_CS))
		return 0;
	pinInitialization();
	servo.attach(PIN_SERVO);
#if EN_STEP_MODES
	digitalWrite(PIN_STEP_MODE_0, (PLT_STEP_MODE & B1) > 0 ? HIGH : LOW);
	digitalWrite(PIN_STEP_MODE_1, (PLT_STEP_MODE & B10) > 0 ? HIGH : LOW);
	digitalWrite(PIN_STEP_MODE_2, (PLT_STEP_MODE & B100) > 0 ? HIGH : LOW);
#endif
	power(true);
	return 1;
}

extern void delay(int ms);

void setAngle(int angle) {
	servo.write(angle);
}

File configFile = SD.open(CONFIG_FILE_NAME, FILE_READ);
if (!configFile) {
	error(ERR_FILE_NOT_READABLE);
}

void leftStep(bool shouldPull) {
	digitalWrite(PLT_REVERSE_MOTORS ? PIN_RIGHT_MOTOR_STEP : PIN_LEFT_MOTOR_STEP, leftLength % 2);
}

void rightStep(bool shouldPull) {
	digitalWrite(PLT_REVERSE_MOTORS ? PIN_LEFT_MOTOR_STEP : PIN_RIGHT_MOTOR_STEP, rightLength % 2);
}

void __attribute__((noreturn)) end() {
	// TODO ring buzzer
	setAngle(PLT_MAX_SERVO_ANGLE);
	digitalWrite(PIN_ENABLE_MOTORS, shouldPower ? LOW : HIGH);
	for (;;);
}

void setDir() {
	digitalWrite(PLT_REVERSE_MOTORS ? PIN_RIGHT_MOTOR_DIR : PIN_LEFT_MOTOR_DIR,
			pullLeft ? PLT_LEFT_DIRECTION : !PLT_LEFT_DIRECTION);

	digitalWrite(PLT_REVERSE_MOTORS ? PIN_LEFT_MOTOR_DIR : PIN_RIGHT_MOTOR_DIR,
			pullRight ? PLT_RIGHT_DIRECTION : !PLT_RIGHT_DIRECTION);
}
