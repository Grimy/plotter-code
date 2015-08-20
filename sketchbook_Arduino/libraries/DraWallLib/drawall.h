#ifndef _H_DRAWALL
#define _H_DRAWALL

#include <math.h>
#include <SD.h>
#include <Servo.h>
#include <Arduino.h>

#define CONFIG_FILE_NAME "config"

#define PIN_ENABLE_MOTORS 2
#define PIN_SERVO 3
#define PIN_MOTOR_DIR  ((int[]) {/*LEFT*/ 4, /*RIGHT*/ 6})
#define PIN_MOTOR_STEP ((int[]) {/*LEFT*/ 5, /*RIGHT*/ 7})
#define PIN_RIGHT_MOTOR_STEPS 7
#define PIN_BUZZER 8
#define PIN_SD_CD 9
#define PIN_SD_CS 10
#define PIN_PAUSE 16
#define PIN_LIMIT_SWITCHES 17

#define PLT_SENSOR_LIMIT 1500
#define PLT_SERIAL_BAUDS 57600
#define PLT_MOTORS_STEPS 200
#define PLT_STEP_MODE 5
#define PLT_PINION_DIAMETER 12730
#define PLT_DIRECTION ((int[]) {/*LEFT*/ 1, /*RIGHT*/ 0})
#define PLT_MIN_SERVO_ANGLE 0
#define PLT_MAX_SERVO_ANGLE 180
#define PLT_PRE_SERVO_DELAY 100
#define PLT_PAUSE_MOVING_SERVO 15
#define PLT_POST_SERVO_DELAY 100
#define PLT_ANTIBOUNCE_BUTTON_DELAY 1000

#define EN_DEBUG 0
#define EN_BUZZER 0
#define EN_PAUSE_BUTTON 0
#define EN_STEP_MODES 1
#define EN_LIMIT_SENSORS 0
#define EN_REMOTE_SUPPORT 0
#define EN_SCREEN 0
#define EN_SERIAL 1

typedef enum {LEFT, RIGHT} Direction;

typedef enum {
	DRAW_START_INSTRUCTIONS, ///< 0. Beginning of the initialization data;
	DRAW_END_INSTRUCTIONS,   ///< 1. End of the initialization data;
	DRAW_RELEASE_LEFT,       ///< 2. Release the left belt for one step;
	DRAW_PULL_LEFT,          ///< 3. Pull the left belt for one step;
	DRAW_RELEASE_RIGHT,      ///< 4. Release the left belt for one step;
	DRAW_PULL_RIGHT,         ///< 5. Pull the left belt for one step;
	DRAW_WRITING,            ///< 6. The plotter is drawing;
	DRAW_MOVING,             ///< 7. The plotter is moving (dot drawing);
	DRAW_WAITING,            ///< 8. The plotter waiting for something;
	DRAW_ENABLE_MOTORS,      ///< 9. Enable the motors and the servo;
	DRAW_DISABLE_MOTORS,     ///< 10. Disable the motors and the servo;
	DRAW_CHANGE_TOOL,        ///< 11. Pause the program to change the tool;
	DRAW_END_DRAWING,        ///< 12. The drawing is finished;
	DRAW_START_MESSAGE,      ///< 13. Beginning of the message to display on the screen (if any) or/and on the computer (if any);
	DRAW_END_MESSAGE,        ///< 14. End of the message to display on the screen (if any) or/and on the computer (if any);
	ERR_CARD_NOT_FOUND,      ///< 15. The SD card is not found or not readable;
	ERR_FILE_NOT_FOUND,      ///< 16. The file not exists;
	ERR_FILE_NOT_READABLE,   ///< 17. Error while opening the file;
	ERR_TOO_FEW_PARAMETERS,  ///< 18. One or several parameters have not been read;
	ERR_TOO_MANY_PARAMETERS, ///< 19. Too many parameters have been read;
	ERR_WRONG_CONFIG_LINE,   ///< 20. Incorrectly formatted line in the configuration file;
	ERR_TOO_LONG_CONFIG_LINE,///< 21. Too long line in the configuration file;
	ERR_UNKNOWN_CONFIG_KEY,  ///< 22. Unknown key in the configuration file;
	WARN_UNKNOWN_GCODE_FUNCTION, ///< 23. Unknown GCode function in the drawing file;
	WARN_UNKNOWN_GCODE_PARAMETER,///< 24. Unknown GCode parameter;
} SerialData;

class Drawall {
public:
	Servo servo;
	unsigned int currentServoAngle;
	File file;
	int pulled[2] = {-1, -1}; // 1/0: last move was/wasn’t a pull, -1: no last move
	unsigned long length[2];
	float drawingScale;
	int drawingWidth;
	int drawingHeight;
	float delayBetweenSteps;
	bool isWriting;
	float plotterPosX;
	float plotterPosY;
	char drawingNamesConf[40];
	unsigned int drawingWidthConf;
	unsigned int drawingPosXConf;
	unsigned int drawingPosYConf;
	unsigned int spanConf;
	unsigned int initDelayConf;
	unsigned int maxSpeedConf;
	unsigned int sheetWidthConf;
	unsigned int sheetHeightConf;
	unsigned int sheetPosXConf;
	unsigned int sheetPosYConf;
	unsigned int drawingInsertConf;
	unsigned int movingInsertConf;
	int initPosXConf;
	int initPosYConf;
	int endPosXConf;
	int endPosYConf;

	void start();
	void pinInitialization();
	void end();
	float getDelay(unsigned int speed);
	void move(float posX, float posY);
	void line(float x, float y);
	void moveServo(unsigned int angle);
	void showArea();
	void draw();
	void draw(char* drawingName);
	void error(SerialData errorNumber);
	void warning(SerialData warningNumber);
	void step(Direction dir, bool shouldPull);
	float positionToLeftLength(float x, float y);
	float positionToRightLength(float x, float y);
	void sdInit(char *fileName);
	void processSDLine();
	void segment(float x, float y);
	void writingPen(bool shouldWrite);
	void message(char* message);
	void loadParameters();
};

#endif
