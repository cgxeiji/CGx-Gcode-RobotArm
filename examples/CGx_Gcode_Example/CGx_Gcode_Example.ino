// Enable this definition if you don't have an Adafruit 16-Channel Servo Driver connected to the Arduino/Teensy
#define CGX_GCODE_DEBUG

#include <InverseK.h>
#include <CGx_Gcode.h>

void setup() {
  Link base, upperarm, forearm, hand;

  // Braccio Robotic Arm
  base.init(0, b2a(0.0), b2a(180.0));
  upperarm.init(125, b2a(15.0), b2a(165.0));
  forearm.init(125, b2a(0.0), b2a(180.0));
  hand.init(190, b2a(0.0), b2a(180.0));

  // Black Servo Robotic Arm
  //base.init(75, b2a(0.0), b2a(180.0));
  //upperarm.init(105, b2a(15.0), b2a(165.0));
  //forearm.init(100, b2a(0.0), b2a(180.0));
  //hand.init(185, b2a(0.0), b2a(180.0));

  // Attach the links to the inverse kinematic model
  InverseK.attach(base, upperarm, forearm, hand);

  Serial.begin(115200);
  
  // Initialize the arm with the upright position
  Gcode.decode("M105");
}

float b2a(float b){ //braccio to angle in radians
  return b / 180.0 * PI - HALF_PI;
}

void loop() {
  // This is all the code necessary at the loop to control the robotic arm using Gcode
  if (Serial.available()) {
    String s = Serial.readStringUntil('\n');
    Gcode.decode(s);
  }
  Gcode.update();
}