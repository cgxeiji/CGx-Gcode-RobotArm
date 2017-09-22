#include "braccio_gcode.h"

_Gcode Gcode;

_Gcode::_Gcode() {
	_isProcessing = false;
	
	_90_in_ms[0] = (int)EEPROM.read(arm_MEM_90_0) | (int)EEPROM.read(arm_MEM_90_0+1)<<8; //347;
	_90_in_ms[0] = ((_90_in_ms[0] > 80) && (_90_in_ms[0] < 800)) ? _90_in_ms[0] : 350;
	_90_in_ms[1] = (int)EEPROM.read(arm_MEM_90_1) | (int)EEPROM.read(arm_MEM_90_1+1)<<8; //360;
	_90_in_ms[1] = ((_90_in_ms[1] > 80) && (_90_in_ms[1] < 800)) ? _90_in_ms[1] : 350;
	_90_in_ms[2] = (int)EEPROM.read(arm_MEM_90_2) | (int)EEPROM.read(arm_MEM_90_2+1)<<8; //377;
	_90_in_ms[2] = ((_90_in_ms[2] > 80) && (_90_in_ms[2] < 800)) ? _90_in_ms[2] : 350;
	_90_in_ms[3] = (int)EEPROM.read(arm_MEM_90_3) | (int)EEPROM.read(arm_MEM_90_3+1)<<8; //360;
	_90_in_ms[3] = ((_90_in_ms[3] > 80) && (_90_in_ms[3] < 800)) ? _90_in_ms[3] : 350;
	_90_in_ms[4] = (int)EEPROM.read(arm_MEM_90_4) | (int)EEPROM.read(arm_MEM_90_4+1)<<8; //318;
	_90_in_ms[4] = ((_90_in_ms[4] > 80) && (_90_in_ms[4] < 800)) ? _90_in_ms[4] : 350;
	
	_0_in_ms[0] = (int)EEPROM.read(arm_MEM_0_0) | (int)EEPROM.read(arm_MEM_0_0+1)<<8;
	_0_in_ms[0] = ((_0_in_ms[0] > 80) && (_0_in_ms[0] < 800)) ? _0_in_ms[0] : 580;
	_0_in_ms[1] = (int)EEPROM.read(arm_MEM_0_1) | (int)EEPROM.read(arm_MEM_0_1+1)<<8;
	_0_in_ms[1] = ((_0_in_ms[1] > 80) && (_0_in_ms[1] < 800)) ? _0_in_ms[1] : 580;
	_0_in_ms[2] = (int)EEPROM.read(arm_MEM_0_2) | (int)EEPROM.read(arm_MEM_0_2+1)<<8;
	_0_in_ms[2] = ((_0_in_ms[2] > 80) && (_0_in_ms[2] < 800)) ? _0_in_ms[2] : 580;
	_0_in_ms[3] = (int)EEPROM.read(arm_MEM_0_3) | (int)EEPROM.read(arm_MEM_0_3+1)<<8;
	_0_in_ms[3] = ((_0_in_ms[3] > 80) && (_0_in_ms[3] < 800)) ? _0_in_ms[3] : 580;
	_0_in_ms[4] = (int)EEPROM.read(arm_MEM_0_4) | (int)EEPROM.read(arm_MEM_0_4+1)<<8;
	_0_in_ms[4] = ((_0_in_ms[4] > 80) && (_0_in_ms[4] < 800)) ? _0_in_ms[4] : 580;
	
	_10_in_ms = (int)EEPROM.read(arm_MEM_10_5) | (int)EEPROM.read(arm_MEM_10_5+1)<<8;
	_10_in_ms = ((_10_in_ms > 80) && (_10_in_ms < 800)) ? _10_in_ms : 215;
	
	_73_in_ms = (int)EEPROM.read(arm_MEM_73_5) | (int)EEPROM.read(arm_MEM_73_5+1)<<8;
	_73_in_ms = ((_73_in_ms > 80) && (_73_in_ms < 800)) ? _73_in_ms : 365;
	
	_arm_buffer[arm_ATTACK] = FREE_ANGLE;
	
	_calibrationMode = false;
	
#ifdef CGX_GCODE_DEBUG
	pwm = Adafruit_PWMServoDriver();
	
	// TODO: test the pwm
	pwm.begin();
	pwm.setPWMFreq(60);
#endif
}

