#include "OledDisplay.h"
#include "Hcsr04Sensor.h"
#include "L9110Motor.h"

// KUMITATE-C3 pin assignments
constexpr uint8_t TRIG_PIN = 7;
constexpr uint8_t ECHO_PIN = 0;
constexpr uint8_t MOTOR_INA_PIN = 6;
constexpr uint8_t MOTOR_INB_PIN = 5;
constexpr uint8_t OLED_SDA_PIN = 1;
constexpr uint8_t OLED_SCL_PIN = 2;

// Motor runs within 50 cm and reaches maximum speed at 5 cm.
constexpr float MOTOR_START_DISTANCE_CM = 50.0f;
constexpr float MOTOR_MAX_SPEED_DISTANCE_CM = 5.0f;
constexpr uint8_t MOTOR_MIN_PWM = 100;
constexpr uint8_t MOTOR_MAX_PWM = 255;

uint8_t calculateMotorPwm(float distanceCm){
  if (distanceCm <= 0.0f || distanceCm > MOTOR_START_DISTANCE_CM) {
    return 0;
  }

  if (distanceCm <= MOTOR_MAX_SPEED_DISTANCE_CM) {
    return MOTOR_MAX_PWM;
  }

  const float ratio =
      (MOTOR_START_DISTANCE_CM - distanceCm) /
      (MOTOR_START_DISTANCE_CM - MOTOR_MAX_SPEED_DISTANCE_CM);

  const int pwm = MOTOR_MIN_PWM + static_cast<int>(
      ratio * (MOTOR_MAX_PWM - MOTOR_MIN_PWM));

  return static_cast<uint8_t>(constrain(pwm, MOTOR_MIN_PWM, MOTOR_MAX_PWM));
}

void setup(){
  Serial.begin(115200);

  hcsr04_begin(TRIG_PIN, ECHO_PIN);
  l9110_begin(MOTOR_INA_PIN, MOTOR_INB_PIN);
  oled_begin(OLED_SDA_PIN, OLED_SCL_PIN);
  oled_show_startup();

  Serial.println("Distance sensor started");
  delay(1000);
}

void loop(){
  const float distanceCm = hcsr04_measure_cm();
  const uint8_t motorPwm = calculateMotorPwm(distanceCm);

  if (motorPwm > 0) {
    l9110_run(motorPwm);
  } else {
    l9110_stop();
  }

  if (distanceCm < 0.0f) {
    Serial.println("Measurement failed / MOTOR STOP");
  } else {
    const int speedPercent = map(motorPwm, 0, 255, 0, 100);

    Serial.print("Distance: ");
    Serial.print(distanceCm, 1);
    Serial.print(" cm / PWM: ");
    Serial.print(motorPwm);
    Serial.print(" / Speed: ");
    Serial.print(speedPercent);
    Serial.println("%");
  }

  oled_show_measurement(distanceCm, motorPwm);
  delay(100);
}
