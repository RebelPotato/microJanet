// tiny library for janet-esp32 interop

#ifndef MJLIB_H
#define MJLIB_H

#include <Arduino.h>
#include <janet.h>

static Janet cfun_delay(int32_t argc, Janet *argv) {
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

static Janet cfun_pinmode(int32_t argc, Janet *argv) {
  janet_fixarity(argc, 2);

  uint16_t pin = janet_getuinteger16(argv, 0);
  if (pin > UINT8_MAX) {
    janet_panicf("pin number must be between 0 and %d", UINT8_MAX);
  }
  JanetSymbol mode = janet_getsymbol(argv, 1);

  if (janet_equals(janet_wrap_symbol(mode), janet_csymbolv("input"))) {
    pinMode(pin, INPUT);
  } else if (janet_equals(janet_wrap_symbol(mode), janet_csymbolv("output"))) {
    pinMode(pin, OUTPUT);
  } else {
    janet_panicf("mode must be either :input or :output");
  }

  return janet_wrap_nil();
}

// TODO: name according to Arduino conventions
static const JanetReg mjlib[] = {
    {"delay", cfun_delay,
     "(delay ms)\n\n"
     "void delay(uint32_t ms) in Arduino.\n"
     "Waits for a given amount of milliseconds. Other tasks will not run during this time."},
    {"digital-write", cfun_digital_write,
     "(digital-write pin value)\n\n"
     "void digitalWrite(uint8_t pin, uint8_t val) in Arduino.\n"
     "Writes a value to a digital pin."},
    {"pinmode", cfun_pinmode,
     "(pinmode pin mode)\n\n"
     "void pinMode(uint8_t pin, uint8_t mode) in Arduino.\n"
     "Configures the specified pin to behave either as an input or an output."},
    {NULL, NULL, NULL}};

#endif