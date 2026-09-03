#include "L9110Motor.h"
#include <Arduino.h>

static uint8_t inaPin;
static uint8_t inbPin;

void l9110_begin(uint8_t newInaPin, uint8_t newInbPin)
{
  inaPin = newInaPin;
  inbPin = newInbPin;
  pinMode(inaPin, OUTPUT);
  pinMode(inbPin, OUTPUT);
  l9110_stop();
}

void l9110_run(uint8_t pwm)
{
  analogWrite(inaPin, pwm);
  digitalWrite(inbPin, LOW);
}

void l9110_stop(void)
{
  analogWrite(inaPin, 0);
  digitalWrite(inbPin, LOW);
}
