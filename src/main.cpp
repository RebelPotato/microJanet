#include <Arduino.h>
#include <janet.h>
#include <setjmp.h>

JanetTable *env;

// tiny library for janet-esp32 interop

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
    janet_panicf("Pin number must be between 0 and %d", UINT8_MAX);
  }
  uint16_t value = janet_getuinteger16(argv, 1);
  if (value != HIGH && value != LOW) {
    janet_panicf("Value must be either HIGH or LOW");
  }
  digitalWrite(pin, value);

  return janet_wrap_nil();
}

// TODO: name according to Arduino conventions
static const JanetReg cfuns[] = {
    {"delay", cfun_delay,
     "(delay ms)\n\n"
     "void delay(uint32_t ms) in Arduino.\n"
     "Stops the world for a given amount of milliseconds."},
    {"dw", cfun_digital_write,
     "(dw pin value)\n\n"
     "void digitalWrite(uint8_t pin, uint8_t val) in Arduino.\n"
     "Writes a value to a digital pin."},
    {NULL, NULL, NULL}};

void log_free() {
  log_d("Free MALLOC_CAP_8BIT heap: %d",
        heap_caps_get_free_size(MALLOC_CAP_8BIT));
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  // if the initialization runs out of memory, the esp32 reboots instantly
  // and it becomes very hard to upload a new sketch
  // so we wait for a while here
  delay(5000);

  // checking before initialization
  if (!psramInit()) {
    log_e("\nPSRAM is not correctly initialized.");
    exit(1);
  }
  log_d("Total heap: %d", ESP.getHeapSize());
  log_d("Total PSRAM: %d", ESP.getPsramSize());
  if (setjmp(oom) != 0) {
    // out of memory error
    log_free();
    exit(1);
  }

  // initialize janet
  janet_init();
  JanetTable *coreEnv = janet_core_env(NULL);
  int handle = janet_gclock();
  env = janet_table(sizeof(cfuns));
  janet_gcroot(janet_wrap_table(env));

  env->proto = coreEnv;
  janet_cfuns(env, "", cfuns);
  janet_def(env, "HIGH", janet_wrap_integer(HIGH), NULL);
  janet_def(env, "LOW", janet_wrap_integer(LOW), NULL);
  janet_def(env, "LED-BUILTIN", janet_wrap_integer(LED_BUILTIN), NULL);

  janet_gcunlock(handle);
  Serial.print("Ready.\n> ");
}

String acc = "";

void loop() {
  if (Serial.available() > 0) {
    char incomingByte = Serial.read();
    if (incomingByte == '\b') {
      if (acc.length() > 0) {
        acc = acc.substring(0, acc.length() - 1);
        Serial.print("\b \b");
      }
    } else if (incomingByte == '\n') {
      Serial.print("\n");

      Janet result;
      int status = janet_dostring(env, acc.c_str(), "main", &result);
      if (status == 0) {
        JanetBuffer *buf = janet_buffer(100);
        janet_pretty(buf, 0, 0, result);
        janet_buffer_push_u8(buf, '\0');
        Serial.println((char *)buf->data);
      }
      acc = "";
      Serial.print("> ");
    } else {
      acc += incomingByte;
      Serial.print(incomingByte);
    }
  }
}

static const unsigned char bytes[] = {};