void _Gcode::_calibrate90(int servo, int value) {
	_90_in_ms[servo] = value;
	
	int addr = arm_MEM_90_0 + servo*2;
	EEPROM.write(addr, (byte)(value & 0xFF));
	EEPROM.write(addr+1, (byte)((value >> 8) & 0xFF));
}

void _Gcode::_calibrate0(int servo, int value) {
	_0_in_ms[servo] = value;
	
	int addr = arm_MEM_0_0 + servo*2;
	EEPROM.write(addr, (byte)(value & 0xFF));
	EEPROM.write(addr+1, (byte)((value >> 8) & 0xFF));
}

void _Gcode::_calibrate10(int value) {
	_10_in_ms = value;
	
	int addr = arm_MEM_10_5;
	EEPROM.write(addr, (byte)(value & 0xFF));
	EEPROM.write(addr+1, (byte)((value >> 8) & 0xFF));
}

void _Gcode::_calibrate73(int value) {
	_73_in_ms = value;
	
	int addr = arm_MEM_73_5;
	EEPROM.write(addr, (byte)(value & 0xFF));
	EEPROM.write(addr+1, (byte)((value >> 8) & 0xFF));
}

void _Gcode::decode(String code) {
	float value;
  
	if (_getValue(code, 'G', value)) {
		int type = (int)value;
		switch(type) {
		case 1: 
			_getValue(code, 'F', _gcode_pdata[gcode_FEED]);
			_gcode_data[gcode_FEED] = _gcode_pdata[gcode_FEED];
			
			_getValue(code, 'X', _gcode_data[gcode_X_AXIS]);
			_getValue(code, 'Y', _gcode_data[gcode_Y_AXIS]);
			_getValue(code, 'Z', _gcode_data[gcode_Z_AXIS]);
			if (_getValue(code, 'W', _arm_buffer[arm_ATTACK])) {
				if (_arm_buffer[arm_ATTACK] != 999.9) {
					_arm_buffer[arm_ATTACK] *= DEGREE_STEP;
				} else {
					_arm_buffer[arm_ATTACK] = FREE_ANGLE;
				}
			} 
			break;
		case 0:
			_gcode_data[gcode_FEED] = gcode_MAX_FEED;
			_getValue(code, 'X', _gcode_data[gcode_X_AXIS]);
			_getValue(code, 'Y', _gcode_data[gcode_Y_AXIS]);
			_getValue(code, 'Z', _gcode_data[gcode_Z_AXIS]);
			if (_getValue(code, 'W', _arm_buffer[arm_ATTACK])) {
				if (_arm_buffer[arm_ATTACK] != 999.9) {
					_arm_buffer[arm_ATTACK] *= DEGREE_STEP;
				} else {
					_arm_buffer[arm_ATTACK] = FREE_ANGLE;
				}
			} 
			break;
		case 28:
			_gcode_data[gcode_FEED] = 50;
			_gcode_data[gcode_X_AXIS] = 195;
			_gcode_data[gcode_Y_AXIS] = 0;
			_gcode_data[gcode_Z_AXIS] = 50;
			_arm_buffer[arm_ATTACK] = -45*DEGREE_STEP;
			break;
		case 53:
			if (_getValue(code, 'A', _arm_buffer[arm_JOINT0])) _arm_buffer[arm_JOINT0] = _b2a(_arm_buffer[arm_JOINT0]);
			if (_getValue(code, 'B', _arm_buffer[arm_JOINT1])) _arm_buffer[arm_JOINT1] = _b2a(_arm_buffer[arm_JOINT1]);
			if (_getValue(code, 'C', _arm_buffer[arm_JOINT2])) _arm_buffer[arm_JOINT2] = _b2a(_arm_buffer[arm_JOINT2]);
			if (_getValue(code, 'D', _arm_buffer[arm_JOINT3])) _arm_buffer[arm_JOINT3] = _b2a(_arm_buffer[arm_JOINT3]);
			if (_getValue(code, 'E', _arm_buffer[arm_JOINT4]));
			if (_getValue(code, 'F', _arm_buffer[arm_JOINT5]));
			
			_arm_buffer[arm_STATE] = arm_AVAILABLE;
			break;
		case 54:
			if (_calibrationMode) {
				float value = 0;
				#ifdef CGX_GCODE_DEBUG
				if (_getValue(code, 'A', value)) pwm.setPWM(0, 0, value);
				if (_getValue(code, 'B', value)) pwm.setPWM(1, 0, value);
				if (_getValue(code, 'C', value)) pwm.setPWM(2, 0, value);
				if (_getValue(code, 'D', value)) pwm.setPWM(3, 0, value);
				if (_getValue(code, 'E', value)) pwm.setPWM(4, 0, value);
				if (_getValue(code, 'F', value)) pwm.setPWM(5, 0, value);
				#endif
			}
			break;
		}
	} else if (_getValue(code, 'M', value)) {
		int type = (int)value;
		float calibration;
		
		switch(type) {
		case 100: 
			_gcode_data[gcode_GRIP] = arm_CLOSE;
			break;
		case 101:
			_gcode_data[gcode_GRIP] = arm_OPEN;
			break;
		case 103:
			_calibrationMode = true;
			break;
		case 104:
			_calibrationMode = false;
			break;
		case 105: // Go to 90 degrees
			_arm_buffer[arm_JOINT0] = _b2a(90);
			_arm_buffer[arm_JOINT1] = _b2a(90); 
			_arm_buffer[arm_JOINT2] = _b2a(90); 
			_arm_buffer[arm_JOINT3] = _b2a(90);
			_arm_buffer[arm_JOINT4] = 90;
			
			_gcode_pdata[gcode_X_AXIS] = 0;
			_gcode_pdata[gcode_Y_AXIS] = 0;
			_gcode_pdata[gcode_Z_AXIS] = 300;
			
			_gcode_data[gcode_X_AXIS] = 0;
			_gcode_data[gcode_Y_AXIS] = 0;
			_gcode_data[gcode_Z_AXIS] = 300;
			
			_gcode_data[gcode_GRIP] = arm_OPEN;
			
			_arm_buffer[arm_STATE] = arm_AVAILABLE;
			break;
		case 106:
			if (_calibrationMode) {
				if (_getValue(code, 'A', calibration)) _calibrate90(0, calibration);
				if (_getValue(code, 'B', calibration)) _calibrate90(1, calibration);
				if (_getValue(code, 'C', calibration)) _calibrate90(2, calibration);
				if (_getValue(code, 'D', calibration)) _calibrate90(3, calibration);
				if (_getValue(code, 'E', calibration)) _calibrate90(4, calibration);
				if (_getValue(code, 'F', calibration)) _calibrate10(calibration);
			}
			break;
		case 107: // Go to 90 degrees
			_arm_buffer[arm_JOINT0] = _b2a(180);
			_arm_buffer[arm_JOINT1] = _b2a(45); 
			_arm_buffer[arm_JOINT2] = _b2a(180); 
			_arm_buffer[arm_JOINT3] = _b2a(0);
			_arm_buffer[arm_JOINT4] = 0;
			
			_gcode_pdata[gcode_X_AXIS] = 0;
			_gcode_pdata[gcode_Y_AXIS] = 0;
			_gcode_pdata[gcode_Z_AXIS] = 300;
			
			_gcode_data[gcode_X_AXIS] = 0;
			_gcode_data[gcode_Y_AXIS] = 0;
			_gcode_data[gcode_Z_AXIS] = 300;
			
			_gcode_data[gcode_GRIP] = arm_CLOSE;
			
			_arm_buffer[arm_STATE] = arm_AVAILABLE;
			break;
		case 108:
			if (_calibrationMode) {
				if (_getValue(code, 'A', calibration)) _calibrate0(0, _90_in_ms[0] * 2 - calibration);
				if (_getValue(code, 'B', calibration)) _calibrate0(1, calibration * 2 - _90_in_ms[1]);
				if (_getValue(code, 'C', calibration)) _calibrate0(2, _90_in_ms[2] * 2 - calibration);
				if (_getValue(code, 'D', calibration)) _calibrate0(3, calibration);
				if (_getValue(code, 'E', calibration)) _calibrate0(4, calibration);
				if (_getValue(code, 'F', calibration)) _calibrate73(calibration);
			}
			break;
		}
	}

  _isProcessing = true;
}

