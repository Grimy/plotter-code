/*
 * This file is part of DraWall, a vertical plotter (aka drawbot) - see http://drawall.cc
 * Drawall is free software and licenced under GNU GPL v3 : http://www.gnu.org/licenses/
 * Copyright (c) 2012-2015 Nathanaël Jourdane
 * 
 * This program tests the motors,
 * by rotating them in both directions and in all step modes.
 * You need to use a DRV8824 (or equivalent) motor driver.
 * See https://www.pololu.com/product/2131 for wiring.
 * You can open the serial monitor window to get informations during execution.
 *
 * To change pins allocation or other parameters,
 * edit pins.h or hardware.h files in the DraWallParameters folder.
 */

// Include parameter files.
#include "SD.h"
#include "Servo.h"
#include "SPI.h"
#include "drawall.h"

#define SPEED_RPM 30
#define STEPS (PLT_MOTORS_STEPS * SPEED_RPM / 60 * 2 * 32)
#define DELAY (1E6 / STEPS)

Drawall d;

void setup() {
	Serial.begin(PLT_SERIAL_BAUDS);
	pinMode(PIN_MOTOR_DIR[LEFT], OUTPUT);
	pinMode(PIN_MOTOR_STEP[LEFT], OUTPUT);
	pinMode(PIN_MOTOR_DIR[RIGHT], OUTPUT);
	pinMode(PIN_MOTOR_STEP[RIGHT], OUTPUT);
}

void move(Direction dir, int pull) {
	digitalWrite(PIN_MOTOR_DIR[dir], pull);
	for (long i = 0 ; i < STEPS; ++i) {
		digitalWrite(PIN_MOTOR_STEP[dir], i % 2);
		// d.step(dir, pull);
		delayMicroseconds(DELAY);
	}
}

void loop() {
	move(LEFT, HIGH);
	move(LEFT, LOW);
	move(RIGHT, HIGH);
	move(RIGHT, LOW);
}
