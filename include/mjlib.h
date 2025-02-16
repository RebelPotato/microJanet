// tiny library for janet-esp32 interop

#ifndef MJLIB_H
#define MJLIB_H

#include <Arduino.h>
#include <janet.h>

static Janet cfun_wait(int32_t argc, Janet *argv) {
  janet_fixarity(argc, 1);

  uint32_t time = janet_getinteger(argv, 0);
  delay(time);

  return janet_wrap_nil();
}

static Janet cfun_digital_write(int32_t argc, Janet *argv) {
  janet_fixarity(argc, 2);

  uint16_t pin = janet_getuinteger16(argv, 0);
  if (pin > UINT8_MAX) {
    janet_panicf("pin number must be between 0 and %d", UINT8_MAX);
  }
  uint16_t value = janet_getuinteger16(argv, 1);
  if (value != HIGH && value != LOW) {
    janet_panicf("value must be either HIGH or LOW");
  }
  
  digitalWrite(pin, value);

  return janet_wrap_nil();
}


// TODO: name according to Arduino conventions
static const JanetReg mjlib[] = {
  {"wait", cfun_wait,
   "(wait ms)\n\n"
   "Stops the world for a given amount of milliseconds."},
  {"digital-write", cfun_digital_write,
   "(digital-write pin value)\n\n"
   "void digitalWrite(uint8_t pin, uint8_t val) in Arduino.\n"
   "Writes a value to a digital pin."},
  {NULL, NULL, NULL}};

#endif