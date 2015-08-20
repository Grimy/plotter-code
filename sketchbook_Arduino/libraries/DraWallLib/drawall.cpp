/*
 * This file is part of DraWall.
 * DraWall is free software: you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 * DraWall is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU
 * General Public License along with DraWall. If not, see <http://www.gnu.org/licenses/>.
 * © 2012–2015 Nathanaël Jourdane
 */

/**
 * Main library file.
 */

#include "drawall.h"

#ifndef EN_SERIAL
	#define serialBegin(x) ((void) 0)
	#define serialWrite(x) ((void) 0)
	#define serialPrint(x) ((void) 0)
	#define serialPrintln(x) ((void) 0)
#else
	#define serialBegin(x) Serial.begin(x)
	#define serialWrite(x) Serial.print(#x)
	#define serialPrint(x) Serial.print(x)
	#define serialPrintln(x) Serial.println(x)
#endif

#define STEP_LENGTH ((PI * PLT_PINION_DIAMETER / 1000) / (PLT_MOTORS_STEPS * 2 << PLT_STEP_MODE))

void Drawall::start() {
	pinInitialization();

	digitalWrite(PIN_ENABLE_MOTORS, LOW); // disable motors

	serialBegin(PLT_SERIAL_BAUDS);

	if (!SD.begin(PIN_SD_CS)) {
		error(ERR_CARD_NOT_FOUND);
	}

	// Load all parameters from the configuration file
	loadParameters();

	drawingInsertConf = drawingInsertConf/100.0*PLT_MAX_SERVO_ANGLE + PLT_MIN_SERVO_ANGLE;
	movingInsertConf = movingInsertConf/100.0*PLT_MAX_SERVO_ANGLE + PLT_MIN_SERVO_ANGLE;
	drawingScale = (sheetWidthConf > sheetHeightConf ? sheetWidthConf : sheetHeightConf) / 65535.0;

	servo.attach(PIN_SERVO);
	servo.write(movingInsertConf);
	currentServoAngle = movingInsertConf;

	isWriting = true; // to make write() works for the first time.

	// Get the belts length
	length[LEFT] = positionToLeftLength(initPosXConf, initPosYConf);
	length[RIGHT] = positionToRightLength(initPosXConf, initPosYConf);

	// Send initialization data to computer
	serialPrintln("READY");
	delay(100);
	serialWrite(DRAW_START_INSTRUCTIONS);
	serialPrintln(spanConf);
	serialPrintln(sheetPosXConf);
	serialPrintln(sheetPosYConf);
	serialPrintln(sheetWidthConf);
	serialPrintln(sheetHeightConf);
	serialPrintln(length[LEFT]);
	serialPrintln(length[RIGHT]);
	serialPrintln(STEP_LENGTH * 1000);
	serialWrite(DRAW_END_INSTRUCTIONS);
	serialWrite(DRAW_WAITING);

	while (digitalRead(PIN_PAUSE) == HIGH);
	digitalWrite(PIN_ENABLE_MOTORS, HIGH); // enable motors
	delay(PLT_ANTIBOUNCE_BUTTON_DELAY);

	// Draw area if long push
	if (digitalRead(PIN_PAUSE) == LOW) {
		delay(PLT_ANTIBOUNCE_BUTTON_DELAY);
		showArea();
		while (digitalRead(PIN_PAUSE) == HIGH);
	}

	delay(initDelayConf);
}

void Drawall::pinInitialization() {
	// Motors
	pinMode(PIN_ENABLE_MOTORS, OUTPUT);
	pinMode(PIN_MOTOR_STEP[LEFT], OUTPUT);
	pinMode(PIN_MOTOR_STEP[RIGHT], OUTPUT);
	pinMode(PIN_MOTOR_DIR[LEFT], OUTPUT);
	pinMode(PIN_MOTOR_DIR[RIGHT], OUTPUT);

	// Memory card
	pinMode(PIN_SD_CS, OUTPUT);

	// Pause button
	pinMode(PIN_PAUSE, INPUT);
	digitalWrite(PIN_PAUSE, HIGH);

#if EN_LIMIT_SENSORS
	pinMode(PIN_LEFT_CAPTOR, INPUT);
	pinMode(PIN_RIGHT_CAPTOR, INPUT);
	// Enable sensors internal pull-ups
	digitalWrite(PIN_LEFT_CAPTOR, HIGH);
	digitalWrite(PIN_RIGHT_CAPTOR, HIGH);
#endif

#if EN_REMOTE_SUPPORT
	pinMode(PIN_REMOTE, INPUT);
#endif

#if EN_SCREEN
	pinMode(PIN_SCREEN_SCE, OUTPUT);
	pinMode(PIN_SCREEN_RST, OUTPUT);
	pinMode(PIN_SCREEN_DC, OUTPUT);
	pinMode(PIN_SCREEN_SDIN, OUTPUT);
	pinMode(PIN_SCREEN_SCLK, OUTPUT);
#endif
}

