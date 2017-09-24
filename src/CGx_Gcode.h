/*
  CGx_Gcode.h - Library for processing Gcode into actions of a 3-link robotic arm.
  Created by Eiji Onchi, September 24, 2017.
  Released into the public domain.
*/

#ifndef H_GCODE
#define H_GCODE

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <EEPROM.h>
#include "InverseK.h"

#define gcode_X_AXIS  0
#define gcode_Y_AXIS  1
#define gcode_Z_AXIS  2
#define gcode_FEED    3
#define gcode_GRIP    4
#define gcode_STEP    10
#define gcode_MAX_FEED 1000
#define gcode_MIN_RESOLUTION 0.0001

#define arm_STATE   0
#define arm_JOINT0  1
#define arm_JOINT1  2
#define arm_JOINT2  3
#define arm_JOINT3  4
#define arm_JOINT4  5
#define arm_JOINT5  6
#define arm_ATTACK  7
#define arm_OPEN    10
#define arm_CLOSE   73
#define arm_AVAILABLE 101
#define arm_STAND_BY 0

#define arm_MEM_90_0 0
#define arm_MEM_90_1 2
#define arm_MEM_90_2 4
#define arm_MEM_90_3 6
#define arm_MEM_90_4 8

#define arm_MEM_0_0 10
#define arm_MEM_0_1 12
#define arm_MEM_0_2 14
#define arm_MEM_0_3 16
#define arm_MEM_0_4 18

#define arm_MEM_10_5 20
#define arm_MEM_73_5 22

class _Gcode {
public:
	_Gcode();
	void decode(String code);
	void update();
	
private:
	float _arm_buffer[8];	// arm buffer
	float _arm_goal[6];
	float _arm_position[6];
	float _gcode_data[5];	// current data
	float _gcode_pdata[5];	// previous data
	long _pTime; // previous time
	long _cTime; // current time
	long _deltaT;	// delta time
	int _90_in_ms[6]; // 90 degrees in ms for servo calibration
	int _0_in_ms[6]; // 0 degrees in ms for servo calibration
	int _10_in_ms;
	int _73_in_ms;
	bool _arm_enabled; //TODO
	bool _calibrationMode;
	
	bool _isProcessing;
	
	Adafruit_PWMServoDriver pwm;
	
	bool _getValue(String code, char cmd, float& value);
	float _b2a(float degree);
	float _a2b(float angle);
	void _processSteps();
	int _b2p(int servo, float degree);
	void _braccioMove(float base, float shoulder, float elbow, float wrist, float hand, float grip);
	void _calibrate90(int servo, int value);
	void _calibrate0(int servo, int value);
	void _calibrate10(int value);
	void _calibrate73(int value);
	//void _checkArm();
};

extern _Gcode Gcode;
extern _Inverse InverseK;

#endif //H_GCODE