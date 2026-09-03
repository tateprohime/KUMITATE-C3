#include "Hcsr04Sensor.h"
#include <Arduino.h>

static const unsigned long ECHO_TIMEOUT_US = 25000UL;
static const float SOUND_SPEED_CM_PER_US = 0.0343f;
static uint8_t trigPin;
static uint8_t echoPin;

void hcsr04_begin(uint8_t newTrigPin, uint8_t newEchoPin)
{
  trigPin = newTrigPin;
  echoPin = newEchoPin;
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  digitalWrite(trigPin, LOW);
}

float hcsr04_measure_cm(void)
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(3);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  const unsigned long durationUs =
      pulseIn(echoPin, HIGH, ECHO_TIMEOUT_US);

  if (durationUs == 0) {
    return -1.0f;
  }

  return durationUs * SOUND_SPEED_CM_PER_US / 2.0f;
}