// TODO use a Macro Expansion
float Drawall::positionToLeftLength(float posX, float posY) {
	float x = sheetPosXConf + posX;
	float y = sheetPosYConf + sheetHeightConf - posY;
	return sqrt(x*x + y*y) / STEP_LENGTH;
}

// TODO use a Macro Expansion
float Drawall::positionToRightLength(float posX, float posY) {
	float x = spanConf - sheetPosXConf - posX;
	float y = sheetPosYConf + sheetHeightConf - posY;
	return sqrt(x*x + y*y) / STEP_LENGTH;
}

void Drawall::writingPen(bool shouldWrite) {
	if (shouldWrite && !isWriting) {
		// If pen is not writing and should write

		delay(PLT_PRE_SERVO_DELAY);
		moveServo(drawingInsertConf);
		delay(PLT_POST_SERVO_DELAY);

		serialWrite(DRAW_WRITING);
		serialPrintln("\nDRAW_WRITING");
		isWriting = true;
	} else if (!shouldWrite && isWriting) {
		// If pen is writing and shouldn't

		delay(PLT_PRE_SERVO_DELAY);
		moveServo(movingInsertConf);
		delay(PLT_POST_SERVO_DELAY);
		serialWrite(DRAW_MOVING);
		serialPrintln("\nDRAW_MOVING");
		isWriting = false;
	}
}

void Drawall::step(Direction dir, bool pull) {
	// TODO digitalWrite() should be called only when the direction is changing
	length[dir] += pull ? -1 : 1;
	if (pulled[dir] != pull) {
		digitalWrite(PIN_MOTOR_DIR[dir], PLT_DIRECTION[dir] ^ pull);
		pulled[dir] = pull;
	}
	digitalWrite(PIN_MOTOR_STEP[dir], length[dir] % 2);
}

void Drawall::line(float x, float y) {
	writingPen(true);
	int longmax = 5;

	float longX = abs(x - plotterPosX);
	float longY = abs(y - plotterPosY);

	float miniX;
	float miniY;
	int boucle;

	if (longX > longmax || longY > longmax) {
		boucle = ceil((longX > longY ? longX : longY) / longmax);
		miniX = (x - plotterPosX) / boucle;
		miniY = (y - plotterPosY) / boucle;

		for (int i = 0; i < boucle; i++) {
			segment(plotterPosX + miniX, plotterPosY + miniY);
		}
	}
	segment(x, y);
}

void Drawall::move(float x, float y) {
	writingPen(false);
	segment(x, y);
}

void Drawall::processSDLine() {
#define PARAM_MAX_LENGTH 5
#define FUNC_NAME_MAX_LENGTH 2

	byte i, j;
	char functionName[FUNC_NAME_MAX_LENGTH + 1];
	char car;
	char parameter[PARAM_MAX_LENGTH + 1];
	unsigned int parameters[2];

	// Get function name
	car = file.read();

	if (car == ';') {
		while (file.read() != '\n');
		return;
	}

	for (i = 0; car != ' ' && car != '\n' && i < FUNC_NAME_MAX_LENGTH + 1;
			i++) {
		functionName[i] = car;
		car = file.read();
	}
	functionName[i] = '\0';

	// Get parameters
	// The first char has been already read.
	for (i = 0; car != '\n'; i++) {
		car = file.read(); // first parameter char (X, Y or Z)
		for (j = 0; car != ' ' && car != '\n' && j < PARAM_MAX_LENGTH + 1;
				j++) {
			if (j == 0) {
				car = file.read(); // pass first parameter char
			}
			parameter[j] = car;
			car = file.read();
		}
		parameter[j] = '\0';
		parameters[i] = atoi(parameter);
	}

	// Process the GCode function
	if (!strcmp(functionName, "G0")) {
		move(parameters[0], parameters[1]);
	} else if (!strcmp(functionName, "G1")) {
		line(parameters[0], parameters[1]);
	} else if (!strcmp(functionName, "G4")) {
		delay(1000 * parameters[0]);
	} else {
		warning(WARN_UNKNOWN_GCODE_FUNCTION);
	}
}