bool _Gcode::_getValue(String code, char cmd, float& value) {
	int sIndex = code.indexOf(cmd);
	if (sIndex == -1) return false;
	
	int eIndex = code.indexOf(' ', sIndex);
	
	String _value;
	if (eIndex == -1) {
		_value = code.substring(sIndex+1);
	} else {
		_value = code.substring(sIndex+1, eIndex);
	}
	
	value = _value.toFloat();
	
	return true;
}

void _Gcode::_processSteps() {
	if (_gcode_data[gcode_GRIP] != _gcode_pdata[gcode_GRIP]) {
		_arm_buffer[arm_STATE] = arm_AVAILABLE;
		_arm_buffer[arm_JOINT5] = _gcode_data[gcode_GRIP];
		_gcode_pdata[gcode_GRIP] = _gcode_data[gcode_GRIP];
	}

	float vector[3];
	vector[0] = _gcode_data[gcode_X_AXIS] - _gcode_pdata[gcode_X_AXIS];
	vector[1] = _gcode_data[gcode_Y_AXIS] - _gcode_pdata[gcode_Y_AXIS];
	vector[2] = _gcode_data[gcode_Z_AXIS] - _gcode_pdata[gcode_Z_AXIS];

	float L = sqrt(vector[0]*vector[0]
					+ vector[1]*vector[1]
					+ vector[2]*vector[2]);
	
	//if (_arm_buffer[arm_STATE] != arm_AVAILABLE)
		if (L < gcode_MIN_RESOLUTION) return;
  
	//_arm_buffer[arm_ATTACK] = -HALF_PI / 2; //HERE!
	if(!InverseK.solve(_gcode_data[gcode_X_AXIS], 
						_gcode_data[gcode_Y_AXIS], 
						_gcode_data[gcode_Z_AXIS], 
						_arm_buffer[arm_JOINT0], 
						_arm_buffer[arm_JOINT1], 
						_arm_buffer[arm_JOINT2], 
						_arm_buffer[arm_JOINT3], 
						_arm_buffer[arm_ATTACK])) 
	{
		Serial.println("Invalid position");
		return;
	}
  
	float stepL = _gcode_data[gcode_FEED] / 1000.0 * gcode_STEP;

	// Normalize
	vector[0] *= stepL/L;
	vector[1] *= stepL/L;
	vector[2] *= stepL/L;
  
	if (L < stepL) {
		InverseK.solve(_gcode_data[gcode_X_AXIS], 
						_gcode_data[gcode_Y_AXIS], 
						_gcode_data[gcode_Z_AXIS], 
						_arm_buffer[arm_JOINT0], 
						_arm_buffer[arm_JOINT1], 
						_arm_buffer[arm_JOINT2], 
						_arm_buffer[arm_JOINT3], 
						_arm_buffer[arm_ATTACK]);
		_gcode_pdata[gcode_X_AXIS] = _gcode_data[gcode_X_AXIS];
		_gcode_pdata[gcode_Y_AXIS] = _gcode_data[gcode_Y_AXIS];
		_gcode_pdata[gcode_Z_AXIS] = _gcode_data[gcode_Z_AXIS];
	} else {
		InverseK.solve(vector[0] + _gcode_pdata[gcode_X_AXIS], 
						vector[1] + _gcode_pdata[gcode_Y_AXIS], 
						vector[2] + _gcode_pdata[gcode_Z_AXIS], 
						_arm_buffer[arm_JOINT0], 
						_arm_buffer[arm_JOINT1], 
						_arm_buffer[arm_JOINT2], 
						_arm_buffer[arm_JOINT3], 
						_arm_buffer[arm_ATTACK]);

		_gcode_pdata[gcode_X_AXIS] += vector[0];
		_gcode_pdata[gcode_Y_AXIS] += vector[1];
		_gcode_pdata[gcode_Z_AXIS] += vector[2];
	}

	_arm_buffer[arm_STATE] = arm_AVAILABLE;
}

