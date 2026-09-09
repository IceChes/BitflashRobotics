//motorcontrols struct used by multiple code areas
struct MotorControls {
  int payload;
  int curve_exponent;
  int maximum_speed;
};


//impulse-specific PWM calculation code
int calculatePWM(MotorControls _MotorControl) {
  float _normalized = map(_MotorControl.payload, 0, 255, 0, 255) / 255.0;
  float _curve_value = _MotorControl.payload * pow(_normalized, _MotorControl.curve_exponent);
  return map(abs(_curve_value), 0, 255, 0, _MotorControl.maximum_speed);
}