void Drawall::moveServo(unsigned int desiredAngle) {
	while (currentServoAngle != desiredAngle) {
		currentServoAngle += (desiredAngle > currentServoAngle ? 1 : -1);
		servo.write(currentServoAngle);
		delay(PLT_PAUSE_MOVING_SERVO);
	}
}

void Drawall::segment(float x, float y) {
	float target[2] = {drawingScale * positionToLeftLength(x, y), drawingScale * positionToRightLength(x, y)};
	float steps[2] = {target[LEFT] - length[LEFT], target[RIGHT] - length[RIGHT]};
	bool pull[2] = {steps[LEFT] < 0, steps[RIGHT] < 0};
	float interval[2];
	unsigned long time[2] = {micros(), micros()};

	// Since we have the direction, we can leave the sign
	steps[LEFT] = abs(steps[LEFT]);
	steps[RIGHT] = abs(steps[RIGHT]);

	if (steps[LEFT] > steps[RIGHT]) {
		interval[LEFT] = delayBetweenSteps;
		interval[RIGHT] = delayBetweenSteps * steps[LEFT] / steps[RIGHT];
	} else {
		interval[RIGHT] = delayBetweenSteps;
		interval[LEFT] = delayBetweenSteps * steps[RIGHT] / steps[LEFT];
	}

	while (steps[LEFT] > 0 || steps[RIGHT] > 0) {
		if (digitalRead(PIN_PAUSE) == LOW) {
			delay(PLT_ANTIBOUNCE_BUTTON_DELAY);
			while (digitalRead(PIN_PAUSE) == HIGH);
			delay(PLT_ANTIBOUNCE_BUTTON_DELAY);
		}

		if ((steps[LEFT] > 0) && (micros() - time[LEFT] >= interval[LEFT])) {
			time[LEFT] = micros();
			step(LEFT, pull[LEFT]);
			steps[LEFT]--;
		}

		if ((steps[RIGHT] > 0) && (micros() - time[RIGHT] >= interval[RIGHT])) {
			time[RIGHT] = micros();
			step(RIGHT, pull[RIGHT]);
			steps[RIGHT]--;
		}
	}

	plotterPosX = x;
	plotterPosY = y;
}

void Drawall::error(SerialData errorNumber) {
	serialPrint((byte) errorNumber);
	// TODO ring buzzer
	delay(1000);
	writingPen(false);
	while (true);
}

void Drawall::warning(SerialData warningNumber) {
	serialPrint((byte) warningNumber);
	// TODO ring buzzer
}

// TODO: do not use CardinalPoint
void Drawall::showArea() {
	file = SD.open(drawingNamesConf);

	if (!file) {
		error(ERR_FILE_NOT_FOUND);
	}

	moveServo(PLT_MAX_SERVO_ANGLE);

	segment(0, 0);
	delay(1000);
	segment(65535, 0);
	delay(1000);
	segment(65535, 65535);
	delay(1000);
	segment(0, 65535);
	delay(1000);
	segment(0, 0);

	drawingHeight = sheetHeightConf; // do not subtract the picture height

	file.close();
	serialPrint(DRAW_END_DRAWING);
}

void Drawall::draw() {
	char name[15];
	char car = '\0';
	int i = 0;
	int j = 0;
	int drawingNumber = 0;

	while (car != '\n') {
		car = drawingNamesConf[i++];
		if (car != ',' && car != '\0') {
			name[j++] = car;
		} else {
			name[j] = '\0';
			j = 0;
			
			// Wait for button press before to start the next drawing
			drawingNumber++;

			if (drawingNumber > 1) {
				while (digitalRead(PIN_PAUSE) == HIGH);
				delay(initDelayConf);
			}
			draw(name);
			
			moveServo(PLT_MAX_SERVO_ANGLE);
			segment(endPosXConf/drawingScale, endPosYConf/drawingScale);
			// TODO ring buzzer
		}
	}
	end();
}