void _Gcode::update() {
	_cTime = millis();
	_deltaT = _cTime - _pTime;
	_pTime = _cTime;
	
	while (_deltaT >= gcode_STEP) {
		_processSteps();
		_deltaT -= gcode_STEP;
	}
	
	if (!_calibrationMode) {
		if (_arm_buffer[arm_STATE] == arm_AVAILABLE) {
			_arm_buffer[arm_STATE] = arm_STAND_BY;

			_braccioMove(_a2b(_arm_buffer[arm_JOINT0]), 
						 _a2b(_arm_buffer[arm_JOINT1]), 
						 _a2b(_arm_buffer[arm_JOINT2]), 
						 _a2b(_arm_buffer[arm_JOINT3]), 
						 _arm_buffer[arm_JOINT4], 
						 _arm_buffer[arm_JOINT5]); // grip in degrees (not radians)


		} else if(_isProcessing) {
			Serial.println("ok");
			_isProcessing = false;
		}
	}

	delay(gcode_STEP);
}

void _Gcode::_braccioMove(float base, float shoulder, float elbow, float wrist, float hand, float grip) {
	#ifdef CGX_GCODE_DEBUG
	pwm.setPWM(0, 0, _b2p(0, base));
	pwm.setPWM(1, 0, _b2p(1, shoulder));
	pwm.setPWM(2, 0, _b2p(2, elbow));
	pwm.setPWM(3, 0, _b2p(3, wrist));
	pwm.setPWM(4, 0, _b2p(4, hand));
	pwm.setPWM(5, 0, _b2p(5, grip));
	#endif
	
	String toSend;
	toSend += "$0 A";
	toSend += String(base, 1);
	toSend += " B";
	toSend += String(shoulder, 1);
	toSend += " C";
	toSend += String(elbow, 1);
	toSend += " D";
	toSend += String(wrist, 1);
	toSend += " E";
	toSend += String(hand, 1);
	toSend += " F";
	toSend += String(grip, 1);
	Serial.println(toSend);
	
	toSend = "$1 X";
	toSend += String(_gcode_pdata[gcode_X_AXIS], 1);
	toSend += " Y";
	toSend += String(_gcode_pdata[gcode_Y_AXIS], 1);
	toSend += " Z";
	toSend += String(_gcode_pdata[gcode_Z_AXIS], 1);
	Serial.println(toSend);
}
/*
void _Gcode::_checkArm() {
	while(_arm_enabled) {
		for (int i = 0; i < 6; i++) {
			
			delay()
		}
	}
}
*/
float _Gcode::_b2a(float degree) {
	return degree / 180.0 * PI - HALF_PI;
}

float _Gcode::_a2b(float angle) {
	return (angle + HALF_PI) * 180 / PI;
}

// TODO: Auto calibration
int _Gcode::_b2p(int servo, float degree) { // braccio to calibrated pulse 
	switch(servo) {
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
		return (int)map(degree, 0, 90, _0_in_ms[servo], _90_in_ms[servo]);
		break;
	case 5:
		return (int)map(degree, 10, 73, _10_in_ms, _73_in_ms);
		break;
	default:
		return (int)map(degree, 0, 180, 565, 130);
	}
	/*
	switch(servo) {
	case 0:
		return (int)map(degree, 0, 180, 565, 130);
		break;
	case 1:
		return (int)map(degree, 0, 180, 580, 140);
		break;
	case 2:
		return (int)map(degree, 0, 180, 580, 175);
		break;
	case 3:
		return (int)map(degree, 0, 180, 575, 145);
		break;
	case 4:
		return (int)map(degree, 0, 180, 507, 130);
		break;
	case 5:
		return (int)map(degree, 10, 73, 210, 365);
		break;
	default:
		return (int)map(degree, 0, 180, 565, 130);
	}
	*/
}