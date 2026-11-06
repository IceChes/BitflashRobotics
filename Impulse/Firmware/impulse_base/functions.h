//motorcontrols struct used by multiple code areas
struct MotorControls {
  int payload;
  int curve_exponent;
  int maximum_speed;
};


//impulse-specific PWM calculation code
int calculatePWM(MotorControls _MotorControl) {

  if (_MotorControl.maximum_speed <= 0) {
    return 0;
  }
  else{
  float _normalized = float(_MotorControl.payload) / float(_MotorControl.maximum_speed);
  float _curved_value = pow(_normalized, _MotorControl.curve_exponent);
  return int(_curved_value * _MotorControl.maximum_speed);
  }


}