void Drawall::draw(char* drawingName) {
	file = SD.open(drawingName);
	if (!file) {
		error(ERR_FILE_NOT_FOUND);
	}

	// process line until we can read the file
	while (file.available()) {
		processSDLine();
	}

	drawingHeight = sheetHeightConf; // do not subtract the picture height

	file.close();
	serialPrint(DRAW_END_DRAWING);
}

void Drawall::end() {
	digitalWrite(PIN_ENABLE_MOTORS, LOW); // disable motors
	while (true);
}

void Drawall::message(char* message) {
	serialWrite(DRAW_START_MESSAGE);
	serialPrintln(message);
	serialWrite(DRAW_END_MESSAGE);
}

void Drawall::loadParameters() {
#define LINE_MAX_LENGTH 50
#define NB_PARAMETERS 17

	char buffer[LINE_MAX_LENGTH + 1];
	char *key;
	char *value;

	byte i;
	int nb_parsed = 0;

	// Check if the file exists
	// TODO Something wrong here with serial communication
	// if (!SD.exists(fileName)) {
	// 	error(ERR_FILE_DONT_EXISTS);
	// }

	File configFile = SD.open(CONFIG_FILE_NAME, FILE_READ);
	if (!configFile) {
		error(ERR_FILE_NOT_READABLE);
	}

	// Until the EOF is not reached
	while (configFile.available() > 0) {
		// Store the full line in buffer
		i = 0;
		while ((buffer[i] = configFile.read()) != '\n') {
			if (i == LINE_MAX_LENGTH) {
				configFile.close();
				error(ERR_TOO_LONG_CONFIG_LINE);
			}
			i++;
		}
		buffer[i] = '\0';

		// Ignore empty or commented lines
		if (buffer[0] == '\0' || buffer[0] == '#') {
			continue;
		}

		for (i = 0; buffer[i] != '='; i++) {
			if (buffer[i] == '\0') {
				error(ERR_WRONG_CONFIG_LINE);
			}
		}
		key = &buffer[0];
		key[i] = '\0';
		value = &buffer[i + 1];

		serialPrint(key);
		serialPrint("=");
		serialPrintln(value);

		// TODO use constants for the parameters names

		// * Convert text data into usable data *

		if (!strcmp(key, "drawingNames")) {
			strcpy(drawingNamesConf, value);
		} else if (!strcmp(key, "drawingWidth")) {
			drawingWidthConf = atoi(value);
			// note : never used yet.
		} else if (!strcmp(key, "drawingPosX")) {
			drawingPosXConf = atoi(value);
		} else if (!strcmp(key, "drawingPosY")) {
			drawingPosYConf = atoi(value);
		} else if (!strcmp(key, "span")) {
			spanConf = atoi(value);
		} else if (!strcmp(key, "initDelay")) {
			initDelayConf = atoi(value);
		} else if (!strcmp(key, "maxSpeed")) {
			delayBetweenSteps = 1E6 * STEP_LENGTH / atof(value);
		} else if (!strcmp(key, "sheetWidth")) {
			sheetWidthConf = atoi(value);
		} else if (!strcmp(key, "sheetHeight")) {
			sheetHeightConf = atoi(value);
		} else if (!strcmp(key, "sheetPosX")) {
			sheetPosXConf = atoi(value);
		} else if (!strcmp(key, "sheetPosY")) {
			sheetPosYConf = atoi(value);
		} else if (!strcmp(key, "initPosX")) {
			initPosXConf = atoi(value);
		} else if (!strcmp(key, "initPosY")) {
			initPosYConf = atoi(value);
		} else if (!strcmp(key, "endPosX")) {
			endPosXConf = atoi(value);
		} else if (!strcmp(key, "endPosY")) {
			endPosYConf = atoi(value);
		} else if (!strcmp(key, "drawingInsert")) {
			drawingInsertConf = atoi(value);
		} else if (!strcmp(key, "movingInsert")) {
			movingInsertConf = atoi(value);
		} else {
			warning(ERR_UNKNOWN_CONFIG_KEY);
		}
		nb_parsed++;
	}

	configFile.close();

	if (nb_parsed < NB_PARAMETERS)
		error(ERR_TOO_FEW_PARAMETERS);
	if (nb_parsed > NB_PARAMETERS)
		error(ERR_TOO_MANY_PARAMETERS);
}
